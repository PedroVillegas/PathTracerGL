#include "window.h"

void WindowResize(GLFWwindow* window, int width, int height);

Window::Window(const char* title, unsigned int width, unsigned int height)
{
    m_Title = title;
    m_Width = width;
    m_Height = height;
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
    // glfwSwapInterval(0);
    glfwSetFramebufferSizeCallback(m_Window, WindowResize);

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
	    std::cout << "\e[1;33m[ERROR]\e[1;37m Failed to load OpenGL extensions" << std::endl;
        return false;
    }

    std::cout << glGetString(GL_VERSION) << std::endl;

    return true;
}

void WindowResize(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void Window::ProcessInput() const
{
	if(glfwGetKey(m_Window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(m_Window, true);
	}
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
