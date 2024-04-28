#pragma once
#include <vector>
#include <unordered_map>
#include <filesystem>
#include "rapidjson/document.h"
#include <regex>
#include <map>
#include <set>
#include <cmath>
#include "EngineUtils.h"
//#include "SceneUtils.h"
#include "Renderer.h"
#include "DBs/ActorDB.h"
#include "DBs/AudioDB.h"


class Scene {
public:
	std::vector<Actor*> newActors;
	std::unordered_map<int, bool> seenMap;
	std::string currentScene = "";
	std::string nextScene = "";

	float cam_ease_factor = 1.0f;
	bool xFlip = false;

	
	float maxCollide = 1.0f;

	Scene() {
	}


	void Update() {
		if (nextScene != "") {
			std::string path = "resources/scenes/" + nextScene + ".scene";
			EngineUtils engineUtils = EngineUtils();
			rapidjson::Document jsonDoc;
			WorldManager::rbs = false;
			WorldManager::contactListener = false;
			ActorDB::Clear();
			//remove 
			LoadScene(path, nextScene, engineUtils, jsonDoc, ComponentDB::L);
		}
		ActorDB::AddAllActors();

		//if newActors is not empty, run all the components 'OnStart' functions in new Actors
		for (Actor* actor : ActorDB::hardcoded_actors) {
			//for each component in actor
			actor->ProcessAddedComponents();
		}

		//run all the components 'OnUpdate' functions in hardcoded actors
		for (Actor* actor : ActorDB::hardcoded_actors) {
			//for each component in actor
			for (auto& it : actor->component_tables) {
				//get component from component_tables
				std::shared_ptr<luabridge::LuaRef> component = it.second;
				//run OnUpdate function
				if (component->operator[]("OnUpdate").isFunction() && component->operator[]("enabled")) {
					try {
						component->operator[]("OnUpdate")(*component);
					}
					catch (luabridge::LuaException const& e) {
						ReportError(actor->actor_name, e);
					}
				}
			}
		}
	
	}

	void LateUpdate() {
		//run all the components 'OnLateUpdate' functions in hardcoded actors
		std::vector<luabridge::LuaRef> destroyedComponents = {};

		for (Actor* actor : ActorDB::hardcoded_actors) {
			//for each component in actor
			for (auto& it : actor->component_tables) {
				//get component from component_tables
				std::shared_ptr<luabridge::LuaRef> component = it.second;
				//run OnLateUpdate function
				if (component->operator[]("OnLateUpdate").isFunction() && component->operator[]("enabled")) {
					try {
						component->operator[]("OnLateUpdate")(*component);
					}
					catch (luabridge::LuaException const& e) {
						ReportError(actor->actor_name, e);
					}
				}
			}

			for(auto&it : actor->removedComponents){
				destroyedComponents.push_back(*actor->component_tables[it["key"].tostring()]);
				actor->component_tables.erase(it["key"].cast<std::string>());
			}
			actor->removedComponents.clear();
			//add new components
			actor->AddAllComponents();
		}
		//destroy all components
		for (luabridge::LuaRef component : destroyedComponents) {
			if (component["OnDestroy"].isFunction()) {
				try {
					component["OnDestroy"](component);
				}
				catch (luabridge::LuaException const& e) {
					ReportError("component", e);
				}
			}
		}
			ActorDB::RemoveDestroyedActors();

			AudioDB::UpdateSubtitles();

	}

	void LoadScene(std::string& levelPath, std::string& levelName, EngineUtils& engineUtils, rapidjson::Document& jsonDoc, lua_State* L) {
		currentScene = levelName;
		nextScene = "";
		std::vector<Actor*> newActors = {};
		//loop through hardcoded actors and remove ones that aren't DontDestroy!
		ActorDB::actorMap.clear();
		for(Actor* actor : ActorDB::hardcoded_actors) {
			if (actor->dontDestroy) {
				newActors.push_back(actor);
				ActorDB::actorMap[actor->actor_name].push_back(actor);
			}
			else {
				//delete actor;
			}
		}
		ActorDB::hardcoded_actors = newActors;
		this->seenMap.clear();
		
		//check for levelPath
		if (!std::filesystem::exists(levelPath)) {
			std::cout << "error: scene " << levelName << " is missing";
			exit(0);
		}
		else {
			//map linking template names to Actor with default values
			
			//parse all actors in 'actors' array 
			engineUtils.ReadJsonFile(levelPath, jsonDoc);
			if (jsonDoc.HasMember("actors")) {
				const rapidjson::Value& actors = jsonDoc["actors"];
				for (rapidjson::SizeType i = 0; i < actors.Size(); i++) {
					//define default values
					std::string actor_name = "";
					//componenet table to copy
					std::map<std::string, std::shared_ptr<luabridge::LuaRef>> component_tables;
					//check if actor has template
					if (actors[i].HasMember("template")) {
						std::string templateName = actors[i]["template"].GetString();
						std::string templatePath = "resources/actor_templates/" + templateName + ".template";
						////check if template is in cache
						if (ActorDB::templateCache.count(templateName) > 0) {
							Actor templateActor = ActorDB::templateCache[templateName];
							actor_name = templateActor.actor_name;
							//add all components from template to actor
							component_tables = templateActor.component_tables;
						}
						else {
							if (!std::filesystem::exists(templatePath)) {
								std::cout << "error: template " << templateName << " is missing";
								exit(0);
							}
							else {
								//override default values with template values
								engineUtils.ReadJsonFile(templatePath, jsonDoc);
								if (jsonDoc.HasMember("name")) {
									actor_name = jsonDoc["name"].GetString();
								}
								//add actor to templateCache
								Actor templateActor = Actor(
									actor_name, i
								);
								//load all components from template as below
								if (jsonDoc.HasMember("components")) {
									const rapidjson::Value& components = jsonDoc["components"];
									//loop through components where each component is a key value pair
									for (rapidjson::Value::ConstMemberIterator it = components.MemberBegin(); it != components.MemberEnd(); it++) {
										//get string value of component
										std::string component_name = it->name.GetString();
										std::string component_type = it->value["type"].GetString();
										if(component_type == "Rigidbody") {
											templateActor.addRigidbodyComponent(component_name, it);
										}
										else {
											//check if component is in componentsDB
											if (ComponentDB::component_tables.count(component_type) > 0) {
												//add component to actor
												templateActor.addComponent(component_name, component_type, it);
											}
											else {
												//look for component lua script in resources/component_types
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

								component_tables = templateActor.component_tables;
								ActorDB::templateCache[templateName] = templateActor;
							}
						}
					}
					//check if actor has name
					else if (actors[i].HasMember("name")) {
						actor_name = actors[i]["name"].GetString();
					}
					//add actor to hardcoded_actors
					Actor* dynamicActor = new Actor(actor_name, ActorDB::global_ids);
					ActorDB::global_ids++;

					if(component_tables.size() > 0) {
						//add all components from template to actor
						//for each component in component_tables, create new table and establish inheritance
						for (auto& it : component_tables) {
							
							luabridge::LuaRef globalRef = *(it.second);
							//create new table
							luabridge::LuaRef newTable = luabridge::newTable(ComponentDB::GetLuaState());
							//copy all values from component to newTable
							ComponentDB::EstablishInheritance(newTable, globalRef);
							//add newTable to component_tables
							dynamicActor->component_tables[it.first] = std::make_shared<luabridge::LuaRef>(newTable);
							dynamicActor->addedComponents.push_back(it.first);
					
						}
						if (actors[i].HasMember("components")) {
							//allow for overriding of template components
							const rapidjson::Value& components = actors[i]["components"];
							//loop through components where each component is a key value pair, if the name is on the template, override it
							for(rapidjson::Value::ConstMemberIterator it = components.MemberBegin(); it != components.MemberEnd(); it++) {
								//get string value of component
								std::string component_name = it->name.GetString();
								if (dynamicActor->component_tables.find(component_name) != dynamicActor->component_tables.end()) {
									//override all values in component aside from type
									for(rapidjson::Value::ConstMemberIterator it2 = it->value.MemberBegin(); it2 != it->value.MemberEnd(); it2++) {
										if (it2->name != "type") {
											if (it2->value.IsString()) {
												dynamicActor->component_tables[component_name]->operator[](it2->name.GetString()) = it2->value.GetString();
											}
											else if(it2->value.IsInt()) {
												dynamicActor->component_tables[component_name]->operator[](it2->name.GetString()) = it2->value.GetInt();
											}
											else if(it2->value.IsBool()) {
												dynamicActor->component_tables[component_name]->operator[](it2->name.GetString()) = it2->value.GetBool();
											}
											else if (it->value.IsDouble()) {
												dynamicActor->component_tables[component_name]->operator[](it2->name.GetString()) = it2->value.GetDouble();
											}
										}
									}
								}
								else {
									std::string component_type = it->value["type"].GetString();
									if (component_type == "Rigidbody") {
										dynamicActor->addRigidbodyComponent(component_name, it);
									}
									else {
										//check if component is in componentsDB
										if (ComponentDB::component_tables.count(component_type) > 0) {
											//get component from component_tables
											std::shared_ptr<luabridge::LuaRef> component = ComponentDB::component_tables[component_type];
											//add component to actor
											dynamicActor->addComponent(component_name, component_type, it);
										}
										else {
											//look for component lua script in resources/component_types
											std::string componentPath = "resources/component_types/" + component_type + ".lua";
											if (!std::filesystem::exists(componentPath)) {
												std::cout << "error: failed to locate component " << component_type;
												exit(0);
											}
											else {
												//register component with DB
												ComponentDB::RegisterComponent(componentPath);
												//add component to actor
												dynamicActor->addComponent(component_name, component_type, it);
											}
										}
									}
								}
								//if component has ready function, run it
								if (dynamicActor->component_tables[component_name]->operator[]("Ready").isFunction()) {
									try {
										dynamicActor->component_tables[component_name]->operator[]("Ready")(*dynamicActor->component_tables[component_name]);
									}
									catch (luabridge::LuaException const& e) {
										ReportError(dynamicActor->actor_name, e);
									}
								}
							}

						}
					}
					else if (actors[i].HasMember("components")) {
						//for each component in components
						const rapidjson::Value& components = actors[i]["components"];
						//loop through components where each component is a key value pair
						for(rapidjson::Value::ConstMemberIterator it = components.MemberBegin(); it != components.MemberEnd(); it++) {
							//get string value of component
							std::string component_name = it->name.GetString();
							std::string component_type = it->value["type"].GetString();
							if (component_type == "Rigidbody") {
								dynamicActor->addRigidbodyComponent(component_name, it);
							}
							else {
								//check if component is in componentsDB
								if (ComponentDB::component_tables.count(component_type) > 0) {
									//get component from component_tables
									std::shared_ptr<luabridge::LuaRef> component = ComponentDB::component_tables[component_type];
									//add component to actor
									dynamicActor->addComponent(component_name, component_type, it);
								}
								else {
									//look for component lua script in resources/component_types
									std::string componentPath = "resources/component_types/" + component_type + ".lua";
									if (!std::filesystem::exists(componentPath)) {
										std::cout << "error: failed to locate component " << component_type;
										exit(0);
									}
									else {
										//register component with DB
										ComponentDB::RegisterComponent(componentPath);
										//add component to actor
										dynamicActor->addComponent(component_name, component_type, it);
									}
								}
							}
							//if component has ready function, run it
							if (dynamicActor->component_tables[component_name]->operator[]("Ready").isFunction()) {
								try {
									dynamicActor->component_tables[component_name]->operator[]("Ready")(*dynamicActor->component_tables[component_name]);
								}
								catch (luabridge::LuaException const& e) {
									ReportError(dynamicActor->actor_name, e);
								}
							}
						}

					}
					
					ActorDB::hardcoded_actors.push_back(dynamicActor);
					this->newActors.push_back(dynamicActor);
					//add to ActorDB with name as key
					ActorDB::actorMap[dynamicActor->actor_name].push_back(dynamicActor);
				}
			}
		}
	}
};
