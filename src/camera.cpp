#include "camera.h"

#define PI 3.14159f

Camera::Camera(float verticalFOV, float nearClip, float farClip)
    :
    m_VerticalFOV(verticalFOV), 
    m_NearClip(nearClip), 
    m_FarClip(farClip)
{
    m_ForwardDirection = glm::vec3(0, 0, -1);
    m_MatOrientation[0] = m_ForwardDirection;
    m_MatOrientation[1] = glm::vec3(0, 1, 0);
    m_MatOrientation[2] = glm::cross(m_ForwardDirection, glm::vec3(0, 1, 0));
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
    m_MatOrientation[0] = m_ForwardDirection;
    m_MatOrientation[1] = glm::vec3(0, 1, 0);
    m_MatOrientation[2] = glm::cross(m_ForwardDirection, glm::vec3(0, 1, 0));
}

Camera::Camera(glm::vec3 position, glm::vec3 forwardDirection, float verticalFOV, float nearClip, float farClip)
    :
    m_VerticalFOV(verticalFOV), 
    m_NearClip(nearClip), 
    m_FarClip(farClip),
    m_Position(position),
    m_ForwardDirection(forwardDirection)
{
    m_MatOrientation[0] = forwardDirection;
    m_MatOrientation[1] = glm::vec3(0, 1, 0);
    m_MatOrientation[2] = glm::cross(m_ForwardDirection, glm::vec3(0, 1, 0));
}

bool Camera::Orbital(float dt, Window* window)
{
    return false;
}

bool Camera::Cinematic(float dt, Window* window)
{
    GLFWwindow* glfw_win = window->GetWindow();

    float speed = 5.0f;

    // Rotation delta
    glm::vec3 dr {0.0f}; // {yaw, pitch, roll}

    // dr.x = (glfwGetKey(glfw_win, GLFW_KEY_RIGHT) == GLFW_PRESS) - (glfwGetKey(glfw_win, GLFW_KEY_LEFT) == GLFW_PRESS);
    // dr.y = (glfwGetKey(glfw_win, GLFW_KEY_UP) == GLFW_PRESS) - (glfwGetKey(glfw_win, GLFW_KEY_DOWN) == GLFW_PRESS);

    dr.x = (glfwGetKey(glfw_win, GLFW_KEY_SEMICOLON) == GLFW_PRESS) - (glfwGetKey(glfw_win, GLFW_KEY_K) == GLFW_PRESS);
    dr.y = (glfwGetKey(glfw_win, GLFW_KEY_O) == GLFW_PRESS) - (glfwGetKey(glfw_win, GLFW_KEY_L) == GLFW_PRESS);

    dr.z = (glfwGetKey(glfw_win, GLFW_KEY_E) == GLFW_PRESS) - (glfwGetKey(glfw_win, GLFW_KEY_Q) == GLFW_PRESS);

    m_RotationMomentum = (m_RotationMomentum * damping) + (dr * (1-damping));

    float rlen = glm::sqrt(glm::dot(m_RotationMomentum, m_RotationMomentum));
    if (rlen > 1e-3f) // still rotating?
    {
        // Construct rotation delta quaternion (q) relative to the current camera orientation
        // by multiplying the rotation momentum vector (m_RotationMomentum) with the current orientation matrix
        glm::quat yaw = glm::normalize(glm::angleAxis(m_RotationMomentum.x * dt, glm::vec3(0, 1, 0)));
        glm::quat pitch = glm::normalize(glm::angleAxis(m_RotationMomentum.z * dt, glm::vec3(1, 0, 0)));
        glm::quat roll = glm::normalize(glm::angleAxis(m_RotationMomentum.y * dt, glm::vec3(0, 0, 1)));

        // float theta = rlen * 0.03f;
        // float cosine_t = glm::cos(theta * 0.5f);
        // float sine_t = glm::sin(theta * 0.5f) / rlen;
        
        // glm::quat q = glm::qua(cosine_t, 
        //                         sine_t * (glm::dot(m_RotationMomentum, m_MatOrientation[0])),
        //                         sine_t * (glm::dot(m_RotationMomentum, m_MatOrientation[1])),
        //                         sine_t * (glm::dot(m_RotationMomentum, m_MatOrientation[2])));

        // Update the quaternion orientation (m_QuatOrientation) by multiplying by the rotation delta quaternion (q)
        //m_QuatOrientation = roll * pitch * yaw * m_QuatOrientation;
        // m_QuatOrientation = glm::normalize(q) * m_QuatOrientation;
        glm::quat q = roll * pitch * yaw;

        // Convert the quaternion orientation into rotation matrix
        //m_MatOrientation = glm::toMat3(m_QuatOrientation);
        m_MatOrientation[0] = glm::rotate(q, m_MatOrientation[0]);
        m_MatOrientation[2] = glm::cross(m_MatOrientation[0], m_MatOrientation[1]);

        // Recalculate view matrix with new orientation
        RecalculateView();
    }

    // Movement delta
    glm::vec3 dm {0.0f};

    dm.x = (glfwGetKey(glfw_win, GLFW_KEY_D) == GLFW_PRESS) - (glfwGetKey(glfw_win, GLFW_KEY_A) == GLFW_PRESS);
    dm.y = (glfwGetKey(glfw_win, GLFW_KEY_SPACE) == GLFW_PRESS) - (glfwGetKey(glfw_win, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS);
    dm.z = (glfwGetKey(glfw_win, GLFW_KEY_W) == GLFW_PRESS) - (glfwGetKey(glfw_win, GLFW_KEY_S) == GLFW_PRESS);
    float mlen = glm::sqrt(glm::dot(dm, dm)) / speed; 
    if (mlen < 1e-3f) mlen = 1.0f;

    glm::vec3 f, u, r;
    f = m_MatOrientation[0];
    u = m_MatOrientation[1];
    r = m_MatOrientation[2];

    m_MovementMomentum.x = m_MovementMomentum.x * damping + (1-damping) * (r.x*dm.x + r.y*dm.y + r.z*dm.z)/mlen;
    m_MovementMomentum.y = m_MovementMomentum.y * damping + (1-damping) * (u.x*dm.x + u.y*dm.y + u.z*dm.z)/mlen;
    m_MovementMomentum.z = m_MovementMomentum.z * damping + (1-damping) * (f.x*dm.x + f.y*dm.y + f.z*dm.z)/mlen;

    m_Position += m_MovementMomentum * dt;

    mlen = glm::sqrt(glm::dot(m_MovementMomentum, m_MovementMomentum));
    rlen = glm::sqrt(glm::dot(m_RotationMomentum, m_RotationMomentum));
    if (mlen < 1e-1f) m_MovementMomentum = {0.0f, 0.0f, 0.0f};
    if (rlen < 1e-1f) m_RotationMomentum = {0.0f, 0.0f, 0.0f};
    if (mlen > 0.0 || rlen > 0.0) return true;
    return false;
}

bool Camera::FPS(float dt, Window* window)
{   
    GLFWwindow* glfw_win = window->GetWindow();

    if(glfwGetKey(glfw_win, GLFW_KEY_1) == GLFW_PRESS)
    {
        glfwSetInputMode(glfw_win, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        m_AllowCameraToMove = false;
    }

    if(glfwGetKey(glfw_win, GLFW_KEY_2) == GLFW_PRESS)
    {
        glfwSetInputMode(glfw_win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        m_AllowCameraToMove = true;
        m_FirstMouse = true;
    }

    if (m_AllowCameraToMove)
    {
        float speed = 5.0f;

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

        delta = (cursor_pos - m_LastMousePosition) * 0.005f;
        m_LastMousePosition = cursor_pos;

        float dx = delta.x;
        float dy = delta.y;

        if (dx != 0.0f || dy != 0.0f)
        {
            // Rotate and clamp rotation speed
            glm::quat pitch_quat = glm::angleAxis(-dy * 0.3f, m_MatOrientation[2]);
            glm::quat yaw_quat = glm::angleAxis(-dx * 0.3f, glm::vec3(0, 1, 0));

            // Delta rotation quat
            //glm::quat q = glm::normalize(glm::cross(pitch_quat, yaw_quat));
            glm::quat q = glm::normalize(pitch_quat * yaw_quat);

            //glm::quat delta = glm::mix(glm::quat(0,0,0,0), q, dt);

            //m_QuatOrientation = q * m_QuatOrientation;

            //m_MatOrientation = glm::toMat3(m_QuatOrientation);
            m_MatOrientation[0] = glm::rotate(q, m_MatOrientation[0]);
            m_MatOrientation[2] = glm::cross(m_MatOrientation[0], m_MatOrientation[1]);

            RecalculateView();
        }

        // Momentum delta
        glm::vec3 M {0.0f, 0.0f, 0.0f};

        M.x = (glfwGetKey(glfw_win, GLFW_KEY_D)     == GLFW_PRESS) - (glfwGetKey(glfw_win, GLFW_KEY_A)          == GLFW_PRESS);
        M.y = (glfwGetKey(glfw_win, GLFW_KEY_SPACE) == GLFW_PRESS) - (glfwGetKey(glfw_win, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS);
        M.z = (glfwGetKey(glfw_win, GLFW_KEY_W)     == GLFW_PRESS) - (glfwGetKey(glfw_win, GLFW_KEY_S)          == GLFW_PRESS);
        float mlen = glm::sqrt(glm::dot(M, M)) / speed; 
        if (mlen < 1e-3f) mlen = 1.0f;

        glm::vec3 f, u, r;
        f = m_MatOrientation[0];
        u = m_MatOrientation[1];
        r = m_MatOrientation[2];

        m_MovementMomentum.x = m_MovementMomentum.x * damping + (1-damping) * (r.x * M.x + r.y * M.y + r.z * M.z) / mlen;
        m_MovementMomentum.y = m_MovementMomentum.y * damping + (1-damping) * (u.x * M.x + u.y * M.y + u.z * M.z) / mlen;
        m_MovementMomentum.z = m_MovementMomentum.z * damping + (1-damping) * (f.x * M.x + f.y * M.y + f.z * M.z) / mlen;

        m_Position += m_MovementMomentum * dt;
        if (glm::sqrt(glm::dot(m_MovementMomentum, m_MovementMomentum)) < 1e-1f) m_MovementMomentum = {0.0f, 0.0f, 0.0f};
        if (glm::sqrt(glm::dot(m_MovementMomentum, m_MovementMomentum)) > 0.0 || dx != 0.0 || dy != 0.0) return true;
    }
    return false;
}

void Camera::OnResize(uint width, uint height)
{
    if (width == m_ViewportWidth && height == m_ViewportHeight)
        return;

    m_ViewportWidth = width;
    m_ViewportHeight = height;
    m_AspectRatio = (float) width / height;

    RecalculateProjection();
}

void Camera::RecalculateProjection()
{
    m_Projection = glm::perspectiveFov(glm::radians(m_VerticalFOV), (float)m_ViewportWidth, (float)m_ViewportHeight, m_NearClip, m_FarClip);
	m_InverseProjection = glm::inverse(m_Projection);
}

void Camera::RecalculateView()
{
    m_View = glm::lookAt(m_Position, m_Position + m_MatOrientation[0], m_MatOrientation[1]);
    m_InverseView = glm::inverse(m_View);
}
