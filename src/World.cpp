
#include "World.hpp"
#include "Assets.hpp"
#include "Utils.hpp"
#include "Gdx.hpp"
#include "CameraUtils.hpp"
#include "Math.hpp"

using namespace Heerbann;

void VoxelWorld::createVAO(GLuint& _vao, GLuint& _vertexBuffer) {

	const uint isize = VOXELS * VOXELS * 5 * 2 * 3;
	static uint indices[isize];
	for (uint i = 0, k = 0; i < isize; i += 6, k += 4) {
		indices[i] = k;
		indices[i + 1] = k + 1;
		indices[i + 2] = k + 2;

		indices[i + 3] = k + 2;
		indices[i + 4] = k + 3;
		indices[i + 5] = k;
	}

	GLuint index;
	const uint vertexSize = (3 + 3 + 3 + 3 + 3) * sizeof(float);

	glGenVertexArrays(1, &_vao);
	glGenBuffers(1, &index);
	glGenBuffers(1, &_vertexBuffer);

	glBindVertexArray(_vao);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertexSize * 4 * 5 * static_cast<uint>(POW(VOXELS, 2)), nullptr, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0); //a_Pos
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vertexSize, (void*)0);

	glEnableVertexAttribArray(1); //a_norm
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, vertexSize, (void*)(3 * sizeof(float)));

	glEnableVertexAttribArray(2); //a_uv
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, vertexSize, (void*)(6 * sizeof(float)));

	glEnableVertexAttribArray(3); //a_tang
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, vertexSize, (void*)(9 * sizeof(float)));

	glEnableVertexAttribArray(4); //a_bitang
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, vertexSize, (void*)(12 * sizeof(float)));

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	GLError("VoxelWorld::build_c");

	debug = new TextureDebugRenderer();

}

VoxelWorld::VoxelWorld() {
	
	renderable = new VSMRenderable();
}

void VoxelWorld::build(WorldBuilderDefinition* _def) {
	GLError("VoxelWorld::build_0");
	voxelComputeShader = reinterpret_cast<ShaderProgram*>(M_Asset->getAsset("assets/shader/voxel/shader_voxel_builder")->data);

	ushort* heightMapCache = new ushort[static_cast<uint>(POW(VOXELS, 2))]{ 0 };

	for (uint x = 1; x < VOXELS - 1; ++x) {
		for (uint y = 1; y < VOXELS - 1; ++y) {
			heightMapCache[y + x * VOXELS] = static_cast<uint>(1 + M_Random * (25 - 1));
		}
	}
	
	//heightmap
	glGenTextures(1, &heightmap);
	glBindTexture(GL_TEXTURE_RECTANGLE, heightmap);
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_R16UI, VOXELS, VOXELS, 0, GL_RED_INTEGER, GL_UNSIGNED_SHORT, heightMapCache);
	//glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	//glTexSubImage2D(GL_TEXTURE_RECTANGLE, 0, 0, 0, VOXELS + 2, VOXELS + 2, GL_RED_INTEGER, GL_UNSIGNED_SHORT, heightMapCache);
	glBindTexture(GL_TEXTURE_RECTANGLE, 0);
	GLError("VoxelWorld::build_a");

	//buffersize buffers
	glGenBuffers(2, bSizeBuffer);
	for (uint i = 0; i < 2; ++i) {
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, bSizeBuffer[i]);
		glBufferStorage(GL_SHADER_STORAGE_BUFFER, 7 * sizeof(uint), nullptr,
			GL_MAP_PERSISTENT_BIT | GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);
		bSizeBufferPntr[i] = reinterpret_cast<uint*>(glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, 7 * sizeof(float),
			GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT));
		bSizeBufferPntrf[i] = reinterpret_cast<float*>(bSizeBufferPntr[i]);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}
	GLError("VoxelWorld::build_b");

	createVAO(vao, vertexBuffer);

	texture = reinterpret_cast<GLuint*>(M_Asset->getAsset("bgTex")->data);

	delete _def; 
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
