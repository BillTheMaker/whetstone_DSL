// Step 12: Dear ImGui shell - Window + empty layout
//
// Dear ImGui + SDL2 scaffold: opens a window with docking enabled.
// Empty panes: file tree (left), editor area (center), panel (bottom).
// Test: app launches, window renders, panes are resizable.

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <stdio.h>
#include <string>
#include <memory>
#include <array>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Simple JSON-RPC client to communicate with orchestrator
class JsonRpcClient {
private:
    FILE* rpc_out;  // For sending requests to orchestrator
    FILE* rpc_in;   // For receiving responses from orchestrator

public:
    JsonRpcClient() : rpc_out(stdout), rpc_in(stdin) {}
    
    // Constructor that could connect to orchestrator process (simplified for now)
    JsonRpcClient(FILE* out, FILE* in) : rpc_out(out), rpc_in(in) {}
    
    json call(const std::string& method, const json& params = json::object()) {
        // Create the JSON-RPC request
        json request = json::object({
            {"jsonrpc", "2.0"},
            {"method", method},
            {"params", params},
            {"id", 1}  // Simple ID for this example
        });
        
        // Send the request to the orchestrator
        fprintf(rpc_out, "%s\n", request.dump().c_str());
        fflush(rpc_out);
        
        // Read the response from the orchestrator
        char buffer[4096];
        if (fgets(buffer, sizeof(buffer), rpc_in)) {
            try {
                json response = json::parse(buffer);
                if (response.contains("result")) {
                    return response["result"];
                } else if (response.contains("error")) {
                    printf("RPC Error: %s\n", response["error"].dump().c_str());
                    return json(nullptr);
                }
            } catch (...) {
                printf("Failed to parse RPC response\n");
                return json(nullptr);
            }
        }
        
        return json(nullptr);
    }
};

static JsonRpcClient g_rpc_client;

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

// Main code
int main(int, char**)
{
    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char* glsl_version = "#version 100";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__)
    // GL 3.2 Core + GLSL 150
    const char* glsl_version = "#version 150";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window = SDL_CreateWindow("Whetstone Editor", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != nullptr);

    bool done = false;
    while (!done)
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        // Create docking environment
        static bool dockspace_open = true;
        static bool opt_fullscreen = true;
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
        // because it would be confusing to have two docking targets within each others.
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
        if (opt_fullscreen)
        {
            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize);
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
            window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        }

        // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
        // and handle the pass-thru hole, so we ask Begin() to not render a background.
        if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
            window_flags |= ImGuiWindowFlags_NoBackground;

        // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
        // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
        // all active windows docked into it will lose their parent and become undocked.
        // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
        // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("DockSpace Demo", &dockspace_open, window_flags);
        ImGui::PopStyleVar();

        if (opt_fullscreen)
            ImGui::PopStyleVar(2);

        // Submit the DockSpace
        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        }

        ImGui::End();

        // Connect to orchestrator and get AST content
        // Request initial AST from orchestrator
        static char textBuffer[4096] = "Loading from orchestrator...";
        static bool initialized = false;
        if (!initialized) {
            json result = g_rpc_client.call("getAST");
            if (!result.is_null()) {
                std::string newText = result.dump(2);
                strncpy(textBuffer, newText.c_str(), sizeof(textBuffer) - 1);
                textBuffer[sizeof(textBuffer) - 1] = '\0';
            } else {
                // Fallback to default content if RPC fails
                std::string defaultContent = 
                    "\"\"\"Module: Calculator\"\"\"\n\n"
                    "PI = 3.14159\n\n"
                    "def add(x: int, y: int):\n"
                    "    result = x + y\n"
                    "    return result\n\n"
                    "def multiply(a: int, b: int):\n"
                    "    return a * b\n\n"
                    "# @deref(batched)\n"
                    "def calculate_sum(items: List[int]):\n"
                    "    total = 0\n"
                    "    for item in items:\n"
                    "        total += item\n"
                    "    return total\n";
                strncpy(textBuffer, defaultContent.c_str(), sizeof(textBuffer) - 1);
                textBuffer[sizeof(textBuffer) - 1] = '\0';
            }
            initialized = true;
        }

        // Show the editor area - this will be docked by the docking system
        ImGui::Begin("Editor Area");
        
        // Add line numbers in the left margin
        ImGui::BeginChild("LineNumberArea", ImVec2(50, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));  // Gray color for line numbers
        
        int lineCount = 1;
        const char* ptr = textBuffer;
        while (*ptr) {
            if (*ptr == '\n') {
                lineCount++;
            }
            ptr++;
        }
        
        for (int i = 1; i <= lineCount; i++) {
            ImGui::Text("%4d", i);
            ImGui::Spacing();
        }
        
        ImGui::PopStyleColor();
        ImGui::EndChild();
        ImGui::SameLine();
        
        // Text display area with scrolling
        ImGui::BeginChild("TextEditArea", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
        
        // Use a monospace font if available
        ImFont* font = nullptr;
        // Try to find a monospace font - for now use default
        // if (io.Fonts->Fonts.Size > 1) font = io.Fonts->Fonts[1];  // Use second font if available
        
        if (font) ImGui::PushFont(font);
        
        // Display the text content
        ImGui::TextUnformatted(textBuffer);
        
        if (font) ImGui::PopFont();
        ImGui::EndChild();
        
        ImGui::End();

        // Add projection toggle buttons and mutation buttons
        static int projectionMode = 0; // 0 = Python, 1 = AST
        
        ImGui::Begin("Toolbar");
        if (ImGui::Button("Python")) projectionMode = 0;
        ImGui::SameLine();
        if (ImGui::Button("AST")) projectionMode = 1;
        ImGui::SameLine();
        
        // Add parameter button - sends RPC to orchestrator to add a parameter
        if (ImGui::Button("Add Parameter")) {
            // Send RPC to orchestrator to insert a new parameter
            json params = json::object({
                {"parentId", "Calc_F001"},  // Example function ID
                {"role", "parameters"},
                {"concept", "Parameter"},
                {"properties", json::object({
                    {"name", "newParam"}
                })}
            });
            
            json result = g_rpc_client.call("insertNode", params);
            // The result would contain the new parameter node that was inserted
        }
        ImGui::SameLine();
        
        // Undo button - sends RPC to orchestrator to undo last operation
        if (ImGui::Button("Undo")) {
            json result = g_rpc_client.call("undo");
            // The result indicates whether the undo was successful
        }
        ImGui::SameLine();
        
        // Redo button - sends RPC to orchestrator to redo last undone operation
        if (ImGui::Button("Redo")) {
            json result = g_rpc_client.call("redo");
            // The result indicates whether the redo was successful
        }
        ImGui::End();
        
        // Show the editor area based on projection mode
        ImGui::Begin("Editor Area");
        
        if (projectionMode == 0) {
            // Python projection - show generated Python code
            
            // Add line numbers in the left margin
            ImGui::BeginChild("LineNumberArea", ImVec2(50, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));  // Gray color for line numbers
            
            // Count lines in the Python code
            int lineCount = 1;
            const char* ptr = textBuffer;
            while (*ptr) {
                if (*ptr == '\n') {
                    lineCount++;
                }
                ptr++;
            }
            
            for (int i = 1; i <= lineCount; i++) {
                ImGui::Text("%4d", i);
                ImGui::Spacing();
            }
            
            ImGui::PopStyleColor();
            ImGui::EndChild();
            ImGui::SameLine();
            
            // Text display area with scrolling
            ImGui::BeginChild("TextEditArea", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
            
            // Display the Python text content
            ImGui::TextUnformatted(textBuffer);
            
            ImGui::EndChild();
        } else {
            // AST projection - show JSON representation of the AST
            ImGui::BeginChild("ASTViewArea", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
            
            // Show a sample AST JSON representation
            static const char astJson[] = 
                "{\n"
                "  \"id\": \"Calc_M001\",\n"
                "  \"concept\": \"Module\",\n"
                "  \"properties\": {\n"
                "    \"name\": \"Calculator\",\n"
                "    \"targetLanguage\": \"python\"\n"
                "  },\n"
                "  \"children\": {\n"
                "    \"functions\": [\n"
                "      {\n"
                "        \"id\": \"Calc_F001\",\n"
                "        \"concept\": \"Function\",\n"
                "        \"properties\": { \"name\": \"add\" },\n"
                "        \"children\": {\n"
                "          \"parameters\": [\n"
                "            {\n"
                "              \"id\": \"Calc_P001\",\n"
                "              \"concept\": \"Parameter\",\n"
                "              \"properties\": { \"name\": \"x\" },\n"
                "              \"children\": {\n"
                "                \"type\": [{\n"
                "                  \"id\": \"Calc_PT001\",\n"
                "                  \"concept\": \"PrimitiveType\",\n"
                "                  \"properties\": { \"kind\": \"int\" }\n"
                "                }]\n"
                "              }\n"
                "            }\n"
                "          ],\n"
                "          \"body\": [\n"
                "            {\n"
                "              \"id\": \"Calc_A001\",\n"
                "              \"concept\": \"Assignment\",\n"
                "              \"children\": {\n"
                "                \"target\": [...],\n"
                "                \"value\": [...]\n"
                "              }\n"
                "            }\n"
                "          ]\n"
                "        }\n"
                "      }\n"
                "    ]\n"
                "  }\n"
                "}";
            
            ImGui::TextUnformatted(astJson);
            
            ImGui::EndChild();
        }
        
        ImGui::End();

        // Also create file tree and bottom panel for completeness
        ImGui::Begin("File Tree");
        ImGui::Text("Calculator.py");
        ImGui::Text("ConditionalExample.py");
        ImGui::Text("SimpleFunctionExample.py");
        ImGui::End();

        ImGui::Begin("Panel");
        if (projectionMode == 0) {
            ImGui::Text("Projection: Python");
        } else {
            ImGui::Text("Projection: AST");
        }
        ImGui::End();
        
        // Update the text content from orchestrator periodically
        static float lastUpdate = 0.0f;
        if (io.DeltaTime > 0) {
            lastUpdate += io.DeltaTime;
            if (lastUpdate > 2.0f) {  // Update every 2 seconds
                // Request AST from orchestrator
                json result = g_rpc_client.call("getAST");
                if (!result.is_null()) {
                    // Update the text buffer with the result
                    std::string newText = result.dump(2);
                    strncpy(textBuffer, newText.c_str(), sizeof(textBuffer) - 1);
                    textBuffer[sizeof(textBuffer) - 1] = '\0';
                }
                lastUpdate = 0.0f;
            }
        }

        // Rendering
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Update and Render additional Platform Windows
        // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
        //  For this specific demo app we could also call SDL_GL_MakeCurrent(window, gl_context) directly)
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
            SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
        }

        SDL_GL_SwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}