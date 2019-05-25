
#include "Gdx.hpp"
#include "Math.hpp"
#include "CameraUtils.hpp"
#include "Utils.hpp"
#include "Assets.hpp"

using namespace Heerbann;

Environment::Environment() {}

void Environment::initialize() {
	staticGeometry = new AABBTree();
	dynamicGeometry = new AABBTree();

	orthoLightCam = M_View->create("orthoLightCam", ViewType::ortho, false);
	persLightCam = M_View->create("persLightCam", ViewType::pers, false);

	glGenBuffers(2, dLightBuffer);
	for (uint i = 0; i < 2; ++i) {
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, dLightBuffer[i]);
		glBufferStorage(GL_SHADER_STORAGE_BUFFER, MAXDYNAMICLIGHTS * sizeof(sLight) + 4 * sizeof(float), nullptr,
			GL_MAP_PERSISTENT_BIT | GL_MAP_WRITE_BIT);
		dLightPntr[i] = reinterpret_cast<float*>(glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, MAXDYNAMICLIGHTS * sizeof(sLight) + sizeof(uint),
			GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_INVALIDATE_RANGE_BIT));
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	GLError("Environment initialization");
}

void Environment::rebuildLight() {
	dlightI = (dlightI + 1) % 2;
	dLightPntr[dlightI][0] = static_cast<float>(dLights.size());
	for (uint i = 0; i < dLights.size(); ++i) {
		Light* l = dLights[i];
		std::memcpy(dLightPntr[dlightI] + 4 + i * sizeof(sLight)/sizeof(float), static_cast<float*>(static_cast<void*>(l->light)), sizeof(sLight));
	}
}

void Environment::update() {

	//update lights
	rebuildLight();

	//update aabb tree
	//if (staticGeometryDirty) {
	//	staticGeometryDirty = false;
	//	staticGeometry->rebuild();
	//}
	//dynamicGeometry->rebuild();

}

Light* Environment::addLight(std::string _id, LightType _type, bool _isStatic, sLight* _light) {
	Light* light = new Light(_id, _type, _isStatic);
	light->light = _light;
	switch (_type) {
	case LightType::Pointlight:
		_light->type.x = 0.f;
		light->cam = persLightCam;
		break;
	case LightType::Directionallight:
		_light->type.x = 2.f;
		light->cam = orthoLightCam;
		break;
	case LightType::Spotlight:
		_light->type.x = 1.f;
		light->cam = persLightCam;
		break;
	}
	if (_isStatic) {
		sLights.emplace_back(light); 
		sLightsDirty = true;
	} else
		dLights.emplace_back(light);
	return light;
}

sLight* Environment::removeLight(std::string _id, bool _isStatic) {
	if (_isStatic) {
		for (auto it = sLights.begin(); it != sLights.end(); ++it) {
			if ((*it)->id == _id) {
				sLightsDirty = true;				
				auto tmp = (*it);
				sLights.erase(it);
				auto out = tmp->light;
				delete tmp;
				return out;
			}
		}
	} else {
		for (auto it = dLights.begin(); it != dLights.end(); ++it) {
			if ((*it)->id == _id) {				
				auto tmp = (*it);
				dLights.erase(it);
				auto out = tmp->light;				
				delete tmp;
				return out;
			}
		}
	}
	return nullptr;
}

Light* Environment::getLight(std::string _id) {
	return nullptr;
}

std::vector<Light*> Environment::queryLights(View*) {
	return dLights; //TODO
}

void Environment::bindLights(uint _binding) {
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, _binding, getDLights());
	GLError("Environment::bindLights");
}

GLuint Environment::getDLights() {
	return dLightBuffer[dlightI];
}

float Light::radius(float _cutOff) {
	return -1.f * (_cutOff * light->funcvalues[1] + SQRT(_cutOff * (4.f * light->funcvalues[2] + _cutOff * (std::pow(light->funcvalues[1], 2) - 4.f * light->funcvalues[0]  * light->funcvalues[2])))) / (2.f * _cutOff *light->funcvalues[2]);
}

void Light::bindLightTransform(uint _location, const Vec3& _target, float _farPlane, float _distance) {
	cam->setViewportBounds(0, 0, shadowMap->bounds.x, shadowMap->bounds.y);
	cam->getCamera()->farPlane = _farPlane;
	if (type == LightType::Directionallight)
		cam->getCamera()->position = Vec4(-light->direction * _distance);
	else cam->getCamera()->position = Vec4(light->position);
	cam->getCamera()->lookAt(Vec4(_target, 1.f));
	cam->apply();
	glUniformMatrix4fv(_location, 1, false, cam->combined());
	GLError("Light::bindLightTransform");
}

Light::Light(std::string _id, LightType _type, bool _isStatic) : id(_id), type(_type), isStatic(_isStatic) {
	cam = M_View->create("light", ViewType::ortho, false);
}





GaussianBlur::GaussianBlur(uint _width, float _deviation) {
	data.m_blurWidth = _width;
	float total = 0.f; //current;

	// Make sure the BlurData is within the valid range.
	if (data.m_blurWidth < 1) data.m_blurWidth = 1;
	if (data.m_blurWidth > 32) data.m_blurWidth = 32;
	data.m_blurWidth2 = 2 * data.m_blurWidth;

	// Calculate the original normal distribution.
	//for (uint i = 0; i < data.m_blurWidth; ++i) {
		//current = std::normal_distribution<float>(FLOAT(data.m_blurWidth - i), 0.f)(_deviation);
		//data.m_weights[i] = data.m_weights[data.m_blurWidth2 - i] = current;
		//total += 2.f * current;
	//}
	//data.m_weights[data.m_blurWidth] = std::normal_distribution<float>(0.0f, 0.0f)(_deviation);
	total += data.m_weights[data.m_blurWidth];

	// Normalize the values so that they sum to 1
	for (uint i = 0; i <= data.m_blurWidth2; ++i) {
		data.m_weights[i] /= total;
	}

	glGenBuffers(1, &uniformBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, uniformBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GBlurData), static_cast<char*>(static_cast<void*>(&data)), GL_STATIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	blurShader = new ShaderProgram("assets/shader/simple forward/sb_sf_blur.comp");
}

GaussianBlur::~GaussianBlur() {
	glDeleteBuffers(1, &uniformBuffer);
}

void GaussianBlur::blur(GLuint _tex[2]) {
	blurShader->bind();
	glActiveTexture(GL_TEXTURE0);
	glBindImageTexture(0, _tex[0], 0, false, 0, GL_READ_ONLY, GL_R32F);
	glActiveTexture(GL_TEXTURE1);
	glBindImageTexture(0, _tex[1], 0, false, 0, GL_WRITE_ONLY, GL_R32F);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, uniformBuffer);
	glUniform2i(0, 1, 0);
	glDispatchCompute(128, 1, 1);
	glUniform2i(0, 0, 1);
	glDispatchCompute(128, 1, 1);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	blurShader->unbind();
}
