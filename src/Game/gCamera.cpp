#include "stdafx.h"
#include "gCamera.h"
//-----------------------------------------------------------------------------
void game::Camera::Update(float deltaTime)
{
	if (type == CameraType::firstperson)
	{
		if (IsMoving())
		{
			glm::vec3 camFront = {
				-cos(glm::radians(rotation.x)) * sin(glm::radians(rotation.y)),
				sin(glm::radians(rotation.x)),
				cos(glm::radians(rotation.x))* cos(glm::radians(rotation.y))
			};
			camFront = glm::normalize(camFront);

			const float moveSpeed = deltaTime * movementSpeed;

			if (keys.up) position += camFront * moveSpeed;
			if (keys.down) position -= camFront * moveSpeed;
			if (keys.left) position -= glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f))) * moveSpeed;
			if (keys.right) position += glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f))) * moveSpeed;
		}
	}
	updateViewMatrix();
}
//-----------------------------------------------------------------------------
void game::Camera::OnKeyPressed(uint32_t key)
{
	if (key == KEY_F2)
	{
		if (type == game::Camera::lookat)
			type = game::Camera::firstperson;
		else
			type = game::Camera::lookat;
	}
	else
	{
		if (type == Camera::firstperson)
		{
			switch (key)
			{
			case KEY_W:
				keys.up = true;
				break;
			case KEY_S:
				keys.down = true;
				break;
			case KEY_A:
				keys.left = true;
				break;
			case KEY_D:
				keys.right = true;
				break;
			}
		}
	}
}
//-----------------------------------------------------------------------------
void game::Camera::OnKeyUp(uint32_t key)
{
	if (type == Camera::firstperson)
	{
		switch (key)
		{
		case KEY_W:
			keys.up = false;
			break;
		case KEY_S:
			keys.down = false;
			break;
		case KEY_A:
			keys.left = false;
			break;
		case KEY_D:
			keys.right = false;
			break;
		}
	}
}
//-----------------------------------------------------------------------------
void game::Camera::OnMouse(const MouseState& mouseState, int32_t dx, int32_t dy)
{
	if (mouseState.buttons.left)
	{
		Rotate(glm::vec3(dy * rotationSpeed, -dx * rotationSpeed, 0.0f));
	}
	if (mouseState.buttons.right)
	{
		Translate(glm::vec3(-0.0f, 0.0f, dy * .005f));
	}
	if (mouseState.buttons.middle)
	{
		Translate(glm::vec3(-dx * 0.005f, -dy * 0.005f, 0.0f));
	}
}
//-----------------------------------------------------------------------------
void game::Camera::SetPerspective(float fov, float aspect, float znear, float zfar)
{
	m_fov = fov;
	m_znear = znear;
	m_zfar = zfar;
	matrices.perspective = glm::perspective(glm::radians(m_fov), aspect, m_znear, m_zfar);
	if (flipY) matrices.perspective[1][1] *= -1.0f;
}
//-----------------------------------------------------------------------------
void game::Camera::UpdateAspectRatio(float aspect)
{
	matrices.perspective = glm::perspective(glm::radians(m_fov), aspect, m_znear, m_zfar);
	if (flipY) matrices.perspective[1][1] *= -1.0f;
}
//-----------------------------------------------------------------------------
void game::Camera::SetPosition(const glm::vec3& position)
{
	this->position = position;
	updateViewMatrix();
}
//-----------------------------------------------------------------------------
void game::Camera::SetRotation(const glm::vec3& rotation)
{
	this->rotation = rotation;
	updateViewMatrix();
}
//-----------------------------------------------------------------------------
void game::Camera::Translate(const glm::vec3& delta)
{
	this->position += delta;
	updateViewMatrix();
}
//-----------------------------------------------------------------------------
void game::Camera::Rotate(const glm::vec3& delta)
{
	this->rotation += delta;
	updateViewMatrix();
}
//-----------------------------------------------------------------------------
void game::Camera::updateViewMatrix()
{
	glm::mat4 rotM = glm::mat4(1.0f);
	rotM = glm::rotate(rotM, glm::radians(rotation.x * (flipY ? -1.0f : 1.0f)), glm::vec3(1.0f, 0.0f, 0.0f));
	rotM = glm::rotate(rotM, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	rotM = glm::rotate(rotM, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

	glm::vec3 translation = position;
	if (flipY) translation.y *= -1.0f;
	glm::mat4 transM = glm::translate(glm::mat4(1.0f), translation);

	if (type == CameraType::firstperson)
	{
		matrices.view = rotM * transM;
	}
	else
	{
		matrices.view = transM * rotM;
	}

	viewPos = glm::vec4(position, 0.0f) * glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f);
}
//-----------------------------------------------------------------------------