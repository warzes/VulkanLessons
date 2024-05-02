#pragma once

namespace game
{
	class Camera final
	{
	public:
		void Update(float deltaTime);
		void OnKeyPressed(uint32_t key);
		void OnKeyUp(uint32_t key);
		void OnMouse(const MouseState& mouseState, int32_t dx, int32_t dy);

		void SetPerspective(float fov, float aspect, float znear, float zfar);
		void UpdateAspectRatio(float aspect);

		void SetPosition(const glm::vec3& position);
		void SetRotation(const glm::vec3& rotation);

		void Rotate(const glm::vec3& delta);
		void Translate(const glm::vec3& delta);

		void SetRotationSpeed(float rotationSpeed) { this->rotationSpeed = rotationSpeed; }
		void SetMovementSpeed(float movementSpeed) { this->movementSpeed = movementSpeed; }

		bool IsMoving() const { return keys.left || keys.right || keys.up || keys.down; }

		float GetNearClip() const { return m_znear; }
		float GetFarClip() const { return m_zfar; }

		enum CameraType { lookat, firstperson };
		CameraType type = CameraType::lookat;

		glm::vec3 rotation = glm::vec3();
		glm::vec3 position = glm::vec3();
		glm::vec4 viewPos = glm::vec4();

		float rotationSpeed = 1.0f;
		float movementSpeed = 1.0f;

		bool flipY = false;

		struct
		{
			glm::mat4 perspective;
			glm::mat4 view;
		} matrices;

		struct
		{
			bool left = false;
			bool right = false;
			bool up = false;
			bool down = false;
		} keys;

	private:
		void updateViewMatrix();
		float m_fov;
		float m_znear, m_zfar;
	};
}