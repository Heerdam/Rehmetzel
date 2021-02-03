
#include "World.hpp"
#include "Assets.hpp"
#include "Utils.hpp"
#include "Gdx.hpp"
#include "CameraUtils.hpp"
#include "Math.hpp"
#include "Renderer.hpp"

using namespace Heerbann;

World::World() {
	renderer = new VSMRenderer("world_vsm_renderer");
}

void World::build(const WorldBuilderDefinition& _def) {

}












void VoxelWorld::draw(View* _view, Renderer* _renderer) {

	BoundingBox* aabb = _view->getCamera()->frustum->toAABB(_view->getCamera());
	Vec3u num_groups(VOXELS - 2, VOXELS - 2, 1);

	bSizeIndex = (bSizeIndex + 1) % 2;
	auto pntr = bSizeBufferPntr[bSizeIndex];
	pntr[0] = 0;

	pntr[1] = VOXELS - 2;
	pntr[2] = VOXELS - 2;

	pntr[3] = 0;
	pntr[4] = 0;

	pntr[5] = 50;
	pntr[6] = 5;

	voxelComputeShader->bind();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_RECTANGLE, heightmap);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, vertexBuffer);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, bSizeBuffer[bSizeIndex]);

	glDispatchCompute(num_groups.x, num_groups.y, num_groups.z);
	glMemoryBarrier(GL_ALL_BARRIER_BITS | GL_SHADER_STORAGE_BARRIER_BIT | GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT);
	voxelComputeShader->unbind();

	GLError("VoxelWorld::draw");

	renderable->offset = 0;
	renderable->count = static_cast<uint>(bSizeBufferPntr[(bSizeIndex + 1) % 2][0]) * 6;
	renderable->isVoxel = true;
	renderable->texVox = texture;
	renderable->vao = vao;
	_renderer->add(renderable);

	//debug->draw(heightmap, GL_TEXTURE_RECTANGLE, 5, VOXELS, VOXELS, VOXELS * 50, VOXELS * 50);
}


