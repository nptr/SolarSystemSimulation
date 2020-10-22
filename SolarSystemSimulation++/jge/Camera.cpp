#include "Camera.h"

// Math lib for matrices and vectors
#include <glm/gtc/matrix_transform.hpp>

// Math, for sin(), cos(), ...
#include <math.h>

// OpenGL
#include "../gl_core_3_3.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#define M_PI_2 1.57079632679489661923f
#endif

namespace jge
{

	Camera::Camera()
		: m_yaw(0.0f)
		, m_pitch(0.0f)
		, m_camMoveSpeed(0.3f)
		, m_camRotationSpeed(M_PI / 180 * 0.15f)
		, m_upVector(0.0f, 1.0f, 0.0f)
	{
		SetPosition(glm::vec3(0, 0, 0));
		SetPerspective(1.0f, 1.0f, 250.0f);
	}

	Camera::~Camera()
	{

	}

	void Camera::Refresh()
	{
		// x & y for horz. plane, z for height
		m_directionVector.x = cos(m_yaw) * cos(m_pitch);
		m_directionVector.y = sin(m_pitch);
		m_directionVector.z = sin(m_yaw) * cos(m_pitch);

		m_strafe_lx = cos(m_yaw - M_PI_2);
		m_strafe_lz = sin(m_yaw - M_PI_2);

		m_cameraViewMatrix = glm::lookAt(m_cameraPosition, m_cameraPosition + m_directionVector, m_upVector);
	}

	void Camera::SetPosition(const glm::vec3& pos)
	{
		m_cameraPosition = pos;

		Refresh();
	}

	void Camera::Move(float direction)
	{
		float lx = cos(m_yaw)*cos(m_pitch);
		float ly = sin(m_pitch);
		float lz = sin(m_yaw)*cos(m_pitch);

		m_cameraPosition.x += direction * m_camMoveSpeed*lx;
		m_cameraPosition.y += direction * m_camMoveSpeed*ly;
		m_cameraPosition.z += direction * m_camMoveSpeed*lz;

		Refresh();
	}

	void Camera::Strafe(float direction)
	{
		m_cameraPosition.x += direction * m_camMoveSpeed * m_strafe_lx;
		m_cameraPosition.z += direction * m_camMoveSpeed * m_strafe_lz;

		Refresh();
	}

	void Camera::Fly(float direction)
	{
		m_cameraPosition.y += direction * m_camMoveSpeed;

		Refresh();
	}

	void Camera::RotateYaw(float strokeIntensity)
	{
		m_yaw += m_camRotationSpeed * strokeIntensity;

		Refresh();
	}

	void Camera::RotatePitch(float strokeIntensity)
	{
		const float limit = 89.0f * M_PI / 180.0f;

		m_pitch += m_camRotationSpeed * strokeIntensity;

		if (m_pitch < -limit)
			m_pitch = -limit;

		if (m_pitch > limit)
			m_pitch = limit;

		Refresh();
	}

	void Camera::SetYaw(float angle)
	{
		m_yaw = angle;

		Refresh();
	}

	void Camera::SetPitch(float angle)
	{
		m_pitch = angle;

		Refresh();
	}

	void Camera::SetMoveSpeed(float moveSpeed)
	{
		m_camMoveSpeed = moveSpeed;
	}

	void Camera::SetTurnRate(float rotationSpeed)
	{
		m_camRotationSpeed = M_PI / 180.0f * rotationSpeed;
	}

	void Camera::SetPerspective(float ratio, float zNear, float zFar)
	{
		m_cameraProjectionMatrix = glm::perspective(glm::radians(50.0f), ratio, zNear, zFar);
	}

}