#include "AI.hpp"
#include "Utils.hpp"
#include "Assets.hpp"

using namespace Heerbann;
















/*
#define size 10

#define CELLSX 50
#define CELLSY 50
#define MAP_WIDTH 1000.0
#define MAP_HEIGHT 1000.0

void AI::AIHandler::create() {

	shader = reinterpret_cast<ShaderProgram*> (Main::getAssetManager()->getAsset("assets/shader/AiTest")->data);
	compShader = reinterpret_cast<ShaderProgram*> (Main::getAssetManager()->getAsset("assets/shader/AiTestComp")->data);

	glGenBuffers(1, &ssbo_debug);
	glGenBuffers(1, &ssbo_spatial);
	glGenBuffers(1, &ssbo_entity);


	//debugbuffer
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_debug);	
	int s = sizeof(Debug) * size;
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, s, nullptr, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo_debug);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);


	//entitybuffer

	float dx = 25;
	float dy = 15;

	Entitiy entities[size];
	for (int i = 0; i < size; ++i) {
		auto& e = entities[i];
		e.isActive = true;
		e.id = i;
		e.position[0] = i * dx;
		e.position[1] = 0;
		float angle = 2 * b2_pi * Main::getRandom();
		e.dir[0] = std::cosf(angle);
		e.dir[1] = std::sinf(angle);
		e.speed = Main::getRandom() * 5.f;
	}

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_entity);
	s = sizeof(Entitiy) * size;
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, s, reinterpret_cast<byte*>(&entities), 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo_entity);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	MapEntry entries[size];

	//spatialbuffer
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_spatial);
	s = sizeof(MapEntry) * size;
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, s, reinterpret_cast<byte*>(&entries), 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssbo_spatial);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);


	//vertex
	unsigned int* data = new unsigned int[size];
	for (unsigned int i = 0; i < size; ++i)
		data[i] = i;

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);

	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(unsigned int) * size, data, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, sizeof(unsigned int), (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	transLoc = glGetUniformLocation(shader->getHandle(), "transform");

	delete data;
	Main::printOpenGlErrors("AI create");
}

void AI::AIHandler::draw(const sf::Transform& _transform) {

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_debug);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_spatial);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_entity);

	//comp
	compShader->bind();
	glDispatchComputeGroupSizeARB(1000, 1, 1, 1, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	compShader->unbind();

	//draw
	shader->bind();
	glUniformMatrix4fv(transLoc, 1, false, _transform.getMatrix());
	glBindVertexArray(vao);
	glDrawArrays(GL_POINTS, 0, size);
	glBindVertexArray(0);
	shader->unbind();

	Main::printOpenGlErrors("ai draw");

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

}
*/
