
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

		std::lock_guard<std::mutex> g2(mapLock);
		finishQueue.emplace(ob);
		objects[ob->id] = ob;
	}
	break;
	}
	return ob->id;
}

void createTrees(int _treeCount, sf::Vector2f _mon, sf::Vector2f _max, float* _out) {

}

WorldOut * WorldBuilder::build(const WorldBuilderDefinition& _def) {
	WorldOut* out = new WorldOut();
	Main::setSeed(_def.seed);

	const float PI = (float)std::atan(1.0) * 4;
	const float toRad = 2 * PI / 360.f;

	int cells = 50;
	float diam = 100;

	int vertex = cells * cells;

	float* data = new float[3 * vertex];

	float xw = cosf(toRad * 30.f) * diam;
	float yw = 1.5f * diam;


	for (int y = 0; y < cells; ++y) {
		for (int x = 0; x < cells; ++x) {

			int index = 3 * (y*cells + x);

			data[index] = x * 2 * xw - ((y % 2 == 0) ? xw : 0);
			data[index + 1] = y * yw;
			data[index + 2] = (y % 2 == 0) ? 3.f : 4.f;

		}
	}


	for (int i = 0; i < 100; ++i) {
		float rx = Main::getRandom();
		float ry = Main::getRandom();
		Main::getWorld()->create(EntityType::tree, sf::Vector2f(2* xw * cells * rx, yw * cells * ry));
	}
		

	out->bgs = new float*[1];
	out->bgs[0] = data;
	out->vertexcount = vertex;

	return out;
}


