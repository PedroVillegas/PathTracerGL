#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "window.h"

struct alignas(16) CameraBlock
{
    glm::mat4 InverseProjection;
    glm::mat4 InverseView;
    glm::vec3 position;
    float pad;
    float aperture;
    float focalLength;
};
class Camera
{
    enum
    {
        CAM_TYPE_FPS = 0,
        CAM_TYPE_CINEMATIC = 1,
        CAM_TYPE_ORBITAL = 2
    };

public:
    Camera() {}
    Camera(float FOV, float nearClip, float farClip);
    Camera(glm::vec3 position, float FOV, float nearClip, float farClip);
    Camera(glm::vec3 position, glm::vec3 forwardDirection, float FOV, float nearClip, float farClip);

    bool Orbital(float dt, Window* window);
    bool Cinematic(float dt, Window* window);
    bool FPS(float dt, Window* window);
    bool OnUpdate(float dt, Window* window);
    void OnResize(uint32_t width, uint32_t height);
    void Reset();

    void UpdateParams();
    void SetFov(float HorizontalFOV);
    const glm::mat4& GetProjection() const { return m_Projection; }
    const glm::mat4& GetInverseProjection() const { return m_InverseProjection; }
    const glm::mat4& GetView() const { return m_View; }
    const glm::mat4& GetInverseView() const { return m_InverseView; }

    const glm::vec3& GetRotationMomentum() const { return m_RotationMomentum; }
    const glm::vec3& GetVelocity() const { return m_MovementMomentum; }
    
    void RecalculateProjection();
    void RecalculateView();

    CameraBlock params;

    glm::vec3 position;
    glm::vec3 forward;

    int type = 0;
    float FOV = 60.0f;
    float damping = 0.9f;
    float focal_length = 4.0f;
    float aperture = 0.0f;
    float sensitivity = 40.0f;
    float MaxVelocity = 40.0f;
    
private:
    void Init();

    float m_NearClip = 0.1f;
    float m_FarClip = 1000.0f;
    float m_AspectRatio;

    glm::vec3 m_Up;
    glm::vec3 m_Right;

    glm::mat4 m_Projection;
    glm::mat4 m_View;
    glm::mat4 m_InverseProjection;
    glm::mat4 m_InverseView;

    glm::quat m_QuatOrientation = glm::quat();

    glm::vec3 m_Velocity;
    glm::vec3 m_MovementMomentum;
    glm::vec3 m_RotationMomentum;

    bool m_AllowCameraToMove = false;
    glm::vec2 m_LastMousePosition { 0.0f, 0.0f };
    bool m_FirstMouse = true;

    uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
};
