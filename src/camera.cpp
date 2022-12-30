#include "camera.h"

Camera::Camera(float verticalFOV, float nearClip, float farClip)
    :
    m_VerticalFOV(verticalFOV), 
    m_NearClip(nearClip), 
    m_FarClip(farClip)
{
    m_ForwardDirection = glm::vec3(0, 0, -1);
    m_Position = glm::vec3(0, 0, 3);
}

Camera::Camera(glm::vec3 position, float verticalFOV, float nearClip, float farClip)
    :
    m_VerticalFOV(verticalFOV), 
    m_NearClip(nearClip), 
    m_FarClip(farClip),
    m_Position(position)
{
    m_ForwardDirection = glm::vec3(0, 0, -1);
}

Camera::Camera(glm::vec3 position, glm::vec3 forwardDirection, float verticalFOV, float nearClip, float farClip)
    :
    m_VerticalFOV(verticalFOV), 
    m_NearClip(nearClip), 
    m_FarClip(farClip),
    m_Position(position),
    m_ForwardDirection(forwardDirection)
{
}

void Camera::MomentumMovement(float dt, GLFWwindow* window)
{
    float speed = 5.0f;
    constexpr glm::vec3 upDirection(0.0f, 1.0f, 0.0f);
    glm::vec3 rightDirection = glm::cross(m_ForwardDirection, upDirection);

    // Momentum delta
    glm::vec3 M {0.0f, 0.0f, 0.0f};
    M.x = (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) - (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS);
    M.y = (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) - (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS);
    M.z = (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) - (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS);
    float mlen = glm::sqrt(glm::dot(M, M)) / speed; if (mlen < 1e-3f) mlen = 1.0f;

    glm::vec3 r = rightDirection, u = upDirection, f = m_ForwardDirection;

    m_Momentum.x = m_Momentum.x * .9f + .1f * (r.x*M.x + r.y*M.y + r.z*M.z)/mlen;
    m_Momentum.y = m_Momentum.y * .9f + .1f * (u.x*M.x + u.y*M.y + u.z*M.z)/mlen;
    m_Momentum.z = m_Momentum.z * .9f + .1f * (f.x*M.x + f.y*M.y + f.z*M.z)/mlen;

    m_Position += m_Momentum * dt;
}

void Camera::OnUpdate(float dt, GLFWwindow* window)
{   
    glm::vec2 delta {0.0f};
    if(glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        m_AllowCameraToMove = false;
    }

    if(glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        m_AllowCameraToMove = true;
    }

    if (m_AllowCameraToMove)
    {
        float speed = 5.0f;
        glm::vec3 upDirection(0.0f, 1.0f, 0.0f);
        glm::vec3 rightDirection = glm::cross(m_ForwardDirection, upDirection);

        // Momentum delta
        glm::vec3 M {0.0f, 0.0f, 0.0f};
        M.x = (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) - (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS);
        M.y = (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) - (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS);
        M.z = (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) - (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS);
        float mlen = glm::sqrt(glm::dot(M, M)) / speed; if (mlen < 1e-3f) mlen = 1.0f;

        glm::vec3 r = rightDirection, u = upDirection, f = m_ForwardDirection;

        m_Momentum.x = m_Momentum.x * .9f + .1f * (r.x * M.x + r.y * M.y + r.z * M.z) / mlen;
        m_Momentum.y = m_Momentum.y * .9f + .1f * (u.x * M.x + u.y * M.y + u.z * M.z) / mlen;
        m_Momentum.z = m_Momentum.z * .9f + .1f * (f.x * M.x + f.y * M.y + f.z * M.z) / mlen;

        m_Position += m_Momentum * dt;

        // Rotation

        m_Roll = m_Roll * .8f + .2f * ((glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) - (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS));
        
        if (m_Roll > 1e-3f)
        {
            float theta = m_Roll; //* .3f;// c = std::cos(theta * .5f), s = std::sin(theta * .5f) / m_Roll;
            glm::quat q = glm::normalize(glm::angleAxis(theta, m_ForwardDirection));
            rightDirection = glm::rotate(q, rightDirection);
            upDirection = glm::rotate(q, upDirection);
        }

        double mousePosX = 0.0f , mousePosY = 0.0f;
        glfwGetCursorPos(window, &mousePosX, &mousePosY);
        glm::vec2 mousePos {mousePosX, mousePosY};
        delta = (mousePos - m_LastMousePosition) * 0.005f;
        m_LastMousePosition = mousePos;
        
        if (delta.x != 0.0f || delta.y != 0.0f)
        {
            float pitchDelta = delta.y * GetRotationSpeed();
            float yawDelta = delta.x * GetRotationSpeed();

            glm::quat q = glm::normalize(glm::cross(glm::angleAxis(-pitchDelta, rightDirection),
                glm::angleAxis(-yawDelta, upDirection)));
            m_ForwardDirection = glm::rotate(q, m_ForwardDirection);
        }

        RecalculateView();
    }
}

void Camera::OnResize(uint width, uint height)
{
    if (width == m_ViewportWidth && height == m_ViewportHeight)
        return;

    m_ViewportWidth = width;
    m_ViewportHeight = height;

    // std::cout << m_ViewportWidth << ", " << m_ViewportHeight << std::endl;

    RecalculateProjection();
}

float Camera::GetRotationSpeed()
{
    return 0.3f;
}

void Camera::RecalculateProjection()
{
    m_Projection = glm::perspectiveFov(glm::radians(m_VerticalFOV), (float)m_ViewportWidth, (float)m_ViewportHeight, m_NearClip, m_FarClip);
	m_InverseProjection = glm::inverse(m_Projection);
}

void Camera::RecalculateView()
{
    m_View = glm::lookAt(m_Position, m_Position + m_ForwardDirection, glm::vec3(0, 1, 0));
    m_InverseView = glm::inverse(m_View);
}
