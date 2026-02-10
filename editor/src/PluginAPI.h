#pragma once
// Step 161: Plugin API definition

#include <string>

struct PluginDescriptor {
    std::string name;
    std::string version;
    std::string apiVersion;
    std::string author;
    std::string entrySymbol;
};

struct PluginHost {
    void* ctx = nullptr;
    void (*registerConcept)(void* ctx, const char* name) = nullptr;
    void (*registerGenerator)(void* ctx, const char* language) = nullptr;
    void (*registerGrammar)(void* ctx, const char* language) = nullptr;
    void (*registerAnnotation)(void* ctx, const char* name) = nullptr;
    void (*registerPanel)(void* ctx, const char* name) = nullptr;
    void (*registerCommand)(void* ctx, const char* commandId) = nullptr;
    void (*registerKeybinding)(void* ctx, const char* commandId,
                               const char* keybinding) = nullptr;
};

using PluginInitFn = bool (*)(const PluginHost* host,
                              PluginDescriptor* outDescriptor);
using PluginShutdownFn = void (*)();
