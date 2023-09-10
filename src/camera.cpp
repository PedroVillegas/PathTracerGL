#include "window.h"
#include "camera.h"

#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <iostream>

#define PI 3.14159f

Camera::Camera(float verticalFOV, float nearClip, float farClip)
    :
    m_VerticalFOV(verticalFOV), 
    m_NearClip(nearClip), 
    m_FarClip(farClip)
{
    m_Forward = glm::vec3(0, 0, -1);
    m_Position = glm::vec3(0, 0, 3);
    RecalculateView();
}

Camera::Camera(glm::vec3 position, float verticalFOV, float nearClip, float farClip)
    :
    m_VerticalFOV(verticalFOV), 
    m_NearClip(nearClip), 
    m_FarClip(farClip),
    m_Position(position)
{
    m_Forward = glm::vec3(0, 0, -1);
    RecalculateView();
}

Camera::Camera(glm::vec3 position, glm::vec3 forwardDirection, float verticalFOV, float nearClip, float farClip)
    :
    m_VerticalFOV(verticalFOV), 
    m_NearClip(nearClip), 
    m_FarClip(farClip),
    m_Position(position),
    m_Forward(forwardDirection)
{
    RecalculateView();
}

bool Camera::Orbital(float dt, Window* window)
{
    return false;
}

bool Camera::Cinematic(float dt, Window* window)
{
    GLFWwindow* glfw_win = window->GetWindow();

    // Rotation delta
    glm::vec3 dr { 0.0f }; // {yaw, pitch, roll}

    dr.x = (glfwGetKey(glfw_win, GLFW_KEY_RIGHT) == GLFW_PRESS) - (glfwGetKey(glfw_win, GLFW_KEY_LEFT) == GLFW_PRESS);
    dr.y = (glfwGetKey(glfw_win, GLFW_KEY_UP) == GLFW_PRESS) - (glfwGetKey(glfw_win, GLFW_KEY_DOWN) == GLFW_PRESS);
    // dr.x = (glfwGetKey(glfw_win, GLFW_KEY_SEMICOLON) == GLFW_PRESS) - (glfwGetKey(glfw_win, GLFW_KEY_K) == GLFW_PRESS);
    // dr.y = (glfwGetKey(glfw_win, GLFW_KEY_O) == GLFW_PRESS) - (glfwGetKey(glfw_win, GLFW_KEY_L) == GLFW_PRESS);
    dr.z = (glfwGetKey(glfw_win, GLFW_KEY_E) == GLFW_PRESS) - (glfwGetKey(glfw_win, GLFW_KEY_Q) == GLFW_PRESS);

    m_RotationMomentum = (m_RotationMomentum * damping) + (dr * (1-damping));

    float rlen = glm::length(m_RotationMomentum);
    if (rlen > 1e-3f) // still rotating?
    {
        float yaw = -glm::radians(m_RotationMomentum.x) * dt * sensitivity;
        float pitch = glm::radians(m_RotationMomentum.y) * dt * sensitivity;
        float roll = glm::radians(m_RotationMomentum.z) * dt * sensitivity;

        // Construct rotation delta quaternion (q) relative to the current camera orientation
        // by multiplying the rotation momentum vector (m_RotationMomentum) with the current orientation matrix
        glm::quat q_yaw = glm::normalize(glm::angleAxis(yaw, m_Up));
        glm::quat q_pitch = glm::normalize(glm::angleAxis(pitch, m_Right));
        glm::quat q_roll = glm::normalize(glm::angleAxis(roll, m_Forward));

        // Update the quaternion orientation (m_QuatOrientation) by multiplying by the rotation delta quaternion (q)
        m_QuatOrientation = glm::normalize(q_roll * q_pitch * q_yaw * m_QuatOrientation);

        m_Forward = glm::normalize(m_QuatOrientation * glm::vec3(0.0f, 0.0f, -1.0f));
        m_Up = glm::normalize(m_QuatOrientation * glm::vec3(0.0f, 1.0f, 0.0f));
        m_Right = glm::normalize(m_QuatOrientation * glm::vec3(1.0f, 0.0f, 0.0f));

        // Recalculate view matrix with new orientation
        RecalculateView();
    }

    // Movement delta
    glm::vec3 dm {0.0f};

    dm.x = (glfwGetKey(glfw_win, GLFW_KEY_D) == GLFW_PRESS) - (glfwGetKey(glfw_win, GLFW_KEY_A) == GLFW_PRESS);
    dm.y = (glfwGetKey(glfw_win, GLFW_KEY_SPACE) == GLFW_PRESS) - (glfwGetKey(glfw_win, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS);
    dm.z = (glfwGetKey(glfw_win, GLFW_KEY_W) == GLFW_PRESS) - (glfwGetKey(glfw_win, GLFW_KEY_S) == GLFW_PRESS);
    float mlen = glm::length(dm) / m_TopSpeed; 
    if (mlen < 1e-3f) mlen = 1.0f;

    glm::mat3 matOrient;

    matOrient[2] = m_Forward; //m_MatOrientation[0];
    matOrient[1] = m_Up; //m_MatOrientation[1];
    matOrient[0] = m_Right; //m_MatOrientation[2];

    m_MovementMomentum = m_MovementMomentum * damping + (1.0f - damping) * matOrient * dm / mlen;

    m_Position += m_MovementMomentum * dt;

    mlen = glm::length(m_MovementMomentum);
    rlen = glm::length(m_RotationMomentum);
    if (mlen < 1e-1f) 
        m_MovementMomentum = {0.0f, 0.0f, 0.0f};

    if (rlen < 1e-1f) 
        m_RotationMomentum = {0.0f, 0.0f, 0.0f};

    if (mlen > 0.0 || rlen > 0.0) 
        return true;
        
    return false;
}

bool Camera::FPS(float dt, Window* window)
{   
    GLFWwindow* glfw_win = window->GetWindow();

    if (glfwGetKey(glfw_win, GLFW_KEY_G) == GLFW_PRESS)
    {
        glfwSetInputMode(glfw_win, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        m_AllowCameraToMove = false;
        m_MovementMomentum = glm::vec3(0.0f);
    }

    if (glfwGetKey(glfw_win, GLFW_KEY_F) == GLFW_PRESS)
    {
        glfwSetInputMode(glfw_win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        m_AllowCameraToMove = true;
        m_FirstMouse = true;
    }

    if (glfwGetKey(glfw_win, GLFW_KEY_CAPS_LOCK) == GLFW_PRESS) m_TopSpeed = 20.0f;
    else if (glfwGetKey(glfw_win, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) m_TopSpeed = 2.0f;
    else if (glfwGetKey(glfw_win, GLFW_KEY_CAPS_LOCK) == GLFW_RELEASE
            && glfwGetKey(glfw_win, GLFW_KEY_LEFT_CONTROL) == GLFW_RELEASE) m_TopSpeed = 5.0f;

    if (m_AllowCameraToMove)
    {
        glm::vec2 delta {0.0f};

        double cursor_x = 0.0f;
        double cursor_y = 0.0f;

        glfwGetCursorPos(glfw_win, &cursor_x, &cursor_y);
        glm::vec2 cursor_pos { cursor_x, cursor_y };

        if (m_FirstMouse)
        {
            m_LastMousePosition = cursor_pos;
            m_FirstMouse = false;
        }

        delta = (cursor_pos - m_LastMousePosition);
        m_LastMousePosition = cursor_pos;
        
        float dx = glm::radians(delta.x) * dt * sensitivity * 0.1f;
        float dy = glm::radians(delta.y) * dt * sensitivity * 0.1f;

        if (dx != 0.0f || dy != 0.0f)
        {
            glm::quat q_yaw = glm::angleAxis(-dx, glm::vec3(0.0f, 1.0f, 0.0f));
            glm::quat q_pitch = glm::angleAxis(-dy, m_Right);

            m_QuatOrientation = glm::normalize(q_pitch * q_yaw * m_QuatOrientation);

            m_Forward = glm::normalize(m_QuatOrientation * glm::vec3(0.0f, 0.0f, -1.0f));
            // m_Up = glm::normalize(m_QuatOrientation * glm::vec3(0.0f, 1.0f, 0.0f)); // Rotating the Up vector introduces roll
            // m_Right = glm::normalize(m_QuatOrientation * glm::vec3(1.0f, 0.0f, 0.0f));
            m_Right = glm::normalize(glm::cross(m_Forward, m_Up));

            // RecalculateView();
        }

        // Movement delta
        glm::vec3 M {0.0f, 0.0f, 0.0f};

        M.x = (glfwGetKey(glfw_win, GLFW_KEY_D)     == GLFW_PRESS) - (glfwGetKey(glfw_win, GLFW_KEY_A)          == GLFW_PRESS);
        M.y = (glfwGetKey(glfw_win, GLFW_KEY_SPACE) == GLFW_PRESS) - (glfwGetKey(glfw_win, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS);
        M.z = (glfwGetKey(glfw_win, GLFW_KEY_W)     == GLFW_PRESS) - (glfwGetKey(glfw_win, GLFW_KEY_S)          == GLFW_PRESS);
        float mlen = glm::length(M) / m_TopSpeed; 
        if (mlen < 1e-2f) mlen = 1.0f;

        glm::mat3 matOrient;

        matOrient[2] = m_Forward; //m_MatOrientation[0];
        matOrient[1] = m_Up; //m_MatOrientation[1];
        matOrient[0] = m_Right; //m_MatOrientation[2];

        m_MovementMomentum = m_MovementMomentum * damping + (1.0f - damping) * matOrient * M / mlen;

        m_Position += m_MovementMomentum * dt;

        // FOV Zoom
        float zoom = (glfwGetKey(glfw_win, GLFW_KEY_E) == GLFW_PRESS) - (glfwGetKey(glfw_win, GLFW_KEY_Q) == GLFW_PRESS);
        horizontalFOV -= zoom;

        if (horizontalFOV < 10.0f)
        {
            horizontalFOV = 10.0f;
        }
        if (horizontalFOV > 120.0f)
        {
            horizontalFOV = 120.0f;
        }
        if (glm::abs(zoom))
        {   
            SetFov(horizontalFOV);
            RecalculateProjection();
        }

        if (glm::length(m_MovementMomentum) < 1e-1f) 
            m_MovementMomentum = {0.0f, 0.0f, 0.0f};

        if (glm::length(m_MovementMomentum) > 0.0 || dx != 0.0 || dy != 0.0 || glm::abs(zoom))
        {
            RecalculateView();
            return true;
        }
    }
    return false;
}

void Camera::OnResize(uint32_t width, uint32_t height)
{
    if (width == m_ViewportWidth && height == m_ViewportHeight)
        return;

    m_ViewportWidth = width;
    m_ViewportHeight = height;
    m_AspectRatio = (float) width / height;

    RecalculateProjection();
}

void Camera::Reset()
{
    m_Forward = glm::vec3(0.0f, 0.0f, -1.0f);
    m_Up = glm::vec3(0.0f, 1.0f, 0.0f);
    m_Right = glm::vec3(1.0f, 0.0f, 0.0f);
    m_Position = glm::vec3(0.0f, 0.0f, 3.0f);
    m_MovementMomentum = glm::vec3(0.0f, 0.0f, 0.0f);
    m_RotationMomentum = glm::vec3(0.0f, 0.0f, 0.0f);
    m_FirstMouse = true;
    RecalculateView();
}

void Camera::RecalculateProjection()
{
    m_Projection = glm::perspective(glm::radians(m_VerticalFOV), m_AspectRatio, m_NearClip, m_FarClip);
	m_InverseProjection = glm::inverse(m_Projection);
}

void Camera::RecalculateView()
{
    m_View = glm::lookAt(m_Position, m_Position + m_Forward, m_Up);
    m_InverseView = glm::inverse(m_View);
}

void Camera::SetFov(float HorizontalFOV) 
    { 
        m_VerticalFOV = HorizontalFOV;
    }
