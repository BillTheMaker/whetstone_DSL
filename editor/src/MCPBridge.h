#pragma once
// Step 212: MCP Bridge
//
// Translates between the MCP stdio transport and Whetstone's internal
// JSON-RPC handler. In embedded mode, directly calls processAgentRequest.
// In standalone mode, would connect via WebSocket to a running editor.

#include <string>
#include <iostream>
#include <nlohmann/json.hpp>
#include "MCPServer.h"

using json = nlohmann::json;

class MCPBridge {
public:
    // Set up the bridge with a direct RPC handler (embedded mode)
    void setRequestHandler(MCPServer::RpcCallback handler) {
        server_.setRpcCallback(std::move(handler));
    }

    // Set up the resource reader
    void setResourceReader(MCPServer::ResourceReader reader) {
        server_.setResourceReader(std::move(reader));
    }

    MCPServer& server() { return server_; }

    // Process a single JSON-RPC message string, return response string
    std::string processMessage(const std::string& message) {
        try {
            json request = json::parse(message);
            json response = server_.handleRequest(request);
            if (response.is_null()) return ""; // notifications have no response
            return response.dump();
        } catch (const json::parse_error& e) {
            json error = {
                {"jsonrpc", "2.0"},
                {"id", nullptr},
                {"error", {{"code", -32700}, {"message", "Parse error"}}}
            };
            return error.dump();
        }
    }

    // Run the stdio transport loop (for standalone mcp_main.cpp)
    // Supports two transports:
    //   - Content-Length framing (MCP protocol 2024-11-05 and earlier)
    //   - Newline-delimited JSON / NDJSON (MCP protocol 2025-03-26 and later,
    //     used by Claude Code 2.x)
    void runStdio() {
        std::string line;
        while (std::getline(std::cin, line)) {
            // Strip trailing \r (CRLF line endings)
            if (!line.empty() && line.back() == '\r') line.pop_back();

            if (line.find("Content-Length:") == 0) {
                // Content-Length framing: header + blank line + body
                int length = 0;
                try {
                    length = std::stoi(line.substr(15));
                } catch (...) {
                    continue;
                }
                // Skip remaining headers and blank line
                while (std::getline(std::cin, line)) {
                    if (!line.empty() && line.back() == '\r') line.pop_back();
                    if (line.empty()) break;
                }
                std::string body(length, '\0');
                std::cin.read(&body[0], length);

                std::string response = processMessage(body);
                if (!response.empty()) {
                    std::cout << "Content-Length: " << response.size() << "\r\n\r\n" << response;
                    std::cout.flush();
                }
            } else if (!line.empty() && line.front() == '{') {
                // NDJSON transport: each line is a complete JSON-RPC message
                std::string response = processMessage(line);
                if (!response.empty()) {
                    std::cout << response << "\n";
                    std::cout.flush();
                }
            }
        }
    }

private:
    MCPServer server_;
};
