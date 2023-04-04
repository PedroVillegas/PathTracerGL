#include "gui.h"

Gui::Gui(Window& window)
    : m_Window(window)
{
    Init();
}

Gui::~Gui() 
{
    Shutdown();
}

void Gui::NewFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
}

void Gui::Render(Renderer& renderer, Camera& camera, Scene& scene, bool& vsync)
{
    if (ImGui::Begin("Overview"))
    {
        ImGui::Text("Viewport: %i x %i", renderer.GetViewportWidth(), renderer.GetViewportHeight());
        ImGui::Checkbox("V-Sync", &vsync);
        ImGui::Text("Render time: %.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
        ImGui::Text("Framerate: %.1f FPS", ImGui::GetIO().Framerate);
        ImGui::Text("Iterations: %i", renderer.GetIterations());
        ImGui::End();
    }

    // TODO: Make renderer shared_ptr
    Gui::CreateCameraWindow(renderer, camera);
    Gui::CreateSceneWindow(renderer, camera, scene);

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

void Gui::CreateCameraWindow(Renderer& renderer, Camera& camera)
{
    if (ImGui::Begin("Camera"))
    {
        ImGui::PushItemWidth(ImGui::GetWindowContentRegionWidth());
        ImGui::Text("Camera Type");
        if (ImGui::Combo("##combo", &camera.type, "FREE\0CINEMATIC\0")) 
        {
            camera.Reset();
            renderer.ResetSamples();
        }

        ImGui::Text("Position : %.2f %.2f %.2f", camera.GetPosition().x, camera.GetPosition().y , camera.GetPosition().z);
        ImGui::Text("Direction: %.2f %.2f %.2f", camera.GetDirection().x, camera.GetDirection().y , camera.GetDirection().z);
        // ImGui::Text("Momentum : %.2f %.2f %.2f", camera.GetMovementMomentum().x, camera.GetMovementMomentum().y, camera.GetMovementMomentum().z);
        // ImGui::Text("Rotation : %.2f %.2f %.2f", camera.GetRotationMomentum().x, camera.GetRotationMomentum().y, camera.GetRotationMomentum().z);
       
        ImGui::Text("Sensitivity");
        ImGui::SliderFloat("##Sensitivity", &camera.sensitivity, 1.0f, 100.0f);
        ImGui::Text("Damping Factor");
        ImGui::SliderFloat("##DampingFactor", &camera.damping, 0.0f, 0.95f);

        ImGui::Text("Focal Length");
        if (ImGui::InputFloat("##FocalLength", &camera.focal_length, 0.05f, 1.0f)) 
            renderer.ResetSamples();

        ImGui::Text("Aperture Diameter");
        if (ImGui::SliderFloat("##Aperture", &camera.aperture, 0.0f, 2.0f)) 
            renderer.ResetSamples();

        ImGui::Text("f-number: f/%0.5f", camera.focal_length / camera.aperture);

        ImGui::Text("FOV");
        if (ImGui::SliderInt("##FOV", &camera.horizontalFOV, 45, 120))
        {
            camera.SetFov(camera.horizontalFOV);
            camera.RecalculateProjection();
            renderer.ResetSamples();
        }
        ImGui::PopItemWidth();
        ImGui::End();
    }
}

void Gui::CreateSceneWindow(Renderer& renderer, const Camera& camera, Scene& scene)
{
    if (ImGui::Begin("Scene"))
    {
        ImGui::PushItemWidth(ImGui::GetWindowContentRegionWidth());
        ImGui::Text("Objects in scene: %lu", scene.spheres.size() + scene.aabbs.size());

        if (ImGui::Button("Day"))
        {
            scene.day = 1;
            renderer.ResetSamples();
        }
        ImGui::SameLine();
        if (ImGui::Button("Night"))
        {
            scene.day = 0;
            renderer.ResetSamples();
        }

        if (ImGui::Button("Load Custom Scene 1"))
        {
            scene.emptyScene();
            scene.CustomScene();
            renderer.ResetSamples();
        }

        if (ImGui::Button("Load RTIOW Scene"))
        {
            scene.emptyScene();
            scene.RTIW();
            renderer.ResetSamples();
        }

        if (ImGui::Button("Load Randomized Scene"))
        {
            scene.emptyScene();
            scene.RandomizeBRDF();
            renderer.ResetSamples();
        }

        if (ImGui::Button("Load Grid Showcase Scene"))
        {
            scene.emptyScene();
            scene.GridShowcase();
            renderer.ResetSamples();
        }
        
        if (ImGui::Button("Load Cornell Box Scene"))
        {
            scene.emptyScene();
            scene.CornellBox();
            renderer.ResetSamples();
        }

        if (ImGui::Button("Load Modified Cornell Box Scene"))
        {
            scene.emptyScene();
            scene.ModifiedCornellBox();
            renderer.ResetSamples();
        }

        if (ImGui::Button("Reset Samples")) 
            renderer.ResetSamples();

        ImGui::Text("SPP");
        if (ImGui::SliderInt("##SPP", &scene.samplesPerPixel, 1, 10)) 
            renderer.ResetSamples();

        ImGui::Text("Max Ray Depth");
        if (ImGui::SliderInt("##MaxRayDepth", &scene.maxRayDepth, 1, 50)) 
            renderer.ResetSamples();

        ImGui::Text("Directional Light");
        if (ImGui::SliderFloat3("##DirectionalLight", glm::value_ptr(scene.lightDirection), -1.0f, 1.0f))
            renderer.ResetSamples();

        if (ImGui::CollapsingHeader("Edit Spheres"))
        {

            // ---------------------------------------------------------------
            //                            EDIT SPHERES
            // ---------------------------------------------------------------
            const char* items[scene.spheres.size()];
            for (int i = 0; i < scene.spheres.size(); i++)
            {
                items[i] = scene.spheres[i].label.c_str();
            }

            static const char* current_item = items[scene.SphereIdx];
            if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_LeftArrow))) 
                scene.SphereIdx = scene.SphereIdx == 0 ? scene.spheres.size() - 1 : scene.SphereIdx - 1;
            if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_RightArrow))) 
                scene.SphereIdx = scene.SphereIdx == scene.spheres.size() - 1 ? 0 : scene.SphereIdx + 1;
                
            ImGui::Text("Selected Sphere Idx: %i", scene.SphereIdx);
            ImGui::Text("Select Sphere");
            if (ImGui::BeginCombo("##Object", current_item)) // The second parameter is the label previewed before opening the combo.
            {
                for (int n = 0; n < IM_ARRAYSIZE(items); n++)
                {
                    bool is_selected = (current_item == items[n]); // You can store your selection however you want, outside or inside your objects
                    if (ImGui::Selectable(items[n], is_selected))
                    {
                        current_item = items[n];
                        scene.SphereIdx = n;
                    }
                    if (is_selected)
                        ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
                }
                ImGui::EndCombo();
            }
            GPUSphere& obj = scene.spheres[scene.SphereIdx].sphere;
            EditObjectProperties(obj, renderer, camera);
        }

        if (ImGui::CollapsingHeader("Edit AABBs"))
        {

            // ---------------------------------------------------------------
            //                            EDIT AABBs
            // ---------------------------------------------------------------
            const char* items[scene.aabbs.size()];
            for (int i = 0; i < scene.aabbs.size(); i++)
            {
                // items[i] = scene.aabbs[i].label.c_str();
                // std::string idx = std::to_string(i);
                std::string aa = "AABB ";
                items[i] = (aa + std::to_string(i)).c_str();
            }

            static const char* current_item = items[scene.AABBIdx];
            if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_LeftArrow))) 
                scene.AABBIdx = scene.AABBIdx == 0 ? scene.aabbs.size() - 1 : scene.AABBIdx - 1;
            if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_RightArrow))) 
                scene.AABBIdx = scene.AABBIdx == scene.aabbs.size() - 1 ? 0 : scene.AABBIdx + 1;

            ImGui::Text("Selected AABB Idx: %i", scene.AABBIdx);
            ImGui::Text("Select AABB");
            if (ImGui::BeginCombo("##Object", current_item)) // The second parameter is the label previewed before opening the combo.
            {
                for (int n = 0; n < IM_ARRAYSIZE(items); n++)
                {
                    bool is_selected = (current_item == items[n]); // You can store your selection however you want, outside or inside your objects
                    if (ImGui::Selectable(items[n], is_selected))
                    {
                        current_item = items[n];
                        scene.AABBIdx = n;
                    }
                    if (is_selected)
                        ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
                }
                ImGui::EndCombo();
            }
            GPUAABB& obj = scene.aabbs[scene.AABBIdx];
            EditObjectProperties(obj, renderer, camera);
        }

        ImGui::PopItemWidth();
        ImGui::End();
    }
}

template <typename T> 
void Gui::EditObjectProperties(T& obj, Renderer& renderer, const Camera& camera)
{
    ImGui::Text("Distance from camera: %.3f", glm::distance(obj.position, camera.GetPosition()));

    ImGui::Text("Position");
    if (ImGui::DragFloat3("##Position", glm::value_ptr(obj.position), 0.1f)) 
        renderer.ResetSamples();
    
    if constexpr (std::is_same_v<T, GPUAABB>)
    {
        ImGui::Text("Dimensions");
        if (ImGui::DragFloat3("##Dimensions", glm::value_ptr(obj.dimensions), 0.1f)) 
            renderer.ResetSamples();
    }

    if constexpr (std::is_same_v<T, GPUSphere>)
    {
        ImGui::Text("Radius");
        if (ImGui::DragFloat("##Radius", &obj.radius, 0.1f, 0.1f, 100.0f, "%.3f", ImGuiSliderFlags_Logarithmic)) 
            renderer.ResetSamples();
    }

    ImGui::Text("Albedo");
    if (ImGui::ColorEdit3("##Albedo", glm::value_ptr(obj.mat.albedo))) 
        renderer.ResetSamples();

    ImGui::Text("Absorption");
    if (ImGui::DragFloat3("##Absorption", glm::value_ptr(obj.mat.absorption), 0.1f, 0.0f, 10.0f)) 
        renderer.ResetSamples();

    ImGui::Text("Roughness");
    if (ImGui::SliderFloat("##roughness", &obj.mat.roughness, 0.0f, 1.0f)) 
        renderer.ResetSamples();

    ImGui::Text("Metallic");
    if (ImGui::SliderFloat("##metallic", &obj.mat.metallic, 0.0f, 1.0f)) 
        renderer.ResetSamples();

    ImGui::Text("Specular Chance");
    if (ImGui::SliderFloat("##SpecularChance", &obj.mat.specularChance, 0.0f, 1.0f)) 
        renderer.ResetSamples();

    ImGui::Text("IOR");
    if (ImGui::SliderFloat("##IOR", &obj.mat.ior, 1.0f, 2.5f)) 
        renderer.ResetSamples();

    ImGui::Text("Refraction Chance");
    if (ImGui::SliderFloat("##RefractionChance", &obj.mat.refractionChance, 0.0f, 1.0f)) 
        renderer.ResetSamples();

    ImGui::Text("Emissive");
    if (ImGui::ColorEdit3("##Emissive", glm::value_ptr(obj.mat.emissive))) 
        renderer.ResetSamples();

    ImGui::Text("Emissive Strength");
    if (ImGui::DragFloat("##EmissiveStrength", &obj.mat.emissiveStrength, 0.005f, 0.0f, 100.0f)) 
        renderer.ResetSamples();
}

void Gui::Init()
{
    GLFWwindow* window = m_Window.GetWindow();

    // Initialise ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
    io.ConfigWindowsMoveFromTitleBarOnly = true;

    ImGui::StyleColorsDark();
    //SetupStyle();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 410");
}

void Gui::Shutdown()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void Gui::SetupStyle()
{
    // https://github.com/ocornut/imgui/issues/707#issuecomment-252413954

    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 5.3f;
    style.FrameRounding = 2.3f;
    style.ScrollbarRounding = 0;

    style.Colors[ImGuiCol_Text]                  = ImVec4(0.90f, 0.90f, 0.90f, 0.90f);
    style.Colors[ImGuiCol_TextDisabled]          = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
    style.Colors[ImGuiCol_WindowBg]              = ImVec4(0.09f, 0.09f, 0.15f, 0.10f);
    style.Colors[ImGuiCol_ChildBg]               = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImGuiCol_PopupBg]               = ImVec4(0.05f, 0.05f, 0.10f, 0.85f);
    style.Colors[ImGuiCol_Border]                = ImVec4(0.70f, 0.70f, 0.70f, 0.65f);
    style.Colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImGuiCol_FrameBg]               = ImVec4(0.00f, 0.00f, 0.01f, 1.00f);
    style.Colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.90f, 0.80f, 0.80f, 0.40f);
    style.Colors[ImGuiCol_FrameBgActive]         = ImVec4(0.90f, 0.65f, 0.65f, 0.45f);
    style.Colors[ImGuiCol_TitleBg]               = ImVec4(0.00f, 0.00f, 0.00f, 0.83f);
    style.Colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.40f, 0.40f, 0.80f, 0.20f);
    style.Colors[ImGuiCol_TitleBgActive]         = ImVec4(0.00f, 0.00f, 0.00f, 0.87f);
    style.Colors[ImGuiCol_MenuBarBg]             = ImVec4(0.01f, 0.01f, 0.02f, 0.80f);
    style.Colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.20f, 0.25f, 0.30f, 0.60f);
    style.Colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.55f, 0.53f, 0.55f, 0.51f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.56f, 0.56f, 0.56f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.56f, 0.56f, 0.56f, 0.91f);
    style.Colors[ImGuiCol_PopupBg]               = ImVec4(0.10f, 0.10f, 0.10f, 0.99f);
    style.Colors[ImGuiCol_CheckMark]             = ImVec4(0.90f, 0.90f, 0.90f, 0.83f);
    style.Colors[ImGuiCol_SliderGrab]            = ImVec4(0.70f, 0.70f, 0.70f, 0.62f);
    style.Colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.30f, 0.30f, 0.30f, 0.84f);
    style.Colors[ImGuiCol_Button]                = ImVec4(0.48f, 0.72f, 0.89f, 0.49f);
    style.Colors[ImGuiCol_ButtonHovered]         = ImVec4(0.50f, 0.69f, 0.99f, 0.68f);
    style.Colors[ImGuiCol_ButtonActive]          = ImVec4(0.80f, 0.50f, 0.50f, 1.00f);
    style.Colors[ImGuiCol_Header]                = ImVec4(0.30f, 0.69f, 1.00f, 0.53f);
    style.Colors[ImGuiCol_HeaderHovered]         = ImVec4(0.44f, 0.61f, 0.86f, 1.00f);
    style.Colors[ImGuiCol_HeaderActive]          = ImVec4(0.38f, 0.62f, 0.83f, 1.00f);
    style.Colors[ImGuiCol_Separator]             = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    style.Colors[ImGuiCol_SeparatorHovered]      = ImVec4(0.70f, 0.60f, 0.60f, 1.00f);
    style.Colors[ImGuiCol_SeparatorActive]       = ImVec4(0.90f, 0.70f, 0.70f, 1.00f);
    style.Colors[ImGuiCol_ResizeGrip]            = ImVec4(1.00f, 1.00f, 1.00f, 0.85f);
    style.Colors[ImGuiCol_ResizeGripHovered]     = ImVec4(1.00f, 1.00f, 1.00f, 0.60f);
    style.Colors[ImGuiCol_ResizeGripActive]      = ImVec4(1.00f, 1.00f, 1.00f, 0.90f);
    style.Colors[ImGuiCol_PlotLines]             = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    style.Colors[ImGuiCol_PlotLinesHovered]      = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogram]         = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_TextSelectedBg]        = ImVec4(0.00f, 0.00f, 1.00f, 0.35f);
    style.Colors[ImGuiCol_ModalWindowDimBg]      = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);

    
    ImGui::GetStyle().FrameRounding = 4.0f;
    ImGui::GetStyle().GrabRounding = 4.0f;
    
    // ImVec4* colors = style.Colors;
    // colors[ImGuiCol_Text] = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
    // colors[ImGuiCol_TextDisabled] = ImVec4(0.36f, 0.42f, 0.47f, 1.00f);
    // colors[ImGuiCol_WindowBg] = ImVec4(0.11f, 0.15f, 0.17f, 0.37f);
    // colors[ImGuiCol_ChildBg] = ImVec4(0.15f, 0.18f, 0.22f, 0.16f);
    // colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.25f, 0.29f, 0.57f);
    // colors[ImGuiCol_FrameBgHovered] = ImVec4(0.12f, 0.20f, 0.28f, 1.00f);
    // colors[ImGuiCol_FrameBgActive] = ImVec4(0.09f, 0.12f, 0.14f, 1.00f);
    // colors[ImGuiCol_TitleBgActive] = ImVec4(0.21f, 0.27f, 0.31f, 1.00f);
    // colors[ImGuiCol_CheckMark] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
    // colors[ImGuiCol_SliderGrab] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
    // colors[ImGuiCol_SliderGrabActive] = ImVec4(0.55f, 0.73f, 1.00f, 1.00f);
    // colors[ImGuiCol_Button] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
    // colors[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
    // colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.39f);
    // colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.18f, 0.22f, 0.25f, 1.00f);
    // colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.09f, 0.21f, 0.31f, 1.00f);
    // colors[ImGuiCol_Tab] = ImVec4(0.07f, 0.10f, 0.15f, 0.00f);
    // colors[ImGuiCol_TabActive] = ImVec4(0.19f, 0.41f, 0.78f, 1.00f);
    // colors[ImGuiCol_TabUnfocused] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    // colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.20f, 0.25f, 0.29f, 0.80f);
    // colors[ImGuiCol_Header] = ImVec4(0.20f, 0.25f, 0.29f, 0.55f);
    // colors[ImGuiCol_Border] = ImVec4(0.08f, 0.10f, 0.12f, 1.00f);
    // colors[ImGuiCol_TitleBg] = ImVec4(0.09f, 0.12f, 0.14f, 0.65f);
    // colors[ImGuiCol_TitleBgActive] = ImVec4(0.08f, 0.10f, 0.12f, 1.00f);
    // colors[ImGuiCol_MenuBarBg] = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
    // colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
    // colors[ImGuiCol_Header] = ImVec4(0.20f, 0.25f, 0.29f, 0.55f);
    // colors[ImGuiCol_Separator] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
    

    /*
    ImGuiStyle& style = ImGui::GetStyle();
    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 10.0f;
        style.Colors[ImGuiCol_WindowBg].w = 0.2f;
    }
    */

    style.ItemSpacing = ImVec2(12, 8);
    style.ItemInnerSpacing = ImVec2(8, 6);
    style.IndentSpacing = 25.0f;
    style.ScrollbarSize = 15.0f;
    //style.ScrollbarRounding = 9.0f;
    style.GrabMinSize = 5.0f;
    style.GrabRounding = 3.0f;
}
