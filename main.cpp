#include <iostream>
#include <AiCore.h>
#include <Shader.h>

#include <Texture.h>

#include <thread>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

void rotateTex(std::shared_ptr<Ai::AiTexQuadObject> obj) 
{
	auto& rotate = obj->getRotate();
	auto& scale = obj->getScale();
	scale.x = 1.2f;
	std::chrono::milliseconds waitTime(10);
	while(true){
		std::this_thread::sleep_for(waitTime);
		rotate.z += 1.0f;
	}
}

int main() 
{
	auto& engineConfig = Ai::getAiEngineConfig();
	engineConfig.antiAliasing = true;
	engineConfig.offScreenRenderingFlag = true;

	Ai::renderAiInit();

	// Demo1
	std::shared_ptr<Ai::AiTexQuadObject> tex = Ai::addTex(8, "azibao", "resources/textures/azi1.jpg");
	auto& posAzi = tex->getTranslate();
	posAzi.z = 0.5f;

	auto tex2 = Ai::addTex(9, "azibao", "resources/textures/azibao.jpg");
	auto& translation = tex2->getTranslate();
	translation.x = 1.0f;
	std::thread t2(rotateTex, tex);
	t2.detach();

	// Demo2
	glm::vec3 lightSorcePosition(1.0f, 1.0f, 1.0f);
	glm::vec3 plColor(1.0f, 1.0f, 1.0f);
	auto lightSource = Ai::addPureCube(9);
	auto& scale = lightSource->getScale();
	scale *= 0.1;
	auto& position = lightSource->getTranslate();
	position = lightSorcePosition;
	lightSource->setColor(1.0f, 1.0f, 1.0f);

	// Demo3
	std::shared_ptr<Shader> basicShader = std::make_shared<Shader>("resources/shaders/01.basicShader.vs", 
		"resources/shaders/01.basicShader.fs");
	auto sp_pl = std::make_shared<Ai::PointLight>(lightSorcePosition, plColor);
	auto quad = Ai::addAiQuad(10, basicShader, sp_pl);
	auto& quadScale = quad->getScale();
	quadScale = glm::vec3(20.0f);
	auto& quadPosition = quad->getTranslate();
	quadPosition.z = 1.0f;
	quadPosition.y = -0.5f;
	auto& quadRotate = quad->getRotate();
	quadRotate.x = -90;
	//material
	Ai::Material material{ {0.174f, 0.0117f, 0.0117f}, {0.61424f, 0.04136f,  0.04136f}, {0.7278f, 0.626959f, 0.626959f}, 128.0f };
	quad->setMaterail(material);

	// Demo4
	// diffuseMap
	std::shared_ptr<Texture2D> diffuseMap = std::make_shared<Texture2D>("resources/textures/container2.png");
	std::shared_ptr<Texture2D> specularMap = std::make_shared<Texture2D>("resources/textures/container2_specular.png");
	std::shared_ptr<Shader> lightingMapShader = std::make_shared<Shader>("resources/shaders/02.lightingMap.vs",
		"resources/shaders/02.lightingMap.fs");
	auto quadLM = Ai::addAiQuadLM(11, lightingMapShader, sp_pl, diffuseMap, specularMap);
	//quadLM->changeSelectedState();
	auto& quadLMPosition = quadLM->getTranslate();
	quadLMPosition.x = -1.5f;

	// Ai::setDirLight({ -1.0f, -1.0f, -1.0f }, {1.0f, 1.0f, 1.0f});
	auto& pl1 = Ai::getPointLight(1);
	pl1.m_position = { -2.0f, -0.5f, 1.0f};
	pl1.m_color = { 1.0f, 0.0f, 0.0f };

	auto& pl2 = Ai::getPointLight(2);
	pl2.m_position = { -1.0f, -0.5f, 1.0f };
	pl2.m_color = { 0.0f, 1.0f, 0.0f };

	auto& pl3 = Ai::getPointLight(3);
	pl3.m_position = { -1.5f, 0.5f, 1.0f };
	pl3.m_color = { 0.0f, 0.0f, 1.0f };

	// Demo5 
	//std::shared_ptr<Shader> azibaoShader = std::make_shared<Shader>("resources/shaders/03.azibaoShader.vs",
	//	"resources/shaders/03.azibaoShader.fs");
	//// Gooch Shading
	////std::shared_ptr<Shader> azibaoShader = std::make_shared<Shader>("resources/shaders/Azibao/GoochShading.vs",
	////	"resources/shaders/Azibao/GoochShading.fs");
	//auto aziModel = Ai::addModelObj("resources/models/azi/azi.obj", azibaoShader);
	//auto& scaleAzibao = aziModel->getScale();
	//scaleAzibao *= 0.05;
	//auto& posAzibao = aziModel->getTranslate();
	//posAzibao.z = 0.5f;
	//posAzibao.y = -0.5f;
	
	// Demo 6 => Render Transparency.
	std::shared_ptr<Shader> transparencyShader = std::make_shared<Shader>("resources/shaders/common/pure/transparency.vs", 
		"resources/shaders/common/pure/transparency.fs");
	auto transparencyObject = Ai::addTranslucentAiQuad(transparencyShader);
	auto& transparencyObjPos = transparencyObject->getTranslate();
	transparencyObjPos = glm::vec3(-1.5f, 0.1f, 0.1f);

	// Demo 7 => Render quad object with normal map.
	std::shared_ptr<Shader> quadNormalMapShader = std::make_shared<Shader>("resources/shaders/NormalMapQuad/vertex.vs", 
																		   "resources/shaders/NormalMapQuad/fragment.fs");
	std::shared_ptr<Texture2D> quadDiffuseTex = std::make_shared<Texture2D>("resources/textures/NormalMap/brickwall.jpg");
	std::shared_ptr<Texture2D> quadNormalTex = std::make_shared<Texture2D>("resources/textures/NormalMap/brickwall_normal.jpg");
	auto quadNormalMap = Ai::addAiQuadNormalMap(quadNormalMapShader, quadDiffuseTex, quadNormalTex);
	auto& quadNormalMapTrans = quadNormalMap->getTranslate();
	quadNormalMapTrans = {1.0f, 0.0f, 1.5f};
	auto& quadNormalMapRotate = quadNormalMap->getRotate();
	quadNormalMapRotate.y = 90.0f;

	// RenderLoop
	Ai::renderAi();

	std::cout << "Waiting...Waiting..." << std::endl;
	std::cout << "Here has a window now..." << std::endl;
}
