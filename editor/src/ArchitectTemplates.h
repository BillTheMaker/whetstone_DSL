#pragma once

// Step 477: Architecture Templates
// Prebuilt architecture skeleton patterns for common project types.

#include "ArchitectSkeletonGenerator.h"

#include <map>
#include <string>
#include <vector>

enum class ArchitectureTemplateKind {
    RestApi,
    CliTool,
    Library,
    Microservice,
    FullStack
};

struct TemplateInput {
    std::string projectName;
    std::string primaryEntity; // e.g. "Book"
    std::vector<std::string> fields;
    std::string authMethod;    // e.g. jwt/oauth/none
};

struct TemplateOutput {
    ArchitectureTemplateKind kind = ArchitectureTemplateKind::RestApi;
    SkeletonProjectSpec skeleton;
    std::vector<std::string> notes;
};

class ArchitectTemplates {
public:
    static TemplateOutput instantiate(ArchitectureTemplateKind kind,
                                      const TemplateInput& in) {
        TemplateOutput out;
        out.kind = kind;
        switch (kind) {
            case ArchitectureTemplateKind::RestApi:
                out.skeleton = restApiTemplate(in);
                out.notes.push_back("REST API template applied");
                break;
            case ArchitectureTemplateKind::CliTool:
                out.skeleton = cliTemplate(in);
                out.notes.push_back("CLI template applied");
                break;
            case ArchitectureTemplateKind::Library:
                out.skeleton = libraryTemplate(in);
                out.notes.push_back("Library template applied");
                break;
            case ArchitectureTemplateKind::Microservice:
                out.skeleton = microserviceTemplate(in);
                out.notes.push_back("Microservice template applied");
                break;
            case ArchitectureTemplateKind::FullStack:
                out.skeleton = fullStackTemplate(in);
                out.notes.push_back("Full Stack template applied");
                break;
        }
        return out;
    }

private:
    static SkeletonModuleSpec module(const std::string& name,
                                     const std::string& lang,
                                     std::initializer_list<SkeletonFunctionSpec> fns,
                                     std::initializer_list<std::string> deps = {}) {
        SkeletonModuleSpec m;
        m.moduleName = name;
        m.language = lang;
        m.dependencies.assign(deps.begin(), deps.end());
        m.functions.assign(fns.begin(), fns.end());
        return m;
    }

    static SkeletonFunctionSpec fn(const std::string& name,
                                   const std::string& sig,
                                   const std::string& intent) {
        SkeletonFunctionSpec f;
        f.name = name;
        f.signature = sig;
        f.annotations = {
            {"@Intent", intent},
            {"@Complexity", "medium"},
            {"@ContextWidth", "module"},
            {"@Automatability", "llm"},
            {"@Contract", "pre: valid input; post: expected output"}
        };
        return f;
    }

    static SkeletonProjectSpec restApiTemplate(const TemplateInput& in) {
        SkeletonProjectSpec sk;
        sk.modules.push_back(module("routes", "typescript", {
            fn("registerRoutes", "void registerRoutes(App app)", "register REST routes")
        }, {"handlers"}));
        sk.modules.push_back(module("handlers", "typescript", {
            fn("list" + in.primaryEntity + "s", "Response list(Request req)", "list entities"),
            fn("create" + in.primaryEntity, "Response create(Request req)", "create entity")
        }, {"models", "database", "auth"}));
        sk.modules.push_back(module("models", "typescript", {
            fn("validate" + in.primaryEntity, "bool validate(Entity e)", "validate entity")
        }));
        sk.modules.push_back(module("middleware", "typescript", {
            fn("authMiddleware", "bool authMiddleware(Request req)", "enforce auth")
        }));
        sk.modules.push_back(module("database", "sql", {
            fn("migrate", "void migrate()", "apply schema migrations")
        }));
        sk.modules.push_back(module("auth", "typescript", {
            fn("authenticate", "AuthResult authenticate(Request req)", "authenticate request")
        }));
        return sk;
    }

    static SkeletonProjectSpec cliTemplate(const TemplateInput& in) {
        SkeletonProjectSpec sk;
        sk.modules.push_back(module("cli", "python", {
            fn("parseArgs", "Options parseArgs(List args)", "parse command-line arguments"),
            fn("runCommand", "int runCommand(Options opts)", "execute command")
        }, {"output"}));
        sk.modules.push_back(module("commands", "python", {
            fn("handle" + in.primaryEntity, "int handle(Options opts)", "handle primary command")
        }));
        sk.modules.push_back(module("output", "python", {
            fn("formatOutput", "str formatOutput(Result r)", "format command output")
        }));
        return sk;
    }

    static SkeletonProjectSpec libraryTemplate(const TemplateInput& in) {
        SkeletonProjectSpec sk;
        sk.modules.push_back(module("public_api", "cpp", {
            fn("create" + in.primaryEntity, "Result create(Input in)", "public API entrypoint")
        }, {"internal"}));
        sk.modules.push_back(module("internal", "cpp", {
            fn("transform", "Data transform(Data in)", "internal transformation")
        }));
        sk.modules.push_back(module("tests", "cpp", {
            fn("testPublicApi", "void testPublicApi()", "library behavior verification")
        }));
        sk.modules.push_back(module("docs", "markdown", {
            fn("generateDocs", "void generateDocs()", "generate documentation stubs")
        }));
        return sk;
    }

    static SkeletonProjectSpec microserviceTemplate(const TemplateInput& in) {
        SkeletonProjectSpec sk;
        sk.modules.push_back(module("service", "go", {
            fn("startService", "error startService(Config cfg)", "start microservice")
        }, {"transport", "storage"}));
        sk.modules.push_back(module("transport", "go", {
            fn("handleRequest", "Response handle(Request req)", "transport request handling")
        }));
        sk.modules.push_back(module("storage", "go", {
            fn("save" + in.primaryEntity, "error save(Entity e)", "persist entity")
        }));
        sk.modules.push_back(module("health", "go", {
            fn("healthCheck", "Status healthCheck()", "service health endpoint")
        }));
        return sk;
    }

    static SkeletonProjectSpec fullStackTemplate(const TemplateInput& in) {
        SkeletonProjectSpec sk;
        sk.modules.push_back(module("frontend", "typescript", {
            fn("renderApp", "View renderApp(State s)", "render frontend app")
        }, {"backend"}));
        sk.modules.push_back(module("backend", "python", {
            fn("handleApi", "Response handleApi(Request req)", "backend API handling")
        }, {"database", "deploy"}));
        sk.modules.push_back(module("database", "sql", {
            fn("migrate", "void migrate()", "apply database migrations")
        }));
        sk.modules.push_back(module("deploy", "yaml", {
            fn("generateDeployConfig", "void generateDeployConfig()", "deployment config generation")
        }));
        return sk;
    }
};
