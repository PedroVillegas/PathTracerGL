#include "application.h"

Application::Application(std::string title, uint32_t width, uint32_t height)
    : m_Title(title)
    , m_ViewportWidth(width)
    , m_ViewportHeight(height)
    , m_Window(nullptr)
    , m_Scene(nullptr)
    , m_Renderer(nullptr)
    , m_DeltaTime(0.0f)
    , m_BVHDebugIBO(0)
    , m_BVHDebugVAO(0)
    , m_BVHDebugVBO(0)
    , m_QuadIBO(0)
    , m_QuadVAO(0)
    , m_QuadVBO(0)
{   
    m_Window   = std::make_unique<Window>(m_Title.c_str(), m_ViewportWidth, m_ViewportHeight);
    m_Scene    = std::make_unique<Scene>();
    m_Renderer = std::make_unique<Renderer>(m_ViewportWidth, m_ViewportHeight, &(*m_Scene));
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
    Setup();

    // Configure app settings
    m_Settings.tonemap = TONY_MCMAPFACE;
    m_Settings.enableBlueNoise = true;
    m_Settings.enableBVH = false;
    m_Settings.enableCrosshair = false;
    m_Settings.enableDebugBVHVisualisation = false;
    m_Settings.enableVsync = false;
    m_Settings.enableGui = true;

    GetEnvMaps();

    // Initialise ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
    io.ConfigDockingNoSplit = true;
    io.ConfigDockingWithShift = true;
    io.ConfigWindowsMoveFromTitleBarOnly = true;

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(m_Window->GetWindow(), true);
    ImGui_ImplOpenGL3_Init("#version 460");

    // https://gafferongames.com/post/fix_your_timestep/
    constexpr double dt = 1.0 / 144.0;
    double t = 0.0;
    double currentTime = glfwGetTime();
    double accumulator = 0.0;
    while (!m_Window->Closed())
    {
        double newTime = glfwGetTime();
        double frameTime = newTime - currentTime;
        currentTime = newTime;

        accumulator += frameTime;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
        //ImGui::ShowDemoWindow();

        Resize();

        while (accumulator >= dt)
        {
            if (m_Scene->Eye->OnUpdate(dt, &(*m_Window))) 
                m_Renderer->ResetSamples();
            accumulator -= dt;
            t += dt;
        }

        Render();

        m_Window->Update();
        m_DeltaTime = 1.0f / ImGui::GetIO().Framerate; // In seconds
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void Application::Render()
{
    if (!m_Renderer->hasPaused)
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
        m_Scene->Data.Day = (int) m_Scene->day;
        m_Scene->Data.SunColour = m_Scene->sunColour;

        m_Renderer->Render(m_QuadVAO, m_Settings);
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0.0f, 0.0f});
    ImGui::Begin("Viewport", 0, ImGuiWindowFlags_NoTitleBar);

    m_ViewportWidth  = (uint32_t) ImGui::GetContentRegionAvail().x;
    m_ViewportHeight = (uint32_t) ImGui::GetContentRegionAvail().y;

    uint32_t image = m_Renderer->GetViewportFramebuffer().GetTextureID();
    ImGui::Image((void*)(intptr_t)image, {(float)m_ViewportWidth, (float)m_ViewportHeight}, {0, 1}, {1, 0});

    ImGui::End();
    ImGui::PopStyleVar();

    if (m_Settings.enableGui) RenderUI();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }
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

void Application::GetEnvMaps()
{
    // https://stackoverflow.com/a/41305019
    using std::filesystem::directory_iterator;

    for (auto& file : directory_iterator(PATH_TO_HDR))
    {
        if (file.path().extension() == ".hdr")
            m_EnvMaps.push_back(file.path().string());
    }
}

void Application::RenderUI()
{
    
    if (ImGui::Begin("Overview"))
    {
        ImGui::Text("Viewport: %i x %i", m_ViewportWidth, m_ViewportHeight);
        ImGui::Text("Render time: %.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
        ImGui::Text("Framerate: %.1f FPS", ImGui::GetIO().Framerate);
        ImGui::Text("Iterations: %i", m_Renderer->GetIterations());
        ImGui::Checkbox("Pause", &m_Renderer->hasPaused);

        if (ImGui::CollapsingHeader("Application Settings"))
        {
            ImGui::Text("Tonemap");
            if (ImGui::Combo(
                "##Tonemap", &m_Settings.tonemap, 
                "Jodie-Reinhard\0ACES film\0ACES fitted\0Tony McMapface\0AgX Punchy\0"
            ))
                m_Renderer->ResetSamples();

            if (ImGui::Checkbox("Enable V-Sync", &m_Settings.enableVsync))
                m_Settings.enableVsync == true ? glfwSwapInterval(1) : glfwSwapInterval(0);

            if (ImGui::Checkbox("Enable Blue Noise", &m_Settings.enableBlueNoise))
                m_Renderer->ResetSamples();
                
            if (ImGui::Checkbox("Enable Crosshair", &m_Settings.enableCrosshair))
                m_Renderer->ResetSamples();

            if (ImGui::Checkbox("Enable BVH", &m_Settings.enableBVH))
                m_Renderer->ResetSamples();

            if (m_Settings.enableBVH == true)
            {
                if (ImGui::Checkbox("Visualise BVH", &m_Settings.enableDebugBVHVisualisation))
                    m_Renderer->ResetSamples();
                ImGui::Checkbox("Draw BVH", &m_Renderer->shouldDrawBVH);
                ImGui::Text("BVH Depth");
                ImGui::SliderInt("##BVH-Depth", &m_Renderer->BVHDepth, 0, 10);
            }
            else
            {
                m_Settings.enableDebugBVHVisualisation = false;
                m_Renderer->shouldDrawBVH = false;
                m_Renderer->BVHDepth = 0;
            }
        }
    }
    ImGui::End();

    if (ImGui::Begin("Camera"))
    {
        ImGui::PushItemWidth(ImGui::GetWindowContentRegionWidth());
        ImGui::Text("Camera Type");
        if (ImGui::Combo("##CameraType", &m_Scene->Eye->type, "FREE\0CINEMATIC\0")) 
        {
            //m_Scene->Eye->Reset();
            m_Renderer->ResetSamples();
        }

        ImGui::Text("Velocity : %.2f", glm::length(m_Scene->Eye->GetVelocity()));
        ImGui::Text("Position : %.2f %.2f %.2f", m_Scene->Eye->position.x, m_Scene->Eye->position.y , m_Scene->Eye->position.z);
        ImGui::Text("Direction: %.2f %.2f %.2f", m_Scene->Eye->GetDirection().x, m_Scene->Eye->GetDirection().y , m_Scene->Eye->GetDirection().z);
        
        ImGui::Text("Sensitivity");
        ImGui::SliderFloat("##Sensitivity", &m_Scene->Eye->sensitivity, 1.0f, 100.0f);
        ImGui::Text("Slow Walk Speed");
        ImGui::SliderFloat("##SlowWalkingSpeed", &m_Scene->Eye->slowSpeed, 3.0f, 10.0f);
        ImGui::Text("Walking Speed");
        ImGui::SliderFloat("##WalkingSpeed", &m_Scene->Eye->walkingSpeed, 10.0f, 50.0f);
        ImGui::Text("Sprinting Speed");
        ImGui::SliderFloat("##SprintingSpeed", &m_Scene->Eye->sprintingSpeed, 50.0f, 150.0f);
        ImGui::Text("Damping Coefficient");
        ImGui::SliderFloat("##DampingCoeff", &m_Scene->Eye->damping, 1.0f, 5.0f);

        ImGui::Text("Focal Length");
        if (ImGui::InputFloat("##FocalLength", &m_Scene->Eye->focal_length, 0.05f, 1.0f))
        {
            m_Scene->Eye->focal_length = glm::max(m_Scene->Eye->focal_length, 1.0f);
            m_Renderer->ResetSamples();
        }

        ImGui::Text("Aperture Diameter");
        if (ImGui::SliderFloat("##Aperture", &m_Scene->Eye->aperture, 0.0f, 2.0f)) 
            m_Renderer->ResetSamples();

        ImGui::Text("f-stop: f/%0.3f", m_Scene->Eye->focal_length / m_Scene->Eye->aperture);

        ImGui::Text("FOV");
        if (ImGui::SliderFloat("##FOV", &m_Scene->Eye->FOV, 1.0f, 120.0f))
        {
            m_Scene->Eye->SetFov(m_Scene->Eye->FOV);
            m_Scene->Eye->RecalculateProjection();
            m_Renderer->ResetSamples();
        }
        ImGui::PopItemWidth();
    }
    ImGui::End();

    if (ImGui::Begin("Scene"))
    {
        ImGui::PushItemWidth(ImGui::GetWindowContentRegionWidth());
        ImGui::Text("Objects in scene: %llu", m_Scene->primitives.size());

        if (ImGui::Button("Clear Scene"))
        {
            m_Scene->EmptyScene();
            m_Renderer->ResetSamples();
        }

        ImGui::Text("Scene: ");
        if (ImGui::Combo("##SceneSelection", &m_Scene->SceneIdx, "Room with window\0Cornell Box\0White room with coloured lights\0")) 
        {
            m_Scene->SelectScene();
            m_Renderer->m_BVH->RebuildBVH(m_Scene->primitives);
            m_Renderer->ResetSamples();
        }

        if (ImGui::Button("Reset Samples")) 
            m_Renderer->ResetSamples();
        
        if (ImGui::Button("Reload Shader"))
        {
            std::cout << "Reloading Shader..." << std::endl;
            m_Renderer->GetShader().ReloadShader();
            std::cout << "Shader Successfully Reloaded" << std::endl;
            m_Renderer->ResetSamples();
        }

        ImGui::Text("SPP");
        if (ImGui::SliderInt("##SPP", &m_Scene->samplesPerPixel, 1, 10)) 
            m_Renderer->ResetSamples();

        ImGui::Text("Max Ray Depth");
        if (ImGui::SliderInt("##MaxRayDepth", &m_Scene->maxRayDepth, 1, 50)) 
            m_Renderer->ResetSamples();

        // Environment maps
        std::vector<const char*> envMapsList;
        for (int i = 0; i < m_EnvMaps.size(); ++i)
            envMapsList.push_back(m_EnvMaps[i].c_str());

        ImGui::Text("Enviroment Maps");
        if (ImGui::Combo("EnvMaps", &m_Scene->envMapIdx, envMapsList.data(), envMapsList.size()))
        {
            m_Scene->AddEnvMap(m_EnvMaps[m_Scene->envMapIdx]);
            m_Scene->envMapHasChanged = true;
            m_Renderer->ResetSamples();
        }
        
        ImGui::Text("Environment Map Rotation");
        if (ImGui::DragFloat("##EnvMapRotation", &m_Scene->envMapRotation, 0.001f, -1.0f, 1.0f)) 
            m_Renderer->ResetSamples();

        if (ImGui::CollapsingHeader("Edit Lights"))
        {
            if (ImGui::Button("Add Sphere Light"))
            {
                m_Scene->AddDefaultSphere();
                m_Scene->AddLight(m_Scene->primitives.size() - 1, glm::vec3(1.0f));
                m_Renderer->m_BVH->RebuildBVH(m_Scene->primitives);
                m_Renderer->ResetSamples();
            }
            ImGui::SameLine();
            if (ImGui::Button("Add Cube Light"))
            {
                m_Scene->AddDefaultCube();
                m_Scene->AddLight(m_Scene->primitives.size() - 1, glm::vec3(1.0f));
                m_Renderer->m_BVH->RebuildBVH(m_Scene->primitives);
                m_Renderer->ResetSamples();
            }

            if (ImGui::Checkbox("Enable Sun", &m_Scene->day))
            {
                m_Renderer->ResetSamples();
            }

            if (m_Scene->day == 1)
            {
                ImGui::Text("Sun Colour");
                if (ImGui::ColorEdit3("##SunColour", glm::value_ptr(m_Scene->sunColour)))
                    m_Renderer->ResetSamples();

                ImGui::Text("Sun Elevation (deg)");
                if (ImGui::DragFloat("##Elevation", &m_Scene->sunElevation, 0.1f, -90.f, 90.f))
                    m_Renderer->ResetSamples();

                ImGui::Text("Sun Azimuth (deg)");
                if (ImGui::DragFloat("##Azimuth", &m_Scene->sunAzimuth, 0.1f, -360.f, 360.f))
                    m_Renderer->ResetSamples();
            }

            if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_LeftArrow))) 
                m_Scene->LightIdx = int(m_Scene->LightIdx == 0 ? m_Scene->lights.size() - 1 : m_Scene->LightIdx - 1);
            if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_RightArrow))) 
                m_Scene->LightIdx = int(m_Scene->LightIdx == m_Scene->lights.size() - 1 ? 0 : m_Scene->LightIdx + 1);
            
            if (m_Scene->lights.size() > 0)
            {
                Primitive& prim = m_Scene->primitives[m_Scene->lights[m_Scene->LightIdx].id];

                prim.type == 0 ? ImGui::Text("Type: Sphere") : ImGui::Text("Type: AABB");
                ImGui::Text("Primitive Index: %i", m_Scene->lights[m_Scene->LightIdx].id);
                ImGui::Text("Distance from camera: %.3f", glm::distance(prim.position, m_Scene->Eye->position));

                ImGui::Text("Position");
                if (ImGui::DragFloat3("##LightPos", glm::value_ptr(prim.position), 0.1f))
                {
                    m_Renderer->ResetSamples();
                    m_Renderer->m_BVH->RebuildBVH(m_Scene->primitives);
                }

                switch (prim.type)
                {
                    case PRIM_SPHERE: // Sphere
                        ImGui::Text("Radius");
                        if (ImGui::DragFloat("##LightRadius", &prim.radius, 0.05f, 0.1f, 1000.0f)) 
                        {
                            m_Renderer->ResetSamples();
                            m_Renderer->m_BVH->RebuildBVH(m_Scene->primitives);
                        }
                        break;
                    case PRIM_AABB: // AABB
                        ImGui::Text("Dimensions");
                        if (ImGui::DragFloat3("##LightDims", glm::value_ptr(prim.dimensions), 0.1f, 0.1f, 1000.0f)) 
                        {
                            m_Renderer->ResetSamples();
                            m_Renderer->m_BVH->RebuildBVH(m_Scene->primitives);
                        }
                        break;
                }

                ImGui::Text("Emissive");
                if (ImGui::ColorEdit3("##Emissive", glm::value_ptr(prim.mat.emissive)))
                    m_Renderer->ResetSamples();

                ImGui::Text("Intensity");
                if (ImGui::DragFloat("##intensity", &prim.mat.intensity, 0.005f, 0.0f, 100.0f)) 
                    m_Renderer->ResetSamples();
            }
            else
            {
                ImGui::Text("No light sources available.");
            }
        }

        if (ImGui::CollapsingHeader("Edit Object Properties"))
        {
            if (ImGui::Button("Add Sphere"))
            {
                m_Scene->AddDefaultSphere();
                m_Renderer->m_BVH->RebuildBVH(m_Scene->primitives);
                m_Renderer->ResetSamples();
                m_Scene->PrimitiveIdx = m_Scene->primitives.size() - 1;
            }
            ImGui::SameLine();
            if (ImGui::Button("Add Cube"))
            {
                m_Scene->AddDefaultCube();
                m_Renderer->m_BVH->RebuildBVH(m_Scene->primitives);
                m_Renderer->ResetSamples();
                m_Scene->PrimitiveIdx = m_Scene->primitives.size() - 1;
            }

            if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_LeftArrow))) 
                m_Scene->PrimitiveIdx = int(m_Scene->PrimitiveIdx == 0 ? m_Scene->primitives.size() - 1 : m_Scene->PrimitiveIdx - 1);
            if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_RightArrow))) 
                m_Scene->PrimitiveIdx = int(m_Scene->PrimitiveIdx == m_Scene->primitives.size() - 1 ? 0 : m_Scene->PrimitiveIdx + 1);
            
            if (m_Scene->primitives.size() > 0)
            {
                Primitive& prim = m_Scene->primitives[m_Scene->PrimitiveIdx];

                prim.type == 0 ? ImGui::Text("Type: Sphere") : ImGui::Text("Type: AABB");
                ImGui::Text("Primitive Index: %i", m_Scene->PrimitiveIdx);
                ImGui::Text("Distance from camera: %.3f", glm::distance(prim.position, m_Scene->Eye->position));

                ImGui::Text("Primitive Id: %i", prim.id);
                ImGui::Text("Position");
                if (ImGui::DragFloat3("##Position", glm::value_ptr(prim.position), 0.1f))
                {
                    m_Renderer->ResetSamples();
                    m_Renderer->m_BVH->RebuildBVH(m_Scene->primitives);
                }

                ImGui::Text("Rotation");
                if (ImGui::DragFloat3("##Rotation", glm::value_ptr(prim.euler), 0.1f, -360.0f, 360.0f))
                {
                    prim.UpdateRotation();
                    m_Renderer->ResetSamples();
                    m_Renderer->m_BVH->RebuildBVH(m_Scene->primitives);
                }

                switch (prim.type)
                {
                    case PRIM_SPHERE: // Sphere
                        ImGui::Text("Radius");
                        if (ImGui::DragFloat("##Radius", &prim.radius, 0.05f, 0.1f, 1000.0f)) 
                        {
                            m_Renderer->ResetSamples();
                            m_Renderer->m_BVH->RebuildBVH(m_Scene->primitives);
                        }
                        break;
                    case PRIM_AABB: // AABB
                        ImGui::Text("Dimensions");
                        if (ImGui::DragFloat3("##Dimensions", glm::value_ptr(prim.dimensions), 0.1f, 0.1f, 1000.0f)) 
                        {
                            m_Renderer->ResetSamples();
                            m_Renderer->m_BVH->RebuildBVH(m_Scene->primitives);
                        }
                        break;
                }

                ImGui::Text("Albedo");
                if (ImGui::ColorEdit3("##Albedo", glm::value_ptr(prim.mat.albedo))) 
                    m_Renderer->ResetSamples();

                ImGui::Text("Absorption");
                if (ImGui::ColorEdit3("##Absorption", glm::value_ptr(prim.mat.absorption)))
                    m_Renderer->ResetSamples();

                ImGui::Text("Roughness");
                if (ImGui::SliderFloat("##roughness", &prim.mat.roughness, 0.0f, 1.0f)) 
                    m_Renderer->ResetSamples();

                ImGui::Text("Metallic");
                if (ImGui::SliderFloat("##metallic", &prim.mat.metallic, 0.0f, 1.0f)) 
                    m_Renderer->ResetSamples();

                ImGui::Text("Transmission");
                if (ImGui::SliderFloat("##transmission", &prim.mat.transmission, 0.0f, 1.0f))
                    m_Renderer->ResetSamples();

                ImGui::Text("IOR");
                if (ImGui::SliderFloat("##ior", &prim.mat.ior, 1.0f, 2.5f))
                    m_Renderer->ResetSamples();
            }
            else
            {
                ImGui::Text("No objects available.");
            }
        }

        ImGui::PopItemWidth();
    }
    ImGui::End();
}
