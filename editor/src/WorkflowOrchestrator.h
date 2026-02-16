#pragma once

#include "WorkflowState.h"
#include "RoutingEngine.h"
#include "WorkerRegistry.h"
#include "ContextAssembler.h"
#include "ReviewGate.h"
#include <functional>
#include <string>
#include <vector>

struct OrchestratorEvent {
    std::string type;
    std::string itemId;
    json detail;
    std::string timestamp;
};

struct BlockerInfo {
    std::string type;
    std::vector<std::string> itemIds;
    std::string description;
};

class WorkflowOrchestrator {
public:
    WorkflowOrchestrator(WorkflowState& workflowState,
                         RoutingEngine& routingEngine,
                         WorkerRegistry& workerRegistry,
                         ContextAssembler& contextAssembler,
                         ReviewGate& reviewGate)
        : state_(&workflowState), routing_(&routingEngine), workers_(&workerRegistry),
          context_(&contextAssembler), review_(&reviewGate) {}

    void setBuffers(const std::map<std::string, BufferInfo>& buffers) {
        buffers_ = buffers;
    }

    void setReviewPolicy(const ReviewPolicy& policy) {
        reviewPolicy_ = policy;
    }

    OrchestratorEvent step() {
        auto ready = state_->queue.getReady();
        if (ready.empty()) {
            return blockedEvent();
        }
        return advanceItem(ready.front()).back();
    }

    std::vector<OrchestratorEvent> advance() {
        std::vector<OrchestratorEvent> all;
        auto ready = state_->queue.getReady();
        for (const auto& item : ready) {
            auto events = advanceItem(item);
            all.insert(all.end(), events.begin(), events.end());
        }
        return all;
    }

    WorkflowStats runToCompletion() {
        while (true) {
            auto events = advance();
            if (events.empty()) break;
            bool anyProgress = false;
            for (const auto& ev : events) {
                if (ev.type != "blocked") {
                    anyProgress = true;
                    break;
                }
            }
            if (!anyProgress) break;
        }
        return state_->getStats();
    }

    WorkflowStats runUntil(const std::function<bool(const WorkflowState&)>& predicate) {
        while (!predicate(*state_)) {
            auto events = advance();
            if (events.empty()) break;
            bool anyProgress = false;
            for (const auto& ev : events) {
                if (ev.type != "blocked") {
                    anyProgress = true;
                    break;
                }
            }
            if (!anyProgress) break;
        }
        return state_->getStats();
    }

    std::vector<BlockerInfo> getBlockers() {
        std::vector<BlockerInfo> out;

        auto reviewItems = state_->queue.getByStatus(WI_REVIEW);
        if (!reviewItems.empty()) {
            BlockerInfo b;
            b.type = "needs-review";
            b.description = "Items are waiting in review.";
            for (const auto& i : reviewItems) b.itemIds.push_back(i.id);
            out.push_back(b);
        }

        BlockerInfo human;
        human.type = "needs-human";
        human.description = "Items are routed to human execution.";

        BlockerInfo external;
        external.type = "needs-external-model";
        external.description = "Items are prepared for SLM/LLM external execution.";

        for (const auto& status : {WI_ASSIGNED, WI_IN_PROGRESS}) {
            for (const auto& item : state_->queue.getByStatus(status)) {
                if (item.workerType == "human") {
                    human.itemIds.push_back(item.id);
                } else if (item.workerType == "slm" || item.workerType == "llm") {
                    external.itemIds.push_back(item.id);
                }
            }
        }

        if (!human.itemIds.empty()) out.push_back(human);
        if (!external.itemIds.empty()) out.push_back(external);
        return out;
    }

private:
    WorkflowState* state_ = nullptr;
    RoutingEngine* routing_ = nullptr;
    WorkerRegistry* workers_ = nullptr;
    ContextAssembler* context_ = nullptr;
    ReviewGate* review_ = nullptr;
    ReviewPolicy reviewPolicy_ = ReviewPolicy::getDefault();
    std::map<std::string, BufferInfo> buffers_;

    static OrchestratorEvent makeEvent(const std::string& type,
                                       const std::string& itemId,
                                       const json& detail = json::object()) {
        OrchestratorEvent ev;
        ev.type = type;
        ev.itemId = itemId;
        ev.detail = detail;
        ev.timestamp = workItemTimestamp();
        return ev;
    }

    OrchestratorEvent blockedEvent() {
        auto blockers = getBlockers();
        if (blockers.empty()) {
            return makeEvent("blocked", "", json{{"reason", "no-ready-items"}});
        }
        return makeEvent("blocked", "", json{{"blockers", toJsonBlockers(blockers)}});
    }

    static json toJsonBlockers(const std::vector<BlockerInfo>& blockers) {
        json arr = json::array();
        for (const auto& b : blockers) {
            arr.push_back({{"type", b.type},
                           {"itemIds", b.itemIds},
                           {"description", b.description}});
        }
        return arr;
    }

    std::vector<OrchestratorEvent> advanceItem(const WorkItem& readyItem) {
        std::vector<OrchestratorEvent> events;

        auto maybeItem = state_->queue.getItem(readyItem.id);
        if (!maybeItem.has_value()) return events;
        WorkItem item = maybeItem.value();

        RoutingDecision decision = routing_->route(item);
        item.workerType = decision.workerType;
        item.contextWidth = decision.contextWidth;
        item.reviewRequired = decision.reviewRequired;

        transitionWorkItem(item, WI_ASSIGNED);
        transitionWorkItem(item, WI_IN_PROGRESS);
        state_->queue.updateItem(item.id, item);

        events.push_back(makeEvent("routed", item.id, decision.toJson()));

        WorkerContext ctx = context_->assembleContext(item, buffers_, item.contextWidth,
                                                      decision.contextBudgetTokens);
        events.push_back(makeEvent("context-assembled", item.id,
                                   json{{"contextWidth", item.contextWidth},
                                        {"tokensBudget", decision.contextBudgetTokens}}));

        WorkerInterface* worker = workers_->getWorker(item.workerType);
        if (!worker) {
            events.push_back(makeEvent("blocked", item.id,
                                       json{{"reason", "worker-not-registered"},
                                            {"workerType", item.workerType}}));
            return events;
        }

        item.result = worker->execute(item, ctx);
        state_->queue.updateItem(item.id, item);
        events.push_back(makeEvent("executed", item.id,
                                   json{{"workerType", item.workerType},
                                        {"confidence", item.result.confidence}}));

        if (item.workerType == "human") {
            events.push_back(makeEvent("blocked", item.id,
                                       json{{"reason", "needs-human"}}));
            return events;
        }
        if (item.workerType == "slm" || item.workerType == "llm") {
            events.push_back(makeEvent("blocked", item.id,
                                       json{{"reason", "needs-external-model"}}));
            return events;
        }

        ReviewDecision reviewDecision = review_->shouldAutoApprove(item, item.result, reviewPolicy_);
        if (reviewDecision.approved) {
            state_->queue.updateItem(item.id, item);
            state_->queue.complete(item.id);
            events.push_back(makeEvent("auto-approved", item.id,
                                       json{{"rule", reviewDecision.ruleMatched}}));
            events.push_back(makeEvent("completed", item.id));
        } else {
            transitionWorkItem(item, WI_REVIEW);
            state_->queue.updateItem(item.id, item);
            events.push_back(makeEvent("sent-to-review", item.id,
                                       json{{"reason", reviewDecision.reasoning}}));
        }

        return events;
    }
};
