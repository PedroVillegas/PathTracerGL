#include "application.h"

Application::Application(std::string title, uint32_t width, uint32_t height)
    : m_Title(title)
    , m_ViewportWidth(width)
    , m_ViewportHeight(height)
{   
}

Application::~Application()
{
    glDeleteVertexArrays(1, &m_QuadVAO); 
    glDeleteBuffers(1, &m_QuadVBO); 
    glDeleteBuffers(1, &m_QuadIBO);

    glDeleteVertexArrays(1, &m_BVHDebugVAO); 
    glDeleteBuffers(1, &m_BVHDebugVBO); 
    glDeleteBuffers(1, &m_BVHDebugIBO);
}

void Application::Run()
{
    m_Window    = std::make_unique<Window>(m_Title.c_str(), m_ViewportWidth, m_ViewportHeight);
    m_Gui       = std::make_unique<Gui>(*m_Window);
    m_Scene     = std::make_unique<Scene>();
    m_Renderer  = std::make_unique<Renderer>(m_ViewportWidth, m_ViewportHeight, &(*m_Scene));
    Setup();

    while (!m_Window->Closed())
    {
        // Input
        //m_Window->ProcessInput();
        /*if (glfwGetKey(m_Window->GetWindow(), GLFW_KEY_G) == GLFW_PRESS)
            m_Settings.enableGui = !m_Settings.enableGui ? true : false;*/

        m_Settings.enableVsync == true ? glfwSwapInterval(1) : glfwSwapInterval(0);

        m_Gui->NewFrame();

        Resize();

        if (m_Scene->Eye->OnUpdate(m_DeltaTime, &(*m_Window))) 
            m_Renderer->ResetSamples();

        Render();

        m_Window->Update();
        m_DeltaTime = 1.0f / ImGui::GetIO().Framerate; // In seconds
    }
}

void Application::Render()
{
    if (!m_Renderer->b_Pause)
    {
        using namespace glm;
        float phi = radians(m_Scene->sunElevation);
        float theta = radians(m_Scene->sunAzimuth);
        float x = sin(theta) * cos(phi);
        float y = sin(phi);
        float z = cos(theta) * cos(phi);
        m_Scene->Data.SunDirection = vec3(x,y,z);
        m_Scene->Data.Depth = m_Scene->maxRayDepth;
        m_Scene->Data.SelectedPrimIdx = m_Scene->PrimitiveIdx;
        m_Scene->Data.Day = m_Scene->day;
        m_Scene->Data.SunColour = m_Scene->sunColour;

        m_Renderer->Render(m_QuadVAO, m_Settings);
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0.0f, 0.0f});
    ImGui::Begin("Viewport", 0, ImGuiWindowFlags_NoTitleBar);

    m_ViewportWidth = (uint32_t) ImGui::GetContentRegionAvail().x;
    m_ViewportHeight = (uint32_t) ImGui::GetContentRegionAvail().y;

    uint32_t image = m_Renderer->GetViewportFramebuffer().GetTextureID();
    ImGui::Image((void*)(intptr_t)image, {(float)m_ViewportWidth, (float)m_ViewportHeight}, {0, 1}, {1, 0});

    ImGui::End();
    ImGui::PopStyleVar();

    m_Gui->Render(*m_Renderer, *m_Scene, m_Settings);
}

void Application::Setup()
{
    // Unit Cube centered about the origin
    std::vector<uint32_t> indices {
        0, 1, 2, 3,
        4, 5, 6, 7,
        0, 4, 1, 5, 
        2, 6, 3, 7
    };

    std::vector<float> vertices {
        -0.5f, -0.5f, -0.5f,
        +0.5f, -0.5f, -0.5f,
        +0.5f, +0.5f, -0.5f,
        -0.5f, +0.5f, -0.5f,
        -0.5f, -0.5f, +0.5f,
        +0.5f, -0.5f, +0.5f,
        +0.5f, +0.5f, +0.5f,
        -0.5f, +0.5f, +0.5f
    };

    GenerateAndCreateVAO(vertices, indices, m_BVHDebugVAO, m_BVHDebugVBO, m_BVHDebugIBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0); 
    m_Renderer->debugVAO = m_BVHDebugVAO;

    // Create Quad as main render target
    std::vector<float> vertices_1 {
        // pos                 // col
        -1.0f, -1.0f, 0.0f,    0.25f, 0.52f, 0.96f,
         1.0f, -1.0f, 0.0f,    0.86f, 0.27f, 0.22f,
         1.0f,  1.0f, 0.0f,    0.96f, 0.71f, 0.00f,
        -1.0f,  1.0f, 0.0f,    0.06f, 0.62f, 0.35f
    };

    std::vector<uint32_t> indices_1 {
        0, 1, 2,
        2, 3, 0
    };

    GenerateAndCreateVAO(vertices_1, indices_1, m_QuadVAO, m_QuadVBO, m_QuadIBO);
    // position attrib
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0); 
    glEnableVertexAttribArray(0); 
    // colour attrib  
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float))); 
    glEnableVertexAttribArray(1); 
}

void Application::Resize()
{
    m_Renderer->OnResize(m_ViewportWidth, m_ViewportHeight);
    m_Scene->Eye->OnResize(m_ViewportWidth, m_ViewportHeight);
}   
