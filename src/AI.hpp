#pragma once

#include <functional>

#include "MainStruct.hpp"
#include "World.hpp"

namespace Heerbann {

	namespace BehaviourTree{


		struct Node{
			std::function<void(float)> func;
			std::vector<Decorator> decorators;

			bool check(float _deltaTime){
				for(auto& d : decorators)
					if(d.eval(_deltaTime))
						return true;
				return false;
			};

			std::vector<Node*> children;
			void step(float _deltaTime){
				for(auto c : children){
					if(check(deltaTime))
						func(_deltaTime);
					c->step(deltaTime);
				}
			};
		};

		struct Decorator{
			std::function<bool(float)> dec;
			bool eval(float deltaTime){
				return dec(__deltaTime);
			};
		};

		struct Root : public Node{
			
		};

	}

	class Brain {

	

		//prio 1, 2, 3
		std::vector<WorldObject*, 3> objects;
		
	public:
		void step(float _deltaTime){
			for(auto v : objects){
				WorldObject* o = v;
				while(o->next != nullptr){
					//do stuff
					o = o->next;
				}
			}
		}

	};

}