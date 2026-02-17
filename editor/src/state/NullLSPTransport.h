#pragma once

struct NullLSPTransport : public LSPTransport {
    void send(const std::string& msg) override { (void)msg; }
    bool receive(std::string& out) override { (void)out; return false; }
    bool isOpen() const override { return false; }
    void close() override {}
};
