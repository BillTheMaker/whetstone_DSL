#pragma once
#include <nlohmann/json.hpp>
#include <functional>
#include <map>
#include <string>

namespace whetstone {

class LSPProxyServer {
public:
    using Handler = std::function<nlohmann::json(const nlohmann::json& params)>;

    LSPProxyServer() {
        handlers_["initialize"] = [](const nlohmann::json&) -> nlohmann::json {
            return {{"capabilities", {
                {"textDocumentSync",   1},
                {"definitionProvider", true},
                {"hoverProvider",      true},
                {"referencesProvider", true}
            }}};
        };
        auto nullStub = [](const nlohmann::json&) -> nlohmann::json { return nullptr; };
        auto emptyStub = [](const nlohmann::json&) -> nlohmann::json { return nlohmann::json::array(); };
        handlers_["textDocument/definition"]  = nullStub;
        handlers_["textDocument/hover"]       = nullStub;
        handlers_["textDocument/references"]  = emptyStub;
        handlers_["shutdown"]                 = nullStub;
        // Notifications (no-op)
        handlers_["textDocument/didOpen"]   = [](const nlohmann::json&){ return nlohmann::json{}; };
        handlers_["textDocument/didChange"] = [](const nlohmann::json&){ return nlohmann::json{}; };
        handlers_["textDocument/didClose"]  = [](const nlohmann::json&){ return nlohmann::json{}; };
        handlers_["exit"]                   = [](const nlohmann::json&){ return nlohmann::json{}; };
    }

    nlohmann::json handle(const nlohmann::json& message) {
        if (!message.contains("method"))
            return makeError(idOf(message), -32600, "Invalid Request");

        std::string method = message["method"].get<std::string>();
        bool isNotification = !message.contains("id");

        nlohmann::json params = message.contains("params") ? message["params"]
                                                           : nlohmann::json::object();
        if (isNotification) {
            auto it = handlers_.find(method);
            if (it != handlers_.end()) it->second(params);
            return nlohmann::json{};   // null — no response for notifications
        }

        nlohmann::json id = message["id"];
        auto it = handlers_.find(method);
        if (it == handlers_.end())
            return makeError(id, -32601, "Method not found");

        nlohmann::json result = it->second(params);
        return makeResponse(id, result);
    }

    void registerHandler(const std::string& method, Handler handler) {
        handlers_[method] = std::move(handler);
    }

private:
    std::map<std::string, Handler> handlers_;

    static nlohmann::json idOf(const nlohmann::json& msg) {
        return msg.contains("id") ? msg["id"] : nullptr;
    }

    nlohmann::json makeResponse(const nlohmann::json& id,
                                const nlohmann::json& result) const {
        return {{"jsonrpc","2.0"}, {"id",id}, {"result",result}};
    }

    nlohmann::json makeError(const nlohmann::json& id, int code,
                             const std::string& msg) const {
        return {{"jsonrpc","2.0"}, {"id",id},
                {"error", {{"code",code}, {"message",msg}}}};
    }
};

} // namespace whetstone
