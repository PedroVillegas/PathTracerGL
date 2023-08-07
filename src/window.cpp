#include "window.h"

void WindowResize(GLFWwindow* window, int width, int height);
void CursorPosition(GLFWwindow* window, double xPos, double yPos);
void ScrollCallback(GLFWwindow* window, double x_Offset, double y_Offset);

Window::Window(const char* title, uint width, uint height)
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
        std::cout << "\e[1;33m[ERROR]\e[1;37m Failed to initialise GLFW" << std::endl;
        return false;
    }

    // Define version and compatibility settings
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2); 
    glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); 
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

    m_Window = glfwCreateWindow(m_Width, m_Height, m_Title, NULL, NULL);
    if (!m_Window)
    {
        std::cout << "\e[1;33m[ERROR]\e[1;37m Failed to create GLFW window" << std::endl;
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
	    std::cout << "\e[1;33m[ERROR]\e[1;37m Failed to load OpenGL extensions" << std::endl;
        return false;
    }

    std::cout << "\033[1mOpenGL Path Tracing\033[0m" << std::endl;
    std::cout << std::endl;
    std::cout << "\033[1mVendor: \033[0m" << (const char*)(glGetString(GL_VENDOR)) << std::endl;
    std::cout << "\033[1mRenderer: \033[0m" << (const char*)(glGetString(GL_RENDERER)) << std::endl;
    std::cout << "\033[1mOpenGL Version: \033[0m" << (const char*)(glGetString(GL_VERSION)) << std::endl;
    return true;
}

void WindowResize(GLFWwindow* window, int width, int height)
{  
    glViewport(0, 0, width, height); GLCall;
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

void Window::SetWidth(uint width)
{
    m_Width = width;
}

void Window::SetHeight(uint height)
{
    m_Height = height;
}
