#pragma once

#include <functional>

#include "MainStruct.hpp"
#include "World.hpp"

namespace Heerbann {

	namespace BehaviourTree{

		enum Status {
			success, failed, running
		};

		struct Node {
			std::vector<Node> children;

			//0 - failed
			//1 - success
			//2 - running
			virtual Status eval(float _deltaTime) = 0;
		};

		//Executes its child nodes consecutively, one at a time, stopping at the first one that succeeds.
		struct Selector : public Node {
			Status eval(float _deltaTime) override {
				for (auto& t : children) {
					Status status = t.eval(_deltaTime);
					if (status == Status::running)
						return status;
					else if (status == Status::success)
						return status;
				}
				return Status::failed;
			};
		};

		//Executes its child nodes one at a time in order.
		struct Sequence : public Node {
			Status eval(float _deltaTime) override {
				for (auto& t : children) {
					Status status = t.eval(_deltaTime);
					if (status == Status::running)
						return status;
					else if (status == Status::failed)
						return status;
				}
				return Status::success;
			};
		};

		struct Task : public Node {				
			std::function<Status(float)> eval;
			Status eval(float _deltaTime) override {
				return eval(_deltaTime);
			}
		};

		struct Root : public Node {
			Node* rootNode;
			Status eval(float _deltaTime) override {
				rootNode->eval(_deltaTime);
			}
		};

	}

	using namespace BehaviourTree;

	class Brain {

		//prio 1, 2, 3
		World::WorldObject* objects[3];
		
	public:
		void step(float _deltaTime){
			for(auto v : objects){
				World::WorldObject* o = v;
				while(o->next != nullptr){
					o->root->eval(_deltaTime);
					o = o->next;
				}
			}
		};

		void add(World::WorldObject* _object) {
			if (objects[0] == nullptr) {
				objects[0] = _object;
				_object->tail = _object;
			} else {
				objects[0]->tail->next = _object;
				objects[0]->tail = _object;
			}
		};

		void remove(World::WorldObject* _object) {
			//TODO
		};

	};

}