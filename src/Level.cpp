
#include "Level.h"
#include "Assets.hpp"
#include "UI.hpp"
#include "CameraUtils.hpp"
#include "Utils.hpp"
#include "World.hpp"
#include "TextUtil.hpp"
#include "AI.hpp"
#include "InputMultiplexer.hpp"
#include "Gdx.hpp"

using namespace Heerbann;
using namespace UI;

Level::Level(std::string _id) : id(_id) {
	neededLoadingState = State::discrete;
}

void LevelManager::initialize() {
	M_Asset->addLevel("PreLoadLevel", new PreLoadLevel());
	M_Asset->addLevel("LoadingScreenLevel", new LoadingScreenLevel());
	M_Asset->addLevel("MainMenuLevel", new MainMenuLevel());
	M_Asset->addLevel("TestWorldLevel", new TestWorldLevel());

	queueLevelToLoad("PreLoadLevel");
};

void LevelManager::loadLevel(Level* _level) {
	_level->preLoad();
	for (auto a : _level->assetToLoad) {
		if (!M_Asset->exists(a->id))
			M_Asset->addAsset(a);
		M_Asset->load(a->id);
	}		
	for (auto a : _level->assetToUnload) {
		if (M_Asset->exists(a->id))
			M_Asset->unload(a->id);
	}
	M_Asset->loadLevel(_level->id);
	if (_level->neededLoadingState == State::discrete && _level->lockIfDiscrete)
		M_Asset->finish();
	_level->assetToLoad.clear();
	_level->assetToUnload.clear();
	activeLevels.emplace_back(_level);
	_level->postLoad();
}

void LevelManager::unloadLevel(Level* _level) {
	_level->preUnload();
	for (auto a : _level->assetToLoad) {
		if (!M_Asset->exists(a->id))
			M_Asset->addAsset(a);
		M_Asset->load(a->id);
	}
	for (auto a : _level->assetToUnload) {
		if (M_Asset->exists(a->id))
			M_Asset->unload(a->id);
	}
	M_Asset->unloadLevel(_level->id);
	_level->assetToLoad.clear();
	_level->assetToUnload.clear();
	for (auto it = activeLevels.begin(); it != activeLevels.end(); ++it)
		if (*it == _level) {
			activeLevels.erase(it);
			break;
		}
}

void LevelManager::update() {
	//this is used so that loading isnt caught in a loop
	if (!loadCache.empty()) {
		for (Level* l : loadCache)
			toLoad.emplace(l);
		loadCache.clear();
	}
	if (!unloadCache.empty()) {
		for (Level* l : unloadCache)
			toUnload.emplace(l);
		unloadCache.clear();
	}
	while (!toUnload.empty()) {
		Level* next = toUnload.front();
		toUnload.pop();
		unloadLevel(next);
	}
	while (!toLoad.empty()) {
		Level* next = toLoad.front();
		toLoad.pop();
		loadLevel(next);
	}
	for (auto l : activeLevels) {
		if (!l->isLocked && l->isLoaded)
			l->update();
	}
}

void LevelManager::draw() {
	for (auto l : activeLevels) {
		if (!l->isLocked && l->isLoaded)
			l->draw();
	}
}

void LevelManager::queueLevelToLoad(std::string _id) {
	Level* level = M_Asset->getLevel(_id);
	queueLevelToLoad(level);
}

void LevelManager::queueLevelToUnLoad(std::string _id) {
	Level* level = M_Asset->getLevel(_id);
	queueLevelToUnLoad(level);
}

void LevelManager::queueLevelToLoad(Level* _level) {
	loadCache.emplace_back(_level);
}

void LevelManager::queueLevelToUnLoad(Level* _level) {
	unloadCache.emplace_back(_level);
}

//---------------------- PreLoadLevel ----------------------\\

void PreLoadLevel::preLoad() {

	assetToLoad.emplace_back(new LoadItem("assets/shader/simple forward/sb_sf", Type::shader));
	assetToLoad.emplace_back(new LoadItem("assets/shader/simple forward/sb_sf_vsm", Type::shader));
	assetToLoad.emplace_back(new LoadItem("assets/shader/simple forward/sb_sf_vsm_notex", Type::shader));
	assetToLoad.emplace_back(new LoadItem("assets/shader/simple forward/sb_vsm", Type::shader));
	assetToLoad.emplace_back(new LoadItem("assets/shader/simple forward/sb_vsm", Type::shader));
	assetToLoad.emplace_back(new LoadItem("assets/shader/simple forward/sb_sf_blur", Type::shader));
	assetToLoad.emplace_back(new LoadItem("assets/shader/fbo_debug_shader", Type::shader));
	assetToLoad.emplace_back(new LoadItem("assets/shader/voxel/shader_voxel_builder", Type::shader));
	assetToLoad.emplace_back(new LoadItem("assets/shader/voxel/shader_voxel_draw", Type::shader));

	//assetToLoad.emplace_back(new LoadItem("assets/fonts/black.ttf", Type::font));

	
	std::vector<std::string> postfix;
	postfix.emplace_back("_ao.png");
	postfix.emplace_back("_basecolor.png");
	postfix.emplace_back("_height.png");
	postfix.emplace_back("_normal.png");
	postfix.emplace_back("_roughness.png");

	for (auto pf : postfix) {	
		assetToLoad.emplace_back(new LoadItem("assets/tex/forest_soil" + pf, Type::image_png));
		assetToLoad.emplace_back(new LoadItem("assets/tex/forest_cliff" + pf, Type::image_png));
		assetToLoad.emplace_back(new LoadItem("assets/tex/forest_dirt" + pf, Type::image_png));
		assetToLoad.emplace_back(new LoadItem("assets/tex/forest_grass" + pf, Type::image_png));
		assetToLoad.emplace_back(new LoadItem("assets/tex/forest_moss" + pf, Type::image_png));
		assetToLoad.emplace_back(new LoadItem("assets/tex/forest_mud" + pf, Type::image_png));
		assetToLoad.emplace_back(new LoadItem("assets/tex/forest_road" + pf, Type::image_png));
		assetToLoad.emplace_back(new LoadItem("assets/tex/forest_rock" + pf, Type::image_png));
		assetToLoad.emplace_back(new LoadItem("assets/tex/forest_wet_mud" + pf, Type::image_png));
	}
	
	//assetToLoad.emplace_back(new LoadItem("assets/shader/bg_shader", Type::shader));

	//assetToLoad.emplace_back(new LoadItem("assets/shader/bg_shader", Type::shader));
	//assetToLoad.emplace_back(new LoadItem("assets/shader/tree_shader", Type::shader));
	//assetToLoad.emplace_back(new LoadItem("assets/shader/model_shader", Type::shader));
	//assetToLoad.emplace_back(new LoadItem("assets/shader/AiTest", Type::shader));
	//assetToLoad.emplace_back(new LoadItem("assets/shader/AiTestComp", Type::shader));

	//assetToLoad.emplace_back(new LoadItem("assets/trees/trees", Type::atlas));

	assetToLoad.emplace_back(new LoadItem("assets/3d/deer/Model/Deer_old.dae", Type::model));
	assetToLoad.emplace_back(new LoadItem("assets/3d/deer/Textures/Deer Common.png", Type::texture_png));
}

void PreLoadLevel::postLoad() {
	M_Level->queueLevelToUnLoad(this);
	M_Level->queueLevelToLoad("LoadingScreenLevel");
}

//---------------------- LoadingScreenLevel ----------------------\\

void LoadingScreenLevel::preLoad() {
	
}

void LoadingScreenLevel::load() {
	//label = new Label("99%", (sf::Font*)((*_manager)["assets/fonts/black.ttf"]->data));
	//label->position = sf::Vector2i(Main::width() / 2, Main::height() / 2);
	//Main::getStage()->add(label);

}

void LoadingScreenLevel::preUnload() {

}

void LoadingScreenLevel::postLoad() {
	M_Level->queueLevelToUnLoad(this);
	M_Level->queueLevelToLoad("TestWorldLevel");
}

void LoadingScreenLevel::unload() {
	
}

void LoadingScreenLevel::update() {

}

//---------------------- MainMenuLevel ----------------------\\

void MainMenuLevel::load() {

}

void MainMenuLevel::unload() {

}

void MainMenuLevel::update() {

}

//---------------------- TestWorldLevel ----------------------\\

void TestWorldLevel::preLoad() {
	
}

void TestWorldLevel::load() {	
	//WorldBuilderDefinition def;
	//world = Main::getWorld()->builder->build(def);	

}

void TestWorldLevel::postLoad() {	
	//world->finalize(bgShader, treeShader);
	//Main::getAI()->create();
	//bgShader = reinterpret_cast<ShaderProgram*>(Main::getAssetManager()->getAsset("assets/shader/bg_shader")->data);
	//treeShader = reinterpret_cast<ShaderProgram*>(Main::getAssetManager()->getAsset("assets/shader/tree_shader")->data);
	
	
	model = reinterpret_cast<Model*>(M_Asset->getAsset("assets/3d/deer/Model/Deer_old.dae")->data);
	mTex = reinterpret_cast<sf::Texture*>(M_Asset->getAsset("assets/3d/deer/Textures/Deer Common.png")->data);


	App::Gdx::printOpenGlErrors("test");
	view = M_View->create("main", ViewType::pers, true);
	view->setViewportBounds(0, 0, M_WIDTH, M_HEIGHT);
	auto cam = reinterpret_cast<ArcballCamera*>(view->getCamera());
	cam ->distance = 400.f;
	cam->fieldOfView = 67.f;
	cam->nearPlane = 0.1f;
	cam->farPlane = 100000.f;
	cam->arcball(cam->target, cam->azimuth, cam->height, cam->distance);
	
	view->setInteractive(true);
	view->panXModifier = 0.25f;
	view->panYModifier = 0.2f;
	view->zoomBounds = Vec2(10.f, 10000.f);
	view->zoomModifier = 25.f;

	sun = new sLight();
	sun->color = Vec4(colF(sf::Color::White.r), colF(sf::Color::White.g), colF(sf::Color::White.b), colF(sf::Color::White.a));
	sun->direction = NOR(Vec4(-1.f, -1.f, 0.f, 0.f));
	sun->dis.x = 0.5f;

	sl = M_Env->addLight("sun", LightType::Directionallight, false, sun);
	sl->shadowMap = new ShadowMap(1024, 1024);
	sl->shadowMap->begin();
	sl->shadowMap->addTex("depth", GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT, GL_NEAREST, GL_CLAMP_TO_BORDER);
	sl->shadowMap->addTex("color", GL_RG32F, GL_RG, GL_FLOAT, GL_NEAREST, GL_CLAMP_TO_BORDER);
	sl->shadowMap->end();

	/*	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, ssize, ssize, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, ssize, ssize, 0, GL_RG, GL_FLOAT, nullptr);	
	*/

	floorModel = new Model();
	
	Mesh* floorMesh = new Mesh();
	floorMesh->indexCount = 6;
	floorMesh->indexOffset = 0;
	floorModel->meshList.emplace_back(floorMesh);

	float col = M_FloatBits(sf::Color::Green);

	float vertices[] = {
		5000.f,
		0.0f,
		5000.f,
		0.f,
		1.f,
		0.f,
		col,

		5000.f,
		0.0f,
		-5000.f,
		0.f,
		1.f,
		0.f,
		col,

		-5000.f,
		0.0f,
		-5000.f,
		0.f,
		1.f,
		0.f,
		col,

		-5000.f,
		0.0f,
		5000.f,
		0.f,
		1.f,
		0.f,
		col
	};

	unsigned int indices[] = { 
		0, 1, 3,  
		1, 2, 3   
	};

	glGenVertexArrays(1, &floorModel->vao);

	GLuint index, vertex;
	glGenBuffers(1, &index);
	glGenBuffers(1, &vertex);

	glBindVertexArray(floorModel->vao);

	glBindBuffer(GL_ARRAY_BUFFER, vertex);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0); //a_Pos
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(1); //a_norm
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));

	glEnableVertexAttribArray(2); //a_Col
	glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, 7 * sizeof(float), (void*)(6 * sizeof(float)));

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	vsm = new VSMRenderer();

	Mesh* mesh = model->meshList[5];

	drawable_1 = new VSMRenderable();
	drawable_1->vao = model->vao;
	drawable_1->count = mesh->indexCount;
	drawable_1->offset = mesh->indexOffset;
	drawable_1->model = model;
	drawable_1->shadowTex = sl->shadowMap->getTex("color");
	drawable_1->matIndex = mesh->matIndex;
	drawable_1->texture = mTex->getNativeHandle();

	drawable_2 = new VSMRenderable();
	drawable_2->hasTex = false;
	drawable_2->vao = floorModel->vao;
	drawable_2->count = floorMesh->indexCount;
	drawable_2->offset = floorMesh->indexOffset;
	drawable_2->model = floorModel;
	drawable_2->shadowTex = sl->shadowMap->getTex("color");

	//textureblocks
	
	std::vector<std::string> files;
	files.emplace_back("assets/tex/forest_soil");
	files.emplace_back("assets/tex/forest_cliff");
	files.emplace_back("assets/tex/forest_dirt");
	files.emplace_back("assets/tex/forest_grass");
	files.emplace_back("assets/tex/forest_moss");
	files.emplace_back("assets/tex/forest_mud");
	files.emplace_back("assets/tex/forest_road");
	files.emplace_back("assets/tex/forest_rock");
	files.emplace_back("assets/tex/forest_wet_mud");

	std::vector<std::string> postFix;
	postFix.emplace_back("_ao.png");
	postFix.emplace_back("_basecolor.png");
	postFix.emplace_back("_height.png");
	postFix.emplace_back("_normal.png");
	postFix.emplace_back("_roughness.png");

	tex = new GLuint[5];
	glGenTextures(5, tex);
	for (uint i = 0; i < 5; ++i) {
		glBindTexture(GL_TEXTURE_2D_ARRAY, tex[i]);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		GLuint internalFormat, format, type;
		uint texSize = POW(2048, 2) * files.size();
	
		switch (i) {
			case 0: //_ao
			{
				format = GL_RED;
				internalFormat = GL_R16;
				type = GL_UNSIGNED_SHORT;
			}
			break;
			case 1://_basecolor
			{
				format = GL_RGBA;
				internalFormat = GL_RGBA8;
				type = GL_UNSIGNED_BYTE;
			}
			break;
			case 2://_height
			{
				format = GL_RED;
				internalFormat = GL_R16;
				type = GL_UNSIGNED_SHORT;
			}
			break;
			case 3://_normal
			{
				format = GL_RGBA;
				internalFormat = GL_RGBA8;
				type = GL_UNSIGNED_BYTE;
			}
			break;
			case 4://_roughness
			{
				format = GL_RED;
				internalFormat = GL_R16;
				type = GL_UNSIGNED_SHORT;
			}
			break;			
		}

		glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, internalFormat, 2048, 2048, 9);
		GLError("TestWorldLevel::postLoad");
		for (uint k = 0; k < files.size(); ++k) {
			auto file = M_Asset->getAsset(files[k] + postFix[i]);
			std::cout << files[k] + postFix[i] << std::endl;
			sf::Image* image = reinterpret_cast<sf::Image*>(file->data);
			glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, k, 2048, 2048, 1, format, type, image->getPixelsPtr());
			GLError("TestWorldLevel::postLoad");
			delete file->data;			
		}

		//glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
		glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
		GLError("TestWorldLevel::postLoad");
	}

	LoadItem* item = new LoadItem("bgTex", Type::texture_png);
	item->isLoaded = true;
	item->isLocked = false;
	item->data = tex;

	M_Asset->addAsset(item);

	

	WorldBuilderDefinition* wdef = new WorldBuilderDefinition();

	M_World->build(wdef);

	debug = new TextureDebugRenderer();
}

void TestWorldLevel::update() {

	angle = std::fmod(++angle, 360.f);

	float cos = COS(TORAD(angle));
	float sin = SIN(TORAD(angle));

	float newX = cos;
	float newY = sin;

	Vec2 tmp = NOR(Vec2(newX, newY)) * -1.f;
	//sun->direction = Vec4(tmp.x, tmp.y, 0.f, 0.f);


	view->clear(sf::Color::White);
	view->apply();	

	//vsm->add(drawable_1);
	//vsm->add(drawable_2);
}

void TestWorldLevel::draw() {
	M_World->draw(view, vsm);
	vsm->draw(view);
	//debug->draw(tex[1], GL_TEXTURE_2D_ARRAY, 6, 2048, 2048, 0, 2048, 2048);
}

