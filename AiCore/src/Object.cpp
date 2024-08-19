#include "Object.h"

namespace Ai {
	extern Camera camera;
	// 1.Directional light
	extern DirLight g_dirLight;
	// 2.Point light container
	//extern const int maxPointLightNum;
	extern std::vector<SceneLight> g_pointLights;

	extern glm::mat4 g_lightSpaceMatrix;
	extern unsigned int g_shadowMappingDepthTexture;

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

	AiObject::~AiObject()
	{
		glDeleteVertexArrays(1, &m_VAO);
		glDeleteBuffers(1, &m_VBO);
		glDeleteBuffers(1, &m_EBO);
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

	// You need to set shader first.
	void AiTexQuadObject::drawShadowMapping(Shader& shader)
	{
		shader.use();
		glm::mat4 model(1.0f);
		model = glm::translate(model, m_translate);
		model = glm::rotate(model, glm::radians(m_rotate.x), glm::vec3(1.0, 0.0, 0.0));
		model = glm::rotate(model, glm::radians(m_rotate.y), glm::vec3(0.0, 1.0, 0.0));
		model = glm::rotate(model, glm::radians(m_rotate.z), glm::vec3(0.0, 0.0, 1.0));
		model = glm::scale(model, m_scale);
		shader.setMat4("model", model);
		
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

	void AiPureCubeObject::drawShadowMapping(Shader& shader)
	{
		shader.use();
		glm::mat4 model(1.0f);
		model = glm::translate(model, m_translate);
		model = glm::rotate(model, glm::radians(m_rotate.x), glm::vec3(1.0, 0.0, 0.0));
		model = glm::rotate(model, glm::radians(m_rotate.y), glm::vec3(0.0, 1.0, 0.0));
		model = glm::rotate(model, glm::radians(m_rotate.z), glm::vec3(0.0, 0.0, 1.0));
		model = glm::scale(model, m_scale);
		shader.setMat4("model", model);

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
		m_shader->setMat4("lightSpaceMatrix", g_lightSpaceMatrix);

		// Set lighting calculation items.
		m_shader->setVec3("dirLight.direction", g_dirLight.m_direction);
		m_shader->setVec3("dirLight.color", g_dirLight.m_color);
		m_shader->setVec3("viewPos", camera.Position);

		m_shader->setVec3("material.ambient", m_material.ambient);
		m_shader->setVec3("material.diffuse", m_material.diffuse);
		m_shader->setVec3("material.specular", m_material.specular);
		m_shader->setFloat("material.shininess", m_material.shininess);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, g_shadowMappingDepthTexture);

		// Bind m_VAO.
		glBindVertexArray(m_VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	}

	void AiQuad::drawShadowMapping(Shader& shader)
	{
		shader.use();
		glm::mat4 model(1.0f);
		model = glm::translate(model, m_translate);
		model = glm::rotate(model, glm::radians(m_rotate.x), glm::vec3(1.0, 0.0, 0.0));
		model = glm::rotate(model, glm::radians(m_rotate.y), glm::vec3(0.0, 1.0, 0.0));
		model = glm::rotate(model, glm::radians(m_rotate.z), glm::vec3(0.0, 0.0, 1.0));
		model = glm::scale(model, m_scale);
		shader.setMat4("model", model);

		glBindVertexArray(m_VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
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
			glStencilFunc(GL_ALWAYS, 1, 0xFF);
			glStencilMask(0xFF);
		}
		glBindVertexArray(m_VAO);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

		if (m_selected)
		{
			glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
			glStencilMask(0x00);
			//glDisable(GL_DEPTH_TEST);
			model = glm::scale(model, m_scale * 1.1f);
			m_shader->setMat4("model", model);
			m_shader->setVec3("stencilColor", glm::vec3(0.0f, 0.0f, 1.0f));
			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
			
			glStencilMask(0xFF); // This is used for the next glclear() function.
			glDisable(GL_STENCIL_TEST);
			glEnable(GL_DEPTH_TEST);
		}

		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void AiQuadLM::drawShadowMapping(Shader& shader)
	{
		shader.use();
		glm::mat4 model(1.0f);
		model = glm::translate(model, m_translate);
		model = glm::rotate(model, glm::radians(m_rotate.x), glm::vec3(1.0, 0.0, 0.0));
		model = glm::rotate(model, glm::radians(m_rotate.y), glm::vec3(0.0, 1.0, 0.0));
		model = glm::rotate(model, glm::radians(m_rotate.z), glm::vec3(0.0, 0.0, 1.0));
		model = glm::scale(model, m_scale);
		shader.setMat4("model", model);

		glBindVertexArray(m_VAO);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
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

	// ----------------------------------------------------------------------------------------------------
	// Class AiQuadNormalMap
	float g_quadPosition[12] = {
		 0.5f,  0.5f, 0.0f,
		 0.5f, -0.5f, 0.0f,
		-0.5f, -0.5f, 0.0f,
		-0.5f,  0.5f, 0.0f
	};

	float g_quadNormal[12] = {
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f
	};

	float g_quadTexCoord[8] = {
		 1.0f,  1.0f,
		 1.0f,  0.0f,
		 0.0f,  0.0f,
		 0.0f,  1.0f
	};

	float g_quadTangent[12] = {
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
	};

	float g_quadBitangent[12] = {
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
	};

	unsigned int g_quadIndices[6] = {
		0, 1, 3, // first triangle
		1, 2, 3  // second triangle
	};

	AiQuadNormalMap::AiQuadNormalMap(std::shared_ptr<Shader>		shader, 
									 std::shared_ptr<Texture2D>		diffuseMap, 
									 std::shared_ptr<Texture2D>		normalMap) :
		m_shader(shader),
		m_diffuseMap(diffuseMap),
		m_normalMap(normalMap)
	{
		init();
	}

	void AiQuadNormalMap::draw()
	{
		// Use shader.
		m_shader->use();
		// ------------------------------------------------------------------------------
		glm::mat4 model(1.0f);
		model = glm::translate(model, m_translate);
		model = glm::rotate(model, glm::radians(m_rotate.x), glm::vec3(1.0, 0.0, 0.0));
		model = glm::rotate(model, glm::radians(m_rotate.y), glm::vec3(0.0, 1.0, 0.0));
		model = glm::rotate(model, glm::radians(m_rotate.z), glm::vec3(0.0, 0.0, 1.0));
		model = glm::scale(model, m_scale);
		m_shader->setMat4("model", model);
		m_shader->setMat4("view", m_view);
		m_shader->setMat4("projection", m_projection);
		// ------------------------------------------------------------------------------
		m_shader->setInt("DiffuseMap", 0);
		m_shader->setInt("NormalMap", 1);
		glActiveTexture(GL_TEXTURE0);
		m_diffuseMap->bind();
		glActiveTexture(GL_TEXTURE1);
		m_normalMap->bind();
		// ------------------------------------------------------------------------------
		m_shader->setVec3("dirLight.direction", g_dirLight.m_direction);
		m_shader->setVec3("dirLight.color", g_dirLight.m_color);
		m_shader->setVec3("viewPos", camera.Position);

		glBindVertexArray(m_VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	}

	void AiQuadNormalMap::drawShadowMapping(Shader& shader)
	{
		shader.use();
		glm::mat4 model(1.0f);
		model = glm::translate(model, m_translate);
		model = glm::rotate(model, glm::radians(m_rotate.x), glm::vec3(1.0, 0.0, 0.0));
		model = glm::rotate(model, glm::radians(m_rotate.y), glm::vec3(0.0, 1.0, 0.0));
		model = glm::rotate(model, glm::radians(m_rotate.z), glm::vec3(0.0, 0.0, 1.0));
		model = glm::scale(model, m_scale);
		shader.setMat4("model", model);

		glBindVertexArray(m_VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	}

	void AiQuadNormalMap::init()
	{
		glGenVertexArrays(1, &m_VAO);
		glGenBuffers(1, &m_VBO);
		glGenBuffers(1, &m_EBO);
		glBindVertexArray(m_VAO);
		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(g_quadPosition) + sizeof(g_quadNormal) + sizeof(g_quadTexCoord)
			+ sizeof(g_quadTangent) + sizeof(g_quadBitangent), 0, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(g_quadPosition), g_quadPosition);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glBufferSubData(GL_ARRAY_BUFFER, sizeof(g_quadPosition), sizeof(g_quadNormal), g_quadNormal);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(sizeof(g_quadPosition)));
		glEnableVertexAttribArray(1);
		glBufferSubData(GL_ARRAY_BUFFER, sizeof(g_quadPosition) + sizeof(g_quadNormal), sizeof(g_quadTexCoord), g_quadTexCoord);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)(sizeof(g_quadPosition) + sizeof(g_quadNormal)));
		glEnableVertexAttribArray(2);
		glBufferSubData(GL_ARRAY_BUFFER, sizeof(g_quadPosition) + sizeof(g_quadNormal) + sizeof(g_quadTexCoord), sizeof(g_quadTangent), g_quadTangent);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(sizeof(g_quadPosition) + sizeof(g_quadNormal) + sizeof(g_quadTexCoord)));
		glEnableVertexAttribArray(3);
		glBufferSubData(GL_ARRAY_BUFFER, sizeof(g_quadPosition) + sizeof(g_quadNormal) + sizeof(g_quadTexCoord) + sizeof(g_quadTangent)
			, sizeof(g_quadBitangent), g_quadBitangent);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(sizeof(g_quadPosition) + sizeof(g_quadNormal) + sizeof(g_quadTexCoord)
			+ sizeof(g_quadTangent)));
		glEnableVertexAttribArray(4);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(g_quadIndices), g_quadIndices, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	// ====================================================================================================
	
	// ----------------------------------------------------------------------------------------------------
	// Class ModelObj
	unsigned int TextureFromFile(const char* path, const std::string& directory, bool gamma = false)
	{
		std::string filename = std::string(path);
		filename = directory + '/' + filename;

		unsigned int textureID;
		glGenTextures(1, &textureID);

		int width, height, nrComponents;
		unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
		if (data)
		{
			GLenum format;
			if (nrComponents == 1)
				format = GL_RED;
			else if (nrComponents == 3)
				format = GL_RGB;
			else if (nrComponents == 4)
				format = GL_RGBA;

			glBindTexture(GL_TEXTURE_2D, textureID);
			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			stbi_image_free(data);
		}
		else
		{
			std::cout << "Texture failed to load at path: " << path << std::endl;
			stbi_image_free(data);
		}

		return textureID;
	}

	void ModelObj::draw()
	{
		m_shader->use();
		// ::Beta Code
		glm::mat4 model(1.0f);
		model = glm::translate(model, m_translate);
		model = glm::rotate(model, glm::radians(m_rotate.x), glm::vec3(1.0, 0.0, 0.0));
		model = glm::rotate(model, glm::radians(m_rotate.y), glm::vec3(0.0, 1.0, 0.0));
		model = glm::rotate(model, glm::radians(m_rotate.z), glm::vec3(0.0, 0.0, 1.0));
		model = glm::scale(model, m_scale);
		m_shader->setMat4("model", model);
		m_shader->setMat4("view", m_view);
		m_shader->setMat4("projection", m_projection);
		//m_shader->setVec3("dirLight.direction", g_dirLight.m_direction);
		//m_shader->setVec3("dirLight.color", g_dirLight.m_color);

		for (unsigned int i = 0; i < meshes.size(); i++)
			meshes[i].Draw(m_shader);
	}

	void ModelObj::loadModel(std::string path)
	{
		Assimp::Importer import;
		const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
			return;
		}

		directory = path.substr(0, path.find_last_of('/'));
		processNode(scene->mRootNode, scene);
	}

	void ModelObj::processNode(aiNode* node, const aiScene* scene)
	{
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			meshes.push_back(processMesh(mesh, scene));
		}

		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			processNode(node->mChildren[i], scene);
		}
	}

	Mesh ModelObj::processMesh(aiMesh* mesh, const aiScene* scene)
	{
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		std::vector<Texture> textures;

		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex;

			glm::vec3 vector;
			vector.x = mesh->mVertices[i].x;
			vector.y = mesh->mVertices[i].y;
			vector.z = mesh->mVertices[i].z;
			vertex.Position = vector;

			vector.x = mesh->mNormals[i].x;
			vector.y = mesh->mNormals[i].y;
			vector.z = mesh->mNormals[i].z;
			vertex.Normal = vector;

			if (mesh->mTextureCoords[0])
			{
				glm::vec2 vec;
				vec.x = mesh->mTextureCoords[0][i].x;
				vec.y = mesh->mTextureCoords[0][i].y;
				vertex.TexCoords = vec;
				// tangent
				vector.x = mesh->mTangents[i].x;
				vector.y = mesh->mTangents[i].y;
				vector.z = mesh->mTangents[i].z;
				vertex.Tangent = vector;
				// bitangent
				vector.x = mesh->mBitangents[i].x;
				vector.y = mesh->mBitangents[i].y;
				vector.z = mesh->mBitangents[i].z;
				vertex.Bitangent = vector;
			}
			else
				vertex.TexCoords = glm::vec2(0.0f, 0.0f);

			vertices.push_back(vertex);
		}

		for (unsigned int i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}
		
		// 处理材质
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

		std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

		std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

		std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
		textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

		std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
		textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

		return Mesh(vertices, indices, textures);
	}

	std::vector<Texture> ModelObj::loadMaterialTextures(aiMaterial* mat, aiTextureType type,
		std::string typeName)
	{
		std::vector<Texture> textures;
		for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
		{
			aiString str;
			mat->GetTexture(type, i, &str);
			bool skip = false;
			for (unsigned int j = 0; j < textures_loaded.size(); j++)
			{
				if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
				{
					textures.push_back(textures_loaded[j]);
					skip = true;
					break;
				}
			}
			if (!skip)
			{
				Texture texture;
				texture.id = TextureFromFile(str.C_Str(), directory);
				texture.type = typeName;
				texture.path = str.C_Str();
				textures.push_back(texture);
				textures_loaded.push_back(texture); // 添加到已加载的纹理中
			}
		}
		return textures;
	}
	// ====================================================================================================

	// ----------------------------------------------------------------------------------------------------
	// Class TranslucentAiQuad
	TranslucentAiQuad::TranslucentAiQuad(std::shared_ptr<Shader> shader):
		m_shader(shader),
		m_material{ {0.25f, 0.21f, 0.21f}, {0.5f, 0.5f, 0.5f}, {0.3f, 0.3f, 0.3f}, 1 / 0.088f },
		m_alpha(0.75f)
	{
		init();
	}

	TranslucentAiQuad::~TranslucentAiQuad()
	{

	}

	void TranslucentAiQuad::draw()
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

		m_shader->setVec3("material.ambient", m_material.ambient);
		m_shader->setVec3("material.diffuse", m_material.diffuse);
		m_shader->setVec3("material.specular", m_material.specular);
		m_shader->setFloat("material.shininess", 32.0f);
		m_shader->setFloat("alpha", m_alpha);

		glBindVertexArray(m_VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void TranslucentAiQuad::drawShadowMapping(Shader& shader)
	{
		shader.use();
		glm::mat4 model(1.0f);
		model = glm::translate(model, m_translate);
		model = glm::rotate(model, glm::radians(m_rotate.x), glm::vec3(1.0, 0.0, 0.0));
		model = glm::rotate(model, glm::radians(m_rotate.y), glm::vec3(0.0, 1.0, 0.0));
		model = glm::rotate(model, glm::radians(m_rotate.z), glm::vec3(0.0, 0.0, 1.0));
		model = glm::scale(model, m_scale);
		shader.setMat4("model", model);

		glBindVertexArray(m_VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	}

	void TranslucentAiQuad::updateDistance()
	{
		double squareDistance;
		double squareX = (camera.Position.x - m_translate.x) * (camera.Position.x - m_translate.x);
		double squareY = (camera.Position.y - m_translate.y) * (camera.Position.y - m_translate.y);
		double squareZ = (camera.Position.z - m_translate.z) * (camera.Position.z - m_translate.z);
		squareDistance = squareX + squareY + squareZ;
		
		double distance = sqrt(squareDistance);
		m_distance = distance;
	}

	double TranslucentAiQuad::getDistance()
	{
		return m_distance;
	}

	void TranslucentAiQuad::init()
	{
		glGenVertexArrays(1, &m_VAO);
		glGenBuffers(1, &m_VBO);
		glGenBuffers(1, &m_EBO);
		glBindVertexArray(m_VAO);
		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(m_vertices), m_vertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(m_indices), m_indices, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	// ====================================================================================================
}
