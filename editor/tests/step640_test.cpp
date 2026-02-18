// Step 640: MQTT pub/sub boilerplate generator (12 tests)

#include "MqttBoilerplateGenerator.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; }

static std::vector<MqttTopicSpec> defaultTopics() {
    return {
        {"borg/energy/context", "EnergyContext", 1},
        {"borg/jobs/pending", "PendingJob", 1},
        {"borg/registry/commit", "RegistryCommit", 2}
    };
}

void test_generate_requires_client_class() {
    TEST(generate_requires_client_class);
    auto out = MqttBoilerplateGenerator::generate("", defaultTopics());
    CHECK(!out.success, "empty client class should fail");
    PASS();
}

void test_generate_requires_topics() {
    TEST(generate_requires_topics);
    auto out = MqttBoilerplateGenerator::generate("DroneMqttClient", {});
    CHECK(!out.success, "empty topic list should fail");
    PASS();
}

void test_generate_succeeds_with_valid_input() {
    TEST(generate_succeeds_with_valid_input);
    auto out = MqttBoilerplateGenerator::generate("DroneMqttClient", defaultTopics());
    CHECK(out.success, "generator should succeed");
    PASS();
}

void test_header_contains_client_class() {
    TEST(header_contains_client_class);
    auto out = MqttBoilerplateGenerator::generate("DroneMqttClient", defaultTopics());
    CHECK(out.headerCode.find("class DroneMqttClient") != std::string::npos, "class missing");
    PASS();
}

void test_header_contains_connect_method() {
    TEST(header_contains_connect_method);
    auto out = MqttBoilerplateGenerator::generate("DroneMqttClient", defaultTopics());
    CHECK(out.headerCode.find("bool connect") != std::string::npos, "connect missing");
    PASS();
}

void test_header_contains_subscribe_method() {
    TEST(header_contains_subscribe_method);
    auto out = MqttBoilerplateGenerator::generate("DroneMqttClient", defaultTopics());
    CHECK(out.headerCode.find("void subscribeAll") != std::string::npos, "subscribe missing");
    PASS();
}

void test_header_contains_reconnect_method() {
    TEST(header_contains_reconnect_method);
    auto out = MqttBoilerplateGenerator::generate("DroneMqttClient", defaultTopics());
    CHECK(out.headerCode.find("void reconnect") != std::string::npos, "reconnect missing");
    PASS();
}

void test_source_contains_energy_topic() {
    TEST(source_contains_energy_topic);
    auto out = MqttBoilerplateGenerator::generate("DroneMqttClient", defaultTopics());
    CHECK(out.sourceCode.find("borg/energy/context") != std::string::npos, "energy topic missing");
    PASS();
}

void test_source_contains_job_topic() {
    TEST(source_contains_job_topic);
    auto out = MqttBoilerplateGenerator::generate("DroneMqttClient", defaultTopics());
    CHECK(out.sourceCode.find("borg/jobs/pending") != std::string::npos, "job topic missing");
    PASS();
}

void test_source_contains_registry_topic() {
    TEST(source_contains_registry_topic);
    auto out = MqttBoilerplateGenerator::generate("DroneMqttClient", defaultTopics());
    CHECK(out.sourceCode.find("borg/registry/commit") != std::string::npos, "registry topic missing");
    PASS();
}

void test_source_contains_json_deserialization_note() {
    TEST(source_contains_json_deserialization_note);
    auto out = MqttBoilerplateGenerator::generate("DroneMqttClient", defaultTopics());
    CHECK(out.sourceCode.find("nlohmann::json") != std::string::npos, "json note missing");
    PASS();
}

void test_source_contains_qos_for_topics() {
    TEST(source_contains_qos_for_topics);
    auto out = MqttBoilerplateGenerator::generate("DroneMqttClient", defaultTopics());
    CHECK(out.sourceCode.find("qos=2") != std::string::npos, "qos marker missing");
    PASS();
}

int main() {
    std::cout << "Step 640: MQTT pub/sub boilerplate generator\n";

    test_generate_requires_client_class();
    test_generate_requires_topics();
    test_generate_succeeds_with_valid_input();
    test_header_contains_client_class();
    test_header_contains_connect_method();
    test_header_contains_subscribe_method();
    test_header_contains_reconnect_method();
    test_source_contains_energy_topic();
    test_source_contains_job_topic();
    test_source_contains_registry_topic();
    test_source_contains_json_deserialization_note();
    test_source_contains_qos_for_topics();

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
