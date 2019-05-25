#pragma once

#include "MainStruct.hpp"

namespace Heerbann {

	struct Material {
		Vec4 COLOR_DIFFUSE;
		Vec4 COLOR_SPECULAR;
		Vec4 COLOR_AMBIENT;
		Vec4 COLOR_EMISSIVE;
		Vec4 COLOR_TRANSPARENT;
		Vec4 vals; //OPACITY, SHININESS, SHININESS_STRENGTH
	};

	enum AnimBehaviour {
		Default, Constant, Linear, Repeat
	};

	struct mNode {
		std::string id;
		Mat4 transform;
		mNode* parent;
		std::vector<mNode*> children;
		std::vector<uint> meshes;
	};

	struct Bone {
		std::string id;
		uint numWeights;
		std::vector<std::tuple<uint, float>> weights;
		Mat4 offset;

		Bone* parent;
		std::vector<Bone*> children;
	};

	struct NodeAnimation {
		std::string affectedNode;
		uint posKey;
		std::vector<VectorKey> positionKeys;
		std::vector<QuatKey> quatKeys;
		std::vector<VectorKey> scalingKeys;
		AnimBehaviour preState;
		AnimBehaviour postState;
	};

	struct MeshKey {
		float time;
		uint meshIndex;
	};

	struct VectorKey {
		float time;
		Vec3 value;
	};

	struct QuatKey {
		float time;
		Quat value;
	};

	struct MeshAnimation {
		std::string affectedMesh;
		std::vector<MeshKey> keys;
	};

	struct Animation {
		std::string id;
		float duration;
		float ticksPerSecond;
		std::vector<NodeAnimation*> nodeChannels;
		std::vector<MeshAnimation*> meshChannels;
	};

	struct Mesh {
		uint vertexOffset;
		uint vertexCount;
		uint indexOffset;
		uint indexCount;
		uint matIndex;

		Bone* root;
		std::unordered_map<std::string, Bone*> boneMap;
	};

	struct ModelData {
		GLuint vao;
		GLuint vbo;
		GLuint indexBuffer;
		GLuint animBuffer;
		GLuint matBuffer;

		uint vertexBufferCacheSize; //elements
		float* vertexBufferCache = nullptr;

		uint indexBufferCacheSize; //elements
		unsigned int* indexBufferCache = nullptr;

		uint animationCacheSize; //elements
		char* animationCache = nullptr;

		uint materialCacheSize; //elements
		char* materialCache = nullptr;

		uint metaCacheSize; //elements
		char* metaCache = nullptr;

		std::unordered_map<std::string, Mesh*> meshMap;
		std::vector<Mesh*> meshList;

		std::vector<std::vector<Bone*>> boneCache;

		mNode* root;
		std::vector<mNode*> nodeCache;
		std::unordered_map<std::string, mNode*> nodeMap;

	};
	
}
