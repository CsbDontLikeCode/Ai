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
	void renderAiInit();

	void renderAi();

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

	void processInput(GLFWwindow* window);

	void mouse_callback(GLFWwindow* window, double xpos, double ypos);

	void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
}
#endif
