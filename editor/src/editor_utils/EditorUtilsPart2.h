
// ---------------------------------------------------------------------------
//  Theme setup — VSCode Dark-inspired
// ---------------------------------------------------------------------------
static void SetupVSCodeDarkTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // Window
    colors[ImGuiCol_WindowBg]           = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    colors[ImGuiCol_ChildBg]            = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    colors[ImGuiCol_PopupBg]            = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);

    // Borders
    colors[ImGuiCol_Border]             = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_BorderShadow]       = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

    // Frame (input fields, checkboxes)
    colors[ImGuiCol_FrameBg]            = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]     = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_FrameBgActive]      = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);

    // Title bar
    colors[ImGuiCol_TitleBg]            = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
    colors[ImGuiCol_TitleBgActive]      = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]   = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);

    // Menu bar
    colors[ImGuiCol_MenuBarBg]          = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);

    // Scrollbar
    colors[ImGuiCol_ScrollbarBg]        = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab]      = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive]  = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);

    // Buttons
    colors[ImGuiCol_Button]             = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_ButtonHovered]      = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);
    colors[ImGuiCol_ButtonActive]       = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);

    // Headers (collapsing headers, tree nodes)
    colors[ImGuiCol_Header]             = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_HeaderHovered]      = ImVec4(0.26f, 0.26f, 0.26f, 1.00f);
    colors[ImGuiCol_HeaderActive]       = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);

    // Separator
    colors[ImGuiCol_Separator]          = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_SeparatorHovered]   = ImVec4(0.28f, 0.56f, 0.89f, 1.00f);
    colors[ImGuiCol_SeparatorActive]    = ImVec4(0.28f, 0.56f, 0.89f, 1.00f);

    // Tabs
    colors[ImGuiCol_Tab]               = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    colors[ImGuiCol_TabHovered]        = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_TabActive]         = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_TabUnfocused]      = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);

    // Docking
    colors[ImGuiCol_DockingPreview]    = ImVec4(0.28f, 0.56f, 0.89f, 0.70f);
    colors[ImGuiCol_DockingEmptyBg]    = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);

    // Text
    colors[ImGuiCol_Text]              = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
    colors[ImGuiCol_TextDisabled]      = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);

    // Selection / highlight
    colors[ImGuiCol_TextSelectedBg]    = ImVec4(0.17f, 0.40f, 0.64f, 0.60f);

    // Style tweaks
    style.WindowRounding    = 0.0f;
    style.FrameRounding     = 2.0f;
    style.ScrollbarRounding = 2.0f;
    style.GrabRounding      = 2.0f;
    style.TabRounding       = 0.0f;
    style.WindowBorderSize  = 1.0f;
    style.FrameBorderSize   = 0.0f;
    style.WindowPadding     = ImVec2(8, 8);
    style.FramePadding      = ImVec2(6, 3);
    style.ItemSpacing       = ImVec2(6, 4);
}

static void SetupVSCodeLightTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    colors[ImGuiCol_WindowBg]           = ImVec4(0.95f, 0.95f, 0.95f, 1.00f);
    colors[ImGuiCol_ChildBg]            = ImVec4(0.95f, 0.95f, 0.95f, 1.00f);
    colors[ImGuiCol_PopupBg]            = ImVec4(0.98f, 0.98f, 0.98f, 1.00f);

    colors[ImGuiCol_Border]             = ImVec4(0.75f, 0.75f, 0.75f, 1.00f);
    colors[ImGuiCol_FrameBg]            = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]     = ImVec4(0.85f, 0.85f, 0.85f, 1.00f);
    colors[ImGuiCol_FrameBgActive]      = ImVec4(0.80f, 0.80f, 0.80f, 1.00f);

    colors[ImGuiCol_TitleBg]            = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    colors[ImGuiCol_TitleBgActive]      = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]   = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);

    colors[ImGuiCol_MenuBarBg]          = ImVec4(0.92f, 0.92f, 0.92f, 1.00f);
    colors[ImGuiCol_Button]             = ImVec4(0.85f, 0.85f, 0.85f, 1.00f);
    colors[ImGuiCol_ButtonHovered]      = ImVec4(0.78f, 0.78f, 0.78f, 1.00f);
    colors[ImGuiCol_ButtonActive]       = ImVec4(0.70f, 0.70f, 0.70f, 1.00f);

    colors[ImGuiCol_Header]             = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
    colors[ImGuiCol_HeaderHovered]      = ImVec4(0.75f, 0.75f, 0.75f, 1.00f);
    colors[ImGuiCol_HeaderActive]       = ImVec4(0.70f, 0.70f, 0.70f, 1.00f);

    colors[ImGuiCol_Text]               = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_TextDisabled]       = ImVec4(0.45f, 0.45f, 0.45f, 1.00f);
    colors[ImGuiCol_TextSelectedBg]     = ImVec4(0.40f, 0.60f, 0.90f, 0.45f);

    style.WindowRounding    = 0.0f;
    style.FrameRounding     = 2.0f;
    style.ScrollbarRounding = 2.0f;
    style.GrabRounding      = 2.0f;
    style.TabRounding       = 0.0f;
    style.WindowBorderSize  = 1.0f;
    style.FrameBorderSize   = 0.0f;
    style.WindowPadding     = ImVec2(8, 8);
    style.FramePadding      = ImVec2(6, 3);
    style.ItemSpacing       = ImVec2(6, 4);
}
