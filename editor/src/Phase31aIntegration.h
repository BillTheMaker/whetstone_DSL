#pragma once
// Step 568: Phase 31a Integration

#include "AllocationOwnershipTraceHooks.h"
#include "CallStackFrameInspector.h"
#include "DebugSessionModel.h"
#include "LeakCorruptionSignalBridge.h"
#include "MemoryInspectorUiModel.h"
#include "MemorySnapshotModel.h"

#include <algorithm>
#include <string>
#include <vector>

struct Phase31aInput {
    std::string sessionId = "session-31a";
    std::string snapshotId = "snapshot-31a";
    std::string threadId = "thread-main";
    std::string targetId = "target-main";
    std::string bufferId = "buffer-main";
    std::string executablePath = "/tmp/whetstone-app";

    std::string stackRegionId = "stack-region";
    std::string heapRegionId = "heap-region";
    std::string pointerText = "0x2008";

    std::string allocationId = "alloc-main";
    std::string signalAllocationId = "";
    std::string initialOwner = "runtime";
    std::string transferredOwner = "inspector";

    std::string frameFunction = "run";
    std::string frameFile = "src/main.ws";
    int frameLine = 42;
    bool emitCorruptionSignal = false;
    bool omitSignalMessage = false;
};

struct Phase31aResult {
    bool snapshotReady = false;
    bool inspectorReady = false;
    bool traceReady = false;
    bool signalBridgeReady = false;
    bool stackCorrelationReady = false;
    bool phase31aPass = false;
    MemorySignalSeverity maxSeverity = MemorySignalSeverity::Info;
    std::vector<std::string> notes;
};

class Phase31aIntegration {
public:
    static Phase31aResult run(const Phase31aInput& input) {
        Phase31aResult result;
        MemorySnapshot snapshot;
        std::string error;

        DebugSessionModel debugSession;
        if (!debugSession.start(input.targetId, input.bufferId, input.executablePath)) {
            result.notes.push_back("fail:debug_session_start");
            return finalize(result);
        }

        if (!MemorySnapshotModel::createSnapshot(input.snapshotId,
                                                 input.sessionId,
                                                 input.threadId,
                                                 1,
                                                 &snapshot,
                                                 &error)) {
            result.notes.push_back("fail:snapshot_create");
            return finalize(result);
        }
        if (!MemorySnapshotModel::addRegion(&snapshot, {input.stackRegionId, MemoryRegionKind::Stack, 0x1000, 0x100, false}, &error) ||
            !MemorySnapshotModel::addRegion(&snapshot, {input.heapRegionId, MemoryRegionKind::Heap, 0x2000, 0x100, false}, &error) ||
            !MemorySnapshotModel::addReference(&snapshot, {0x1010, 0x2008, "stack_ptr"}, &error)) {
            result.notes.push_back("fail:snapshot_content");
            return finalize(result);
        }
        result.snapshotReady = true;

        MemoryValueNode root;
        MemoryValueNode pointerNode;
        if (!MemoryInspectorUiModel::createComposite("root", "Memory", MemoryValueKind::Struct, &root, &error) ||
            !MemoryInspectorUiModel::createLeaf("ptr", "head", MemoryValueKind::Pointer, input.pointerText, &pointerNode, &error) ||
            !MemoryInspectorUiModel::appendChild(&root, pointerNode, &error)) {
            result.notes.push_back("fail:inspector_node_build");
            return finalize(result);
        }
        const auto rows = MemoryInspectorUiModel::collectVisibleRows(root, 8);
        const MemoryRegionRecord* pointerRegion = nullptr;
        if (rows.size() < 2 ||
            !MemoryInspectorUiModel::resolvePointerRegion(snapshot, pointerNode, &pointerRegion, &error) ||
            !pointerRegion) {
            result.notes.push_back("fail:inspector_pointer_resolve");
            return finalize(result);
        }
        result.inspectorReady = true;

        AllocationOwnershipTraceHooks hooks;
        if (!hooks.recordAllocation(input.sessionId, input.allocationId, 0x2008, 64, input.initialOwner, 10, &error) ||
            !hooks.transferOwnership(input.sessionId, input.allocationId, input.transferredOwner, 20, &error)) {
            result.notes.push_back("fail:allocation_trace");
            return finalize(result);
        }
        result.traceReady = hooks.ownerFor(input.allocationId) == input.transferredOwner;
        if (!result.traceReady) {
            result.notes.push_back("fail:allocation_owner_mismatch");
            return finalize(result);
        }

        LeakCorruptionSignalBridge bridge;
        const std::string signalAllocId = input.signalAllocationId.empty() ? input.allocationId : input.signalAllocationId;
        const std::string leakMessage = input.omitSignalMessage ? "" : "Potential leak near allocation";
        if (!bridge.ingestSignal({"sig-1", input.sessionId, signalAllocId, MemorySignalType::LeakSuspected,
                                  leakMessage, 30}, &error)) {
            result.notes.push_back("fail:signal_bridge_ingest");
            return finalize(result);
        }
        if (input.emitCorruptionSignal &&
            !bridge.ingestSignal({"sig-2", input.sessionId, signalAllocId, MemorySignalType::CorruptionConfirmed,
                                  "Corruption confirmed near allocation", 31}, &error)) {
            result.notes.push_back("fail:signal_bridge_corruption_ingest");
            return finalize(result);
        }
        result.signalBridgeReady = !bridge.diagnosticsForSession(input.sessionId).empty();
        result.maxSeverity = bridge.highestSeverityForSession(input.sessionId);

        FrameScope frame;
        frame.frameIndex = 0;
        frame.functionName = input.frameFunction;
        frame.filePath = input.frameFile;
        frame.line = input.frameLine;
        frame.variables["ptr"] = input.pointerText;
        const auto frameResult = CallStackFrameInspector::inspect({frame}, 0);
        const auto vars = CallStackFrameInspector::variableNames(frame);
        const bool hasPointerVar = std::find(vars.begin(), vars.end(), "ptr") != vars.end();
        result.stackCorrelationReady = frameResult.ok && hasPointerVar && debugSession.hasTargetForBuffer(input.bufferId);
        if (!result.stackCorrelationReady) {
            result.notes.push_back("fail:stack_correlation");
            return finalize(result);
        }

        result.notes.push_back("phase31a:memory_reflection_cycle_ready");
        return finalize(result);
    }

private:
    static Phase31aResult finalize(Phase31aResult result) {
        result.phase31aPass = result.snapshotReady &&
                             result.inspectorReady &&
                             result.traceReady &&
                             result.signalBridgeReady &&
                             result.stackCorrelationReady;
        if (!result.phase31aPass) result.notes.push_back("phase31a:blocked");
        return result;
    }
};
