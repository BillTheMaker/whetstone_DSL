// Step 1908: LSPProxyServer
// LSP protocol handler (JSON-RPC 2.0) that receives requests and dispatches them.
// Models per-language servers as stubs — no real network required.
//
//  t1: initialize request returns capabilities response with correct id
//  t2: textDocument/definition dispatches and returns a result object
//  t3: textDocument/hover dispatches and returns a result object
//  t4: unknown method returns JSON-RPC error (code -32601 method not found)
//  t5: notification (no id) is handled without returning a response

#include "LSPProxyServer.h"
#include <iostream>
#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
namespace ws = whetstone;

static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

static json makeRequest(int id, const std::string& method, json params = json::object()) {
    return {{"jsonrpc","2.0"},{"id",id},{"method",method},{"params",params}};
}

static json makeNotification(const std::string& method, json params = json::object()) {
    return {{"jsonrpc","2.0"},{"method",method},{"params",params}};
}

void t1(){
    T(initialize_request_returns_capabilities_with_correct_id);
    ws::LSPProxyServer server;
    auto req = makeRequest(1, "initialize", {{"capabilities",json::object()}});

    auto resp = server.handle(req);

    C(resp.contains("id") && resp["id"] == 1,
      "response id must match request id");
    C(resp.contains("result"),
      "initialize must return a result");
    C(resp["result"].contains("capabilities"),
      "result must have capabilities");
    P();
}

void t2(){
    T(textDocument_definition_dispatches_returns_result);
    ws::LSPProxyServer server;
    auto req = makeRequest(2, "textDocument/definition", {
        {"textDocument", {{"uri","file:///sort.py"}}},
        {"position",     {{"line",5},{"character",10}}}
    });

    auto resp = server.handle(req);

    C(resp.contains("id") && resp["id"] == 2,
      "response id must be 2");
    C(resp.contains("result"),
      "textDocument/definition must return result (may be null)");
    P();
}

void t3(){
    T(textDocument_hover_dispatches_returns_result);
    ws::LSPProxyServer server;
    auto req = makeRequest(3, "textDocument/hover", {
        {"textDocument", {{"uri","file:///sort.py"}}},
        {"position",     {{"line",5},{"character",10}}}
    });

    auto resp = server.handle(req);

    C(resp.contains("id") && resp["id"] == 3,
      "response id must be 3");
    C(resp.contains("result"),
      "textDocument/hover must return result");
    P();
}

void t4(){
    T(unknown_method_returns_method_not_found_error);
    ws::LSPProxyServer server;
    auto req = makeRequest(4, "textDocument/nonexistent", json::object());

    auto resp = server.handle(req);

    C(resp.contains("id") && resp["id"] == 4,
      "error response must echo id");
    C(resp.contains("error"),
      "unknown method must return error");
    C(resp["error"].contains("code") && resp["error"]["code"] == -32601,
      "error code must be -32601 (MethodNotFound)");
    P();
}

void t5(){
    T(notification_handled_without_response);
    ws::LSPProxyServer server;
    auto notif = makeNotification("textDocument/didOpen", {
        {"textDocument", {{"uri","file:///sort.py"},{"languageId","python"},{"version",1},{"text",""}}}
    });

    auto resp = server.handle(notif);

    // Notifications must not produce a response (return null/empty JSON)
    C(resp.is_null() || (resp.is_object() && resp.empty()),
      "notification must return null or empty JSON, not a response");
    P();
}

int main(){
    std::cout << "Step 1908: LSPProxyServer\n";
    t1(); t2(); t3(); t4(); t5();
    std::cout << "\n" << p << "/" << (p+f) << " passed\n";
    return f > 0 ? 1 : 0;
}
