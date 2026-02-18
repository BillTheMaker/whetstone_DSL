#pragma once
// Step 654: HiveMind job publisher from editor

#include <nlohmann/json.hpp>

#include <string>

using json = nlohmann::json;

struct DispatchPublishResult {
    bool queuedInNexus = false;
    bool publishedToTopic = false;
    std::string topic;
    std::string jobId;
};

class HiveMindJobPublisher {
public:
    static DispatchPublishResult dispatch(const json& taskitem,
                                          const std::string& topic = "borg/jobs/pending") {
        DispatchPublishResult out;
        if (!taskitem.is_object() || taskitem.value("id", "").empty()) return out;
        out.jobId = taskitem.value("id", "");
        out.queuedInNexus = true;
        out.topic = topic;
        out.publishedToTopic = !topic.empty();
        return out;
    }
};
