// Step 630: Chat session persistence (12 tests)

#include "AgentChatSessionPersistence.h"

#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static fs::path tempWorkspace() {
    fs::path root = fs::temp_directory_path() / "whetstone_step630";
    fs::remove_all(root);
    fs::create_directories(root);
    return root;
}

static AgentChatState sampleChat(const std::string& content) {
    AgentChatState chat;
    chat.systemContext = "Context";
    chat.messages.push_back({AgentChatRole::User, content, "13:00"});
    return chat;
}

void test_session_store_path_uses_workspace_root() {
    TEST(session_store_path_uses_workspace_root);
    std::string path = AgentChatSessionPersistence::sessionStorePath("/tmp/ws");
    CHECK(path.find("/tmp/ws") != std::string::npos, "workspace root missing in path");
    CHECK(path.find(".whetstone.chat_sessions.json") != std::string::npos, "filename mismatch");
    PASS();
}

void test_save_session_requires_project_file() {
    TEST(save_session_requires_project_file);
    std::string error;
    CHECK(!AgentChatSessionPersistence::saveSession("/tmp/ws", "", "ts", sampleChat("x"), &error),
          "save should fail");
    CHECK(error == "project_file_missing", "wrong error");
    PASS();
}

void test_save_session_requires_timestamp() {
    TEST(save_session_requires_timestamp);
    std::string error;
    CHECK(!AgentChatSessionPersistence::saveSession("/tmp/ws", "src/main.cpp", "", sampleChat("x"), &error),
          "save should fail");
    CHECK(error == "timestamp_missing", "wrong error");
    PASS();
}

void test_save_session_creates_store_file() {
    TEST(save_session_creates_store_file);
    auto ws = tempWorkspace();
    std::string error;
    CHECK(AgentChatSessionPersistence::saveSession(ws.string(), "src/main.cpp", "t1", sampleChat("hello"), &error),
          "save should succeed");
    CHECK(fs::exists(AgentChatSessionPersistence::sessionStorePath(ws.string())), "store file should exist");
    PASS();
}

void test_load_latest_returns_saved_session() {
    TEST(load_latest_returns_saved_session);
    auto ws = tempWorkspace();
    std::string error;
    (void)AgentChatSessionPersistence::saveSession(ws.string(), "src/main.cpp", "t1", sampleChat("hello"), &error);
    AgentChatSavedSession loaded;
    CHECK(AgentChatSessionPersistence::loadLatestSession(ws.string(), "src/main.cpp", &loaded, &error),
          "load should succeed");
    CHECK(loaded.timestamp == "t1", "timestamp mismatch");
    CHECK(loaded.chat.messages.size() == 1, "message count mismatch");
    CHECK(loaded.chat.messages[0].content == "hello", "message content mismatch");
    PASS();
}

void test_load_latest_uses_most_recent_timestamp_entry() {
    TEST(load_latest_uses_most_recent_timestamp_entry);
    auto ws = tempWorkspace();
    std::string error;
    (void)AgentChatSessionPersistence::saveSession(ws.string(), "src/main.cpp", "t1", sampleChat("first"), &error);
    (void)AgentChatSessionPersistence::saveSession(ws.string(), "src/main.cpp", "t2", sampleChat("second"), &error);
    AgentChatSavedSession loaded;
    CHECK(AgentChatSessionPersistence::loadLatestSession(ws.string(), "src/main.cpp", &loaded, &error),
          "load should succeed");
    CHECK(loaded.timestamp == "t2", "latest timestamp mismatch");
    CHECK(loaded.chat.messages[0].content == "second", "latest content mismatch");
    PASS();
}

void test_load_latest_fails_when_project_missing() {
    TEST(load_latest_fails_when_project_missing);
    auto ws = tempWorkspace();
    std::string error;
    AgentChatSavedSession loaded;
    CHECK(!AgentChatSessionPersistence::loadLatestSession(ws.string(), "src/main.cpp", &loaded, &error),
          "load should fail");
    CHECK(error == "session_not_found", "wrong error");
    PASS();
}

void test_load_latest_requires_project_file() {
    TEST(load_latest_requires_project_file);
    auto ws = tempWorkspace();
    std::string error;
    AgentChatSavedSession loaded;
    CHECK(!AgentChatSessionPersistence::loadLatestSession(ws.string(), "", &loaded, &error),
          "load should fail");
    CHECK(error == "project_file_missing", "wrong error");
    PASS();
}

void test_list_session_timestamps_returns_all_entries() {
    TEST(list_session_timestamps_returns_all_entries);
    auto ws = tempWorkspace();
    std::string error;
    (void)AgentChatSessionPersistence::saveSession(ws.string(), "src/main.cpp", "t1", sampleChat("a"), &error);
    (void)AgentChatSessionPersistence::saveSession(ws.string(), "src/main.cpp", "t2", sampleChat("b"), &error);
    auto stamps = AgentChatSessionPersistence::listSessionTimestamps(ws.string(), "src/main.cpp", &error);
    CHECK(stamps.size() == 2, "timestamp count mismatch");
    CHECK(stamps[0] == "t1" && stamps[1] == "t2", "timestamp order mismatch");
    PASS();
}

void test_list_session_timestamps_requires_project_file() {
    TEST(list_session_timestamps_requires_project_file);
    auto ws = tempWorkspace();
    std::string error;
    auto stamps = AgentChatSessionPersistence::listSessionTimestamps(ws.string(), "", &error);
    CHECK(stamps.empty(), "stamps should be empty");
    CHECK(error == "project_file_missing", "wrong error");
    PASS();
}

void test_save_sessions_are_scoped_per_project_file() {
    TEST(save_sessions_are_scoped_per_project_file);
    auto ws = tempWorkspace();
    std::string error;
    (void)AgentChatSessionPersistence::saveSession(ws.string(), "src/a.cpp", "t1", sampleChat("A"), &error);
    (void)AgentChatSessionPersistence::saveSession(ws.string(), "src/b.cpp", "t2", sampleChat("B"), &error);
    AgentChatSavedSession loadedA;
    AgentChatSavedSession loadedB;
    CHECK(AgentChatSessionPersistence::loadLatestSession(ws.string(), "src/a.cpp", &loadedA, &error), "load A failed");
    CHECK(AgentChatSessionPersistence::loadLatestSession(ws.string(), "src/b.cpp", &loadedB, &error), "load B failed");
    CHECK(loadedA.chat.messages[0].content == "A", "A content mismatch");
    CHECK(loadedB.chat.messages[0].content == "B", "B content mismatch");
    PASS();
}

void test_saved_system_context_roundtrips() {
    TEST(saved_system_context_roundtrips);
    auto ws = tempWorkspace();
    std::string error;
    AgentChatState chat = sampleChat("hello");
    chat.systemContext = "Context: activeFile src/main.cpp";
    (void)AgentChatSessionPersistence::saveSession(ws.string(), "src/main.cpp", "t3", chat, &error);
    AgentChatSavedSession loaded;
    CHECK(AgentChatSessionPersistence::loadLatestSession(ws.string(), "src/main.cpp", &loaded, &error), "load failed");
    CHECK(loaded.chat.systemContext == "Context: activeFile src/main.cpp", "system context mismatch");
    PASS();
}

int main() {
    std::cout << "Step 630: Chat session persistence\n";

    test_session_store_path_uses_workspace_root();           // 1
    test_save_session_requires_project_file();               // 2
    test_save_session_requires_timestamp();                  // 3
    test_save_session_creates_store_file();                 // 4
    test_load_latest_returns_saved_session();               // 5
    test_load_latest_uses_most_recent_timestamp_entry();    // 6
    test_load_latest_fails_when_project_missing();          // 7
    test_load_latest_requires_project_file();               // 8
    test_list_session_timestamps_returns_all_entries();     // 9
    test_list_session_timestamps_requires_project_file();   // 10
    test_save_sessions_are_scoped_per_project_file();       // 11
    test_saved_system_context_roundtrips();                 // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
