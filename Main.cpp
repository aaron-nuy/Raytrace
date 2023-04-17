#pragma once
#include <vector>
#include <filesystem>
#include <Windows.h>
#include <chrono>
#include <thread>
#include "rtre.h"
#include "GLFW/rtre_Window.h"
#include "engine_movement/controller.h"

#define LOG(x) std::cout << x << "\n"

void prv(glm::vec3 v) {
	std::cout << v.x << " " << v.y << " " << v.z << "\n";
}

float getTime() {
	using std::chrono::milliseconds;
	using std::chrono::duration_cast;
	return (float)duration_cast<milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

GLfloat random() {
	return rand() / 32767.0;
}

glm::mat4 matrix(const rtre::Camera& c) {
	auto view = glm::lookAt(c.position(), c.position() + c.orientation(), glm::vec3(0, 1, 0));
	return glm::inverse(view);
	
}

enum class ShapeType {
	eNONE = -1,
	eSphere,
	eBox
};

struct Hitdata {
	GLfloat distance;
	uint32_t index;
	ShapeType type;
};


Hitdata mymin(Hitdata p1, Hitdata p2) {
	if (p1.distance < 0.0)
		return p2;
	if (p2.distance < 0.0)
		return p1;
	if (p1.distance < p2.distance)
		return p1;
	return p2;
}

void sphereMenu(rtre::Sphere& sphere) {

	ImGui::SliderFloat("Sphere Radius", &sphere.radius, 0, 10);
	ImGui::SliderFloat3("Sphere Position", (float*)&sphere.position, -20, 20);
	ImGui::SliderFloat("Roughness", (float*)&sphere.material.roughness, 0.0, 1.0);
	ImGui::SliderFloat("Specular", (float*)&sphere.material.specular, 0.0, 1.0);
	ImGui::SliderFloat("Metalic", (float*)&sphere.material.metalic, 0.0, 1.0);
	ImGui::SliderFloat("Emissive", (float*)&sphere.material.emissive, 0.0, 1.0);

	ImGui::ColorEdit3("Material Color", (float*)& sphere.material.albedo);
}


void boxMenu(rtre::Box& box) {

	ImGui::SliderFloat3("Box Position", (float*)&box.position, -20, 20);
	ImGui::SliderFloat3("Box Dimensions", (float*)&box.dimensions, 0, 10);
	ImGui::SliderFloat("Roughness", (float*)&box.material.roughness, 0.0, 1.0);
	ImGui::SliderFloat("Specular", (float*)&box.material.specular, 0.0, 1.0);
	ImGui::SliderFloat("Metalic", (float*)&box.material.metalic, 0.0, 1.0);
	ImGui::SliderFloat("Emissive", (float*)&box.material.emissive, 0.0, 1.0);

	ImGui::ColorEdit3("Material Color", (float*)&box.material.albedo);
}

// don't flip the state of it, causes black screen?
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{	
	int cursorMode = glfwGetInputMode(window, GLFW_CURSOR);
	if (key == GLFW_KEY_F && action == GLFW_PRESS) {

		if (cursorMode == GLFW_CURSOR_DISABLED)
			cursorMode = GLFW_CURSOR_NORMAL;
		else
			cursorMode = GLFW_CURSOR_DISABLED;

		glfwSetInputMode(window, GLFW_CURSOR, cursorMode);
	}
}

int main()
{

	rtre::Window::init();
	rtre::Window::initHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	rtre::Window::initHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	rtre::Window::initHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	rtre::Window window(800, 480,"Engine");
	window.makeContextCurrent();
	rtre::init(800, 480, window,glm::vec3(5,5,5));

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window.getWindow(), true);
	ImGui_ImplOpenGL3_Init("#version 430");

	bool show_another_window = true;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	std::shared_ptr<rtre::RenderShader> shader = std::make_shared<rtre::RenderShader>("engine_resources\\vert.vert", "engine_resources\\frag.frag", "");
	rtre::Quad screen = rtre::Quad(shader);
	GLfloat fov = 75.f;

	
	float stime = getTime();
	
	Hitdata selectedShape = {-1,-1,ShapeType::eNONE};

	// Uniforms
	GLfloat aspectRatio;
	glm::vec4 matColor = glm::vec4(1.0, 0.32, 0.32, 1.0);
	glm::vec3 sphereloc = glm::vec3(0,0,3);
	glm::vec3 boxPos = glm::vec3(1, 1, 3);
	glm::vec3 boxBounds = glm::vec3(1, 1, 1);
	GLfloat sphereRadius = 0.5f;
	GLfloat speed = 5;
	GLfloat mytime = 0;
	glm::mat4 pmatrix;
	rtre::Material material = rtre::Material{
		glm::vec3(1.0f),
		0.1f,
		0.9f,
		0.0,
		1.0
	};
	GLuint bounces = 2;

	srand(time(0));

	for (int i = 0; i < 10; i++) {
		rtre::sphereList.emplace_back(
			new rtre::Sphere(
				glm::vec3(random()*10, random() * 10, random() * 10),
				random()*2,
				rtre::Material(
					glm::vec3(random(), random(), random()),
					random(),
					random(),
					0.0f,
					random()
				)
			)
		);

	}

	for (int i = 0; i < 10; i++) {
		rtre::boxList.emplace_back(
			new rtre::Box(
				glm::vec3(random()*10, random() * 10, random() * 10),
				glm::vec3(random() * 2, random() * 2, random() * 2),
				rtre::Material(
					glm::vec3(random(), random(), random()),
					random(),
					random(),
					0.0f,
					random()
				)
			)
		);

	}

	glViewport(0, 0, 800, 480);

	// Initialization
	GLuint pathTracerFBO, pathTracerTexture;

	// Create the FBO
	glGenFramebuffers(1, &pathTracerFBO);
	

	std::shared_ptr<rtre::RenderShader> mshader = std::make_shared<rtre::RenderShader>("engine_resources\\main.vert", "engine_resources\\main.frag", "");
	rtre::Quad secondscreen = rtre::Quad(mshader);

	float frameCounter = 1;

	GLuint lasty = 0;
	GLuint lastx = 0;
	unsigned long long count = 1;
	bool reset = false;
	glm::vec3 lastcampos, lastcamor;
	while (!window.shouldClose() && !window.isKeyPressed(GLFW_KEY_ESCAPE)) {
		count++;
		unsigned long long buff = 0;
		screen.m_Shader->checkAndHotplug();
		secondscreen.m_Shader->checkAndHotplug();
		rtre::Window::pollEvents();
		reset = false;
		int display_w ,display_h ;
		glfwGetFramebufferSize(window.getWindow(), &display_w, &display_h);


		if (lasty != display_h || lastx != display_w) {
			reset = true;
			frameCounter = 1;
			lastcampos = rtre::camera.position();
			lastcamor = rtre::camera.orientation();
			glBindFramebuffer(GL_FRAMEBUFFER, pathTracerFBO);
			glGenTextures(1, &pathTracerTexture);
			glBindTexture(GL_TEXTURE_2D, pathTracerTexture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, display_w, display_h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pathTracerTexture, 0);
			

			lasty = display_h;
			lastx = display_w;
			glViewport(0, 0, display_w, display_h);

		}
		glBindFramebuffer(GL_FRAMEBUFFER, pathTracerFBO);

		aspectRatio = aspectRatio = float(display_w) / display_h;
		float xCoord = (window.getCursorPosition().x / (float)display_w) * 2.0 - 1.0;
		float yCoord = (window.getCursorPosition().y / (float)display_h) * 2.0 - 1.0;

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		if (show_another_window)
		{
			ImGui::Begin("Control Panel", &show_another_window);

			if (selectedShape.distance > 0.0) {
				if(selectedShape.type == ShapeType::eSphere)
					sphereMenu(*(rtre::sphereList[selectedShape.index]));
				else if(selectedShape.type == ShapeType::eBox)
					boxMenu(*(rtre::boxList[selectedShape.index]));

				if (ImGui::Button("Delete Object")) {

					if (selectedShape.type == ShapeType::eSphere) {
						rtre::sphereList[selectedShape.index] = rtre::sphereList[rtre::sphereList.size() - 1];
						rtre::sphereList.pop_back();
					}
					else if (selectedShape.type == ShapeType::eBox) {
						rtre::boxList[selectedShape.index] = rtre::boxList[rtre::boxList.size() - 1];
						rtre::boxList.pop_back();
					}

					selectedShape.index = 0;

				}

			}
				

			ImGui::SliderFloat("Speed", (float*)&speed, 5, 100, "%.1f");

			ImGui::SliderInt("Bounces", (int*)&bounces, 0, 50);
			if (ImGui::Button("Reset position")) {
				rtre::camera.setPosition(glm::vec3(1));
				rtre::camera.setOrientation(glm::vec3(0,0,-1));
			}




			rtre::camera.setSpeed(glm::vec3(speed/1000000));

			ImGui::End();
		}



		if ( glm::length( lastcampos-rtre::camera.position() ) > 0.0001f  || glm::length(lastcamor - rtre::camera.orientation()) > 0.0001f || window.isClickingLeft()) {
			reset = true;
			frameCounter = 1;
			lastcampos = rtre::camera.position();
			lastcamor = rtre::camera.orientation();
		}

		pmatrix = matrix(rtre::camera);
		if (window.isClickingLeft()) {
			glm::vec2 coord = glm::vec2(xCoord*aspectRatio, -yCoord) / glm::vec2(2.0);
			glm::vec4 rd = pmatrix * glm::vec4(glm::normalize(glm::vec3(coord, -1)), 0);
			glm::vec3 rayDirection = glm::vec3(rd.x, rd.y, rd.z);
			rtre::Ray ray = rtre::Ray(rayDirection, rtre::camera.position());


			Hitdata data = { -1, -1, ShapeType::eNONE };
			for (uint32_t i = 0; i < rtre::sphereList.size(); i++) {
				data = mymin(data, Hitdata{ rtre::sphereList[i]->intersect(ray),i,ShapeType::eSphere });
			}

	
			for (uint32_t i = 0; i < rtre::boxList.size(); i++) {
				data = mymin(data, Hitdata{ rtre::boxList[i]->intersect(ray),i,ShapeType::eBox });
			}

			if (data.distance > 0.0) {
				selectedShape.index = data.index;
				selectedShape.type = data.type;
				selectedShape.distance = data.distance;
			}
		
			
		}


		screen.m_Shader->activate();
		screen.m_Shader->SetUniform("cameraPos", rtre::camera.position());
		screen.m_Shader->SetUniform("sphereNum", (uint32_t)rtre::sphereList.size());
		screen.m_Shader->SetUniform("boxNum", (uint32_t)rtre::boxList.size());
		screen.m_Shader->SetUniform("time", mytime);
		screen.m_Shader->SetUniform("aspec", aspectRatio);
		screen.m_Shader->SetUniform("sphereList", rtre::sphereList);
		screen.m_Shader->SetUniform("boxList", rtre::boxList);
		screen.m_Shader->SetUniform("matrix", pmatrix);
		screen.m_Shader->SetUniform("reset", reset);
		screen.m_Shader->SetUniform("counter", (float)(frameCounter));
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, pathTracerTexture);
		screen.m_Shader->SetUniform("text",0);
		glUniform1i(screen.m_Shader->getUnifromID("bounce"), bounces);
		screen.m_Shader->SetUniform("state", (uint32_t)rand());
		// Don't use uint
		screen.m_Shader->SetUniform("cubemap", (GLint)rtre::skyBox->unit());


		rtre::skyBox->bind();
		screen.draw();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		secondscreen.m_Shader->activate();
		secondscreen.m_Shader->SetUniform("aspec", aspectRatio);
		secondscreen.m_Shader->SetUniform("current", (GLint)0);
		secondscreen.m_Shader->SetUniform("prev", (GLint)1);
		secondscreen.m_Shader->SetUniform("counter", frameCounter);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, pathTracerTexture);

		secondscreen.draw();

		// Rendering
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSetKeyCallback(window.getWindow(), key_callback);

		mytime = getTime() - stime;

		frameCounter = frameCounter + 0.5;
		rtre::controller::control();
		//window.swapInterval(1);
		window.swapBuffers();
	}


	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	rtre::Window::terminate();
}
