#pragma once
// Step 90: LSP client core
//
// Minimal JSON-RPC LSP client with injectable transport.

#include <string>
#include <memory>
#include <vector>
#include <nlohmann/json.hpp>

class LSPTransport {
public:
    virtual ~LSPTransport() = default;
    virtual void send(const std::string& msg) = 0;
    virtual bool receive(std::string& out) = 0;
    virtual bool isOpen() const = 0;
    virtual void close() = 0;
};

class LSPClient {
public:
    explicit LSPClient(std::shared_ptr<LSPTransport> transport)
        : transport_(std::move(transport)) {}

    void initialize(const std::string& rootUri,
                    const std::string& clientName,
                    int processId) {
        nlohmann::json params;
        params["processId"] = processId;
        params["rootUri"] = rootUri;
        params["clientInfo"] = { {"name", clientName}, {"version", "0.1"} };
        params["capabilities"] = nlohmann::json::object();
        sendRequest("initialize", params);
        sendNotification("initialized", nlohmann::json::object());
    }

    void shutdown() {
        sendRequest("shutdown", nlohmann::json::object());
        sendNotification("exit", nlohmann::json::object());
        if (transport_) transport_->close();
    }

    void sendNotification(const std::string& method, const nlohmann::json& params) {
        nlohmann::json msg;
        msg["jsonrpc"] = "2.0";
        msg["method"] = method;
        msg["params"] = params;
        sendMessage(msg);
    }

    int sendRequest(const std::string& method, const nlohmann::json& params) {
        nlohmann::json msg;
        msg["jsonrpc"] = "2.0";
        msg["id"] = nextId_++;
        msg["method"] = method;
        msg["params"] = params;
        sendMessage(msg);
        return msg["id"].get<int>();
    }

private:
    void sendMessage(const nlohmann::json& msg) {
        if (!transport_) return;
        std::string body = msg.dump();
        std::string header = "Content-Length: " + std::to_string(body.size()) + "\r\n\r\n";
        transport_->send(header + body);
    }

    std::shared_ptr<LSPTransport> transport_;
    int nextId_ = 1;
};
