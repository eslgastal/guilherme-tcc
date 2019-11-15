#ifndef CLASS_ADD_HEADERS
#define CLASS_ADD_HEADERS
#include "headers.h"
#endif

#ifndef CLASS_HEADER_INITIALIZE_GLOBALS
#define CLASS_HEADER_INITIALIZE_GLOBALS
#include "globals.h"
#endif

// Play it nice with Windows users. Notepad in 2017 still doesn't display text data with Unix-style \n.
#ifdef _WIN32
#define IM_NEWLINE  "\r\n"
#else
#define IM_NEWLINE  "\n"
#endif

#ifndef CLASS_INTERFACE_CLASS_HEADER
#define CLASS_INTERFACE_CLASS_HEADER
class Interface {
  private:
    bool m_show_demo_window;
    bool m_show_app_metrics;
    bool m_show_app_style_editor;
    bool m_show_app_about;
    ImGui::FileBrowser fileDialog;

    void Start();
    void SetInterface(bool show_demo_window);
    void ShowExampleMenuFile();
  public:
    Interface(bool show_demo_window);
    void Init(GLFWwindow *window, const char* glsl_version);
    void Show(GLFWwindow *window);
    void LoadFonts();
    void CleanUp();
};
#endif