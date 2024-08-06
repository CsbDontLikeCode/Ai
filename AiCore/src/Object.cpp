#include "Object.h"

namespace Ai {
	extern Camera camera;
	// 1.Directional light
	extern DirLight g_dirLight;
	// 2.Point light container
	//extern const int maxPointLightNum;
	extern std::vector<SceneLight> g_pointLights;

	// ----------------------------------------------------------------------------------------------------
	// Class PointLight
	PointLight::PointLight(glm::vec3 position, glm::vec3 color)
	{
		m_position = position;
		m_color = color;
	}

	glm::vec3& PointLight::getPosition()
	{
		return m_position;
	}

	void PointLight::setPosition(glm::vec3 position)
	{
		m_position = position;
	}

	glm::vec3& PointLight::getColor()
	{
		return m_color;
	}

	void PointLight::setColor(glm::vec3 color)
	{
		m_color = color;
	}
	// ====================================================================================================

	unsigned int AiObject::getObjectId() 
	{
		return m_objectId;
	}

	std::string AiObject::getObjectName() 
	{
		return m_objectName;
	}

	glm::vec3& AiObject::getTranslate() 
	{
		return m_translate;
	}

	glm::vec3& AiObject::getRotate() 
	{
		return m_rotate;
	}

	glm::vec3& AiObject::getScale() 
	{
		return m_scale;
	}

	glm::mat4& AiObject::getView()
	{
		return m_view;
	}

	glm::mat4& AiObject::getProjection()
	{
		return m_projection;
	}

	// ----------------------------------------------------------------------------------------------------
	// Class AiTexQuadObject
	unsigned int AiTexQuadObject::m_shaderID = 0;
	bool AiTexQuadObject::m_shaderIsGenerated = false;

	AiTexQuadObject::AiTexQuadObject(unsigned int id, std::string name, std::string imgPath) 
        : AiObject(id, name), m_imgPath(imgPath)
    {
        // Generate vertex array object.
        init();
        // Init the only texture.
        m_texture = std::make_shared<Texture2D>(m_imgPath);
		// Init the shader.
		if (!m_shaderIsGenerated) {
			m_shaderID = initShader();
			m_shaderIsGenerated = true;
		}
	}

	AiTexQuadObject::~AiTexQuadObject()
	{
		glDeleteVertexArrays(1, &m_VAO);
		glDeleteBuffers(1, &m_VBO);
		glDeleteBuffers(1, &m_EBO);
	}

	void AiTexQuadObject::init() 
	{
        glGenVertexArrays(1, &m_VAO);
        glGenBuffers(1, &m_VBO);
        glGenBuffers(1, &m_EBO);
        glBindVertexArray(m_VAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(m_vertices), m_vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(m_indices), m_indices, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
	}

	void AiTexQuadObject::draw() 
	{
		// Active texture0 slot and bind m_texture.
		glActiveTexture(GL_TEXTURE0);
		m_texture->bind();
		// Use shader.
		glUseProgram(m_shaderID);

		//----------------------------------------------------------------------------------------------------
		// ::Beta Code
		glm::mat4 model(1.0f);
		model = glm::translate(model, m_translate);
		model = glm::rotate(model, glm::radians(m_rotate.x), glm::vec3(1.0, 0.0, 0.0));
		model = glm::rotate(model, glm::radians(m_rotate.y), glm::vec3(0.0, 1.0, 0.0));
		model = glm::rotate(model, glm::radians(m_rotate.z), glm::vec3(0.0, 0.0, 1.0));
		model = glm::scale(model, m_scale);
		unsigned int modelLoc = glGetUniformLocation(m_shaderID, "model");
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		unsigned int viewLoc = glGetUniformLocation(m_shaderID, "view");
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(m_view));
		unsigned int projectionLoc = glGetUniformLocation(m_shaderID, "projection");
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(m_projection));
		//----------------------------------------------------------------------------------------------------
		
		// Bind m_VAO.
		glBindVertexArray(m_VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	}

    unsigned int AiTexQuadObject::initShader() noexcept
	{
		unsigned int vertexShader;
		vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &vertexShaderCode, NULL);
		glCompileShader(vertexShader);
		int  success;
		char infoLog[512];
		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
		}
		unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &fragmentShaderCode, NULL);
		glCompileShader(fragmentShader);
		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
		}
		unsigned int shaderProgram = glCreateProgram();
		glAttachShader(shaderProgram, vertexShader);
		glAttachShader(shaderProgram, fragmentShader);
		glLinkProgram(shaderProgram);
		glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
		if (!success) 
		{
			glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
		}
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);

		return shaderProgram;
    }
	// ====================================================================================================


	// ----------------------------------------------------------------------------------------------------
	// Class AiPureCubeObject
	unsigned int AiPureCubeObject::m_shaderID = 0;
	bool AiPureCubeObject::m_shaderIsGenerated = false;

	AiPureCubeObject::AiPureCubeObject(unsigned int id) :
		AiObject(id)
	{
		m_color = glm::vec4(1.0f);
		// Generate vertex array object.
		init();

		// Init the shader.
		if (!m_shaderIsGenerated) {
			m_shaderID = initShader();
			m_shaderIsGenerated = true;
		}
	}

	AiPureCubeObject::~AiPureCubeObject()
	{
		glDeleteVertexArrays(1, &m_VAO);
		glDeleteBuffers(1, &m_VBO);
		glDeleteBuffers(1, &m_EBO);
	}

	void AiPureCubeObject::draw()
	{
		// Use shader.
		glUseProgram(m_shaderID);

		glm::mat4 model(1.0f);
		model = glm::translate(model, m_translate);
		model = glm::rotate(model, glm::radians(m_rotate.x), glm::vec3(1.0, 0.0, 0.0));
		model = glm::rotate(model, glm::radians(m_rotate.y), glm::vec3(0.0, 1.0, 0.0));
		model = glm::rotate(model, glm::radians(m_rotate.z), glm::vec3(0.0, 0.0, 1.0));
		model = glm::scale(model, m_scale);
		unsigned int modelLoc = glGetUniformLocation(m_shaderID, "model");
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		unsigned int viewLoc = glGetUniformLocation(m_shaderID, "view");
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(m_view));
		unsigned int projectionLoc = glGetUniformLocation(m_shaderID, "projection");
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(m_projection));
		unsigned int colorLoc = glGetUniformLocation(m_shaderID, "color");
		glUniform4fv(colorLoc, 1, glm::value_ptr(m_color));

		// Bind m_VAO.
		glBindVertexArray(m_VAO);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	}

	void AiPureCubeObject::setColor(float r, float g, float b)
	{
		m_color = glm::vec4(r, g, b, 1.0f);
	}

	void AiPureCubeObject::init()
	{
		glGenVertexArrays(1, &m_VAO);
		glGenBuffers(1, &m_VBO);
		glGenBuffers(1, &m_EBO);
		glBindVertexArray(m_VAO);
		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(m_vertices), m_vertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(m_indices), m_indices, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	// TODO::Simplify shader initialization code.
	unsigned int AiPureCubeObject::initShader() noexcept
	{
		unsigned int vertexShader;
		vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &vertexShaderCode, NULL);
		glCompileShader(vertexShader);
		int  success;
		char infoLog[512];
		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
		}
		unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &fragmentShaderCode, NULL);
		glCompileShader(fragmentShader);
		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
		}
		unsigned int shaderProgram = glCreateProgram();
		glAttachShader(shaderProgram, vertexShader);
		glAttachShader(shaderProgram, fragmentShader);
		glLinkProgram(shaderProgram);
		glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
		}
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);

		return shaderProgram;
	}
	// ====================================================================================================

	// ----------------------------------------------------------------------------------------------------
	// Class AiQuad
	AiQuad::AiQuad(unsigned int id, std::shared_ptr<Shader> shader, std::shared_ptr<PointLight> lightSource) : 
		AiObject(id), 
		m_shader(shader), 
		m_lightSource(lightSource), 
		m_material{ {0.25f, 0.21f, 0.21f}, {1.0f, 1.0f, 1.0f}, {0.3f, 0.3f, 0.3f}, 1/0.088f }
	{
		
		init();
	}

	AiQuad::~AiQuad()
	{
		glDeleteVertexArrays(1, &m_VAO);
		glDeleteBuffers(1, &m_VBO);
		glDeleteBuffers(1, &m_EBO);
	}

	void AiQuad::setMaterail(Material& material)
	{
		m_material = material;
	}

	void AiQuad::draw()
	{
		// Use shader.
		m_shader->use();

		glm::mat4 model(1.0f);
		model = glm::translate(model, m_translate);
		model = glm::rotate(model, glm::radians(m_rotate.x), glm::vec3(1.0, 0.0, 0.0));
		model = glm::rotate(model, glm::radians(m_rotate.y), glm::vec3(0.0, 1.0, 0.0));
		model = glm::rotate(model, glm::radians(m_rotate.z), glm::vec3(0.0, 0.0, 1.0));
		model = glm::scale(model, m_scale);
		m_shader->setMat4("model", model);
		m_shader->setMat4("view", m_view);
		m_shader->setMat4("projection", m_projection);

		// Set lighting calculation items.
		m_shader->setVec3("lightPos", m_lightSource->getPosition());
		m_shader->setVec3("lightColor", m_lightSource->getColor());
		m_shader->setVec3("viewPos", camera.Position);

		m_shader->setVec3("material.ambient", m_material.ambient);
		m_shader->setVec3("material.diffuse", m_material.diffuse);
		m_shader->setVec3("material.specular", m_material.specular);
		m_shader->setFloat("material.shininess", m_material.shininess);

		// Bind m_VAO.
		glBindVertexArray(m_VAO);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	}

	// TODO::Simplify initialization function.
	void AiQuad::init()
	{
		glGenVertexArrays(1, &m_VAO);
		glGenBuffers(1, &m_VBO);
		glGenBuffers(1, &m_EBO);
		glBindVertexArray(m_VAO);
		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(m_vertices), m_vertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3*sizeof(float)));
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(m_indices), m_indices, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	// ====================================================================================================

	// ----------------------------------------------------------------------------------------------------
	// Class AiQuadLM
	AiQuadLM::AiQuadLM(unsigned int id, std::shared_ptr<Shader> shader, std::shared_ptr<PointLight> lightSource,
		std::shared_ptr<Texture2D> diffuse, std::shared_ptr<Texture2D> specular):
		AiObject(id),
		m_shader(shader),
		m_lightSource(lightSource),
		m_diffuse(diffuse),
		m_specular(specular),
		m_selected(false)
	{
		init();
	}

	AiQuadLM::~AiQuadLM() 
	{
		glDeleteVertexArrays(1, &m_VAO);
		glDeleteBuffers(1, &m_VBO);
		glDeleteBuffers(1, &m_EBO);
	}

	void AiQuadLM::setLightingMaps(std::shared_ptr<Texture2D> diffuse, std::shared_ptr<Texture2D> specular)
	{
		m_diffuse = diffuse;
		m_specular = specular;
	}

	void AiQuadLM::changeSelectedState()
	{
		m_selected = !m_selected;
	}

	void AiQuadLM::draw()
	{
		// Use shader.
		m_shader->use();

		glm::mat4 model(1.0f);
		model = glm::translate(model, m_translate);
		model = glm::rotate(model, glm::radians(m_rotate.x), glm::vec3(1.0, 0.0, 0.0));
		model = glm::rotate(model, glm::radians(m_rotate.y), glm::vec3(0.0, 1.0, 0.0));
		model = glm::rotate(model, glm::radians(m_rotate.z), glm::vec3(0.0, 0.0, 1.0));
		model = glm::scale(model, m_scale);
		m_shader->setMat4("model", model);
		m_shader->setMat4("view", m_view);
		m_shader->setMat4("projection", m_projection);


		m_shader->setVec3("viewPos", camera.Position);
		// Set Directional Light
		m_shader->setVec3("dirLight.direction", g_dirLight.m_direction);
		m_shader->setVec3("dirLight.color", g_dirLight.m_color);
		// Set Point Lights.
		for (int i = 0; i < g_pointLights.size(); i++)
		{
			std::string shaderAttributeName = "pointLights[";
			shaderAttributeName += std::to_string(i);
			shaderAttributeName += "].";

			std::string pointLightPosition = shaderAttributeName + "position";
			m_shader->setVec3(pointLightPosition.c_str(), g_pointLights[i].m_position);
			std::string pointLightColor = shaderAttributeName + "color";
			m_shader->setVec3(pointLightColor.c_str(), g_pointLights[i].m_color);
			std::string pointLightConstant = shaderAttributeName + "constant";
			m_shader->setFloat(pointLightConstant.c_str(), g_pointLights[i].m_constant);
			std::string pointLightLinear = shaderAttributeName + "linear";
			m_shader->setFloat(pointLightLinear.c_str(), g_pointLights[i].m_linear);
			std::string pointLightQuadratic = shaderAttributeName + "quadratic";
			m_shader->setFloat(pointLightQuadratic.c_str(), g_pointLights[i].m_quadratic);
		}
		m_shader->setFloat("pointLigthNum", g_pointLights.size());

		m_shader->setInt("material.diffuse", 0);
		m_shader->setInt("material.specular", 1);
		// bind diffuse map
		glActiveTexture(GL_TEXTURE0);
		m_diffuse->bind();
		//// bind specular map
		glActiveTexture(GL_TEXTURE1);
		m_specular->bind();
		m_shader->setFloat("material.shininess", 32.0f);
		m_shader->setVec3("stencilColor", glm::vec3(1.0f, 1.0f, 1.0f));

		if (m_selected)
		{
			glEnable(GL_STENCIL_TEST);
			glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
			glStencilFunc(GL_ALWAYS, 1, 0xFF); // 所有的片段都应该更新模板缓冲
			glStencilMask(0xFF); // 启用模板缓冲写入
		}

		// Bind m_VAO.
		glBindVertexArray(m_VAO);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

		if (m_selected)
		{
			glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
			glStencilMask(0x00); // 禁止模板缓冲的写入
			glDisable(GL_DEPTH_TEST);
			model = glm::scale(model, m_scale * 1.1f);
			m_shader->setMat4("model", model);
			m_shader->setVec3("stencilColor", glm::vec3(0.0f, 0.0f, 1.0f));
			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
			glStencilMask(0xFF);

			glDisable(GL_STENCIL_TEST);
			glEnable(GL_DEPTH_TEST);
		}

		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void AiQuadLM::init()
	{
		glGenVertexArrays(1, &m_VAO);
		glGenBuffers(1, &m_VBO);
		glGenBuffers(1, &m_EBO);
		glBindVertexArray(m_VAO);
		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(m_vertices), m_vertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(m_indices), m_indices, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	// ====================================================================================================
}