#pragma once
// Step 640: MQTT pub/sub boilerplate generator

#include <string>
#include <vector>

struct MqttTopicSpec {
    std::string topic;
    std::string payloadType;
    int qos = 1;
};

struct MqttBoilerplateOutput {
    bool success = false;
    std::string headerCode;
    std::string sourceCode;
    std::vector<std::string> errors;
};

class MqttBoilerplateGenerator {
public:
    static MqttBoilerplateOutput generate(const std::string& clientClass,
                                          const std::vector<MqttTopicSpec>& topics) {
        MqttBoilerplateOutput out;
        if (clientClass.empty()) {
            out.errors.push_back("client_class_required");
            return out;
        }
        if (topics.empty()) {
            out.errors.push_back("topics_required");
            return out;
        }

        out.success = true;
        out.headerCode = "#pragma once\n"
                         "#include <string>\n"
                         "#include <nlohmann/json.hpp>\n\n"
                         "class " + clientClass + " {\n"
                         "public:\n"
                         "    bool connect(const std::string& uri);\n"
                         "    void subscribeAll();\n"
                         "    void reconnect();\n"
                         "};\n";

        out.sourceCode = "bool " + clientClass + "::connect(const std::string& uri){ (void)uri; return true; }\n"
                         "void " + clientClass + "::reconnect(){}\n"
                         "void " + clientClass + "::subscribeAll(){\n";
        for (const auto& t : topics) {
            out.sourceCode += "  // topic: " + t.topic + " qos=" + std::to_string(t.qos) + " payload=" + t.payloadType + "\n";
            out.sourceCode += "  // deserialize via nlohmann::json\n";
        }
        out.sourceCode += "}\n";

        return out;
    }
};
