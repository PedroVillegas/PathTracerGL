#pragma once

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

    void CinematicMovement(float dt, GLFWwindow* window);
    bool OnUpdate(float dt, GLFWwindow* window);
    void OnResize(uint width, uint height);

    void SetFov(float HorizontalFOV) { m_VerticalFOV = HorizontalFOV * (m_ViewportWidth / m_ViewportHeight); }

    const glm::mat4& GetProjection() const { return m_Projection; }
    const glm::mat4& GetInverseProjection() const { return m_InverseProjection; }
    const glm::mat4& GetView() const { return m_View; }
    const glm::mat4& GetInverseView() const { return m_InverseView; }

    const glm::vec3& GetPosition() const { return m_Position; }
    const glm::vec3& GetMomentum() const { return m_Momentum; }
    const glm::vec3& GetDirection() const { return m_ForwardDirection; }

    float GetRotationSpeed();
    int horizontalFOV = 90;

public:
    void RecalculateProjection();
    void RecalculateView();

private:
    glm::mat4 m_Projection { 1.0f };
    glm::mat4 m_View { 1.0f };
    glm::mat4 m_InverseProjection { 1.0f };
    glm::mat4 m_InverseView { 1.0f };

    float m_VerticalFOV = 45.0f;
    float m_NearClip = 0.1f;
    float m_FarClip = 100.0f;

    glm::vec3 m_Position { 0.0f, 0.0f, 0.0f };
    glm::vec3 m_ForwardDirection { 0.0f, 0.0f, 0.0f };

    glm::vec3 m_Momentum { 0.0f, 0.0f, 0.0f };

    bool m_AllowCameraToMove = false;
    glm::vec2 m_LastMousePosition { 0.0f, 0.0f };

    uint m_ViewportWidth = 0, m_ViewportHeight = 0;
};
