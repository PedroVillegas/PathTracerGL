#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "window.h"

class Camera
{
public:
    Camera() {}
    Camera(float verticalFOV, float nearClip, float farClip);
    Camera(glm::vec3 position, float verticalFOV, float nearClip, float farClip);
    Camera(glm::vec3 position, glm::vec3 forwardDirection, float verticalFOV, float nearClip, float farClip);

    bool Orbital(float dt, Window* window);
    bool Cinematic(float dt, Window* window);
    bool FPS(float dt, Window* window);
    void OnResize(uint32_t width, uint32_t height);
    void Reset();

    void SetFov(float HorizontalFOV);
    const glm::mat4& GetProjection() const { return m_Projection; }
    const glm::mat4& GetInverseProjection() const { return m_InverseProjection; }
    const glm::mat4& GetView() const { return m_View; }
    const glm::mat4& GetInverseView() const { return m_InverseView; }

    const glm::vec3& GetPosition() const { return m_Position; }
    const glm::vec3& GetMovementMomentum() const { return m_MovementMomentum; }
    const glm::vec3& GetRotationMomentum() const { return m_RotationMomentum; }
    const glm::vec3& GetDirection() const { return m_Forward; }
    
    void RecalculateProjection();
    void RecalculateView();
public:
    int type = 0;
    float horizontalFOV = 90.0f;
    float damping = 0.9f;
    float focal_length = 4.0f;
    float aperture = 0.0f;
    float sensitivity = 20.0f;
private:
    float m_VerticalFOV = 45.0f;
    float m_NearClip = 0.1f;
    float m_FarClip = 100.0f;
    float m_AspectRatio;

    glm::vec3 m_Position { 0.0f, 0.0f, 0.0f };
    glm::vec3 m_Forward { 0.0f, 0.0f, -1.0f };
    glm::vec3 m_Up { 0.0f, 1.0f, 0.0f };
    glm::vec3 m_Right { 1.0f, 0.0f, 0.0f };

    glm::mat4 m_Projection { 0.0f };
    glm::mat4 m_View { 0.0f };
    glm::mat4 m_InverseProjection { 0.0f };
    glm::mat4 m_InverseView { 0.0f };

    glm::quat m_QuatOrientation = glm::quat();

    glm::vec3 m_MovementMomentum { 0.0f, 0.0f, 0.0f };
    glm::vec3 m_RotationMomentum { 0.0f, 0.0f, 0.0f };
    float m_TopSpeed = 5.0f;

    bool m_AllowCameraToMove = false;
    glm::vec2 m_LastMousePosition { 0.0f, 0.0f };
    bool m_FirstMouse = true;

    uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
};
