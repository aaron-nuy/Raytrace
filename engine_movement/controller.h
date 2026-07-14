#pragma once
#include <chrono>
#include "GLFW/rtre_window.h"
#include "../engine_rendering/camera.h"
#include "../rtre_base.h"


namespace rtre {
	typedef uint64_t rTtime;

	inline double getTime()
	{
		return glfwGetTime();
	}

	namespace controller {
		inline double lastTime, crntTime, deltaTime;
		static const float mSensitivity = 7.0f;
		static bool firstCall = true;
		static const glm::vec2 sensitivityModifier(100, 50);
		static int mouseMode = 0;
		inline glm::vec2 prevCursorPos(0, 0);
		inline glm::vec2 crntCursorPos(0, 0);
		inline glm::vec2 cursorDelta(0, 0);

		inline void control()
		{
			crntTime = getTime();
			if (firstCall)
			{
				firstCall = false;
				lastTime = crntTime;
				eWindow->setInputMode(GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				eWindow->setCursorPosition(viewportWidth / 2, viewportHeight / 2);
				crntCursorPos = glm::vec2(viewportWidth / 2, viewportHeight / 2);
				prevCursorPos = glm::vec2(viewportWidth / 2, viewportHeight / 2);
			}

			deltaTime = crntTime - lastTime;
			lastTime = crntTime;

			auto crntCursorPos_PlaceHolder = eWindow->getCursorPosition();
			crntCursorPos = glm::vec2(crntCursorPos_PlaceHolder.x, crntCursorPos_PlaceHolder.y);

			cursorDelta = -mSensitivity * ((crntCursorPos - prevCursorPos) / sensitivityModifier);

			mouseMode = glfwGetInputMode(eWindow->getWindow(), GLFW_CURSOR);

			glm::vec3 baseSpeed = camera.speed();
			camera.setSpeed(baseSpeed * (float) deltaTime);

			// Guards so camera doesn't get cuckery

			if ((camera.orientation().y <= -0.99f) && cursorDelta.y < 0)
			{
				cursorDelta.y = 0;
				camera.setOrientation(glm::vec3(camera.orientation().x, -0.99f, camera.orientation().z));
			} else if ((camera.orientation().y >= 0.99f) && cursorDelta.y > 0)
			{
				cursorDelta.y = 0;
				camera.setOrientation(glm::vec3(camera.orientation().x, 0.99f, camera.orientation().z));
			}


			if (eWindow->isKeyPressed(GLFW_KEY_SPACE))
			{
				camera.moveUp();
			} else if (eWindow->isKeyPressed(GLFW_KEY_LEFT_CONTROL))
			{
				camera.moveDown();
			}
			if (eWindow->isKeyPressed(GLFW_KEY_W))
			{
				camera.moveForward();
			} else if (eWindow->isKeyPressed(GLFW_KEY_S))
			{
				camera.moveBackward();
			}
			if (eWindow->isKeyPressed(GLFW_KEY_A))
			{
				camera.moveLeft();
			} else if (eWindow->isKeyPressed(GLFW_KEY_D))
			{
				camera.moveRight();
			}


			if (mouseMode == GLFW_CURSOR_DISABLED)
			{
				//	Rotate World around the vertical axis
				camera.setOrientation(glm::rotate(camera.orientation(), glm::radians(GLfloat(cursorDelta.x)),
				                                  camera.upDirection()));
				//	Rotate World around the horizontal axis
				camera.setOrientation(
					glm::normalize(camera.orientation() + glm::vec3(0.0f, glm::radians(cursorDelta.y), 0.0f)));
			}
			prevCursorPos = crntCursorPos;
			camera.setSpeed(baseSpeed);
		}
	}
}
