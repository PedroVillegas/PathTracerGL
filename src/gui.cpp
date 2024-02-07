#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include <glm/gtc/type_ptr.hpp>

#include "window.h"
#include "renderer.h"
#include "camera.h"
#include "scene.h"
#include "gui.h"
#include "utils.h"

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

void Gui::Render(Renderer& renderer, Scene& scene, ApplicationSettings& settings)
{
    if (ImGui::Begin("Overview"))
    {
        ImGui::Text("Viewport: %i x %i", renderer.GetViewportWidth(), renderer.GetViewportHeight());
        ImGui::Text("Render time: %.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
        ImGui::Text("Framerate: %.1f FPS", ImGui::GetIO().Framerate);
        ImGui::Text("Iterations: %i", renderer.GetIterations());
        ImGui::Checkbox("Pause", &renderer.b_Pause);

        if (ImGui::CollapsingHeader("Application Settings"))
        {
            ImGui::Checkbox("V-Sync", &settings.vsync);
            ImGui::Text("Tonemap");
            if (ImGui::Combo("##Tonemap", &settings.tonemap, "Jodie-Reinhard\0ACES film\0ACES fitted\0AgX\0AgX Punchy\0")) 
                renderer.ResetSamples();

            if (ImGui::Checkbox("Enable BVH", &settings.BVHEnabled))
                renderer.ResetSamples();
            if (settings.BVHEnabled == true)
            {
                if (ImGui::Checkbox("Visualise BVH", &settings.debugBVHVisualisation))
                    renderer.ResetSamples();
                ImGui::Checkbox("Draw BVH", &renderer.b_DrawBVH);
                ImGui::Text("BVH Depth");
                ImGui::SliderInt("##BVH-Depth", &renderer.BVHDepth, 0, 10);
            }
            else
            {
                renderer.b_DrawBVH = false;
                renderer.BVHDepth = 0;
            }
        }
    }
    ImGui::End();

    Gui::CreateCameraWindow(renderer, scene);
    Gui::CreateSceneWindow(renderer, scene);

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

void Gui::CreateCameraWindow(Renderer& renderer, Scene& scene)
{
    if (ImGui::Begin("Camera"))
    {
        ImGui::PushItemWidth(ImGui::GetWindowContentRegionWidth());
        ImGui::Text("Camera Type");
        if (ImGui::Combo("##CameraType", &scene.Eye->type, "FREE\0CINEMATIC\0")) 
        {
            scene.Eye->Reset();
            renderer.ResetSamples();
        }

        ImGui::Text("Velocity : %.2f", glm::length(scene.Eye->GetVelocity()));
        ImGui::Text("Position : %.2f %.2f %.2f", scene.Eye->position.x, scene.Eye->position.y , scene.Eye->position.z);
        ImGui::Text("Direction: %.2f %.2f %.2f", scene.Eye->forward.x, scene.Eye->forward.y , scene.Eye->forward.z);
        // ImGui::Text("Momentum : %.2f %.2f %.2f", scene.Eye->GetMovementMomentum().x, scene.Eye->GetMovementMomentum().y, scene.Eye->GetMovementMomentum().z);
        // ImGui::Text("Rotation : %.2f %.2f %.2f", scene.Eye->GetRotationMomentum().x, scene.Eye->GetRotationMomentum().y, scene.Eye->GetRotationMomentum().z);
       
        ImGui::Text("Sensitivity");
        ImGui::SliderFloat("##Sensitivity", &scene.Eye->sensitivity, 1.0f, 100.0f);
        ImGui::Text("Max Velocity");
        ImGui::SliderFloat("##Max Velocity", &scene.Eye->MaxVelocity, 1.0f, 50.0f);
        ImGui::Text("Damping Factor");
        ImGui::SliderFloat("##DampingFactor", &scene.Eye->damping, 0.0f, 0.95f);

        ImGui::Text("Focal Length");
        if (ImGui::InputFloat("##FocalLength", &scene.Eye->focal_length, 0.05f, 1.0f)) 
            renderer.ResetSamples();

        ImGui::Text("Aperture Diameter");
        if (ImGui::SliderFloat("##Aperture", &scene.Eye->aperture, 0.0f, 2.0f)) 
            renderer.ResetSamples();

        ImGui::Text("f-stop: f/%0.3f", scene.Eye->focal_length / scene.Eye->aperture);

        ImGui::Text("FOV");
        if (ImGui::SliderFloat("##FOV", &scene.Eye->FOV, 10.0f, 120.0f))
        {
            scene.Eye->SetFov(scene.Eye->FOV);
            scene.Eye->RecalculateProjection();
            renderer.ResetSamples();
        }
        ImGui::PopItemWidth();
    }
    ImGui::End();
}

void Gui::CreateSceneWindow(Renderer& renderer, Scene& scene)
{
    if (ImGui::Begin("Scene"))
    {
        ImGui::PushItemWidth(ImGui::GetWindowContentRegionWidth());
        ImGui::Text("Objects in scene: %llu", scene.primitives.size());

        ImGui::Text("Scene: ");
        if (ImGui::Combo("##SceneSelection", &scene.SceneIdx, "Room with window\0Cornell Box\0White room with coloured lights\0")) 
        {
            scene.SelectScene();
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

        if (ImGui::Button("Reset Samples")) 
            renderer.ResetSamples();
        
        if (ImGui::Button("Reload Shader"))
        {

            std::cout << "Reloading Shader..." << std::endl;
            renderer.GetShader().ReloadShader();
            std::cout << "Shader Successfully Reloaded" << std::endl;
            renderer.ResetSamples();
        }

        ImGui::Text("SPP");
        if (ImGui::SliderInt("##SPP", &scene.samplesPerPixel, 1, 10)) 
            renderer.ResetSamples();

        ImGui::Text("Max Ray Depth");
        if (ImGui::SliderInt("##MaxRayDepth", &scene.maxRayDepth, 1, 50)) 
            renderer.ResetSamples();

        if (ImGui::CollapsingHeader("Edit Lights"))
        {
            if (ImGui::Button("Add Sphere Light"))
            {
                scene.AddDefaultSphere();
                scene.AddLight(scene.primitives.size() - 1, glm::vec3(1.0f));
                renderer.m_BVH->RebuildBVH(scene.primitives);
                renderer.ResetSamples();
            }
            ImGui::SameLine();
            if (ImGui::Button("Add Cube"))
            {
                scene.AddDefaultCube();
                scene.AddLight(scene.primitives.size() - 1, glm::vec3(1.0f));
                renderer.m_BVH->RebuildBVH(scene.primitives);
                renderer.ResetSamples();
        }

            if (scene.day == 1)
            {
                ImGui::Text("Sun Colour");
                if (ImGui::ColorEdit3("##SunColour", glm::value_ptr(scene.sunColour)))
                    renderer.ResetSamples();

                ImGui::Text("Sun Elevation");
                if (ImGui::SliderFloat("##Elevation", &scene.sunElevation, -90.0f, 90.0f))
                    renderer.ResetSamples();

                ImGui::Text("Sun Azimuth");
                if (ImGui::SliderFloat("##Azimuth", &scene.sunAzimuth, -360.0f, 360.0f))
                    renderer.ResetSamples();
            }

            if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_LeftArrow))) 
                scene.LightIdx = int(scene.LightIdx == 0 ? scene.lights.size() - 1 : scene.LightIdx - 1);
            if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_RightArrow))) 
                scene.LightIdx = int(scene.LightIdx == scene.lights.size() - 1 ? 0 : scene.LightIdx + 1);
            
            if (scene.lights.size() > 0)
            {
                Primitive& prim = scene.primitives[scene.lights[scene.LightIdx].id];

                prim.type == 0 ? ImGui::Text("Type: Sphere") : ImGui::Text("Type: AABB");
                ImGui::Text("Primitive Index: %i", scene.lights[scene.LightIdx].id);
                ImGui::Text("Distance from camera: %.3f", glm::distance(prim.position, scene.Eye->position));

                ImGui::Text("Position");
                if (ImGui::DragFloat3("##LightPos", glm::value_ptr(prim.position), 0.1f))
                {
                    renderer.ResetSamples();
                    renderer.m_BVH->RebuildBVH(scene.primitives);
                }

                switch (prim.type)
                {
                    case PRIM_SPHERE: // Sphere
                        ImGui::Text("Radius");
                        if (ImGui::DragFloat("##LightRadius", &prim.radius, 0.05f, 0.1f, 1000.0f)) 
                            renderer.ResetSamples();
                        break;
                    case PRIM_AABB: // AABB
                        ImGui::Text("Dimensions");
                        if (ImGui::DragFloat3("##LightDims", glm::value_ptr(prim.dimensions), 0.1f, 0.1f, 1000.0f)) 
                            renderer.ResetSamples();
                        break;
                }

                ImGui::Text("Emissive");
                if (ImGui::ColorEdit3("##Emissive", glm::value_ptr(prim.mat.emissive)))
                    renderer.ResetSamples();

                ImGui::Text("Intensity");
                if (ImGui::DragFloat("##intensity", &prim.mat.intensity, 0.005f, 0.0f, 100.0f)) 
                    renderer.ResetSamples();
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

            if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_LeftArrow))) 
                scene.PrimitiveIdx = int(scene.PrimitiveIdx == 0 ? scene.primitives.size() - 1 : scene.PrimitiveIdx - 1);
            if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_RightArrow))) 
                scene.PrimitiveIdx = int(scene.PrimitiveIdx == scene.primitives.size() - 1 ? 0 : scene.PrimitiveIdx + 1);
            
            if (scene.primitives.size() > 0)
            {
                Primitive& prim = scene.primitives[scene.PrimitiveIdx];

                prim.type == 0 ? ImGui::Text("Type: Sphere") : ImGui::Text("Type: AABB");
                ImGui::Text("Primitive Index: %i", scene.PrimitiveIdx);
                ImGui::Text("Distance from camera: %.3f", glm::distance(prim.position, scene.Eye->position));

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
                if (ImGui::DragFloat3("##Absorption", glm::value_ptr(prim.mat.absorption), 0.001f, 0.0f, 1.0f)) 
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
   return;
}
