#pragma once
// Step 90: LSP client core
//
// Minimal JSON-RPC LSP client with injectable transport.

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
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

    struct Position {
        int line = 0;
        int character = 0;
    };

    struct Range {
        Position start;
        Position end;
    };

    struct Diagnostic {
        std::string uri;
        Range range;
        int severity = 0;
        std::string message;
        std::string source;
    };

    struct CompletionItem {
        std::string label;
        int kind = 0;
        std::string detail;
        std::string insertText;
        std::string filterText;
    };

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

    void didOpen(const std::string& uri,
                 const std::string& languageId,
                 const std::string& text,
                 int version) {
        nlohmann::json params;
        params["textDocument"] = {
            {"uri", uri},
            {"languageId", languageId},
            {"version", version},
            {"text", text}
        };
        sendNotification("textDocument/didOpen", params);
    }

    void didChange(const std::string& uri,
                   const std::string& text,
                   int version) {
        nlohmann::json params;
        params["textDocument"] = {
            {"uri", uri},
            {"version", version}
        };
        params["contentChanges"] = nlohmann::json::array({{
            {"text", text}
        }});
        sendNotification("textDocument/didChange", params);
    }

    void didSave(const std::string& uri, const std::string& text = std::string()) {
        nlohmann::json params;
        params["textDocument"] = {{"uri", uri}};
        if (!text.empty()) params["text"] = text;
        sendNotification("textDocument/didSave", params);
    }

    int requestCompletion(const std::string& uri, int line, int character) {
        nlohmann::json params;
        params["textDocument"] = {{"uri", uri}};
        params["position"] = {{"line", line}, {"character", character}};
        int id = sendRequest("textDocument/completion", params);
        lastCompletionRequestId_ = id;
        return id;
    }

    void handleMessage(const std::string& msg) {
        nlohmann::json j;
        try {
            j = nlohmann::json::parse(msg);
        } catch (...) {
            return;
        }
        if (j.contains("method")) {
            if (j["method"].get<std::string>() != "textDocument/publishDiagnostics") return;
            if (!j.contains("params")) return;
            const auto& params = j["params"];
            if (!params.contains("uri") || !params.contains("diagnostics")) return;

            std::string uri = params["uri"].get<std::string>();
            std::vector<Diagnostic> parsed;
            for (const auto& diag : params["diagnostics"]) {
                Diagnostic d;
                d.uri = uri;
                if (diag.contains("range")) {
                    const auto& range = diag["range"];
                    if (range.contains("start")) {
                        d.range.start.line = range["start"].value("line", 0);
                        d.range.start.character = range["start"].value("character", 0);
                    }
                    if (range.contains("end")) {
                        d.range.end.line = range["end"].value("line", 0);
                        d.range.end.character = range["end"].value("character", 0);
                    }
                }
                d.severity = diag.value("severity", 0);
                d.message = diag.value("message", "");
                d.source = diag.value("source", "");
                parsed.push_back(std::move(d));
            }

            diagnosticsByUri_[uri] = std::move(parsed);
            return;
        }

        if (j.contains("id") && j.contains("result")) {
            int id = j["id"].get<int>();
            if (lastCompletionRequestId_ > 0 && id != lastCompletionRequestId_) return;
            const auto& result = j["result"];
            const nlohmann::json* items = nullptr;
            if (result.is_array()) {
                items = &result;
            } else if (result.is_object() && result.contains("items")) {
                items = &result["items"];
            }
            if (!items || !items->is_array()) return;

            std::vector<CompletionItem> parsed;
            for (const auto& item : *items) {
                CompletionItem ci;
                ci.label = item.value("label", "");
                ci.kind = item.value("kind", 0);
                ci.detail = item.value("detail", "");
                ci.insertText = item.value("insertText", ci.label);
                ci.filterText = item.value("filterText", "");
                parsed.push_back(std::move(ci));
            }
            completionItems_ = std::move(parsed);
        }
    }

    std::vector<Diagnostic> getDiagnostics() const {
        std::vector<Diagnostic> out;
        for (const auto& kv : diagnosticsByUri_) {
            out.insert(out.end(), kv.second.begin(), kv.second.end());
        }
        return out;
    }

    std::vector<Diagnostic> getDiagnosticsForUri(const std::string& uri) const {
        auto it = diagnosticsByUri_.find(uri);
        if (it == diagnosticsByUri_.end()) return {};
        return it->second;
    }

    std::vector<CompletionItem> getCompletionItems() const {
        return completionItems_;
    }

    void clearCompletionItems() {
        completionItems_.clear();
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
    std::unordered_map<std::string, std::vector<Diagnostic>> diagnosticsByUri_;
    int lastCompletionRequestId_ = -1;
    std::vector<CompletionItem> completionItems_;
};
