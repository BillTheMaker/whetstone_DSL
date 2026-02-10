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
    // Reads JSON-RPC messages from stdin, writes responses to stdout.
    // MCP uses Content-Length framing over stdio.
    void runStdio() {
        std::string line;
        while (std::getline(std::cin, line)) {
            // MCP stdio transport: Content-Length header followed by JSON body
            if (line.find("Content-Length:") == 0) {
                int length = 0;
                try {
                    length = std::stoi(line.substr(15));
                } catch (...) {
                    continue;
                }
                // Skip empty line after headers
                std::getline(std::cin, line);
                // Read body
                std::string body(length, '\0');
                std::cin.read(&body[0], length);

                std::string response = processMessage(body);
                if (!response.empty()) {
                    std::cout << "Content-Length: " << response.size() << "\r\n\r\n" << response;
                    std::cout.flush();
                }
            }
        }
    }

private:
    MCPServer server_;
};
