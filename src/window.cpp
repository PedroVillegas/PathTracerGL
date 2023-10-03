#include "window.h"

void WindowResize(GLFWwindow* window, int width, int height);
void CursorPosition(GLFWwindow* window, double xPos, double yPos);
void ScrollCallback(GLFWwindow* window, double x_Offset, double y_Offset);

void APIENTRY glDebugOutput(GLenum source, 
                            GLenum type, 
                            unsigned int id, 
                            GLenum severity, 
                            GLsizei length, 
                            const char *message, 
                            const void *userParam)
{
    if(id == 131169 || id == 131185 || id == 131218 || id == 131204) return; // ignore these non-significant error codes

    std::cout << "---------------" << std::endl;
    std::cout << "Debug message (" << id << "): " <<  message << std::endl;

    switch (source)
    {
        case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
        case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
        case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
    } std::cout << std::endl;

    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break; 
        case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
        case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
        case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
        case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
        case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
    } std::cout << std::endl;
    
    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
        case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
        case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
    } std::cout << std::endl;
    std::cout << std::endl;
}

Window::Window(const char* title, uint32_t width, uint32_t height)
    :
    m_Title(title),
    m_Width(width),
    m_Height(height)
{
    if (!Window::Init())
        glfwTerminate();
}

Window::~Window() 
{
    glfwTerminate();
}

bool Window::Init()
{
    if (!glfwInit())
    {
        std::cout << "\e[1;31m[ERROR]\e[1;37m Failed to initialise GLFW" << std::endl;
        return false;
    }

    // Define version and compatibility settings
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6); 
    glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true); // comment this line in a release build! 

    m_Window = glfwCreateWindow(m_Width, m_Height, m_Title, NULL, NULL);
    if (!m_Window)
    {
        std::cout << "\e[1;31m[ERROR]\e[0;37m Failed to create GLFW window" << std::endl;
        return false;
    }

    glfwMakeContextCurrent(m_Window);
    glfwSetFramebufferSizeCallback(m_Window, WindowResize);
    glfwSetCursorPosCallback(m_Window, CursorPosition);

    if (glfwRawMouseMotionSupported())
        glfwSetInputMode(m_Window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
	    std::cout << "\e[1;31m[ERROR]\e[0;37m Failed to load OpenGL extensions" << std::endl;
        return false;
    }

    // enable OpenGL debug context if context allows for debug context
    int flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); // makes sure errors are displayed synchronously
        glDebugMessageCallback(glDebugOutput, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }

    std::cout << "\033[1;33mOpenGL Path Tracing\033[0m" << std::endl;
    std::cout << std::endl;
    std::cout << "\033[1;33mVendor: \033[0m" << (const char*)(glGetString(GL_VENDOR)) << std::endl;
    std::cout << "\033[1;33mRenderer: \033[0m" << (const char*)(glGetString(GL_RENDERER)) << std::endl;
    std::cout << "\033[1;33mOpenGL Version: \033[0m" << (const char*)(glGetString(GL_VERSION)) << std::endl;
    std::cout << std::endl;
    return true;
}

void WindowResize(GLFWwindow* window, int width, int height)
{  
    glViewport(0, 0, width, height); 
}

void CursorPosition(GLFWwindow* window, double xPos, double yPos) {}

void Window::ProcessInput()
{
	if (glfwGetKey(m_Window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(m_Window, true);
}

void Window::Clear() const
{
    glClear(GL_COLOR_BUFFER_BIT);
}

void Window::Update() const
{
    glfwPollEvents();
    glfwSwapBuffers(m_Window);
}

bool Window::Closed() const
{
    return glfwWindowShouldClose(m_Window);
}

void Window::SetWidth(uint32_t width)
{
    m_Width = width;
}

void Window::SetHeight(uint32_t height)
{
    m_Height = height;
}
