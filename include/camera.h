#pragma once

#include "window.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <vector>
#include <iostream>

class Camera
{
public:
    Camera(float verticalFOV, float nearClip, float farClip);
    Camera(glm::vec3 position, float verticalFOV, float nearClip, float farClip);
    Camera(glm::vec3 position, glm::vec3 forwardDirection, float verticalFOV, float nearClip, float farClip);

    bool Orbital(float dt, Window* window);
    bool Cinematic(float dt, Window* window);
    bool FPS(float dt, Window* window);
    void OnResize(uint width, uint height);
    void Reset();

    void SetFov(float HorizontalFOV) { m_VerticalFOV = HorizontalFOV * (m_ViewportWidth / m_ViewportHeight); }

    const glm::mat4& GetProjection() const { return m_Projection; }
    const glm::mat4& GetInverseProjection() const { return m_InverseProjection; }
    const glm::mat4& GetView() const { return m_View; }
    const glm::mat4& GetInverseView() const { return m_InverseView; }

    const glm::vec3& GetPosition() const { return m_Position; }
    const glm::vec3& GetMovementMomentum() const { return m_MovementMomentum; }
    const glm::vec3& GetRotationMomentum() const { return m_RotationMomentum; }
    const glm::vec3& GetDirection() const { return m_Forward; }

    int type = 0;
    int horizontalFOV = 90;
    float damping = 0.9f;
    float focal_length = 5.0f;
    float aperture = 0.2f;
    float sensitivity = 20.0f;

public:
    void RecalculateProjection();
    void RecalculateView();

private:
    float m_VerticalFOV = 45.0f;
    float m_NearClip = 0.1f;
    float m_FarClip = 100.0f;
    float m_AspectRatio;

    glm::mat4 m_Projection { 0.0f };
    glm::mat4 m_View { 0.0f };
    glm::mat4 m_InverseProjection { 0.0f };
    glm::mat4 m_InverseView { 0.0f };

    glm::quat m_QuatOrientation = glm::quat();

    glm::vec3 m_Position { 0.0f, 0.0f, 0.0f };
    glm::vec3 m_Forward { 0.0f, 0.0f, -1.0f };
    glm::vec3 m_Up { 0.0f, 1.0f, 0.0f };
    glm::vec3 m_Right { 1.0f, 0.0f, 0.0f };

    glm::vec3 m_MovementMomentum { 0.0f, 0.0f, 0.0f };
    glm::vec3 m_RotationMomentum { 0.0f, 0.0f, 0.0f };

    bool m_AllowCameraToMove = false;
    glm::vec2 m_LastMousePosition { 0.0f, 0.0f };
    bool m_FirstMouse = true;

    uint m_ViewportWidth = 0, m_ViewportHeight = 0;
};
