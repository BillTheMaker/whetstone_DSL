#pragma once
// Step 59: WebSocket agent endpoint
//
// WebSocketAgentServer: manages agent sessions and routes JSON-RPC messages.
// Agents connect via WebSocket and exchange JSON-RPC over the connection.
//
// Architecture:
//   WebSocketTransport (abstract) — handles raw WebSocket I/O
//   WebSocketAgentServer          — session management + JSON-RPC dispatch
//   MockWebSocketTransport        — for testing without real sockets
//
// The server runs alongside the existing stdin/stdout JSON-RPC in
// orchestrator_main.cpp.  Both share the same processRequest() handler
// via the setRequestHandler() callback.

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <memory>
#include <chrono>
#include <unordered_map>
#include <unordered_set>
#include <nlohmann/json.hpp>

#include "PrimitivesRegistry.h"
#include "ast/Module.h"
using json = nlohmann::json;

// ---------------------------------------------------------------------------
//  AgentSession — represents one connected agent
// ---------------------------------------------------------------------------
struct AgentSession {
    std::string sessionId;
    std::string agentName;       // optional, set via setAgentName RPC
    bool connected = false;
    uint64_t connectedAtMs = 0;
    uint64_t lastMessageAtMs = 0;
    int messageCount = 0;
};

// Callback types
using JsonRpcHandler =
    std::function<json(const json& request, const std::string& sessionId)>;
using SessionEventCallback =
    std::function<void(const AgentSession& session, const std::string& event)>;
using RequestLogCallback =
    std::function<void(const std::string& sessionId,
                       const json& request,
                       const json& response)>;
using LibraryContextProvider =
    std::function<json(const std::string& sessionId)>;

// ---------------------------------------------------------------------------
//  WebSocketTransport — abstract transport interface
// ---------------------------------------------------------------------------
class WebSocketTransport {
public:
    virtual ~WebSocketTransport() = default;

    virtual bool start(int port) = 0;
    virtual void stop() = 0;
    virtual bool isRunning() const = 0;

    // Send a text message to a connected session
    virtual bool sendMessage(const std::string& sessionId,
                             const std::string& message) = 0;

    // --- handler registration (called by WebSocketAgentServer) ---

    void setMessageHandler(
        std::function<void(const std::string& sessionId,
                           const std::string& message)> handler) {
        messageHandler_ = std::move(handler);
    }

    void setConnectHandler(
        std::function<void(const std::string& sessionId)> handler) {
        connectHandler_ = std::move(handler);
    }

    void setDisconnectHandler(
        std::function<void(const std::string& sessionId)> handler) {
        disconnectHandler_ = std::move(handler);
    }

protected:
    std::function<void(const std::string&, const std::string&)> messageHandler_;
    std::function<void(const std::string&)> connectHandler_;
    std::function<void(const std::string&)> disconnectHandler_;
};

// ---------------------------------------------------------------------------
//  WebSocketAgentServer
// ---------------------------------------------------------------------------
class WebSocketAgentServer {
public:
    struct LibraryContext {
        json data;
        static LibraryContext fromRegistry(PrimitivesRegistry& registry,
                                           Module* root,
                                           const std::string& language) {
            LibraryContext ctx;
            registry.setRoot(root);
            registry.setLanguage(language);

            json funcs = json::array();
            json types = json::array();
            json consts = json::array();

            auto addSymbol = [](json& arr, const PrimitiveSymbol& sym) {
                arr.push_back({
                    {"name", sym.name},
                    {"kind", sym.kind},
                    {"source", sym.source},
                    {"library", sym.library}
                });
            };

            for (const auto& sym : registry.getAvailableFunctions()) addSymbol(funcs, sym);
            for (const auto& sym : registry.getAvailableTypes()) addSymbol(types, sym);
            for (const auto& sym : registry.getAvailableConstants()) addSymbol(consts, sym);

            ctx.data = {
                {"language", language},
                {"functions", funcs},
                {"types", types},
                {"constants", consts}
            };
            return ctx;
        }
    };

    explicit WebSocketAgentServer(std::unique_ptr<WebSocketTransport> transport)
        : transport_(std::move(transport))
    {
        transport_->setMessageHandler(
            [this](const std::string& sid, const std::string& msg) {
                onMessage(sid, msg);
            });
        transport_->setConnectHandler(
            [this](const std::string& sid) { onConnect(sid); });
        transport_->setDisconnectHandler(
            [this](const std::string& sid) { onDisconnect(sid); });
    }

    bool start(int port) {
        port_ = port;
        return transport_->start(port);
    }

    void stop() { transport_->stop(); }

    bool isRunning() const { return transport_->isRunning(); }
    int  getPort()   const { return port_; }

    // Register a fallback JSON-RPC handler for methods the server doesn't
    // handle itself (e.g. the orchestrator's processRequest).
    void setRequestHandler(JsonRpcHandler handler) {
        requestHandler_ = std::move(handler);
    }

    // Register a callback for session lifecycle events ("connected" /
    // "disconnected").
    void setSessionEventCallback(SessionEventCallback cb) {
        sessionEventCallback_ = std::move(cb);
    }

    void setRequestLogCallback(RequestLogCallback cb) {
        requestLogCallback_ = std::move(cb);
    }

    void setLibraryContextProvider(LibraryContextProvider cb) {
        libraryContextProvider_ = std::move(cb);
    }

    // --- session queries ---

    const AgentSession* getSession(const std::string& sessionId) const {
        auto it = sessions_.find(sessionId);
        return it != sessions_.end() ? &it->second : nullptr;
    }

    std::vector<AgentSession> getActiveSessions() const {
        std::vector<AgentSession> result;
        for (const auto& [id, s] : sessions_) {
            if (s.connected) result.push_back(s);
        }
        return result;
    }

    size_t getActiveSessionCount() const {
        size_t n = 0;
        for (const auto& [id, s] : sessions_) {
            if (s.connected) ++n;
        }
        return n;
    }

private:
    // --- transport callbacks ---

    void onConnect(const std::string& sessionId) {
        AgentSession s;
        s.sessionId     = sessionId;
        s.connected     = true;
        s.connectedAtMs = nowMs();
        s.lastMessageAtMs = s.connectedAtMs;
        sessions_[sessionId] = s;

        if (sessionEventCallback_)
            sessionEventCallback_(s, "connected");

        if (libraryContextProvider_) {
            json payload = libraryContextProvider_(sessionId);
            json message = {
                {"jsonrpc", "2.0"},
                {"method", "libraryContext"},
                {"params", payload}
            };
            transport_->sendMessage(sessionId, message.dump());
        }
    }

    void onDisconnect(const std::string& sessionId) {
        auto it = sessions_.find(sessionId);
        if (it != sessions_.end()) {
            it->second.connected = false;
            if (sessionEventCallback_)
                sessionEventCallback_(it->second, "disconnected");
        }
    }

    void onMessage(const std::string& sessionId, const std::string& message) {
        // Update session stats
        auto it = sessions_.find(sessionId);
        if (it != sessions_.end()) {
            it->second.lastMessageAtMs = nowMs();
            it->second.messageCount++;
        }

        // Parse + dispatch
        json response;
        try {
            json request = json::parse(message);
            response = processRequest(request, sessionId);
        } catch (const std::exception& e) {
            response = {
                {"jsonrpc", "2.0"},
                {"id", nullptr},
                {"error", {{"code", -32700},
                           {"message", std::string("Parse error: ") + e.what()}}}
            };
        }

        if (requestLogCallback_) {
            try {
                requestLogCallback_(sessionId, json::parse(message), response);
            } catch (...) {
                // Ignore logging parse errors
            }
        }
        transport_->sendMessage(sessionId, response.dump());
    }

    // Route a JSON-RPC request.  Built-in methods are handled first;
    // everything else is forwarded to the registered handler.
    json processRequest(const json& request, const std::string& sessionId) {
        json response;
        response["jsonrpc"] = "2.0";
        response["id"] = request.contains("id") ? request["id"] : json(nullptr);

        try {
            std::string method = request.at("method").get<std::string>();

            if (method == "ping") {
                response["result"] = json{
                    {"message", "pong"},
                    {"sessionId", sessionId}
                };
            }
            else if (method == "getSessionInfo") {
                auto* sess = getSession(sessionId);
                if (sess) {
                    response["result"] = {
                        {"sessionId",    sess->sessionId},
                        {"agentName",    sess->agentName},
                        {"connected",    sess->connected},
                        {"messageCount", sess->messageCount}
                    };
                } else {
                    response["error"] = {
                        {"code", -32603},
                        {"message", "Session not found"}
                    };
                }
            }
            else if (method == "setAgentName") {
                std::string name =
                    request.at("params").at("name").get<std::string>();
                auto sit = sessions_.find(sessionId);
                if (sit != sessions_.end()) {
                    sit->second.agentName = name;
                    response["result"] = true;
                } else {
                    response["error"] = {
                        {"code", -32603},
                        {"message", "Session not found"}
                    };
                }
            }
            else if (method == "listSessions") {
                json arr = json::array();
                for (const auto& [id, s] : sessions_) {
                    if (s.connected) {
                        arr.push_back({
                            {"sessionId",    s.sessionId},
                            {"agentName",    s.agentName},
                            {"messageCount", s.messageCount}
                        });
                    }
                }
                response["result"] = arr;
            }
            else if (requestHandler_) {
                // Delegate to the orchestrator's processRequest
                response = requestHandler_(request, sessionId);
                // Ensure jsonrpc and id are set
                response["jsonrpc"] = "2.0";
                if (!response.contains("id"))
                    response["id"] = request.contains("id")
                                         ? request["id"] : json(nullptr);
            }
            else {
                response["error"] = {
                    {"code", -32601},
                    {"message", "Method not found: " + method}
                };
            }
        } catch (const std::exception& e) {
            response["error"] = {
                {"code", -32600},
                {"message", std::string("Invalid request: ") + e.what()}
            };
        }

        return response;
    }

    static uint64_t nowMs() {
        using namespace std::chrono;
        return static_cast<uint64_t>(
            duration_cast<milliseconds>(
                steady_clock::now().time_since_epoch())
            .count());
    }

    std::unique_ptr<WebSocketTransport> transport_;
    std::map<std::string, AgentSession> sessions_;
    JsonRpcHandler requestHandler_;
    SessionEventCallback sessionEventCallback_;
    RequestLogCallback requestLogCallback_;
    LibraryContextProvider libraryContextProvider_;
    int port_ = 0;
};

// ---------------------------------------------------------------------------
//  MockWebSocketTransport — for testing without real sockets
// ---------------------------------------------------------------------------
class MockWebSocketTransport : public WebSocketTransport {
public:
    bool start(int port) override {
        port_    = port;
        running_ = true;
        return true;
    }

    void stop() override { running_ = false; }

    bool isRunning() const override { return running_; }

    bool sendMessage(const std::string& sessionId,
                     const std::string& message) override {
        sentMessages_[sessionId].push_back(message);
        return true;
    }

    // --- test helpers: simulate agent behaviour ---

    // Simulate an agent opening a WebSocket connection
    std::string simulateConnect() {
        std::string sessionId = "agent_" + std::to_string(nextId_++);
        if (connectHandler_) connectHandler_(sessionId);
        return sessionId;
    }

    // Simulate an agent closing its connection
    void simulateDisconnect(const std::string& sessionId) {
        if (disconnectHandler_) disconnectHandler_(sessionId);
    }

    // Simulate an agent sending a text message
    void simulateMessage(const std::string& sessionId,
                         const std::string& message) {
        if (messageHandler_) messageHandler_(sessionId, message);
    }

    // Get all messages the server sent back to a session
    std::vector<std::string> getSentMessages(
        const std::string& sessionId) const {
        auto it = sentMessages_.find(sessionId);
        return it != sentMessages_.end()
                   ? it->second
                   : std::vector<std::string>{};
    }

    // Get the most recent message sent to a session
    std::string getLastSentMessage(const std::string& sessionId) const {
        auto it = sentMessages_.find(sessionId);
        if (it != sentMessages_.end() && !it->second.empty())
            return it->second.back();
        return "";
    }

    // Clear recorded messages (useful between test cases)
    void clearSentMessages() { sentMessages_.clear(); }

private:
    bool running_ = false;
    int  port_    = 0;
    int  nextId_  = 1;
    std::map<std::string, std::vector<std::string>> sentMessages_;
};
