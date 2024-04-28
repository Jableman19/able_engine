#pragma once
#include <map>
#include <string>
#include <vector>
#include "ComponentDB.h"
#include "Actor.h"
#include "EngineUtils.h"
#include "rapidjson/document.h"

namespace ActorDB {
	inline std::map<std::string, std::vector<Actor*>> actorMap;
	inline std::unordered_map<std::string, Actor> templateCache;
	inline std::vector<Actor*> hardcoded_actors;
	inline std::vector<Actor*> actorsToAdd;
	inline std::vector<Actor*> actorsToRemove;
	inline int global_ids = 0;

	//Clear DB
	inline void Clear() {
		hardcoded_actors.clear();
		actorMap.clear();
	}


	//Find actor by name return luaref to actor
	inline luabridge::LuaRef Find(std::string name) {
		if (actorMap.find(name) != actorMap.end() && actorMap[name].size() > 0) {
			//get first actor and converts to luaref
			Actor* actor = actorMap[name][0];
			return luabridge::LuaRef(ComponentDB::GetLuaState(), actor);
		}
		else {
			return luabridge::LuaRef(ComponentDB::GetLuaState());;
		}
	}

	//FindAll returns lua table of all actors that match
	inline luabridge::LuaRef FindAll(std::string type_name) {
		luabridge::LuaRef actors = luabridge::newTable(ComponentDB::GetLuaState());
		//get vector of actors
		std::vector<Actor*> actorVector = actorMap[type_name];
		//iterate through vector and add to table
		for (int i = 0; i < actorVector.size(); i++) {
			actors[i + 1] = actorVector[i];
		}
		return actors;
	}

	inline void AddAllActors() {
		for (auto& actor : actorsToAdd) {
			hardcoded_actors.push_back(actor);
		}
		actorsToAdd.clear();
	}

	//Instantiate actor with template name
	inline luabridge::LuaRef Instantiate(std::string name) {
		std::string templatePath = "resources/actor_templates/" + name + ".template";
		Actor* actor = new Actor(name, global_ids);
		Actor templateActor = Actor();
		global_ids++;

		////check if template is in cache
		if (templateCache.count(name) > 0 && false) {
			//populate actor with template
			actor->actor_name = templateCache[name].actor_name;
			actor->component_tables = templateCache[name].component_tables;
		}
		else {
			if (!std::filesystem::exists(templatePath)) {
				std::cout << "error: template " << name << " is missing";
				exit(0);
			}
			else {
				EngineUtils engineUtils;
				rapidjson::Document jsonDoc;
				//override default values with template values
				engineUtils.ReadJsonFile(templatePath, jsonDoc);
				if (jsonDoc.HasMember("name")) {
					actor->actor_name = jsonDoc["name"].GetString();
					templateActor.actor_name = jsonDoc["name"].GetString();
				}
				//load all components from template as below
				if (jsonDoc.HasMember("components")) {
					const rapidjson::Value& components = jsonDoc["components"];
					//loop through components where each component is a key value pair
					for (rapidjson::Value::ConstMemberIterator it = components.MemberBegin(); it != components.MemberEnd(); it++) {
						//get string value of component
						std::string component_name = it->name.GetString();
						std::string component_type = it->value["type"].GetString();
						//check if component is in componentsDB
						if (ComponentDB::component_tables.count(component_type) > 0) {
							//get component from component_tables
							//std::shared_ptr<luabridge::LuaRef> component = ComponentDB::component_tables[component_type];
							//add component to actor
							templateActor.addComponent(component_name, component_type, it);
						}
						else {
							//look for component lua script in resources/component_types
							if (component_type == "Rigidbody") {
								templateActor.addRigidbodyComponent(component_name, it);
							}
							else {
								std::string componentPath = "resources/component_types/" + component_type + ".lua";
								if (!std::filesystem::exists(componentPath)) {
									std::cout << "error: failed to locate component " << component_type;
									exit(0);
								}
								else {
									//register component with DB
									ComponentDB::RegisterComponent(componentPath);
									//add component to actor
									templateActor.addComponent(component_name, component_type, it);
								}
							}
						}
					}
				}

				actor->component_tables = templateActor.component_tables;
				ActorDB::templateCache[name] = templateActor;
			}
		}

		if (actor->component_tables.size() > 0) {
			//add all components from template to actor
			//for each component in component_tables, create new table and establish inheritance
			for (auto& it : actor->component_tables) {
				//get type of component
				std::string type = it.second->operator[]("type").cast<std::string>();

				//if component has ready function, call it
				if (it.second->operator[]("Ready").isFunction()) {
					it.second->operator[]("Ready")(*it.second);
				}

				if(type == "Rigidbody") {
					actor->addRigidbodyIns(it.first, templateActor);
					continue;
				}
				//get global ref to component table
				luabridge::LuaRef globalRef = *(it.second);
				//create new table
				luabridge::LuaRef newTable = luabridge::newTable(ComponentDB::GetLuaState());
				//copy all values from component to newTable
				ComponentDB::EstablishInheritance(newTable, globalRef);
				//add newTable to component_tables

				newTable["key"] = it.first;
				actor->component_tables[it.first] = std::make_shared<luabridge::LuaRef>(newTable);
				actor->InjectConvenienceReference(actor->component_tables[it.first]);
				actor->InjectEnabled(actor->component_tables[it.first], true);
				actor->addedComponents.push_back(it.first);

			}
		}

		//add actor to actors to add
		actorsToAdd.push_back(actor);
		actorMap[actor->actor_name].push_back(actor);
		return luabridge::LuaRef(ComponentDB::GetLuaState(), actor);
	}

	inline void RemoveDestroyedActors() {
		for (Actor* actor : actorsToRemove) {
			//remove actor from hardcoded_actors
			for (int i = 0; i < hardcoded_actors.size(); i++) {
				if (hardcoded_actors[i]->id == actor->id) {
					//loop through all components and remove them
					for(auto& it : actor->component_tables) {
						//check if OnDestroy exists
						if (it.second->operator[]("OnDestroy").isFunction()) {
							//call OnDestroy
							it.second->operator[]("OnDestroy")(actor);
						}
					}
					hardcoded_actors.erase(hardcoded_actors.begin() + i);
					break;
				}
			}
			delete actor;
		}
		actorsToRemove.clear();
	}

	//Destroy actor by actor ref
	inline void Destroy(Actor actor) {
		//get actor id
		int id = actor.id;
		std::string name = actor.actor_name;
		
		for(Actor* actor : hardcoded_actors) {
			if (actor->id == id) {
				//remove all components
				actor->RemoveAllComponents();
				actorsToRemove.push_back(actor);
				break;
			}
		}

		//look for actor in actorsToAdd
		for (int i = 0; i < actorsToAdd.size(); i++) {
			if (actorsToAdd[i]->id == id) {
				actorsToAdd.erase(actorsToAdd.begin() + i);
				break;
			}
		}

		//remove actor from actorMap
		for (int i = 0; i < actorMap[name].size(); i++) {
			if (actorMap[name][i]->id == id) {
				actorMap[name].erase(actorMap[name].begin() + i);
				break;
			}
		}
	}

};
