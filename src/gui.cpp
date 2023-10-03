#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include <glm/gtc/type_ptr.hpp>

#include "window.h"
#include "renderer.h"
#include "camera.h"
#include "scene.h"
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
    // ImGui::ShowDemoWindow();

}

void Gui::Render(Renderer& renderer, Camera& camera, Scene& scene, bool& vsync)
{
    if (ImGui::Begin("Overview"))
    {
        ImGui::Text("Viewport: %i x %i", renderer.GetViewportWidth(), renderer.GetViewportHeight());
        // int res[2] = { int(renderer.GetViewportWidth()), int(renderer.GetViewportHeight()) };
        // if (ImGui::InputInt2("Resolution: ", res))
        //     renderer.OnResize(res[0], res[1]);
        ImGui::Text("Render time: %.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
        ImGui::Text("Framerate: %.1f FPS", ImGui::GetIO().Framerate);
        ImGui::Text("Iterations: %i", renderer.GetIterations());
        ImGui::Checkbox("V-Sync", &vsync);
        ImGui::Checkbox("Pause", &renderer.b_Pause);
        ImGui::Checkbox("Draw BVH", &renderer.b_DrawBVH);
        ImGui::Text("BVH Depth");
        ImGui::SliderInt("##BVH-Depth", &renderer.BVHDepth, 0, 10);
        ImGui::End();
    }

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

        ImGui::Text("Velocity : %.2f", glm::length(camera.GetVelocity()));
        ImGui::Text("Position : %.2f %.2f %.2f", camera.GetPosition().x, camera.GetPosition().y , camera.GetPosition().z);
        ImGui::Text("Direction: %.2f %.2f %.2f", camera.GetDirection().x, camera.GetDirection().y , camera.GetDirection().z);
        // ImGui::Text("Momentum : %.2f %.2f %.2f", camera.GetMovementMomentum().x, camera.GetMovementMomentum().y, camera.GetMovementMomentum().z);
        // ImGui::Text("Rotation : %.2f %.2f %.2f", camera.GetRotationMomentum().x, camera.GetRotationMomentum().y, camera.GetRotationMomentum().z);
       
        ImGui::Text("Sensitivity");
        ImGui::SliderFloat("##Sensitivity", &camera.sensitivity, 1.0f, 100.0f);
        ImGui::Text("Max Velocity");
        ImGui::SliderFloat("##Max Velocity", &camera.MaxVelocity, 1.0f, 50.0f);
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
        if (ImGui::SliderFloat("##FOV", &camera.horizontalFOV, 10.0f, 120.0f))
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
        ImGui::Text("Objects in scene: %llu", scene.primitives.size());

        if (ImGui::Button("Add Sphere"))
        {
            scene.AddDefaultSphere();
            renderer.m_BVH->RebuildBVH(scene.primitives);
            renderer.ResetSamples();
        }
        ImGui::SameLine();
        if (ImGui::Button("Add Cube"))
        {
            scene.AddDefaultCube();
            renderer.m_BVH->RebuildBVH(scene.primitives);
            renderer.ResetSamples();
        }

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

        if (ImGui::Button("Load RTIOW Scene"))
        {
            scene.EmptyScene();
            scene.RTIW();
            renderer.m_BVH->RebuildBVH(scene.primitives);
            renderer.ResetSamples();
        }
        
        if (ImGui::Button("Load Cornell Box Scene"))
        {
            scene.EmptyScene();
            scene.CornellBox();
            renderer.m_BVH->RebuildBVH(scene.primitives);
            renderer.ResetSamples();
        }

        if (ImGui::Button("Reset Samples")) 
            renderer.ResetSamples();
        
        if (ImGui::Button("Reload Shader"))
        {

            std::cout << "Reloading Shader..." << std::endl;
            renderer.GetShader()->ReloadShader();
            std::cout << "Shader Successfully Reloaded" << std::endl;
            renderer.ResetSamples();
        }

        ImGui::Text("SPP");
        if (ImGui::SliderInt("##SPP", &scene.samplesPerPixel, 1, 10)) 
            renderer.ResetSamples();

        ImGui::Text("Max Ray Depth");
        if (ImGui::SliderInt("##MaxRayDepth", &scene.maxRayDepth, 1, 50)) 
            renderer.ResetSamples();

        ImGui::Text("Directional Light");
        if (ImGui::SliderFloat3("##DirectionalLight", glm::value_ptr(scene.lightDirection), -1.0f, 1.0f))
            renderer.ResetSamples();

        if (ImGui::CollapsingHeader("Edit Object Properties"))
        {
            if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_LeftArrow))) 
                scene.PrimitiveIdx = scene.PrimitiveIdx == 0 ? scene.primitives.size() - 1 : scene.PrimitiveIdx - 1;
            if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_RightArrow))) 
                scene.PrimitiveIdx = scene.PrimitiveIdx == scene.primitives.size() - 1 ? 0 : scene.PrimitiveIdx + 1;
            
            Primitive& prim = scene.primitives[scene.PrimitiveIdx];

            prim.type == 0 ? ImGui::Text("Type: Sphere") : ImGui::Text("Type: AABB");
            ImGui::Text("Primitive Index: %i", scene.PrimitiveIdx);
            ImGui::Text("Distance from camera: %.3f", glm::distance(prim.position, camera.GetPosition()));

            ImGui::Text("Primitive Id: %i", prim.id);
            ImGui::Text("Position");
            if (ImGui::DragFloat3("##Position", glm::value_ptr(prim.position), 0.1f))
            {
                renderer.ResetSamples();
                renderer.m_BVH->RebuildBVH(scene.primitives);
            }

            switch (prim.type)
            {
                case PRIM_SPHERE: // Sphere
                    ImGui::Text("Radius");
                    if (ImGui::DragFloat("##Radius", &prim.radius, 0.05f, 0.1f, 1000.0f)) 
                        renderer.ResetSamples();
                    break;
                case PRIM_AABB: // AABB
                    ImGui::Text("Dimensions");
                    if (ImGui::DragFloat3("##Dimensions", glm::value_ptr(prim.dimensions), 0.1f, 0.1f, 1000.0f)) 
                        renderer.ResetSamples();
                    break;
            }

            ImGui::Text("Albedo");
            if (ImGui::ColorEdit3("##Albedo", glm::value_ptr(prim.mat.albedo))) 
                renderer.ResetSamples();

            ImGui::Text("Absorption");
            if (ImGui::DragFloat3("##Absorption", glm::value_ptr(prim.mat.absorption), 0.01f, 0.0f, 5.0f)) 
                renderer.ResetSamples();

            ImGui::Text("Roughness");
            if (ImGui::SliderFloat("##roughness", &prim.mat.roughness, 0.0f, 1.0f)) 
                renderer.ResetSamples();

            ImGui::Text("Metallic");
            if (ImGui::SliderFloat("##metallic", &prim.mat.metallic, 0.0f, 1.0f)) 
                renderer.ResetSamples();

            ImGui::Text("Specular Chance");
            if (ImGui::SliderFloat("##SpecularChance", &prim.mat.specularChance, 0.0f, 1.0f)) 
                renderer.ResetSamples();

            ImGui::Text("IOR");
            if (ImGui::SliderFloat("##IOR", &prim.mat.ior, 1.0f, 2.5f)) 
                renderer.ResetSamples();

            ImGui::Text("Refraction Chance");
            if (ImGui::SliderFloat("##RefractionChance", &prim.mat.refractionChance, 0.0f, 1.0f)) 
                renderer.ResetSamples();

            ImGui::Text("Emissive");
            if (ImGui::ColorEdit3("##Emissive", glm::value_ptr(prim.mat.emissive))) 
                renderer.ResetSamples();

            ImGui::Text("Emissive Strength");
            if (ImGui::DragFloat("##EmissiveStrength", &prim.mat.emissiveStrength, 0.005f, 0.0f, 100.0f)) 
                renderer.ResetSamples();
        }

        ImGui::PopItemWidth();
        ImGui::End();
    }
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
    ImGui_ImplOpenGL3_Init("#version 460");
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
