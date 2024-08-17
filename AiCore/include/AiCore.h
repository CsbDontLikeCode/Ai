#pragma once
#ifndef AI_CORE
#define AI_CORE

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <memory>
#include <cmath>

#include <vector>
#include <map>
#include <string>

#include <Painter.h>
#include <Object.h>
#include <Camera.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Ai 
{
	struct AiEngineConfig
	{
		// RE Cinfog 1 >> Use off-screen rendering or not.
		bool offScreenRenderingFlag = false;
		bool antiAliasing = false;
		bool shadowMapping = true;
	};

	AiEngineConfig& getAiEngineConfig();

	void renderAiInit();

	void renderAi();

	void setDirLight(glm::vec3 direction, glm::vec3 color);
	void setDirLight(DirLight dirLight);

	SceneLight& getPointLight(int index);

	void addLineShader();
	void addPolygonShader();

	void addLine(unsigned int id, float startPointX, float startPointY, float endPointX, float endPointY);
	void addLine(unsigned int id, float startPointX, float startPointY, float endPointX, float endPointY, 
		float red, float green, float blue);

	void addTriangle(unsigned int id, float xscale, float yscale, float xpos, float ypos);
	void addTriangle(unsigned int id, float xscale, float yscale, float xpos, float ypos, float red, float green, float blue);

	void addSquare(unsigned int id, float xscale, float yscale, float xpos, float ypos);
	void addSquare(unsigned int id, float xscale, float yscale, float xpos, float ypos, float red, float green, float blue);

	void addCircle(unsigned int id, float xscale, float yscale, float xpos, float ypos);
	void addCircle(unsigned int id, float xscale, float yscale, float xpos, float ypos, float red, float green, float blue);

	std::shared_ptr<AiTexQuadObject> addTex(unsigned int id, std::string name, std::string imgPath);

	std::shared_ptr<AiPureCubeObject> addPureCube(unsigned int id);

	std::shared_ptr<AiQuad> addAiQuad(unsigned int id, std::shared_ptr<Shader> shader, std::shared_ptr<PointLight> lightPoint);

	std::shared_ptr<AiQuadLM> addAiQuadLM(unsigned int id, std::shared_ptr<Shader> shader, std::shared_ptr<PointLight> lightPoint, 
										  std::shared_ptr<Texture2D> diffuse, std::shared_ptr<Texture2D> specular);

	std::shared_ptr<AiQuadNormalMap> addAiQuadNormalMap(std::shared_ptr<Shader> shader, 
														std::shared_ptr<Texture2D> diffuseMap,
														std::shared_ptr<Texture2D> normalMap);

	std::shared_ptr<ModelObj> addModelObj(std::string path, std::shared_ptr<Shader> shader);

	std::shared_ptr<TranslucentAiQuad> addTranslucentAiQuad(std::shared_ptr<Shader> shader);

	void processInput(GLFWwindow* window);

	void mouse_callback(GLFWwindow* window, double xpos, double ypos);

	void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
}
#endif
