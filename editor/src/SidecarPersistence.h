#pragma once
// Step 267: Sidecar AST Persistence
//
// Save/load annotated ASTs as .whetstone/<path>.ast.json sidecar files.
// Annotations live alongside the codebase without polluting source code.

#include "ast/ASTNode.h"
#include "ast/Serialization.h"
#include "ast/Annotation.h"
#include "ASTUtils.h"
#include "CompactAST.h"
#include "SemannoSidecar.h"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>

using json = nlohmann::json;

// --- Sidecar path resolution ---
inline std::string sidecarPath(const std::string& workspaceRoot,
                                const std::string& filePath) {
    namespace fs = std::filesystem;
    fs::path base = fs::path(workspaceRoot) / ".whetstone";
    return (base / (filePath + ".ast.json")).string();
}

// --- Check if a node type is a semantic annotation ---
inline bool isSemanticAnnotation(const std::string& conceptType) {
    return conceptType == "IntentAnnotation" ||
           conceptType == "ComplexityAnnotation" ||
           conceptType == "RiskAnnotation" ||
           conceptType == "ContractAnnotation" ||
           conceptType == "SemanticTagAnnotation" ||
           // Type System (Steps 272-273)
           conceptType == "BitWidthAnnotation" ||
           conceptType == "EndianAnnotation" ||
           conceptType == "LayoutAnnotation" ||
           conceptType == "NullabilityAnnotation" ||
           conceptType == "VarianceAnnotation" ||
           conceptType == "IdentityAnnotation" ||
           conceptType == "MutAnnotation" ||
           conceptType == "TypeStateAnnotation" ||
           // Concurrency (Step 274)
           conceptType == "AtomicAnnotation" ||
           conceptType == "SyncAnnotation" ||
           conceptType == "ThreadModelAnnotation" ||
           conceptType == "MemoryBarrierAnnotation" ||
           // Async / Error Handling (Step 275)
           conceptType == "ExecAnnotation" ||
           conceptType == "BlockingAnnotation" ||
           conceptType == "ParallelAnnotation" ||
           conceptType == "TrapAnnotation" ||
           conceptType == "ExceptionAnnotation" ||
           conceptType == "PanicAnnotation" ||
           // Scope & Namespace (Step 276)
           conceptType == "BindingAnnotation" ||
           conceptType == "LookupAnnotation" ||
           conceptType == "CaptureAnnotation" ||
           conceptType == "VisibilityAnnotation" ||
           conceptType == "NamespaceAnnotation" ||
           conceptType == "ScopeAnnotation" ||
           // Shim & Escape Hatch (Step 278)
           conceptType == "IntrinsicAnnotation" ||
           conceptType == "RawAnnotation" ||
           conceptType == "CallingConvAnnotation" ||
           conceptType == "LinkAnnotation" ||
           conceptType == "ShimAnnotation" ||
           conceptType == "PointerArithmeticAnnotation" ||
           conceptType == "OpaqueAnnotation" ||
           // Platform & Provenance (Step 279)
           conceptType == "TargetAnnotation" ||
           conceptType == "FeatureAnnotation" ||
           conceptType == "OriginalAnnotation" ||
           conceptType == "MappingAnnotation" ||
           // Optimization Completion (Step 280)
           conceptType == "TailCallAnnotation" ||
           conceptType == "LoopAnnotation" ||
           conceptType == "DataAnnotation" ||
           conceptType == "AlignAnnotation" ||
           conceptType == "PackAnnotation" ||
           conceptType == "BoundsCheckAnnotation" ||
           conceptType == "OverflowAnnotation" ||
           // Meta-Programming (Step 281)
           conceptType == "MetaAnnotation" ||
           conceptType == "SymbolAnnotation" ||
           conceptType == "EvaluateAnnotation" ||
           conceptType == "TemplateAnnotation" ||
           conceptType == "SyntheticAnnotation" ||
           // Strategy & Policy (Step 282)
           conceptType == "PolicyAnnotation" ||
           conceptType == "AmbiguityAnnotation" ||
           conceptType == "CandidateAnnotation" ||
           conceptType == "TradeoffAnnotation" ||
           conceptType == "ChoiceAnnotation" ||
           conceptType == "DecisionAnnotation" ||
           // Subject 9: Workflow Routing (Step 315)
           conceptType == "ContextWidthAnnotation" ||
           conceptType == "ReviewAnnotation" ||
           conceptType == "AutomatabilityAnnotation" ||
           conceptType == "PriorityAnnotation" ||
           conceptType == "ImplementationStatusAnnotation" ||
           // Environment Layer (Step 285)
           conceptType == "CapabilityRequirement";
}

// --- Count semantic annotations in an AST ---
inline int countSemanticAnnotations(const ASTNode* node) {
    if (!node) return 0;
    int count = 0;
    auto annos = node->getChildren("annotations");
    for (const auto* a : annos) {
        if (isSemanticAnnotation(a->conceptType))
            ++count;
    }
    for (const auto* child : node->allChildren()) {
        count += countSemanticAnnotations(child);
    }
    return count;
}

// --- Save annotated AST to sidecar file ---
struct SidecarSaveResult {
    bool success = false;
    std::string sidecarPath;
    int annotationCount = 0;
    std::string error;
};

inline SidecarSaveResult saveSidecarAST(const std::string& workspaceRoot,
                                         const std::string& filePath,
                                         Module* ast) {
    SidecarSaveResult result;
    if (!ast) {
        result.error = "No AST to save";
        return result;
    }

    result.sidecarPath = sidecarPath(workspaceRoot, filePath);
    namespace fs = std::filesystem;
    fs::create_directories(fs::path(result.sidecarPath).parent_path());

    json astJson = toJson(ast);
    std::ofstream out(result.sidecarPath);
    if (!out.is_open()) {
        result.error = "Cannot write to " + result.sidecarPath;
        return result;
    }
    out << astJson.dump(2);
    out.close();

    result.annotationCount = countSemanticAnnotations(ast);
    result.success = true;
    return result;
}

// --- Find a matching node in the live AST ---
// Tries ID match first, then falls back to concept+name match
// (node IDs are ephemeral and change across re-parses).
inline ASTNode* findMatchingNode(ASTNode* liveRoot,
                                  const ASTNode* sidecarNode) {
    if (!liveRoot || !sidecarNode) return nullptr;
    // Try exact ID match first
    ASTNode* byId = findNodeById(liveRoot, sidecarNode->id);
    if (byId) return byId;
    // Fallback: match by concept type + name
    std::string name = getNodeName(sidecarNode);
    if (name.empty()) return nullptr;
    for (auto* child : liveRoot->allChildren()) {
        if (child->conceptType == sidecarNode->conceptType &&
            getNodeName(child) == name)
            return child;
    }
    return nullptr;
}

// --- Merge annotations from sidecar AST into live AST ---
// Matches nodes by ID or concept+name, copies semantic annotations.
struct SidecarMergeResult {
    bool success = false;
    int mergedCount = 0;
    int staleCount = 0;
    std::string error;
};

inline void mergeAnnotationsRecursive(ASTNode* sidecarNode,
                                       ASTNode* liveRoot,
                                       int& mergedCount,
                                       int& staleCount) {
    if (!sidecarNode) return;

    // Check if this sidecar node has semantic annotations to merge
    auto annos = sidecarNode->getChildren("annotations");
    for (auto* anno : annos) {
        if (!isSemanticAnnotation(anno->conceptType)) continue;

        // Find matching node in live AST
        ASTNode* liveNode = findMatchingNode(liveRoot, sidecarNode);
        if (liveNode) {
            // Clone the annotation via JSON roundtrip
            json annoJson = toJson(anno);
            ASTNode* cloned = fromJson(annoJson);
            if (cloned) {
                liveNode->addChild("annotations", cloned);
                ++mergedCount;
            }
        } else {
            ++staleCount;
        }
    }

    // Recurse into children
    for (auto* child : sidecarNode->allChildren()) {
        // Skip annotation children themselves
        if (isSemanticAnnotation(child->conceptType)) continue;
        mergeAnnotationsRecursive(child, liveRoot, mergedCount, staleCount);
    }
}

inline SidecarMergeResult loadSidecarAST(const std::string& workspaceRoot,
                                          const std::string& filePath,
                                          Module* liveAST) {
    SidecarMergeResult result;
    if (!liveAST) {
        result.error = "No live AST to merge into";
        return result;
    }

    std::string path = sidecarPath(workspaceRoot, filePath);
    std::ifstream in(path);
    if (!in.is_open()) {
        result.error = "No sidecar file: " + path;
        return result;
    }

    json astJson;
    try {
        in >> astJson;
    } catch (const std::exception& e) {
        result.error = std::string("Failed to parse sidecar: ") + e.what();
        return result;
    }
    in.close();

    ASTNode* sidecarAST = fromJson(astJson);
    if (!sidecarAST) {
        result.error = "Failed to deserialize sidecar AST";
        return result;
    }

    mergeAnnotationsRecursive(sidecarAST, liveAST,
                               result.mergedCount, result.staleCount);

    // Clean up sidecar AST
    delete sidecarAST;

    result.success = true;
    return result;
}

// --- List available sidecar files ---
inline std::vector<std::string> listSidecarFiles(
    const std::string& workspaceRoot) {
    namespace fs = std::filesystem;
    std::vector<std::string> files;
    fs::path dir = fs::path(workspaceRoot) / ".whetstone";
    if (!fs::exists(dir)) return files;

    for (const auto& entry : fs::directory_iterator(dir)) {
        if (entry.is_regular_file()) {
            std::string name = entry.path().filename().string();
            // Remove .ast.json suffix to get original path
            const std::string suffix = ".ast.json";
            if (name.size() > suffix.size() &&
                name.substr(name.size() - suffix.size()) == suffix) {
                files.push_back(name.substr(0, name.size() - suffix.size()));
            }
        }
    }
    return files;
}
