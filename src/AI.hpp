#pragma once

#include <functional>

#include "MainStruct.hpp"
#include "World.hpp"

#define MAX_ANIMAL_COUNT 1000
#define ANIMAL_DATA_COUNT 1

#define MAX_ANIMAL_SENSE 50

#define MapCellEntryCount 100

namespace Heerbann {

	class ShaderProgram;

	namespace AI {
		

		struct DataEntry {
			float walkSpeed;
			float canterSpeed;
			float runSpeed;
			float sprintSpeed;
			float baseSenseDistance;
			float deltaHunger;
			float deltaThirst;
			float deltaEnergy;
		};

		struct SicknessEffect {
			unsigned int type;
			float effectStrength;
		};

		struct Sickness {
			unsigned int type;
			float duration;
			SicknessEffect effects[10];
		};

		struct BodyPart {
			unsigned int type;
			//[0, 1]
			float health;
		};

		struct Body {
			//[0, 1]
			float health;
			//[kg]
			//float size;
			//bool isMale;
			//unsigned int age;
			//hunger[0, 1], thirst[0, 1], energy[0, 1]
			float status[3];
			//BodyPart bodyParty [10];
			//Sickness sicknesses[10];
		};

		struct Animal {
			unsigned int locked;
			int id;
			//if this animal is active and in the game
			bool isActive;
			bool isAlive;
			float position[2];
			float target[2];
			//current index in aabb
			int index;
			DataEntry baseValues;
			unsigned int type;
			unsigned int stateMind;
			unsigned int stateBody;
			Body body;
		};

		struct MinAnimal {
			unsigned int locked;
			bool isActive;
			unsigned int id;
			unsigned int type;
			unsigned int index;
			float position[2];
		};

		struct MapEntry {
			int count;
			MinAnimal entries[MapCellEntryCount];
		};

		struct AABB {
			int size;
			MinAnimal found[MAX_ANIMAL_SENSE];
		};

		class AIHandler {

			ShaderProgram* shader, *compShader;

			GLuint vao, ssbo;

		public:
			void create();
			void draw();
		};

		
	}
}