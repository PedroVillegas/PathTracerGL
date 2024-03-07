#include "window.h"
#include "camera.h"

#include <glfw/include/GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <vector>
#include <iostream>

#define PI 3.14159265358979323

Camera::Camera(float FOV, float nearClip, float farClip)
    :
    FOV(FOV),
    m_NearClip(nearClip), 
    m_FarClip(farClip)
{
    position = glm::vec3(0, 0, 3);
    Init();
}

Camera::Camera(glm::vec3 position, float FOV, float nearClip, float farClip)
    :
    FOV(FOV), 
    m_NearClip(nearClip), 
    m_FarClip(farClip),
    position(position)
{
    Init();
}

void Camera::Init()
{
    RecalculateView();
    RecalculateProjection();
    UpdateParams();
}

void Camera::UpdateParams()
{
    params.InverseProjection = m_InverseProjection;
    params.InverseView = m_InverseView;
    params.position = position;
    params.aperture = aperture;
    params.focalLength = focal_length;
}

void Camera::SetupCamera(
    glm::vec3 position = {0.0f, 0.0f, 0.0f}, 
    glm::vec3 forward = {0.0f, 0.0f, -1.0f}, 
    float fov = 60.0f)
{
    this->position = position;
    this->FOV = fov;
    glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);
    m_Forward = glm::normalize(forward);
    m_QuatOrientation = glm::quatLookAt(m_Forward, Up);

    m_Pitch = glm::asin(m_Forward.y); // in Radians
    m_Yaw   = glm::asin(m_Forward.z / glm::cos(m_Pitch));

    m_Pitch = glm::degrees(m_Pitch);
    m_Yaw   = glm::degrees(m_Yaw);

    RecalculateProjection();
    RecalculateView();
}

bool Camera::OnUpdate(float dt, Window* window)
{
    bool IsCamMoving = false;
    switch (type)
    {
        case CAM_TYPE_FPS:
            IsCamMoving = FPS(dt, window);
            UpdateParams();
            break;
        case CAM_TYPE_CINEMATIC:
            IsCamMoving = Cinematic(dt, window);
            UpdateParams();
            break;
        case CAM_TYPE_ORBITAL:
            IsCamMoving = Orbital(dt, window);
            break;
    }
    return IsCamMoving;
}

bool Camera::Orbital(float dt, Window* window)
{
    return false;
}

bool Camera::Cinematic(float dt, Window* window)
{
    GLFWwindow* glfw_win = window->GetWindow();
    
    if (glfwGetKey(glfw_win, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetInputMode(glfw_win, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        m_AllowCameraToMove = false;
        m_Velocity = glm::vec3(0.0f);
    }

    if (glfwGetKey(glfw_win, GLFW_KEY_F) == GLFW_PRESS)
    {
        glfwSetInputMode(glfw_win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        m_AllowCameraToMove = true;
    }

    if (m_AllowCameraToMove)
    {
        // Rotation delta
        glm::vec3 dR { 0.0f }; // {yaw, pitch, roll}

        dR.x = float((glfwGetKey(glfw_win, GLFW_KEY_RIGHT) == GLFW_PRESS) - (glfwGetKey(glfw_win, GLFW_KEY_LEFT) == GLFW_PRESS));
        dR.y = float((glfwGetKey(glfw_win, GLFW_KEY_UP) == GLFW_PRESS) - (glfwGetKey(glfw_win, GLFW_KEY_DOWN) == GLFW_PRESS));
        dR.z = float((glfwGetKey(glfw_win, GLFW_KEY_E) == GLFW_PRESS) - (glfwGetKey(glfw_win, GLFW_KEY_Q) == GLFW_PRESS));

        glm::vec3 rotAcceleration = glm::length(dR) > 0.0f ? glm::normalize(dR) : dR;
        m_RotVelocity += rotAcceleration * sensitivity * 0.1f * dt;

        float rlen = glm::length(m_RotVelocity);
        if (rlen > 0.0f) // still rotating?
        {
            float yaw   =  glm::radians(m_RotVelocity.x);
            float pitch = -glm::radians(m_RotVelocity.y);
            float roll  =  glm::radians(m_RotVelocity.z);

            // Construct rotation delta quaternions relative to the current camera orientation
            // by multiplying the rotation momentum vector with the current orientation matrix
            glm::quat qYaw   = glm::normalize(glm::angleAxis(yaw,   m_Up));
            glm::quat qPitch = glm::normalize(glm::angleAxis(pitch, m_Right));
            glm::quat qRoll  = glm::normalize(glm::angleAxis(roll,  m_Forward));

            // Update the quaternion orientation (m_QuatOrientation) by multiplying by the rotation delta quaternions
            m_QuatOrientation = glm::normalize(m_QuatOrientation * qRoll * qPitch * qYaw);

            m_Forward = glm::normalize(glm::vec3(0.0f, 0.0f, 1.0f) * m_QuatOrientation);
            m_Up      = glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f) * m_QuatOrientation);
            m_Right   = glm::normalize(glm::vec3(1.0f, 0.0f, 0.0f) * m_QuatOrientation);

            // Recalculate view matrix with new orientation
            RecalculateView();
        }
        m_RotVelocity = glm::mix(m_RotVelocity, glm::vec3(0.0f), damping * dt);

        float topSpeed = walkingSpeed;
        if (glfwGetKey(glfw_win, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        {
            topSpeed = sprintingSpeed;
        }

        // Movement delta
        glm::vec3 dM {0.0f};

        dM.x = float((glfwGetKey(glfw_win, GLFW_KEY_D) == GLFW_PRESS) - (glfwGetKey(glfw_win, GLFW_KEY_A) == GLFW_PRESS));
        dM.y = float((glfwGetKey(glfw_win, GLFW_KEY_SPACE) == GLFW_PRESS) - (glfwGetKey(glfw_win, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS));
        dM.z = float((glfwGetKey(glfw_win, GLFW_KEY_S) == GLFW_PRESS) - (glfwGetKey(glfw_win, GLFW_KEY_W) == GLFW_PRESS));

        glm::vec3 acceleration = glm::length(dM) > 0.0f ? glm::normalize(dM) :dM;
        m_Velocity += acceleration * topSpeed * 5.0f * dt;
        position   += m_Velocity * m_QuatOrientation * dt;
        m_Velocity  = glm::mix(m_Velocity, glm::vec3(0.0f), damping * dt);
        float mlen = glm::length(m_Velocity);

        if (mlen < 0.5f)
            m_Velocity = glm::vec3(0.0f);

        if (rlen < 0.025f)
            m_RotVelocity = glm::vec3(0.0f);

        if (mlen > 0.0 || rlen > 0.0f)
        {
            RecalculateView();
            return true;
        }
    }
    return false;
}

bool Camera::FPS(float dt, Window* window)
{   
    GLFWwindow* glfw_win = window->GetWindow();

    if (glfwGetKey(glfw_win, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetInputMode(glfw_win, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        m_AllowCameraToMove = false;
        m_Velocity = glm::vec3(0.0f);
    }

    if (glfwGetKey(glfw_win, GLFW_KEY_F) == GLFW_PRESS)
    {
        glfwSetInputMode(glfw_win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        m_AllowCameraToMove = true;
        m_FirstMouse = true;
    }

    if (m_AllowCameraToMove)
    {
        glm::vec2 delta { 0.0f };

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
        
        float dx = glm::radians(delta.x) * sensitivity * 8.0f * dt;
        float dy = glm::radians(delta.y) * sensitivity * 8.0f * dt;

        if (dx != 0.0f || dy != 0.0f)
        {
            m_Yaw   += dx;
            m_Pitch -= dy;

            m_Yaw   = glm::mod(m_Yaw, 360.0f);
            m_Pitch = glm::clamp(m_Pitch, -89.0f, 89.0f);

            glm::vec3 direction;
            direction.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
            direction.y = sin(glm::radians(m_Pitch));
            direction.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
            m_Forward = glm::normalize(direction);
            m_Right = glm::cross(m_Forward, glm::vec3(0, 1, 0));
        }
        
        float topSpeed = walkingSpeed;
        if (glfwGetKey(glfw_win, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        {
            topSpeed = sprintingSpeed;
        } 
        if (glfwGetKey(glfw_win, GLFW_KEY_LEFT_ALT) == GLFW_PRESS) 
        {
            topSpeed = slowSpeed;
        }

        // Movement delta
        glm::vec3 M { 0.0f, 0.0f, 0.0f };

        M.x = float((glfwGetKey(glfw_win, GLFW_KEY_D) == GLFW_PRESS) - (glfwGetKey(glfw_win, GLFW_KEY_A) == GLFW_PRESS));
        M.y = float((glfwGetKey(glfw_win, GLFW_KEY_SPACE) == GLFW_PRESS) - (glfwGetKey(glfw_win, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS));
        M.z = float((glfwGetKey(glfw_win, GLFW_KEY_W) == GLFW_PRESS) - (glfwGetKey(glfw_win, GLFW_KEY_S) == GLFW_PRESS));

        glm::mat3 orient;

        orient[0] = m_Right;
        orient[1] = glm::vec3(0, 1, 0);
        orient[2] = m_Forward;

        glm::vec3 acceleration = glm::length(M) > 0.0f ? glm::normalize(M) : M;
        m_Velocity += acceleration * topSpeed * 5.0f * dt;
        position   += orient * m_Velocity * dt;
        m_Velocity  = glm::mix(m_Velocity, glm::vec3(0.0f), damping * dt);
        float mlen = glm::length(m_Velocity);

        // FOV Zoom
        float zoom = float((glfwGetKey(glfw_win, GLFW_KEY_E) == GLFW_PRESS) - (glfwGetKey(glfw_win, GLFW_KEY_Q) == GLFW_PRESS));
        FOV -= zoom * 100.0f * dt;
        FOV  = glm::clamp(FOV, 10.0f, 120.0f);
        FOV  = glm::mix(FOV, m_LastFOV, 15.0f * dt);
        m_LastFOV = FOV;

        if (glm::abs(zoom))
        {
            SetFov(FOV);
            RecalculateProjection();
        }

        if (mlen < 0.1f)
            m_Velocity = glm::vec3(0.0f);

        if (mlen > 0.0 || dx != 0.0 || dy != 0.0 || glm::abs(zoom))
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
    m_QuatOrientation = glm::quatLookAt(glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));
    position = glm::vec3(0.0f, 0.0f, 3.0f);
    m_Velocity = glm::vec3(0.0f);
    //m_RotationMomentum = glm::vec3(0.0f);
    m_FirstMouse = true;
    RecalculateView();
}

void Camera::RecalculateProjection()
{
    m_Projection = glm::perspective(glm::radians(FOV), m_AspectRatio, m_NearClip, m_FarClip);
	m_InverseProjection = glm::inverse(m_Projection);
}

void Camera::RecalculateView()
{
    switch (type)
    {
    case CAM_TYPE_FPS:
        m_View = glm::lookAt(position, position + m_Forward, glm::vec3(0, 1, 0));
        break;
    case CAM_TYPE_CINEMATIC:
        m_View = glm::mat4_cast(m_QuatOrientation);
        break;
    }

    m_InverseView = glm::inverse(m_View);
}

void Camera::SetFov(float HorizontalFOV) 
    { 
        FOV = HorizontalFOV;
    }
