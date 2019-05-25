#pragma once

#include "MainStruct.hpp"

namespace Heerbann {

	enum RenderType {

	};

	class Environment {

		const uint MAXDYNAMICLIGHTS = 10000u;
		const uint MAXSTATICLIGHTS = 100000u;

		View* orthoLightCam;
		View* persLightCam;

		GLuint dLightBuffer[2];
		float* dLightPntr[2];
		uint dlightI = 0;

		AABBTree* staticGeometry;
		AABBTree* dynamicGeometry;
		bool staticGeometryDirty = true;

		std::vector<Light*> dLights;
		std::vector<Light*> sLights;
		bool sLightsDirty = true;
		
		void rebuildLight();

	public:

		Environment();

		void initialize();

		void update();
		Light* addLight(std::string, LightType, bool, sLight*);
		sLight* removeLight(std::string, bool);
		Light* getLight(std::string);

		std::vector<Light*> queryLights(View*);
		
		void bindLights(uint);
		GLuint getDLights();
	};

	struct sLight {
		Vec4 type;
		Vec4 position;
		Vec4 direction;
		Vec4 color;
		Vec4 funcvalues;
		Vec4 dis; //intensity, maxDistance, 0, 0
		Vec4 sl; //sl_innerAngle, sl_outerAngle, sl_maxRadius
	};

	enum LightType {
		Pointlight, Directionallight, Spotlight
	};

	struct Light {

		const std::string id;
		const LightType type;
		bool isStatic = true;
		sLight* light;

		Light(std::string, LightType, bool);
		float radius(float = 0.05f);

		View* cam;

		ShadowMap* shadowMap;

		//distance only needed for directional light
		void bindLightTransform(uint, const Vec3&, float = 250.f, float = 0.f);

	};

	struct GBlurData {
		uint m_blurWidth;
		uint m_blurWidth2;
		float m_weights[65];
	};

}