#include <AiCore.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <iostream>

namespace Ai 
{
    // --------------------------------------------------
    // Rendering Engine configuration.
    AiEngineConfig g_AiEngineConfig;
        
    // Global light source
    // 1.Directional light
    DirLight g_dirLight = { {-0.2f, -1.0f, -0.3f}, {1.0f, 1.0f, 1.0f} };
    // 2.Point light container
    std::vector<SceneLight> g_pointLights;

    // Rendering window configuration.
    GLFWwindow* window;
    static unsigned int SCR_WIDTH = 1000;
    static unsigned int SCR_HEIGHT = 1000;
    
    // Camera to generate view transformation matrix.
    Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
    float lastX = SCR_WIDTH / 2.0f;
    float lastY = SCR_HEIGHT / 2.0f;
    bool firstMouse = true;

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    static float WH_Ratio = float(SCR_WIDTH) / float(SCR_HEIGHT);

    struct PainterObject 
    {
        unsigned int VAO;
        unsigned int VBO;
        unsigned int EBO;
    };

    PainterObject g_line;
    PainterObject g_triangle;
    PainterObject g_square;
    PainterObject g_circle;
    
    float lineVertices[6] = { 1.0f };

    static float triangleVertices[9] = 
    {
         0.0f, 0.1f, 1.0f,
         0.1f,-0.1f, 1.0f,
        -0.1f,-0.1f, 1.0f
    };

    static float squareVertices[18] = 
    {
        -0.1f, 0.1f, 1.0f,
         0.1f, 0.1f, 1.0f,
         0.1f,-0.1f, 1.0f,

        -0.1f, 0.1f, 1.0f,
        -0.1f,-0.1f, 1.0f,
         0.1f,-0.1f, 1.0f,
    };

    static float circleVertices[361 * 3] = { 1.0f };

    static unsigned int circleElementVertices[360 * 3] = { 0 };

    static float skyboxVertices[] = {      
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    constexpr double PI = 3.1415926;

    void setCircleVertices() 
    {
        circleVertices[0] = 0.0f;
        circleVertices[1] = 0.0f;
        for (unsigned int i = 1; i < 361; ++i) 
        {
            float angleInRadians = static_cast<float>(i) * PI / 180.0;
            float sinValue = std::sin(angleInRadians);
            float cosValue = std::cos(angleInRadians);
            circleVertices[3 * i + 0] = sinValue * 0.1;
            circleVertices[3 * i + 1] = cosValue * 0.1;
        }
    }

    void setCircleElementVertices() 
    {
        for (int i = 0; i < 359; i++) 
        {
            circleElementVertices[i * 3 + 1] = i + 1;
            circleElementVertices[i * 3 + 2] = i + 2;
        }
        circleElementVertices[359 * 3 + 1] = 359 + 1;
        circleElementVertices[359 * 3 + 2] = 1;
    }
    
    // Painter objects container.
    std::vector<std::shared_ptr<Painter>> RenderPainterVector; // TODO::Container type independent
    // AiObject(3D) container.
    std::vector<std::shared_ptr<AiObject>> RenderObjectVector;
    // Shader container.
    std::map<std::string, unsigned int> ShaderMap;
    // Point light container.
    std::vector<PointLight> PointLights;
    // Transparent Object container
    std::vector<std::shared_ptr<TranslucentAiObject>> TransparencyContainer;

    AiEngineConfig& getAiEngineConfig()
    {
        return g_AiEngineConfig;
    }

    void renderAiInit() 
    {
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        if (!g_AiEngineConfig.offScreenRenderingFlag && g_AiEngineConfig.antiAliasing)
        {
            glfwWindowHint(GLFW_SAMPLES, 4);
        }
        

        window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "HelloWindow~", NULL, NULL);
        if (window == NULL)
        {
            std::cout << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            exit(1);
        }

        glfwMakeContextCurrent(window);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            std::cout << "Failed to initialize GLAD" << std::endl;
            exit(1);
        }

        glfwSetCursorPosCallback(window, mouse_callback);
        glfwSetScrollCallback(window, scroll_callback);

        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        unsigned int VBO, VAO, EBO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        g_triangle.VAO = VAO;
        g_triangle.VBO = VBO;

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(squareVertices), squareVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        g_square.VAO = VAO;
        g_square.VBO = VBO;

        setCircleVertices();
        setCircleElementVertices();
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(circleVertices), circleVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(circleElementVertices), circleElementVertices, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        g_circle.VAO = VAO;
        g_circle.VBO = VBO;
        g_circle.EBO = EBO;

        addLineShader();
        addPolygonShader();

        for (int i = 0; i < 10; i++)
        {
            g_pointLights.push_back({ {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, 1.0f, 0.09f, 0.032f });
        }
    }

    void renderAi()
    {
        // screen quad VAO
        unsigned int quadVAO, quadVBO;
        if (g_AiEngineConfig.offScreenRenderingFlag)
        {
            float quadVertices[] = {
                -1.0f,  1.0f,  0.0f, 1.0f,
                -1.0f, -1.0f,  0.0f, 0.0f,
                 1.0f, -1.0f,  1.0f, 0.0f,

                -1.0f,  1.0f,  0.0f, 1.0f,
                 1.0f, -1.0f,  1.0f, 0.0f,
                 1.0f,  1.0f,  1.0f, 1.0f
            };

            glGenVertexArrays(1, &quadVAO);
            glGenBuffers(1, &quadVBO);
            glBindVertexArray(quadVAO);
            glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        }
        // TODO::Encapsulate the following operations
        Shader screenShader("resources/shaders/common/screen/screenQuad.vs", "resources/shaders/common/screen/screenQuad.fs");
        screenShader.use();
        screenShader.setInt("screenTexture", 0);

        for (int i = 0; i < 10; ++i)
        {
            // If point light in the contianer has been set, render them.
            if (g_pointLights[i].m_color != glm::vec3(0.0f, 0.0f, 0.0f))
            {
                auto lightSource = addPureCube(0);
                auto& scale = lightSource->getScale();
                scale *= 0.1;
                auto& position = lightSource->getTranslate();
                position = g_pointLights[i].m_position;
                auto color = g_pointLights[i].m_color;
                lightSource->setColor(color.r, color.g, color.b);
            }
        }

        // SkyBox
        // skybox VAO
        unsigned int skyboxVAO, skyboxVBO;
        glGenVertexArrays(1, &skyboxVAO);
        glGenBuffers(1, &skyboxVBO);
        glBindVertexArray(skyboxVAO);
        glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

        Shader skyboxShader("resources/shaders/common/skybox/skybox.vs", "resources/shaders/common/skybox/skybox.fs");
        skyboxShader.use();
        skyboxShader.setInt("skybox", 0);

        SkyBoxTexture skyboxTexture("resources/textures/skybox/default");

        // Shadow Mapping Shader
        Shader shaderMappingShader("resources/shaders/common/shadow/shadowMapping.vs", "resources/shaders/common/shadow/shadowMapping.fs");

        // Off-Screen framebuffer configuration
        unsigned int offScreenFramebuffer;
        unsigned int offScreenTexture;
        unsigned int rbo;

        // Shadow mapping framebuffer.
        unsigned int shadowMappingFramebuffer;
        const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
        unsigned int shadowMappingDepthTexture;

        // configure second post-processing framebuffer
        unsigned int antialisingFramebuffer;
        unsigned int screenTexture;

        if (g_AiEngineConfig.offScreenRenderingFlag)
        {
            glGenFramebuffers(1, &offScreenFramebuffer);
            glBindFramebuffer(GL_FRAMEBUFFER, offScreenFramebuffer);

            // create a color attachment texture
            glGenTextures(1, &offScreenTexture);
            
            glGenRenderbuffers(1, &rbo);
            glBindRenderbuffer(GL_RENDERBUFFER, rbo);

            if (g_AiEngineConfig.antiAliasing) 
            {
                glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, offScreenTexture);
                glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB, SCR_WIDTH, SCR_HEIGHT, GL_TRUE);
                glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, offScreenTexture, 0);

                glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

                glGenFramebuffers(1, &antialisingFramebuffer);
                glBindFramebuffer(GL_FRAMEBUFFER, antialisingFramebuffer);
                
                glGenTextures(1, &screenTexture);
                glBindTexture(GL_TEXTURE_2D, screenTexture);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screenTexture, 0);

            }
            else
            {
                glBindTexture(GL_TEXTURE_2D, offScreenTexture);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, offScreenTexture, 0);

                glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
            }



            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
                std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        if (g_AiEngineConfig.shadowMapping)
        {
            glGenFramebuffers(1, &shadowMappingFramebuffer);

            glGenTextures(1, &shadowMappingDepthTexture);
            glBindTexture(GL_TEXTURE_2D, shadowMappingDepthTexture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

            glBindFramebuffer(GL_FRAMEBUFFER, shadowMappingFramebuffer);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMappingDepthTexture, 0);
            glDrawBuffer(GL_NONE);
            glReadBuffer(GL_NONE);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        while (!glfwWindowShouldClose(window))
        {
            float currentFrame = static_cast<float>(glfwGetTime());
            deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;

            processInput(window);

            glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

            // camera/view transformation
            glm::mat4 view = camera.GetViewMatrix();

            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

            if (!g_AiEngineConfig.offScreenRenderingFlag && g_AiEngineConfig.antiAliasing)
            {
                glEnable(GL_MULTISAMPLE);
            }
            
            if (RenderPainterVector.size() != 0 || RenderObjectVector.size() != 0)
            {
                float near_plane = 1.0f, far_plane = 7.5f;
                glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
                glm::mat4 lightView = glm::lookAt(glm::vec3(-2.0f, 4.0f, -1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                glm::mat4 lightSpaceMatrix = lightProjection * lightView;
                shaderMappingShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

                if (g_AiEngineConfig.shadowMapping)
                {
                    // Rendering shadow mapping depth texture first.
                    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
                    glBindFramebuffer(GL_FRAMEBUFFER, shadowMappingFramebuffer);
                    glClear(GL_DEPTH_BUFFER_BIT);
                    glEnable(GL_DEPTH_TEST);
                    
                    // Render the scene
                    for (int i = 0; i < RenderObjectVector.size(); i++)
                    {
                        shaderMappingShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
                        RenderObjectVector[i]->drawShadowMapping(shaderMappingShader);
                    }

                    glBindFramebuffer(GL_FRAMEBUFFER, 0);
                }

                // Bind the off-screen framebuffer.
                if (g_AiEngineConfig.offScreenRenderingFlag)
                {
                    glBindFramebuffer(GL_FRAMEBUFFER, offScreenFramebuffer);
                }

                glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                int vertexScaleLocation;
                int vertexPosLocation;
                int vertexColorLocation;
                glEnable(GL_DEPTH_TEST);

                // TODO::Change to Range-Based For.
                for (int i = 0; i < RenderObjectVector.size(); i++) 
                {
                    RenderObjectVector[i]->getView() = view;
                    RenderObjectVector[i]->getProjection() = projection;
                    RenderObjectVector[i]->draw();
                }

                //for (int i = 0; i < RenderObjectVector.size(); i++)
                //{
                //    shaderMappingShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
                //    RenderObjectVector[i]->drawShadowMapping(shaderMappingShader);
                //}

                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                for (auto transparency : TransparencyContainer)
                {
                    // TODO-1::Two pass transparency rendering, one for back face and one for front face.
                    // TODO-2::Reoreder the transparency container by distance.
                    //glEnable(GL_CULL_FACE);
                    // Set front face surround order.
                    // CCW means Conuterclockwise, and CW means Clockwise.
                    //glFrontFace(GL_CW);
                    // Select culling face.
                    //glCullFace(GL_BACK); // TODO...
                    transparency->getView() = view;
                    transparency->getProjection() = projection;
                    transparency->draw();
                    //glDisable(GL_CULL_FACE);
                }
                glDisable(GL_BLEND);

                glDisable(GL_DEPTH_TEST);
                for (int i = 0; i < RenderPainterVector.size(); i++) 
                {
                    std::pair<float, float> point;
                    std::pair<float, float> scale;
                    std::pair<float, float> pos;
                    std::tuple<float, float, float> color;

                    switch (RenderPainterVector[i]->getObjectType()) 
                    {
                    case ObjectType::LINE:
                        glUseProgram(ShaderMap["Line"]);
                        glLineWidth(1.0f);
                        point = std::dynamic_pointer_cast<Line>(RenderPainterVector[i])->getStartPoint();
                        lineVertices[0] = point.first;
                        lineVertices[1] = point.second;
                        point = std::dynamic_pointer_cast<Line>(RenderPainterVector[i])->getEndPoint();
                        lineVertices[3] = point.first;
                        lineVertices[4] = point.second;
                        glGenVertexArrays(1, &g_line.VAO);
                        glGenBuffers(1, &g_line.VBO);
                        glBindVertexArray(g_line.VAO);
                        glBindBuffer(GL_ARRAY_BUFFER, g_line.VBO);
                        glBufferData(GL_ARRAY_BUFFER, sizeof(lineVertices), lineVertices, GL_STATIC_DRAW);
                        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
                        glEnableVertexAttribArray(0);
                        color = std::dynamic_pointer_cast<Line>(RenderPainterVector[i])->getColor();
                        vertexColorLocation = glGetUniformLocation(ShaderMap["Line"], "ourColor");
                        glUniform4f(vertexColorLocation, std::get<0>(color), std::get<1>(color), std::get<2>(color), 1.0f);
                        glDrawArrays(GL_LINES, 0, 2);
                        glDeleteVertexArrays(1, &g_line.VAO);
                        glDeleteBuffers(1, &g_line.VBO);
                        glBindBuffer(GL_ARRAY_BUFFER, 0);
                        glBindVertexArray(0);
                        break;
                    case ObjectType::TRIANGLE:
                        glUseProgram(ShaderMap["Polygon"]);
                        glBindVertexArray(g_triangle.VAO);
                        vertexScaleLocation = glGetUniformLocation(ShaderMap["Polygon"], "scale");
                        scale = std::dynamic_pointer_cast<Triangle>(RenderPainterVector[i])->getScale();
                        glUniform2f(vertexScaleLocation, scale.first, scale.second * WH_Ratio);
                        pos = std::dynamic_pointer_cast<Triangle>(RenderPainterVector[i])->getPosition();
                        vertexPosLocation = glGetUniformLocation(ShaderMap["Polygon"], "pos");
                        glUniform2f(vertexPosLocation, pos.first, pos.second);
                        color = std::dynamic_pointer_cast<Triangle>(RenderPainterVector[i])->getColor();
                        vertexColorLocation = glGetUniformLocation(ShaderMap["Polygon"], "ourColor");
                        glUniform4f(vertexColorLocation, std::get<0>(color), std::get<1>(color), std::get<2>(color), 1.0f);
                        glDrawArrays(GL_TRIANGLES, 0, 3);
                        break;
                    case ObjectType::SQUARE:
                        glUseProgram(ShaderMap["Polygon"]);
                        glBindVertexArray(g_square.VAO);
                        vertexScaleLocation = glGetUniformLocation(ShaderMap["Polygon"], "scale");
                        scale = std::dynamic_pointer_cast<Square>(RenderPainterVector[i])->getScale();
                        glUniform2f(vertexScaleLocation, scale.first, scale.second * WH_Ratio);
                        pos = std::dynamic_pointer_cast<Square>(RenderPainterVector[i])->getPosition();
                        vertexPosLocation = glGetUniformLocation(ShaderMap["Polygon"], "pos");
                        glUniform2f(vertexPosLocation, pos.first, pos.second);
                        color = std::dynamic_pointer_cast<Square>(RenderPainterVector[i])->getColor();
                        vertexColorLocation = glGetUniformLocation(ShaderMap["Polygon"], "ourColor");
                        glUniform4f(vertexColorLocation, std::get<0>(color), std::get<1>(color), std::get<2>(color), 1.0f);
                        glDrawArrays(GL_TRIANGLES, 0, 6);
                        break;
                    case ObjectType::CIRCLE:
                        glUseProgram(ShaderMap["Polygon"]);
                        glBindVertexArray(g_circle.VAO);
                        vertexScaleLocation = glGetUniformLocation(ShaderMap["Polygon"], "scale");
                        scale = std::dynamic_pointer_cast<Circle>(RenderPainterVector[i])->getScale();
                        glUniform2f(vertexScaleLocation, scale.first, scale.second * WH_Ratio);
                        pos = std::dynamic_pointer_cast<Circle>(RenderPainterVector[i])->getPosition();
                        vertexPosLocation = glGetUniformLocation(ShaderMap["Polygon"], "pos");
                        glUniform2f(vertexPosLocation, pos.first, pos.second);
                        color = std::dynamic_pointer_cast<Circle>(RenderPainterVector[i])->getColor();
                        vertexColorLocation = glGetUniformLocation(ShaderMap["Polygon"], "ourColor");
                        glUniform4f(vertexColorLocation, std::get<0>(color), std::get<1>(color), std::get<2>(color), 1.0f);
                        glDrawElements(GL_TRIANGLES, 360 * 3, GL_UNSIGNED_INT, 0);
                        break;
                    }
                }
            }

            // skybox cube
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);
            skyboxShader.use();
            view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
            skyboxShader.setMat4("view", view);
            skyboxShader.setMat4("projection", projection);
            glBindVertexArray(skyboxVAO);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture.getTextureId());
            glDrawArrays(GL_TRIANGLES, 0, 36);
            glBindVertexArray(0);
            glDepthFunc(GL_LESS);

            // Render off - screen framebuffer.
            if (g_AiEngineConfig.offScreenRenderingFlag)
            {
                if (g_AiEngineConfig.antiAliasing)
                {
                    glBindFramebuffer(GL_READ_FRAMEBUFFER, offScreenFramebuffer);
                    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, antialisingFramebuffer);
                    glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_COLOR_BUFFER_BIT, GL_NEAREST);
                }

                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                glDisable(GL_DEPTH_TEST);
                glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT);

                screenShader.use();
                glBindVertexArray(quadVAO);
                glActiveTexture(GL_TEXTURE0);

                if (g_AiEngineConfig.antiAliasing)
                {
                    //glBindTexture(GL_TEXTURE_2D, screenTexture);
                    glBindTexture(GL_TEXTURE_2D, shadowMappingDepthTexture);
                }
                else
                {
                     glBindTexture(GL_TEXTURE_2D, offScreenTexture);
                }
               
                glDrawArrays(GL_TRIANGLES, 0, 6);
            }

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
        glDeleteVertexArrays(1, &g_triangle.VAO);
        glDeleteBuffers(1, &g_triangle.VBO);
        glDeleteVertexArrays(1, &g_square.VAO);
        glDeleteBuffers(1, &g_square.VBO);
        glDeleteVertexArrays(1, &g_circle.VAO);
        glDeleteBuffers(1, &g_circle.VBO);
        glDeleteBuffers(1, &g_circle.EBO);
        
        // Delete shader programs in ShaderMap
        glUseProgram(0);
        for (auto iter = ShaderMap.begin(); iter != ShaderMap.end(); iter++)
        {
            glDeleteProgram(iter->second);
        }

        glfwTerminate();
    }

    void setDirLight(glm::vec3 direction, glm::vec3 color)
    {
        g_dirLight = { direction, color };
    }

    void setDirLight(DirLight dirLight)
    {
        g_dirLight = dirLight;
    }

    SceneLight& getPointLight(int index)
    {
        return g_pointLights[index];
    }

    void addLineShader()
    {
        const char* vertexShaderSource = "#version 330 core\n"
            "layout (location = 0) in vec3 aPos;\n"
            "void main()\n"
            "{\n"
            "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
            "}\0";
        const char* fragmentShaderSource = "#version 330 core\n"
            "out vec4 FragColor;\n"
            "uniform vec4 ourColor;\n"
            "void main()\n"
            "{\n"
            "   FragColor = ourColor;\n"
            "}\n\0";
        unsigned int vertexShader;
        vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
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
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
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
        if (!success) {
            glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        }
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        ShaderMap["Line"] = shaderProgram;
    }

    void addPolygonShader()
	{
        const char* vertexShaderSource = "#version 330 core\n"
            "layout (location = 0) in vec3 aPos;\n"
            "uniform vec2 pos;\n"
            "uniform vec2 scale;\n"
            "void main()\n"
            "{\n"
            "   gl_Position = vec4(aPos.x * scale.x, aPos.y * scale.y, aPos.z, 1.0) + vec4(pos, 0.0, 0.0);\n"
            "}\0";
        const char* fragmentShaderSource = "#version 330 core\n"
            "out vec4 FragColor;\n"
            "uniform vec4 ourColor;\n"
            "void main()\n"
            "{\n"
            "   FragColor = ourColor;\n"
            "}\n\0";
        unsigned int vertexShader;
        vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
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
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
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
        if (!success) {
            glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        }
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        ShaderMap["Polygon"] = shaderProgram;

	}

    void addLine(unsigned int id, float startPointX, float startPointY, float endPointX, float endPointY) 
    {
        RenderPainterVector.push_back(std::make_shared<Line>(id, startPointX, startPointY, endPointX, endPointY));
    }

    void addLine(unsigned int id, float startPointX, float startPointY, float endPointX, float endPointY,
        float red, float green, float blue) 
    {
        RenderPainterVector.push_back(std::make_shared<Line>(id, startPointX, startPointY, endPointX, endPointY, red, green, blue));
    }

    void addTriangle(unsigned int id, float xscale, float yscale, float xpos, float ypos) 
    {
        RenderPainterVector.push_back(std::make_shared<Triangle>(id, xscale, yscale, xpos, ypos));
    }

    void addTriangle(unsigned int id, float xscale, float yscale, float xpos, float ypos, float red, float green, float blue) 
    {
        RenderPainterVector.push_back(std::make_shared<Triangle>(id, xscale, yscale, xpos, ypos, red, green, blue));
    }

    void addSquare(unsigned int id, float xscale, float yscale, float xpos, float ypos) 
    {
        RenderPainterVector.push_back(std::make_shared<Square>(id, xscale, yscale, xpos, ypos));
    }

    void addSquare(unsigned int id, float xscale, float yscale, float xpos, float ypos, float red, float green, float blue) 
    {
        RenderPainterVector.push_back(std::make_shared<Square>(id, xscale, yscale, xpos, ypos, red, green, blue));
    }

    void addCircle(unsigned int id, float xscale, float yscale, float xpos, float ypos) 
    {
        RenderPainterVector.push_back(std::make_shared<Circle>(id, xscale, yscale, xpos, ypos));
    }

    void addCircle(unsigned int id, float xscale, float yscale, float xpos, float ypos, float red, float green, float blue) 
    {
        RenderPainterVector.push_back(std::make_shared<Circle>(id, xscale, yscale, xpos, ypos, red, green, blue));
    }

    std::shared_ptr<AiTexQuadObject> addTex(unsigned int id, std::string name, std::string imgPath) 
    {
        std::shared_ptr<AiTexQuadObject> sp = std::make_shared<AiTexQuadObject>(id, name, imgPath);
        RenderObjectVector.push_back(sp);
        return sp;
    }

    std::shared_ptr<AiPureCubeObject> addPureCube(unsigned int id)
    {
        std::shared_ptr<AiPureCubeObject> sp = std::make_shared<AiPureCubeObject>(id);
        RenderObjectVector.push_back(sp);
        return sp;
    }

    std::shared_ptr<AiQuad> addAiQuad(unsigned int id, std::shared_ptr<Shader> shader, std::shared_ptr<PointLight> lightPoint)
    {
        std::shared_ptr<AiQuad> sp = std::make_shared<AiQuad>(id, shader, lightPoint);
        RenderObjectVector.push_back(sp);
        return sp;
    }

    std::shared_ptr<AiQuadLM> addAiQuadLM(unsigned int id, std::shared_ptr<Shader> shader, std::shared_ptr<PointLight> lightPoint,
        std::shared_ptr<Texture2D> diffuse, std::shared_ptr<Texture2D> specular)
    {
        std::shared_ptr<AiQuadLM> sp = std::make_shared<AiQuadLM>(id, shader, lightPoint, diffuse, specular);
        RenderObjectVector.push_back(sp);
        return sp;
    }

    std::shared_ptr<ModelObj> addModelObj(std::string path, std::shared_ptr<Shader> shader)
    {
        std::shared_ptr<ModelObj> sp = std::make_shared<ModelObj>(path, shader);
        RenderObjectVector.push_back(sp);
        return sp;
    }

    std::shared_ptr<TranslucentAiQuad> addTranslucentAiQuad(std::shared_ptr<Shader> shader)
    {
        std::shared_ptr<TranslucentAiQuad> sp = std::make_shared<TranslucentAiQuad>(shader);
        TransparencyContainer.push_back(sp);
        return sp;
    }

    void processInput(GLFWwindow* window)
    {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera.ProcessKeyboard(FORWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera.ProcessKeyboard(BACKWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera.ProcessKeyboard(LEFT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera.ProcessKeyboard(RIGHT, deltaTime);
    }

    void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
    {
        float xpos = static_cast<float>(xposIn);
        float ypos = static_cast<float>(yposIn);

        if (firstMouse)
        {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }

        float xoffset = xpos - lastX;
        float yoffset = lastY - ypos;

        lastX = xpos;
        lastY = ypos;

        camera.ProcessMouseMovement(xoffset, yoffset);
    }

    void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
    {
        camera.ProcessMouseScroll(static_cast<float>(yoffset));
    }
}
