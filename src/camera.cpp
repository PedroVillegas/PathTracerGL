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

    m_Yaw = 90.f;

    //m_Yaw = std::atan2f(m_Forward.x, m_Forward.z) * 180.f / PI;
    //m_Pitch = -glm::asin(m_Forward.y) * 180.f / PI;

    //m_Pitch = glm::degrees(glm::eulerAngles(m_QuatOrientation).x);
    //m_Yaw   = glm::degrees(glm::eulerAngles(m_QuatOrientation).y);

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
    /*
    GLFWwindow* glfw_win = window->GetWindow();

    // Rotation delta
    glm::vec3 dr { 0.0f }; // {yaw, pitch, roll}

    dr.x = float((glfwGetKey(glfw_win, GLFW_KEY_RIGHT) == GLFW_PRESS) - (glfwGetKey(glfw_win, GLFW_KEY_LEFT) == GLFW_PRESS));
    dr.y = float((glfwGetKey(glfw_win, GLFW_KEY_UP) == GLFW_PRESS) - (glfwGetKey(glfw_win, GLFW_KEY_DOWN) == GLFW_PRESS));
    dr.z = float((glfwGetKey(glfw_win, GLFW_KEY_E) == GLFW_PRESS) - (glfwGetKey(glfw_win, GLFW_KEY_Q) == GLFW_PRESS));

    m_RotationMomentum = (m_RotationMomentum * damping) + (dr * (1-damping));

    float rlen = glm::length(m_RotationMomentum);
    if (rlen > 1e-3f) // still rotating?
    {
        float yaw   = -glm::radians(m_RotationMomentum.x) * dt * sensitivity * 5.f;
        float pitch =  glm::radians(m_RotationMomentum.y) * dt * sensitivity * 5.f;
        float roll  =  glm::radians(m_RotationMomentum.z) * dt * sensitivity * 5.f;

        // Construct rotation delta quaternion (q) relative to the current camera orientation
        // by multiplying the rotation momentum vector (m_RotationMomentum) with the current orientation matrix
        glm::quat q_yaw = glm::normalize(glm::angleAxis(yaw, m_Up));
        glm::quat q_pitch = glm::normalize(glm::angleAxis(pitch, m_Right));
        glm::quat q_roll = glm::normalize(glm::angleAxis(roll, forward));

        // Update the quaternion orientation (m_QuatOrientation) by multiplying by the rotation delta quaternion (q)
        m_QuatOrientation = glm::normalize(m_QuatOrientation * q_roll * q_pitch * q_yaw);

        forward = glm::normalize(m_QuatOrientation * glm::vec3(0.0f, 0.0f, -1.0f));
        m_Up = glm::normalize(m_QuatOrientation * glm::vec3(0.0f, 1.0f, 0.0f));
        m_Right = glm::normalize(m_QuatOrientation * glm::vec3(1.0f, 0.0f, 0.0f));

        // Recalculate view matrix with new orientation
        RecalculateView();
    }

    // Movement delta
    glm::vec3 dm {0.0f};

    dm.x = float((glfwGetKey(glfw_win, GLFW_KEY_D) == GLFW_PRESS) - (glfwGetKey(glfw_win, GLFW_KEY_A) == GLFW_PRESS));
    dm.y = float((glfwGetKey(glfw_win, GLFW_KEY_SPACE) == GLFW_PRESS) - (glfwGetKey(glfw_win, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS));
    dm.z = float((glfwGetKey(glfw_win, GLFW_KEY_W) == GLFW_PRESS) - (glfwGetKey(glfw_win, GLFW_KEY_S) == GLFW_PRESS));
    float mlen = glm::length(dm) / walkingSpeed; 
    if (mlen < 1e-3f) mlen = 1.0f;

    glm::mat3 matOrient;

    matOrient[2] = forward;
    matOrient[1] = m_Up;
    matOrient[0] = m_Right;

    m_MovementMomentum = m_MovementMomentum * damping + (1.0f - damping) * matOrient * dm / mlen;

    position += m_MovementMomentum * dt;

    mlen = glm::length(m_MovementMomentum);
    rlen = glm::length(m_RotationMomentum);
    if (mlen < 1e-1f) 
        m_MovementMomentum = {0.0f, 0.0f, 0.0f};

    if (rlen < 1e-1f) 
        m_RotationMomentum = {0.0f, 0.0f, 0.0f};

    if (mlen > 0.0 || rlen > 0.0) 
        return true;
    */
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

            //m_QuatOrientation *= glm::angleAxis(-dx, glm::vec3(0, 1, 0) * m_QuatOrientation);
            //m_QuatOrientation *= glm::angleAxis( dy, glm::vec3(1, 0, 0) * m_QuatOrientation);

            glm::vec3 direction;
            direction.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
            direction.y = sin(glm::radians(m_Pitch));
            direction.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
            m_Forward = glm::normalize(direction);
            m_Right = glm::cross(m_Forward, glm::vec3(0, 1, 0));
        }
        
        float topSpeed = walkingSpeed;
        if (glfwGetKey(glfw_win, GLFW_KEY_CAPS_LOCK) == GLFW_PRESS)
        {
            topSpeed = sprintingSpeed;
        } 
        if (glfwGetKey(glfw_win, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) 
        {
            topSpeed = slowSpeed;
        }

        // Movement delta
        glm::vec3 M { 0.0f, 0.0f, 0.0f };

        M.x = float((glfwGetKey(glfw_win, GLFW_KEY_D) == GLFW_PRESS) - (glfwGetKey(glfw_win, GLFW_KEY_A) == GLFW_PRESS));
        M.y = float((glfwGetKey(glfw_win, GLFW_KEY_SPACE) == GLFW_PRESS) - (glfwGetKey(glfw_win, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS));
        M.z = float((glfwGetKey(glfw_win, GLFW_KEY_W) == GLFW_PRESS) - (glfwGetKey(glfw_win, GLFW_KEY_S) == GLFW_PRESS));

        glm::mat3 orient;

        orient[0] = m_Right;
        orient[1] = glm::vec3(0, 1, 0);
        orient[2] = m_Forward;

        glm::vec3 acceleration = glm::length(M) > 0.0f ? glm::normalize(M) : M;
        m_Velocity += acceleration * topSpeed * 5.0f * dt;
        //position   += (m_Velocity * m_QuatOrientation) * dt;
        position   += orient * m_Velocity * dt;
        m_Velocity  = glm::mix(m_Velocity, glm::vec3(0.0f), damping * dt);

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

        if (glm::length(m_Velocity) < 0.5f)
            m_Velocity = glm::vec3(0.0f);

        if (glm::length(m_Velocity) > 0.0 || dx != 0.0 || dy != 0.0 || glm::abs(zoom))
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
    m_RotationMomentum = glm::vec3(0.0f);
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
    //glm::vec3 X = m_QuatOrientation * glm::vec3(1, 0, 0);
    //glm::vec3 Y = m_QuatOrientation * glm::vec3(0, 1, 0);
    //glm::vec3 Z = m_QuatOrientation * glm::vec3(0, 0, 1);

    //glm::vec3 P = position;

    //m_View = glm::mat4
    //(
    //    X.x, X.y, X.z, 0.0f,
    //    Y.x, Y.y, Y.z, 0.0f,
    //    Z.x, Z.y, Z.z, 0.0f,
    //    P.x, P.y, P.z, 1.0f
    //);

    m_View = glm::lookAt(position, position + m_Forward, glm::vec3(0, 1, 0));
    m_InverseView = glm::inverse(m_View);
}

void Camera::SetFov(float HorizontalFOV) 
    { 
        FOV = HorizontalFOV;
    }
