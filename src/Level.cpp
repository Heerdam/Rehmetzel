
#include "Level.h"
#include "Assets.hpp"
#include "UI.hpp"
#include "CameraUtils.hpp"
#include <iostream>

using namespace Heerbann;
using namespace UI;

void LevelManager::initialize() {
	auto assets = Main::getAssetManager();
	assets->addLevel("LoadingScreenLevel", new LoadingScreenLevel());
	assets->addLevel("MainMenuLevel", new MainMenuLevel());
	loadLevel("LoadingScreenLevel");
	assets->finish();
};

void LevelManager::loadLevel(std::string _id) {
	auto assets = Main::getAssetManager();
	Level* level = assets->getLevel(_id);
	for (auto a : level->assetToLoad)
		assets->load(a->id);
	assets->loadLevel(_id);
	activeLevels.emplace_back(level);
}

void LevelManager::unloadLevel(std::string _id) {
	auto assets = Main::getAssetManager();
	Level* level = assets->getLevel(_id);
	for (auto a : level->assetToUnload)
		assets->unload(a->id);
	assets->unloadLevel(_id);
	for (auto it = activeLevels.begin(); it != activeLevels.end(); ++it)
		if (*it == level) activeLevels.erase(it);
}

void LevelManager::update(float _deltaTime) {
	for (auto l : activeLevels) {
		if (!l->isLocked && l->isLoaded)
			l->update(_deltaTime);
	}
}

void LevelManager::draw(float _deltaTime, sf::RenderWindow& _window) {
	for (auto l : activeLevels) {
		if (!l->isLocked && l->isLoaded)
			l->draw(_deltaTime, _window);
	}
}

//---------------------- LoadingScreenLevel ----------------------\\

Heerbann::LoadingScreenLevel::LoadingScreenLevel() {

	bgShader = new sf::Shader();

		/**
	const char *vertexShaderSource = "#version 330 core\n"
		"layout (location = 0) in vec3 aPos;\n"
		"uniform mat4 transform;"
		"void main()\n"
		"{\n"
		"   gl_Position = transform * vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
		"}\0";
	const char *fragmentShaderSource = "#version 330 core\n"
		"out vec4 FragColor;\n"
		"void main()\n"
		"{\n"
		"   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
		"}\n\0";

	;
	bgShader->loadFromMemory(vertexShaderSource, fragmentShaderSource);
	*/
	
	if (!bgShader->loadFromFile("assets/shader/bg_shader.vert", "assets/shader/bg_shader.geom", "assets/shader/bg_shader.frag"))
		std::exception("vertex/ geom/ fragment failed");
	

	GLint link;
	glGetProgramiv(bgShader->getNativeHandle(), GL_LINK_STATUS, &link);

	std::cout << "link: " << link << std::endl;

	uniformlocation = glGetUniformLocation(bgShader->getNativeHandle(), "transform");

	GLint err;
	while ((err = glGetError()) != GL_NO_ERROR) {
		std::cout << err << std::endl;
	}

	/**

	float vertices[] = {
		 100.f, 100.f, 0.0f,  // top right
		 100.f, -100.f, 0.0f,  // bottom right
		-100.f, -100.f, 0.0f,  // bottom left
		-100.f,  100.f, 0.0f   // top left 
	};
	unsigned int indices[] = {  // note that we start from 0!
		0, 1, 3,  // first Triangle
		1, 2, 3   // second Triangle
	};
	
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
	// VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
	glBindVertexArray(0);
	*/

	
	pos = new float[8]{
	-0.5f,  0.5f, // top-left
	 0.5f,  0.5f, // top-right
	 0.5f, -0.5f, // bottom-right
	-0.5f, -0.5f  // bottom-left
	};
	index = new unsigned char[1]{ 1 };

	glGenVertexArrays(1, &vao);
	glGenBuffers(2, vbo);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8, pos, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);


	
	/**
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(index), index, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 1, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(unsigned char), (void*)0);
	glEnableVertexAttribArray(1);
	*/
	//glEnableVertexAttribArray(0);
	//glBindVertexArray(0);

	//glViewport(0, 0, Main::width(), Main::height());


	/**
	assetToLoad.emplace_back(new LoadItem("assets/tex/Forest_soil_diffuse.png",Type::texture));
	assetToLoad.emplace_back(new LoadItem("assets/tex/ForestCliff_basecolor.png", Type::texture));
	assetToLoad.emplace_back(new LoadItem("assets/tex/ForestDirt_diffuse.png", Type::texture));
	
	assetToLoad.emplace_back(new LoadItem("assets/tex/ForestGrass_basecolor.png", Type::texture));
	assetToLoad.emplace_back(new LoadItem("assets/tex/ForestMoss_basecolor.png", Type::texture));
	assetToLoad.emplace_back(new LoadItem("assets/tex/ForestMud_baseColor.png", Type::texture));
	
	assetToLoad.emplace_back(new LoadItem("assets/tex/ForestRoad_diffuse.png", Type::texture));
	assetToLoad.emplace_back(new LoadItem("assets/tex/ForestRock_basecolor.png", Type::texture));
	assetToLoad.emplace_back(new LoadItem("assets/tex/ForestWetMud_baseColor.png", Type::texture));
*/
	//assetToLoad.emplace_back(new LoadItem("assets/fonts/black.ttf", Type::font));
	//assetToLoad.emplace_back(new LoadItem("assets/shader/bg_shader", Type::shader));
	
	auto assets = Main::getAssetManager();
	for (auto a : assetToLoad)
		assets->addAsset(a);
}

void Heerbann::LoadingScreenLevel::load(AssetManager* _manager) {
	//label = new Label("99%", (sf::Font*)((*_manager)["assets/fonts/black.ttf"]->data));
	//label->position = sf::Vector2i(Main::width() / 2, Main::height() / 2);
	//Main::getStage()->add(label);

	//uniformlocation = glGetUniformLocation(bgShader->getNativeHandle(), "tex");
	/**
	tex[0] = (sf::Texture*)(Main::asset_getAsset("assets/tex/Forest_soil_diffuse.png")->data);
	tex[1] = (sf::Texture*)(Main::asset_getAsset("assets/tex/ForestCliff_basecolor.png")->data);
	tex[2] = (sf::Texture*)(Main::asset_getAsset("assets/tex/ForestDirt_diffuse.png")->data);

	tex[3] = (sf::Texture*)(Main::asset_getAsset("assets/tex/ForestGrass_basecolor.png")->data);
	tex[4] = (sf::Texture*)(Main::asset_getAsset("assets/tex/ForestMoss_basecolor.png")->data);
	tex[5] = (sf::Texture*)(Main::asset_getAsset("assets/tex/ForestMud_baseColor.png")->data);

	tex[6] = (sf::Texture*)(Main::asset_getAsset("assets/tex/ForestRoad_diffuse.png")->data);
	tex[7] = (sf::Texture*)(Main::asset_getAsset("assets/tex/ForestRock_basecolor.png")->data);
	tex[8] = (sf::Texture*)(Main::asset_getAsset("assets/tex/ForestWetMud_baseColor.png")->data);
	*/
	//TODO: create buffers

	



}


void Heerbann::LoadingScreenLevel::unload(AssetManager* _manager) {

}

void Heerbann::LoadingScreenLevel::update(float _deltaTime) {

}

void Heerbann::LoadingScreenLevel::draw(float _deltaTime, sf::RenderWindow& _window) {
	//sf::Color clearColor = sf::Color::White;
	//glClearColor(1.f / 255.f * clearColor.r, 1.f / 255.f * clearColor.g, 1.f / 255.f * clearColor.b, 1.f / 255.f * clearColor.a);
	//glClear(GL_COLOR_BUFFER_BIT);

	//_window.resetGLStates();
	//_window.pushGLStates(); 
	//glUseProgram(bgShader->getNativeHandle());
	sf::Shader::bind(bgShader);
	glUniformMatrix4fv(uniformlocation, 1, false, Main::getViewport()->cam.getTransform().getMatrix());
	

	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR) {
		std::cout << err << std::endl;
	}


	/**
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex[0]->getNativeHandle());

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, tex[1]->getNativeHandle());

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, tex[2]->getNativeHandle());

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, tex[3]->getNativeHandle());

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, tex[4]->getNativeHandle());

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, tex[5]->getNativeHandle());

	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, tex[6]->getNativeHandle());

	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, tex[7]->getNativeHandle());

	glActiveTexture(GL_TEXTURE8);
	glBindTexture(GL_TEXTURE_2D, tex[8]->getNativeHandle());
	*/
	
	//const GLint samplers[9]{ 0, 1, 2, 3, 4, 5, 6, 7, 8 };
	//glUniform1iv(uniformlocation, 9, samplers);


	//glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
	  //glDrawArrays(GL_TRIANGLES, 0, 6);
	//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);


	glBindVertexArray(vao);
	glDrawArrays(GL_POINTS, 0, 4);

	glBindVertexArray(0);

	glUseProgram(0);

	//_window.popGLStates();

	
}

//---------------------- MainMenuLevel ----------------------\\

Heerbann::MainMenuLevel::MainMenuLevel() {

}

void Heerbann::MainMenuLevel::load(AssetManager* _manager) {

}

void Heerbann::MainMenuLevel::unload(AssetManager* _manager) {

}

void Heerbann::MainMenuLevel::update(float _deltaTime) {

}

void Heerbann::MainMenuLevel::draw(float _deltaTime, sf::RenderWindow& _window) {

}

