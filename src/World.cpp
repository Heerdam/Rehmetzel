
#include "World.hpp"
#include "Assets.hpp"
#include "Utils.hpp"

#include <math.h>

using namespace Heerbann;

void World::raycast(b2RayCastCallback* callback, const sf::Vector2f& _p1, const sf::Vector2f& _p2) {
	std::lock_guard<std::mutex> guard(worldLock);
	bworld->RayCast(callback, b2Vec2(_p1.x, _p1.y), b2Vec2(_p2.x, _p2.y));
};

void World::AABB(b2QueryCallback* callback, const sf::Vector2f& _p1, const sf::Vector2f& _p2) {
	std::lock_guard<std::mutex> guard(worldLock);
	b2AABB aabb;
	aabb.lowerBound.Set(_p1.x, _p1.y);
	aabb.upperBound.Set(_p2.x, _p2.y);
	bworld->QueryAABB(callback, aabb);
}

World::World() {
	bworld->SetDebugDraw(debug = new DebugDraw());
}

void World::update(float _deltaTime) {
	std::lock_guard<std::mutex> g1(mapLock);
	while (!finishQueue.empty()) {
		WorldObject* object = finishQueue.front();
		finishQueue.pop();
		object->isLoaded = true;
		if (object->finishedLoading != nullptr)
			object->finishedLoading(object);
	}
	std::lock_guard<std::mutex> g2(worldLock);
	bworld->Step(_deltaTime, 8, 3);
}

void World::debugDraw() {
	bworld->DrawDebugData();
}

long World::create(EntityType _type, sf::Vector2f _pos) {
	WorldObject *ob = new WorldObject();

	switch (_type) {
	case red_deer:
	{

	}
	break;
	case tree:
	{
		std::lock_guard<std::mutex> g1(worldLock);
		b2BodyDef* bodyDef = new b2BodyDef();
		bodyDef->userData = ob;
		bodyDef->type = b2BodyType::b2_staticBody;
		bodyDef->position = b2Vec2(_pos.x * UNRATIO, _pos.y * UNRATIO);
		ob->body = bworld->CreateBody(bodyDef);

		b2CircleShape circle;
		circle.m_p.Set(0.f, 0.f);
		circle.m_radius = 1;

		b2FixtureDef def;
		def.userData = ob;
		def.shape = &circle;

		ob->body->CreateFixture(&def);

		/**
		ob->finishedLoading = [](WorldObject* _object)->void {
			sf::Texture* tex = new sf::Texture();
			tex->loadFromImage(*(sf::Image*)(Main::asset_getAsset("assets/trees/poplar_07_top.png")->data)); //TODO
			_object->sprite = new sf::Sprite(*tex);
			_object->sprite->setOrigin(_object->sprite->getGlobalBounds().width * 0.5f, _object->sprite->getGlobalBounds().height * 0.5f);
			
			b2PolygonShape shape;
			shape.SetAsBox(_object->sprite->getGlobalBounds().width * 0.5f * UNRATIO, _object->sprite->getGlobalBounds().height * 0.5f * UNRATIO);

			b2FixtureDef def;
			def.isSensor = true;
			def.shape = &shape;
			
			_object->body->CreateFixture(&def);
		};

		ob->draw = [](WorldObject* _object, float, sf::RenderWindow& _window)->void {	
			_window.resetGLStates();
			auto pos = _object->body->GetPosition();
			_object->sprite->setPosition(sf::Vector2f(pos.x * RATIO, pos.y * RATIO));
			_window.draw(*_object->sprite);
		};
		*/
		std::lock_guard<std::mutex> g2(mapLock);
		finishQueue.emplace(ob);
		objects[ob->id] = ob;
	}
	break;
	}
	return ob->id;
}
//pos: lower left corner
BGVAO* WorldBuilder::createBGVAO(sf::Vector2f _pos, int _tex) {
	int cellvertex = BG_CELLCOUNT * BG_CELLCOUNT;

	float* data = new float[3 * cellvertex];

	float xw = cosf(DEGTORAD * 30.f) * BG_CELLDIAMETER;
	float yw = 1.5f * BG_CELLDIAMETER;

	for (int y = 0; y < BG_CELLCOUNT; ++y) {
		for (int x = 0; x < BG_CELLCOUNT; ++x) {

			int index = 3 * (y * BG_CELLCOUNT + x);

			data[index] = x * 2 * xw - ((y % 2 == 0) ? xw : 0);
			data[index + 1] = y * yw;
			data[index + 2] = (float)_tex;
		}
	}

	BGVAO* out = new BGVAO();
	out->set(data, cellvertex, 3);

	return out;
}

IndexedVAO* WorldBuilder::createTrees(int _treeCount, sf::Vector2f _low, sf::Vector2f _high) {

	TextureAtlas* atlas = (TextureAtlas*)Main::getAssetManager()->getAsset("assets/trees/trees")->data;

	int treeSpriteCount = atlas->regions.size();

	float* vertex = new float[_treeCount * 4 * (2 + 2 + 1)]; //#trees * 4 vertices/tree * (position + uv + index)
	GLuint* index = new GLuint[_treeCount * 2 * 3]; //#trees * triangles/tree * vertex/triangle

	float xw = cosf(DEGTORAD * 30.f) * BG_CELLDIAMETER;
	float yw = 1.5f * BG_CELLDIAMETER;

	auto world = Main::getWorld();	

	for (int i = 0; i < _treeCount; ++i) {

		sf::Vector2f pos(Main::getRandom(_low.x, _high.x), Main::getRandom(_low.y, _high.y));

		AtlasRegion* region = (*atlas)[(int)(Main::getRandom(0.f, (float)treeSpriteCount))];

		sf::Vector2f u = region->getU();
		sf::Vector2f v = region->getV();

		float w = region->width * 0.5f;
		float h = region->height * 0.5f;

		vertex[20 * i] = pos.x + w; //x
		vertex[20 * i + 1] = pos.y - h; //y
		vertex[20 * i + 2] = (float)region->texIndex;
		vertex[20 * i + 3] = u.y; //u
		vertex[20 * i + 4] = v.x; //v

		vertex[20 * i + 5] = pos.x + w; //x
		vertex[20 * i + 6] = pos.y + h; //y
		vertex[20 * i + 7] = (float)region->texIndex;
		vertex[20 * i + 8] = u.y; //u
		vertex[20 * i + 9] = v.y; //v

		vertex[20 * i + 10] = pos.x - w; //x
		vertex[20 * i + 11] = pos.y + h; //y
		vertex[20 * i + 12] = (float)region->texIndex;
		vertex[20 * i + 13] = u.x; //u
		vertex[20 * i + 14] = v.y; //v

		vertex[20 * i + 15] = pos.x - w; //x
		vertex[20 * i + 16] = pos.y - h; //y
		vertex[20 * i + 17] = (float)region->texIndex;
		vertex[20 * i + 18] = u.x; //u
		vertex[20 * i + 19] = v.x; //v

		//t1
		index[6 * i] = 4 * i;
		index[6 * i + 1] = 4 * i + 1;
		index[6 * i + 2] = 4 * i + 2;

		//t2
		index[6 * i + 3] = 4 * i + 2;
		index[6 * i + 4] = 4 * i + 3;
		index[6 * i + 5] = 4 * i;

		WorldObject *ob = new WorldObject();
		world->objects[ob->id] = ob;
		ob->isLoaded = true;
		ob->isVAO = true;

		b2BodyDef* bodyDef = new b2BodyDef();
		bodyDef->userData = ob;
		bodyDef->type = b2BodyType::b2_staticBody;
		bodyDef->position = b2Vec2(pos.x * UNRATIO, pos.y * UNRATIO);
		ob->body = world->bworld->CreateBody(bodyDef);

		b2CircleShape circle;
		circle.m_p.Set(0.f, 0.f);
		circle.m_radius = 1;

		b2FixtureDef def;
		def.userData = ob;
		def.shape = &circle;

		ob->body->CreateFixture(&def);

		b2PolygonShape shape;
		shape.SetAsBox(w * UNRATIO, h * UNRATIO);

		b2FixtureDef def1;
		def1.isSensor = true;
		def1.shape = &shape;

		ob->body->CreateFixture(&def1);
	}

	IndexedVAO* out = new IndexedVAO();
	out->set(vertex, index, _treeCount * 4, 5);
	return out;
}

WorldOut * WorldBuilder::build(const WorldBuilderDefinition& _def) {
	WorldOut* out = new WorldOut();
	Main::setSeed(_def.seed);

	out->bgVAOs.emplace_back(createBGVAO(sf::Vector2f(0, 0), 4));
	out->indexVAOs.emplace_back(createTrees(10, sf::Vector2f(0, 0), sf::Vector2f(cosf(DEGTORAD * 30.f) * BG_CELLDIAMETER * BG_CELLCOUNT, 1.5f * BG_CELLDIAMETER * BG_CELLCOUNT)));

	return out;
}

void WorldOut::finalize(sf::Shader* _bg, sf::Shader* _tree) {
	for (auto v : bgVAOs)
		v->build(_bg);
	for (auto v : indexVAOs)
		v->build(_tree);
}
