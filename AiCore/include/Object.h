#pragma once 
#ifndef OBJECT_H

#include <string>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <Texture.h>
#include <Shader.h>

namespace Ai {
	class AiObject {
	private:
		unsigned int m_objectId;
		std::string m_objectName;
	protected:
		glm::vec3 m_translate;
		glm::vec3 m_rotate;
		glm::vec3 m_scale;
	public:
		AiObject() = delete;

		AiObject(unsigned int id, std::string name): 
			m_objectId(id), 
			m_objectName(name), 
			m_translate(0.0f),
			m_rotate(0.0f),
			m_scale(1.0f)
		{}

		virtual ~AiObject() {}

		unsigned int getObjectId();

		std::string getObjectName();

		virtual void draw() = 0;

		virtual glm::vec3& getTranslate();

		virtual glm::vec3& getRotate();
		
		virtual glm::vec3& getScale();
	};

	class AiTexQuadObject : public AiObject {
	private:
		unsigned int m_VAO;
		unsigned int m_VBO;
		unsigned int m_EBO;

		// TODO:: Optimize by singleton mode.
		unsigned int m_shaderID;

		std::string m_imgPath;
		std::shared_ptr<Texture2D> m_texture;
	public:
		AiTexQuadObject() = delete;

		AiTexQuadObject(unsigned int id, std::string name, std::string imgPath);
		
		void init();

		virtual void draw() override;

		virtual ~AiTexQuadObject(){}
	private:
		unsigned int initShader();
	public:
		static constexpr float m_vertices[20] = {
			 0.5f,  0.5f, 0.0f, 1.0f, 1.0f, // top right
			 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, // bottom right
			-0.5f, -0.5f, 0.0f, 0.0f, 0.0f, // bottom left
			-0.5f,  0.5f, 0.0f, 0.0f, 1.0f  // top left 
		};

		static constexpr unsigned int m_indices[6] = {
			0, 1, 3, // first triangle
			1, 2, 3  // second triangle
		};

		// Shader source code.
		static constexpr char* vertexShaderCode = "#version 330 core\n"
			"layout (location = 0) in vec3 aPos;\n"
			"layout (location = 1) in vec2 aTex;\n"
			"uniform mat4 transform;\n"
			"out vec2 TexCoord;\n"
			"void main()\n"
			"{\n"
			"   gl_Position = transform * vec4(aPos, 1.0);\n"
			"   TexCoord = aTex;\n"
			"}\0";
		static constexpr char* fragmentShaderCode = "#version 330 core\n"
			"out vec4 FragColor;\n"
			"in vec2 TexCoord;\n"
			"uniform sampler2D tex1;\n"
			"void main()\n"
			"{\n"
			"   FragColor = texture(tex1, TexCoord);\n"
			"}\n\0";
	};
}
#endif