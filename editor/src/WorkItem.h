#pragma once
// Step 320: WorkItem — Core Execution Model
//
// Extends SkeletonTask with execution lifecycle tracking: status transitions,
// worker assignment, timestamps, and results. This is the unit of work that
// flows through the routing engine.

#include "SkeletonAST.h"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <chrono>
#include <random>
#include <sstream>
#include <iomanip>

using json = nlohmann::json;

// --- WorkItemResult: output produced by a worker ---

struct WorkItemResult {
    std::string generatedCode;
    json astJson;
    std::vector<json> diagnostics;
    float confidence = 0.0f;
    int tokensBudget = 0;
    int tokensGenerated = 0;
    std::string reasoning;

    json toJson() const {
        json j;
        j["generatedCode"] = generatedCode;
        j["astJson"] = astJson;
        j["diagnostics"] = diagnostics;
        j["confidence"] = confidence;
        j["tokensBudget"] = tokensBudget;
        j["tokensGenerated"] = tokensGenerated;
        j["reasoning"] = reasoning;
        return j;
    }

    static WorkItemResult fromJson(const json& j) {
        WorkItemResult r;
        if (j.contains("generatedCode")) r.generatedCode = j["generatedCode"].get<std::string>();
        if (j.contains("astJson")) r.astJson = j["astJson"];
        if (j.contains("diagnostics")) r.diagnostics = j["diagnostics"].get<std::vector<json>>();
        if (j.contains("confidence")) r.confidence = j["confidence"].get<float>();
        if (j.contains("tokensBudget")) r.tokensBudget = j["tokensBudget"].get<int>();
        if (j.contains("tokensGenerated")) r.tokensGenerated = j["tokensGenerated"].get<int>();
        if (j.contains("reasoning")) r.reasoning = j["reasoning"].get<std::string>();
        return r;
    }
};

struct RejectionAttempt {
    std::string workerType;
    WorkItemResult result;
    std::string feedback;
    std::string rejectedAt;
    std::string rejectedBy;

    json toJson() const {
        return json{
            {"workerType", workerType},
            {"result", result.toJson()},
            {"feedback", feedback},
            {"rejectedAt", rejectedAt},
            {"rejectedBy", rejectedBy}
        };
    }

    static RejectionAttempt fromJson(const json& j) {
        RejectionAttempt a;
        if (j.contains("workerType")) a.workerType = j["workerType"].get<std::string>();
        if (j.contains("result")) a.result = WorkItemResult::fromJson(j["result"]);
        if (j.contains("feedback")) a.feedback = j["feedback"].get<std::string>();
        if (j.contains("rejectedAt")) a.rejectedAt = j["rejectedAt"].get<std::string>();
        if (j.contains("rejectedBy")) a.rejectedBy = j["rejectedBy"].get<std::string>();
        return a;
    }
};

// --- WorkItem status values ---

inline const std::string WI_PENDING     = "pending";
inline const std::string WI_READY       = "ready";
inline const std::string WI_ASSIGNED    = "assigned";
inline const std::string WI_IN_PROGRESS = "in-progress";
inline const std::string WI_REVIEW      = "review";
inline const std::string WI_COMPLETE    = "complete";
inline const std::string WI_REJECTED    = "rejected";

// --- WorkItem struct ---

struct WorkItem {
    std::string id;
    std::string nodeId;
    std::string nodeName;
    std::string nodeType;
    std::string bufferId;

    // Routing
    std::string contextWidth;
    std::string workerType;
    bool reviewRequired = false;
    std::string priority;
    std::vector<std::string> dependencies;

    // Lifecycle
    std::string status = WI_PENDING;

    // Assignment
    std::string assignee;
    std::string createdAt;
    std::string assignedAt;
    std::string completedAt;

    // Result
    WorkItemResult result;
    std::string rejectionFeedback;
    std::vector<RejectionAttempt> rejectionHistory;
};

// --- Helper: ISO timestamp ---

inline std::string workItemTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
    gmtime_r(&time, &tm);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    return oss.str();
}

// --- Helper: generate unique ID ---

inline std::string generateWorkItemId() {
    static std::mt19937 rng(std::random_device{}());
    static std::uniform_int_distribution<uint64_t> dist;
    std::ostringstream oss;
    oss << "wi-" << std::hex << dist(rng);
    return oss.str();
}

// --- createWorkItem from SkeletonTask ---

inline WorkItem createWorkItem(const SkeletonTask& task, const std::string& bufferId) {
    WorkItem wi;
    wi.id = generateWorkItemId();
    wi.nodeId = task.nodeId;
    wi.nodeName = task.nodeName;
    wi.nodeType = task.nodeType;
    wi.bufferId = bufferId;
    wi.contextWidth = task.contextWidth;
    wi.workerType = task.automatability;
    wi.reviewRequired = task.reviewRequired;
    wi.priority = task.priority;
    wi.dependencies = task.dependencies;
    wi.status = WI_PENDING;
    wi.createdAt = workItemTimestamp();
    return wi;
}

// --- State machine helpers ---

inline bool isTerminal(const std::string& status) {
    return status == WI_COMPLETE || status == WI_REJECTED;
}

inline bool canTransition(const std::string& from, const std::string& to) {
    // pending → ready
    if (from == WI_PENDING && to == WI_READY) return true;
    // ready → assigned
    if (from == WI_READY && to == WI_ASSIGNED) return true;
    // assigned → in-progress
    if (from == WI_ASSIGNED && to == WI_IN_PROGRESS) return true;
    // in-progress → review | complete
    if (from == WI_IN_PROGRESS && (to == WI_REVIEW || to == WI_COMPLETE)) return true;
    // review → complete | rejected
    if (from == WI_REVIEW && (to == WI_COMPLETE || to == WI_REJECTED)) return true;
    // rejected → ready (retry)
    if (from == WI_REJECTED && to == WI_READY) return true;
    return false;
}

inline bool transitionWorkItem(WorkItem& item, const std::string& newStatus) {
    if (!canTransition(item.status, newStatus)) return false;
    item.status = newStatus;
    if (newStatus == WI_ASSIGNED) {
        item.assignedAt = workItemTimestamp();
    } else if (newStatus == WI_COMPLETE || newStatus == WI_REJECTED) {
        item.completedAt = workItemTimestamp();
    }
    return true;
}

// --- Priority ordering ---

inline int priorityToInt(const std::string& p) {
    if (p == "critical") return 0;
    if (p == "high") return 1;
    if (p == "medium") return 2;
    if (p == "low") return 3;
    return 4; // unknown
}

// --- JSON serialization ---

inline json workItemToJson(const WorkItem& wi) {
    json j;
    j["id"] = wi.id;
    j["nodeId"] = wi.nodeId;
    j["nodeName"] = wi.nodeName;
    j["nodeType"] = wi.nodeType;
    j["bufferId"] = wi.bufferId;
    j["contextWidth"] = wi.contextWidth;
    j["workerType"] = wi.workerType;
    j["reviewRequired"] = wi.reviewRequired;
    j["priority"] = wi.priority;
    j["dependencies"] = wi.dependencies;
    j["status"] = wi.status;
    j["assignee"] = wi.assignee;
    j["createdAt"] = wi.createdAt;
    j["assignedAt"] = wi.assignedAt;
    j["completedAt"] = wi.completedAt;
    j["result"] = wi.result.toJson();
    j["rejectionFeedback"] = wi.rejectionFeedback;
    json attempts = json::array();
    for (const auto& a : wi.rejectionHistory) attempts.push_back(a.toJson());
    j["rejectionHistory"] = attempts;
    return j;
}

inline WorkItem workItemFromJson(const json& j) {
    WorkItem wi;
    if (j.contains("id")) wi.id = j["id"].get<std::string>();
    if (j.contains("nodeId")) wi.nodeId = j["nodeId"].get<std::string>();
    if (j.contains("nodeName")) wi.nodeName = j["nodeName"].get<std::string>();
    if (j.contains("nodeType")) wi.nodeType = j["nodeType"].get<std::string>();
    if (j.contains("bufferId")) wi.bufferId = j["bufferId"].get<std::string>();
    if (j.contains("contextWidth")) wi.contextWidth = j["contextWidth"].get<std::string>();
    if (j.contains("workerType")) wi.workerType = j["workerType"].get<std::string>();
    if (j.contains("reviewRequired")) wi.reviewRequired = j["reviewRequired"].get<bool>();
    if (j.contains("priority")) wi.priority = j["priority"].get<std::string>();
    if (j.contains("dependencies")) wi.dependencies = j["dependencies"].get<std::vector<std::string>>();
    if (j.contains("status")) wi.status = j["status"].get<std::string>();
    if (j.contains("assignee")) wi.assignee = j["assignee"].get<std::string>();
    if (j.contains("createdAt")) wi.createdAt = j["createdAt"].get<std::string>();
    if (j.contains("assignedAt")) wi.assignedAt = j["assignedAt"].get<std::string>();
    if (j.contains("completedAt")) wi.completedAt = j["completedAt"].get<std::string>();
    if (j.contains("result")) wi.result = WorkItemResult::fromJson(j["result"]);
    if (j.contains("rejectionFeedback")) wi.rejectionFeedback = j["rejectionFeedback"].get<std::string>();
    if (j.contains("rejectionHistory")) {
        for (const auto& a : j["rejectionHistory"]) {
            wi.rejectionHistory.push_back(RejectionAttempt::fromJson(a));
        }
    }
    return wi;
}
