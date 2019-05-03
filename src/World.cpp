
#include "World.hpp"
#include "Assets.hpp"
#include "Utils.hpp"

#include <math.h>

using namespace Heerbann;

World::World() {

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

}

void World::debugDraw() {
	
}

long long World::create(EntityType _type, Vec2 _pos) {
	return 0l;
}
/*
//pos: lower left corner
BGVAO* WorldBuilder::createBGVAO(Vec2 _pos, int _tex) {
	int cellvertex = 50 * 50;

	float* data = new float[3 * cellvertex];

	float xw = cosf(DEGTORAD * 30.f) * 50.f;
	float yw = 1.5f * 50.f;

	for (int y = 0; y < 50; ++y) {
		for (int x = 0; x < 50; ++x) {

			int index = 3 * (y * 50 + x);

			data[index] = x * 2 * xw - ((y % 2 == 0) ? xw : 0);
			data[index + 1] = y * yw;
			data[index + 2] = (float)_tex;
		}
	}

	BGVAO* out = new BGVAO();
	out->set(data, cellvertex, 3);

	return out;
}

IndexedVAO* WorldBuilder::createTrees(int _treeCount, Vec2 _low, Vec2 _high) {

	TextureAtlas* atlas = (TextureAtlas*)M_Asset->getAsset("assets/trees/trees")->data;

	uint treeSpriteCount = static_cast<uint>(atlas->regions.size());

	float* vertex = new float[_treeCount * 4 * (2 + 2 + 1)]; //#trees * 4 vertices/tree * (position + uv + index)
	GLuint* index = new GLuint[_treeCount * 2 * 3]; //#trees * triangles/tree * vertex/triangle

	float xw = cosf(DEGTORAD * 30.f) * 50.f;
	float yw = 1.5f * 50.f;

	for (int i = 0; i < _treeCount; ++i) {

		Vec2 pos(M_Main->getRandom(_low.x, _high.x), M_Main->getRandom(_low.y, _high.y));
		//Vec2 pos(0, 0);
		AtlasRegion* region = (*atlas)[(int)(M_Main->getRandom(0.f, (float)treeSpriteCount))];

		//AtlasRegion* region = (*atlas)["strees_05_top"];
		Vec2 u = region->getU();
		Vec2 v = region->getV();

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
		M_World->objects[ob->id] = ob;
		ob->isLoaded = true;
		ob->isVAO = true;

		b2BodyDef* bodyDef = new b2BodyDef();
		bodyDef->userData = ob;
		bodyDef->type = b2BodyType::b2_staticBody;
		bodyDef->position = b2Vec2(pos.x * UNRATIO, pos.y * UNRATIO);
		ob->body = M_World->bworld->CreateBody(bodyDef);

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
	M_Main->setSeed(_def.seed);

	out->bgVAOs.emplace_back(createBGVAO(Vec2(0, 0), 4));
	//out->indexVAOs.emplace_back(createTrees(50, Vec2(0, 0), Vec2(cosf(DEGTORAD * 30.f) * BG_CELLDIAMETER * BG_CELLCOUNT, 1.5f * BG_CELLDIAMETER * BG_CELLCOUNT)));

	return out;
}

void WorldOut::finalize(ShaderProgram* _bg, ShaderProgram* _tree) {
	for (auto v : bgVAOs)
		v->build(_bg);
	//for (auto v : indexVAOs)
		//v->build(_tree);
}
*/