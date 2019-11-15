#include "interface.h"

Interface::Interface(bool show_demo_window) {
  SetInterface(show_demo_window);

  fileDialog.SetTitle("title");
  fileDialog.SetTypeFilters({ ".h", ".cpp" });
}

void Interface::Init(GLFWwindow *window, const char* glsl_version) {
  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  Globals::g_Io = &ImGui::GetIO(); (void)Globals::g_Io;
  //Globals::g_Io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
  //Globals::g_Io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  //ImGui::StyleColorsClassic();

  // Setup Platform/Renderer bindings
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);

  // LoadFonts();
}

void Interface::Show(GLFWwindow *window) {
  IM_ASSERT(ImGui::GetCurrentContext() != NULL && "Missing dear imgui context. Init() didn't run!");

  Start();

  if (m_show_app_metrics)             { ImGui::ShowMetricsWindow(&m_show_app_metrics); }
  if (m_show_app_style_editor)        { ImGui::Begin("Style Editor", &m_show_app_style_editor); ImGui::ShowStyleEditor(); ImGui::End(); }
  if (m_show_app_about)               { ImGui::ShowAboutWindow(&m_show_app_about); }

  //1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
  if (m_show_demo_window)
    ImGui::ShowDemoWindow(&m_show_demo_window);

  // Usamos um par Begin/End para criar uma nova janela nomeada.
  {
    ImGui::Begin("Settings", NULL, ImGuiWindowFlags_MenuBar);

    // Menu Bar
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("Menu"))
        {
            ShowExampleMenuFile();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Tools"))
        {
            ImGui::MenuItem("Metrics", NULL, &m_show_app_metrics);
            ImGui::MenuItem("Style Editor", NULL, &m_show_app_style_editor);
            ImGui::MenuItem("About Dear ImGui", NULL, &m_show_app_about);
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
    ImGui::Text("Imgui Version (%s)", IMGUI_VERSION);

      ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
    if (ImGui::BeginTabBar("MyTabBar", ImGuiTabBarFlags_None))
    {
      if (ImGui::BeginTabItem("Display"))
      {
        // Edit 3 floats representing a color
        ImGui::ColorEdit3("clear color", (float*)&g_ClearColor);

        ImGui::Checkbox("Perspective Projection", &g_UsePerspectiveProjection);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem("Models"))
      {
        ImGui::SliderFloat("Angle Z", &g_AngleZ, -10.0f, 10.0f);
        ImGui::SliderFloat("Angle Y", &g_AngleY, -10.0f, 10.0f);
        ImGui::SliderFloat("Angle X", &g_AngleX, -10.0f, 10.0f);
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem("Frustum"))
      {
        ImGui::SliderFloat("Near Plane", &g_FrustumNearPlane, -10.0f, 10.0f);
        ImGui::SliderFloat("Far Plane", &g_FrustumFarPlane, -10.0f, 10.0f);
        ImGui::EndTabItem();
      }
      ImGui::EndTabBar();
    }
  ImGui::End();
  }

  fileDialog.Display();
    if(fileDialog.HasSelected())
    {
        // std::cout << "Selected filename" << fileDialog.GetSelected().string() << std::endl;
        fileDialog.ClearSelected();
    }

  // Rendering
  ImGui::Render();

  int display_w, display_h;
  glfwGetFramebufferSize(window, &display_w, &display_h);
  glViewport(0, 0, display_w, display_h);

  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

 void Interface::LoadFonts() {
    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'misc/fonts/README.txt' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //Globals::g_Io.Fonts->AddFontDefault();
    //Globals::g_Io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //Globals::g_Io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //Globals::g_Io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //Globals::g_Io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    //ImFont* font = Globals::g_Io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, Globals::g_Io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);
  }

void Interface::CleanUp() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

void Interface::Start(){
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}

void Interface::SetInterface(bool show_demo_window){
  m_show_demo_window = show_demo_window;
  m_show_app_metrics = false;
  m_show_app_style_editor = false;
  m_show_app_about = false;
}


// Note that shortcuts are currently provided for display only (future version will add flags to BeginMenu to process shortcuts)
void Interface::ShowExampleMenuFile()
{
    if (ImGui::MenuItem("New")) {}
    if (ImGui::MenuItem("Open", "Ctrl+O")) {
        fileDialog.Open();
    }
    if (ImGui::MenuItem("Save", "Ctrl+S")) {}
    if (ImGui::MenuItem("Save As..")) {}
    ImGui::Separator();
    if (ImGui::BeginMenu("Options"))
    {
        static bool enabled = true;
        ImGui::MenuItem("Enabled", "", &enabled);
        ImGui::BeginChild("child", ImVec2(0, 60), true);
        for (int i = 0; i < 10; i++)
            ImGui::Text("Scrolling Text %d", i);
        ImGui::EndChild();
        static float f = 0.5f;
        static int n = 0;
        static bool b = true;
        ImGui::SliderFloat("Value", &f, 0.0f, 1.0f);
        ImGui::InputFloat("Input", &f, 0.1f);
        ImGui::Combo("Combo", &n, "Yes\0No\0Maybe\0\0");
        ImGui::Checkbox("Check", &b);
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Colors"))
    {
        float sz = ImGui::GetTextLineHeight();
        for (int i = 0; i < ImGuiCol_COUNT; i++)
        {
            const char* name = ImGui::GetStyleColorName((ImGuiCol)i);
            ImVec2 p = ImGui::GetCursorScreenPos();
            ImGui::GetWindowDrawList()->AddRectFilled(p, ImVec2(p.x+sz, p.y+sz), ImGui::GetColorU32((ImGuiCol)i));
            ImGui::Dummy(ImVec2(sz, sz));
            ImGui::SameLine();
            ImGui::MenuItem(name);
        }
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Disabled", false)) // Disabled
    {
        IM_ASSERT(0);
    }
    if (ImGui::MenuItem("Checked", NULL, true)) {}
    if (ImGui::MenuItem("Quit", "Alt+F4")) {}
}
