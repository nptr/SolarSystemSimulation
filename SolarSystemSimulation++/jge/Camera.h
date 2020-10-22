/*
Generic camera class by Nghia Ho
Modified for usage in modern OpenGL
*/

#ifndef __CAMERA_H__
#define __CAMERA_H__

#include <glm/glm.hpp>

namespace jge
{
	class Camera
	{
	public:
		Camera();
		~Camera();

		void SetPerspective(float ratio, float nearPlane, float farPlane);

		void SetPosition(const glm::vec3& pos);
		inline glm::vec3 GetPosition() const { return m_cameraPosition; }
		
		void SetMoveSpeed(float moveSpeed);
		inline float GetMoveSpeed() const { return m_camMoveSpeed; }

		void SetTurnRate(float rotationSpeed);
		inline float GetTurnRate() const { return m_camRotationSpeed; }

		void SetYaw(float angle);
		void SetPitch(float angle);
		
		inline glm::vec3 GetDirectionVector() const { return m_directionVector; }
		inline glm::mat4 GetProjectionMatrix() const { return m_cameraProjectionMatrix; }
		inline glm::mat4 GetViewMatrix() const { return m_cameraViewMatrix; }
		
		/**
		* From the cameras POV, moves forward or backward.
		* @param increment Amount to move
		*/
		void Move(float increment);

		/**
		* From the cameras POV, moves sideways.
		* @param increment Amount to move
		*/
		void Strafe(float increment);

		/**
		* From the cameras POV, move up or down..
		* @param increment Amount to move
		*/
		void Fly(float increment);

		/*
		* Rotate the camera on the yaw axis.
		* @param strokeIntensity Represents the strength of the cursor deflection
		* Valid values are between -1.0f ... 1.0f. (+/- direction, value as strength)
		* rotation = turnrate * strokeIntensity
		*/
		void RotateYaw(float strokeIntensity);

		/*
		* Rotate the camera on the pitch axis.
		* @param strokeIntensity Represents the strength of the cursor deflection
		* Valid values are between -1.0f ... 1.0f. (+/- direction, value as strength)
		* rotation = turnrate * strokeIntensity
		*/
		void RotatePitch(float strokeIntensity);

	private:
		void Refresh();

		float m_yaw, m_pitch;
		float m_strafe_lx, m_strafe_lz; // Always 90 degree to direction vector

		float m_camMoveSpeed;
		float m_camRotationSpeed;

		glm::vec3 m_cameraPosition;
		glm::vec3 m_directionVector;
		glm::vec3 m_upVector;

		glm::mat4 m_cameraProjectionMatrix;
		glm::mat4 m_cameraViewMatrix;
	};
}
#endif
