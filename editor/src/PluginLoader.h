#pragma once
// Step 161: Plugin loader

#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include <algorithm>
#include "PluginAPI.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

struct PluginRegistry {
    std::vector<std::string> concepts;
    std::vector<std::string> generators;
    std::vector<std::string> grammars;
    std::vector<std::string> annotations;
    std::vector<std::string> panels;
    std::vector<std::string> commands;
    std::vector<std::pair<std::string, std::string>> keybindings;

    void clear() {
        concepts.clear();
        generators.clear();
        grammars.clear();
        annotations.clear();
        panels.clear();
        commands.clear();
        keybindings.clear();
    }
};

struct PluginHandle {
    std::string path;
    PluginDescriptor descriptor;
    void* handle = nullptr;
    PluginShutdownFn shutdown = nullptr;
};

class PluginLoader {
public:
    bool loadFromDirectory(const std::string& dir, std::string& error) {
        namespace fs = std::filesystem;
        std::error_code ec;
        if (!fs::exists(dir, ec)) {
            error = "Directory not found: " + dir;
            return false;
        }
        bool any = false;
        for (const auto& entry : fs::directory_iterator(dir)) {
            if (!entry.is_regular_file()) continue;
            auto path = entry.path();
#ifdef _WIN32
            if (path.extension() != ".dll") continue;
#elif __APPLE__
            if (path.extension() != ".dylib") continue;
#else
            if (path.extension() != ".so") continue;
#endif
            std::string err;
            if (!loadPlugin(path.string(), err)) {
                error = err;
                return false;
            }
            any = true;
        }
        if (!any) {
            error = "No plugin libraries found in: " + dir;
        }
        return any;
    }

    bool loadPlugin(const std::string& path, std::string& error) {
#ifdef _WIN32
        HMODULE lib = LoadLibraryA(path.c_str());
        if (!lib) {
            error = "LoadLibrary failed: " + path;
            return false;
        }
        auto init = reinterpret_cast<PluginInitFn>(
            GetProcAddress(lib, "WhetstonePluginInit"));
        auto shutdown = reinterpret_cast<PluginShutdownFn>(
            GetProcAddress(lib, "WhetstonePluginShutdown"));
#else
        void* lib = dlopen(path.c_str(), RTLD_LAZY);
        if (!lib) {
            error = std::string("dlopen failed: ") + dlerror();
            return false;
        }
        auto init = reinterpret_cast<PluginInitFn>(
            dlsym(lib, "WhetstonePluginInit"));
        auto shutdown = reinterpret_cast<PluginShutdownFn>(
            dlsym(lib, "WhetstonePluginShutdown"));
#endif
        if (!init) {
            error = "Missing WhetstonePluginInit in: " + path;
            unloadLibrary(lib);
            return false;
        }

        PluginDescriptor desc;
        if (!invokeInit(init, desc)) {
            error = "Plugin init failed: " + path;
            unloadLibrary(lib);
            return false;
        }
        PluginHandle handle;
        handle.path = path;
        handle.descriptor = desc;
        handle.handle = lib;
        handle.shutdown = shutdown;
        plugins_.push_back(std::move(handle));
        return true;
    }

    bool loadFromEntryPoint(PluginInitFn init,
                            const std::string& pathLabel,
                            std::string& error) {
        PluginDescriptor desc;
        if (!invokeInit(init, desc)) {
            error = "Plugin init failed: " + pathLabel;
            return false;
        }
        PluginHandle handle;
        handle.path = pathLabel;
        handle.descriptor = desc;
        plugins_.push_back(std::move(handle));
        return true;
    }

    void unloadAll() {
        for (auto& plugin : plugins_) {
            if (plugin.shutdown) plugin.shutdown();
            if (plugin.handle) {
                unloadLibrary(plugin.handle);
            }
        }
        plugins_.clear();
        registry_.clear();
    }

    const std::vector<PluginHandle>& plugins() const { return plugins_; }
    const PluginRegistry& registry() const { return registry_; }

private:
    bool invokeInit(PluginInitFn init, PluginDescriptor& desc) {
        PluginHost host;
        host.ctx = this;
        host.registerConcept = [](void* ctx, const char* name) {
            static_cast<PluginLoader*>(ctx)->registry_.concepts.emplace_back(name ? name : "");
        };
        host.registerGenerator = [](void* ctx, const char* language) {
            static_cast<PluginLoader*>(ctx)->registry_.generators.emplace_back(language ? language : "");
        };
        host.registerGrammar = [](void* ctx, const char* language) {
            static_cast<PluginLoader*>(ctx)->registry_.grammars.emplace_back(language ? language : "");
        };
        host.registerAnnotation = [](void* ctx, const char* name) {
            static_cast<PluginLoader*>(ctx)->registry_.annotations.emplace_back(name ? name : "");
        };
        host.registerPanel = [](void* ctx, const char* name) {
            static_cast<PluginLoader*>(ctx)->registry_.panels.emplace_back(name ? name : "");
        };
        host.registerCommand = [](void* ctx, const char* commandId) {
            static_cast<PluginLoader*>(ctx)->registry_.commands.emplace_back(commandId ? commandId : "");
        };
        host.registerKeybinding = [](void* ctx, const char* commandId, const char* keybinding) {
            static_cast<PluginLoader*>(ctx)->registry_.keybindings.emplace_back(
                commandId ? commandId : "", keybinding ? keybinding : "");
        };

        return init(&host, &desc);
    }

    static void unloadLibrary(void* handle) {
#ifdef _WIN32
        FreeLibrary(reinterpret_cast<HMODULE>(handle));
#else
        dlclose(handle);
#endif
    }

    PluginRegistry registry_;
    std::vector<PluginHandle> plugins_;
};
