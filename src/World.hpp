#pragma once

#include "MainStruct.hpp"

namespace Heerbann {

#define VOXELS 12 //voxels + 2 border

	using namespace Heerbann;

	struct WorldBuilderDefinition {

		uint heightMapWidth = 1000;
	};

	class VoxelWorld {

		ShaderProgram* voxelComputeShader;
		TextureDebugRenderer* debug;

		GLuint* texture;
		GLuint heightmap;
		GLuint vertexBuffer;
		GLuint vao;

		GLuint bSizeBuffer[2];
		uint* bSizeBufferPntr[2];
		float* bSizeBufferPntrf[2];
		uint bSizeIndex = 0;

		VSMRenderable* renderable;

		void createVAO(GLuint&, GLuint&);

	public:
		VoxelWorld();
		void build(WorldBuilderDefinition*);
		void draw(View*, Renderer*);
	};

}