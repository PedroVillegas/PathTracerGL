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
    Camera()
        : position(glm::vec3(0))
        , type(0)
        , FOV(60.0f)
        , damping(4.0f)
        , focal_length(1.0f)
        , aperture(0.0f)
        , sensitivity(40.0f)
        , walkingSpeed(20.0f)
        , sprintingSpeed(60.0f)
        , slowSpeed(5.0f)
        , m_NearClip(0.1f)
        , m_FarClip(1000.0f)
        , m_AspectRatio(0.0f)
        , m_Projection(glm::mat4(1.0f))
        , m_View(glm::mat4(1.0f))
        , m_InverseProjection(glm::mat4(1.0f))
        , m_InverseView(glm::mat4(1.0f))
        , m_QuatOrientation(glm::quatLookAt(glm::vec3(0, 0, -1), glm::vec3(0, 1, 0)))
        , m_Velocity(glm::vec3(0))
        , m_RotVelocity(glm::vec3(0))
        , m_Forward(glm::vec3(0, 0, -1))
        , m_Up(glm::vec3(0, 1, 0))
        , m_Right(glm::vec3(1, 0, 0))
        , m_Yaw(0.0f)
        , m_Pitch(0.0f)
        , m_LastMousePosition(glm::vec2(0))
        , m_LastFOV(0.0f)
        , m_AllowCameraToMove(false)
        , m_FirstMouse(true)
        , m_ViewportWidth(0)
        , m_ViewportHeight(0)
    {};

    bool Orbital(float dt, Window* window);
    bool Cinematic(float dt, Window* window);
    bool FPS(float dt, Window* window);
    bool OnUpdate(float dt, Window* window);
    void OnResize(uint32_t width, uint32_t height);
    void Reset();

    void UpdateParams();
    void SetFov(float HorizontalFOV);
    void SetupCamera(glm::vec3 position, glm::vec3 forward, float fov);
    const glm::mat4& GetProjection() const { return m_Projection; }
    const glm::mat4& GetInverseProjection() const { return m_InverseProjection; }
    const glm::mat4& GetView() const { return m_View; }
    const glm::mat4& GetInverseView() const { return m_InverseView; }

    //const glm::vec3& GetRotationMomentum() const { return m_RotationMomentum; }
    const glm::vec3& GetVelocity() const { return m_Velocity; }
    //const glm::vec3& GetDirection() { return m_QuatOrientation * glm::vec3(0, 0, -1); };
    const glm::vec3& GetDirection() { return m_Forward; };

    void RecalculateProjection();
    void RecalculateView();

    CameraBlock params;

    glm::vec3 position;

    int type;
    float FOV;
    float damping;
    float focal_length;
    float aperture;
    float sensitivity;
    float walkingSpeed;
    float sprintingSpeed;
    float slowSpeed;

private:
    float m_NearClip;
    float m_FarClip;
    float m_AspectRatio;

    glm::mat4 m_Projection;
    glm::mat4 m_View;
    glm::mat4 m_InverseProjection;
    glm::mat4 m_InverseView;

    glm::quat m_QuatOrientation;

    glm::vec3 m_Velocity;
    glm::vec3 m_RotVelocity;
    glm::vec3 m_Forward;
    glm::vec3 m_Right;
    glm::vec3 m_Up;

    float m_Yaw;
    float m_Pitch;

    glm::vec2 m_LastMousePosition;
    float m_LastFOV;
    bool m_AllowCameraToMove;
    bool m_FirstMouse;

    uint32_t m_ViewportWidth;
    uint32_t m_ViewportHeight;
};
