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
	//std::map<glm::vec2, std::vector<Actor*>, Vec2Comparator> colActors;
	//std::map<glm::vec2, std::vector<Actor*>, Vec2Comparator> trigActors;
	std::vector<Actor*> collidingActors;
	std::vector<Actor*> triggeringActors;
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
		/*this->colActors.clear();
		this->trigActors.clear();*/
		std::vector<Actor*> newActors = {};
		//loop through hardcoded actors and remove ones that aren't DontDestroy!
		ActorDB::actorMap.clear();
		for(Actor* actor : ActorDB::hardcoded_actors) {
			if (actor->dontDestroy) {
				newActors.push_back(actor);
				ActorDB::actorMap[actor->actor_name].push_back(actor);
			}
			else {
				delete actor;
			}
		}
		ActorDB::hardcoded_actors = newActors;
		this->seenMap.clear();
		/*ActorDB::global_ids = 0*/;
		
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
												//get component from component_tables
												//std::shared_ptr<luabridge::LuaRef> component = ComponentDB::component_tables[component_type];
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
							//get type of component
							/*std::string type = it.second->operator[]("type").cast<std::string>();
							if (type == "Rigidbody") {
								dynamicActor->addRigidbodyComponent(it.first);

								Rigidbody* rigidbody = new Rigidbody();
								luabridge::LuaRef componentRB(ComponentDB::GetLuaState(), rigidbody);
								componentRB["key"] = name;
								component_tables[name] = std::make_shared<luabridge::LuaRef>(componentRB);
								this->InjectConvenienceReference(component_tables[name]);
								this->InjectEnabled(component_tables[name], true);
								addedComponents.push_back(name);
							}
							else {*/
								//get global ref to component table
								luabridge::LuaRef globalRef = *(it.second);
								//create new table
								luabridge::LuaRef newTable = luabridge::newTable(ComponentDB::GetLuaState());
								//copy all values from component to newTable
								ComponentDB::EstablishInheritance(newTable, globalRef);
								//add newTable to component_tables
								dynamicActor->component_tables[it.first] = std::make_shared<luabridge::LuaRef>(newTable);
								dynamicActor->addedComponents.push_back(it.first);
							//}
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

					////if the actor is the player, set player_a to it
					//if (dynamicActor->actor_name == "player") {
					//	//set cam pos to player pos
					//	this->camPos = glm::vec2(dynamicActor->position.x, dynamicActor->position.y);
					//	*player_a = dynamicActor;
					//	player->actor = dynamicActor;
					//	player->new_pos = dynamicActor->position;
					//	//look in jsonDoc for hp_image
					//	if (player->hp_name != "") {
					//		imageDB->addImage(player->hp_name);
					//		player->setHpTexture(imageDB->GetImage(player->hp_name));
					//	}
					//	else {
					//		std::cout << "error: player actor requires an hp_image be defined";
					//		exit(0);
					//	}
					//}
					
					ActorDB::hardcoded_actors.push_back(dynamicActor);
					this->newActors.push_back(dynamicActor);
					//add to ActorDB with name as key
					ActorDB::actorMap[dynamicActor->actor_name].push_back(dynamicActor);
					//if(dynamicActor->colliderHeight > 0 && dynamicActor->colliderWidth > 0) {
					//	this->collidingActors.push_back(dynamicActor);
					//}
					//if(dynamicActor->triggerHeight > 0 && dynamicActor->triggerWidth > 0) {
					//	this->triggeringActors.push_back(dynamicActor);
					//}
				}
			}
		}
		
		//loop through hardcoded actors and add them to the map
		//for (int i = 0; i < this->collidingActors.size(); i++) {
			//Actor* actor = this->collidingActors[i];
			//add to collidingActors
			//std::vector<glm::vec2> regions = getCollisionRegions(actor, actor->position, maxCollide);
			/*for (int j = 0; j < regions.size(); j++) {
				glm::vec2 region = regions[j];
				if (this->colActors.count(region) > 0) {
					this->colActors[region].push_back(actor);
				}
				else {
					this->colActors[region] = { actor };
				}
			}*/
		//}

		//loop through triggering actors and add them to the map
		//for (int i = 0; i < this->triggeringActors.size(); i++) {
		//	Actor* actor = this->triggeringActors[i];

		//	//if valid trigger, add to posActors
		//	if (actor->triggerHeight != 0 && actor->triggerWidth != 0) {
		//		std::vector<glm::vec2> regions = getCollisionRegions(actor, actor->position, maxCollide);
		//		for (int j = 0; j < regions.size(); j++) {
		//			glm::vec2 region = regions[j];
		//			if (this->trigActors.count(region) > 0) {
		//				this->trigActors[region].push_back(actor);
		//			}
		//			else {
		//				this->trigActors[region] = { actor };
		//			}
		//		}
		//	}
		//}
	}

	

	//void RenderActors(Renderer* renderer, Player* player, std::string game_title) {
	//	float totalOffsetX = renderer->camera_offset_x;
	//	float totalOffsetY = renderer->camera_offset_y;
	//	//check how far player is from current cam pos
	//	//if player, ease camera to player
	//	if (player->actor != nullptr) {
	//		camPos = glm::mix(camPos, glm::vec2(player->actor->position.x, player->actor->position.y), cam_ease_factor);
	//		totalOffsetX -= (player->actor->position.x - camPos.x);
	//		totalOffsetY -= (player->actor->position.y - camPos.y);
	//	}

	//	//set sdl zoom
	//	SDL_RenderSetScale(renderer->sdlRenderer, renderer->zoom_factor, renderer->zoom_factor);

	//	//if there is a player, add the player's position to the offset to center it
	//	if (player->actor != nullptr) {
	//		totalOffsetX += player->actor->position.x;
	//		totalOffsetY += player->actor->position.y;
	//	}

	//	//create copy of hardcoded actors
	//	std::vector<Actor*> actors = this->hardcoded_actors;

	//	//sort actors by y position, if actor has render_order, sort by that
	//	std::sort(actors.begin(), actors.end(), [](Actor* a, Actor* b) { 
	//		float a_order = a->position.y;
	//		float b_order = b->position.y;
	//		//use std::optional to check if render_order exists
	//		if (a->render_order.has_value()) {
	//			a_order = a->render_order.value();
	//		}
	//		if (b->render_order.has_value()) {
	//			b_order = b->render_order.value();
	//		}
	//		//if equal, tiebreak with id
	//		if(a_order == b_order) {
	//			return a->id < b->id;
	//		}
	//		else {
	//			return a_order < b_order;
	//		}
	//	});
	//	/*int actorCount = actors.size();
	//	int actorsNotNull = 0;*/
	//	for(Actor* actor : actors) {
	//		if (actor->view_texture != nullptr) {
	//			SDL_Texture* targetTexture;
	//			//if actor is player and time since hit is less than 180, make the player texture damage
	//			if (actor->actor_name == "player" && player->actor->image_damage && Helper::GetFrameNumber() - player->lastFrameHit < 30) {
	//				targetTexture = player->actor->image_damage;
	//			}
	//			else if(Helper::GetFrameNumber() - actor->frameLastAttacked < 30 && actor->image_attack) {
	//				targetTexture = actor->image_attack;
	//			}
	//			else {
	//				if (actor->targetTexture) {
	//					targetTexture = actor->targetTexture;
	//				}
	//				else {
	//					targetTexture = actor->view_texture;
	//				}
	//				if (actor->velocity.y < 0) {
	//					
	//					targetTexture = actor->back_view_texture ? actor->back_view_texture : actor->view_texture;
	//					
	//				}
	//				else if (actor->velocity.y > 0) {
	//					
	//					targetTexture = actor->view_texture;
	//					
	//				}
	//				actor->targetTexture = targetTexture;
	//			}
	//			int width, height;

	//			//get width and height of texture via query
	//			SDL_QueryTexture(targetTexture, NULL, NULL, &width, &height);
	//			//define dest taking offset into account
	//			float scaledPivotX = actor->view_pivot_offset_x * std::abs(actor->transform_scale_x);
	//			float scaledPivotY = actor->view_pivot_offset_y * std::abs(actor->transform_scale_y);
	//			float destX = actor->position.x * 100.0 - scaledPivotX + (renderer->window_width / 2 * (1 / renderer->zoom_factor)) - (totalOffsetX * 100);
	//			float destY = actor->position.y * 100.0  - scaledPivotY + (renderer->window_height / 2 * (1 / renderer->zoom_factor)) - (totalOffsetY * 100);
	//			float destW = width * std::abs(actor->transform_scale_x);
	//			float destH = height * std::abs(actor->transform_scale_y);

	//			if (actor->bouncing && (actor->velocity.x != 0 || actor->velocity.y != 0)) {
	//				destY += -glm::abs(glm::sin(Helper::GetFrameNumber() * 0.15f)) * 10.0f;
	//			}



	//			SDL_Rect dest = {
	//				static_cast<int>(std::round(destX)),
	//				static_cast<int>(std::round(destY)),
	//				static_cast<int>(destW),
	//				static_cast<int>(destH)
	//			};
	//			

	//			//print attributes of dest
	//			//if(game_title == "testcase 6-5" && actor->actor_name == "player" && actor->position.first == -1.0 && actor->position.second == -2.0){
	//			// std::cout << "dest!: " << dest.x << ", " << dest.y << ", " << dest.w << ", " << dest.h << " scaledPivot: " << scaledPivotX << ", " <<scaledPivotY << " Casted w/h: " << static_cast<int>(actor->width) << ", " << static_cast<int>(actor->height)<< "\n";
	//			//	}

	//			//if scale is negative, flip the texture
	//			SDL_RendererFlip flip = SDL_FLIP_NONE;
	//			if (xFlip) {
	//				if (actor->velocity.x < 0) {
	//					flip = SDL_FLIP_HORIZONTAL;
	//				}
	//				else if (actor->velocity.x > 0) {
	//					flip = SDL_FLIP_NONE;
	//				}
	//				else {
	//					flip = actor->flip;
	//				}
	//			}
	//			else {
	//				if (actor->transform_scale_x < 0) {
	//					flip = SDL_FLIP_HORIZONTAL;
	//				}
	//			}
	//			actor->flip = flip;
	//			if (flip != SDL_FLIP_NONE && actor->transform_scale_y < 0) {
	//				flip = (SDL_RendererFlip)(SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL);
	//			}
	//			else if(actor->transform_scale_y < 0) {
	//				flip = SDL_FLIP_VERTICAL;
	//			}

	//			//std::cout << "flip: " << flip << "\n";
	//			//make pointer for rotation pivot based on offset
	//			SDL_Point* pivot = new SDL_Point();
	//			pivot->x = (int)scaledPivotX;
	//			pivot->y = (int)scaledPivotY;
	//			//clear sdl errors
	//			SDL_ClearError();
	//			//render only if within camera
	//			//if (destX + destW > 0 && destX < renderer->window_width && destY + destH > 0 && destY < renderer->window_height) {
	//				//render texture
	//				//std::cout << "rendering actor: " << actor->actor_name << " at " << destX << ", " << destY << "\n";
	//			Helper::SDL_RenderCopyEx498(0,"",
	//				renderer->sdlRenderer,
	//				targetTexture,
	//				NULL,
	//				&dest,
	//				actor->transform_rotation_degrees,
	//				pivot,
	//				flip
	//			);
	//			//}

	//			//do above but consider renderer->zoom_factor for if it is in camera beep


	//			//render red rectangles for colliders
	//			//if(actor->colliderHeight != 0 && actor->colliderWidth != 0) {
	//			//	SDL_SetRenderDrawColor(renderer->sdlRenderer, 255, 0, 0, 255);
	//			//	SDL_Rect collider = {
	//			//		(int)std::round(destX + (actor->view_pivot_offset_x - actor->colliderWidth*100/2) * std::abs(actor->transform_scale_x)),
	//			//		(int)std::round(destY + (actor->view_pivot_offset_y - actor->colliderHeight*100/2) * std::abs(actor->transform_scale_y)),
	//			//		(int)std::round(actor->colliderWidth*100 * std::abs(actor->transform_scale_x)),
	//			//		(int)std::round(actor->colliderHeight*100 * std::abs(actor->transform_scale_y))
	//			//	};
	//			//	SDL_RenderDrawRect(renderer->sdlRenderer, &collider);
	//			//	SDL_SetRenderDrawColor(renderer->sdlRenderer, 0, 0, 0, 255);
	//			//}
	//			////render green rectangles for triggers
	//			//if (actor->triggerHeight != 0 && actor->triggerWidth != 0) {
	//			//	SDL_SetRenderDrawColor(renderer->sdlRenderer, 0, 255, 0, 255);
	//			//	SDL_Rect trigger = {
	//			//		(int)std::round(destX + (actor->view_pivot_offset_x - actor->triggerWidth*100/2) * std::abs(actor->transform_scale_x)),
	//			//		(int)std::round(destY + (actor->view_pivot_offset_y - actor->triggerHeight*100/2) * std::abs(actor->transform_scale_y)),
	//			//		(int)std::round(actor->triggerWidth*100 * std::abs(actor->transform_scale_x)),
	//			//		(int)std::round(actor->triggerHeight*100 * std::abs(actor->transform_scale_y))
	//			//	};
	//			//	SDL_RenderDrawRect(renderer->sdlRenderer, &trigger);
	//			//	SDL_SetRenderDrawColor(renderer->sdlRenderer, 0, 0, 0, 255);
	//			//}
	//			//actorsNotNull++

	//			//if turned, flip vel
	//			if (actor->turned) {
	//				actor->turned = false;
	//				actor->velocity.x *= -1;
	//				actor->velocity.y *= -1;
	//			}
	//		}
	//	}

	//	// set scale back 
	//	SDL_RenderSetScale(renderer->sdlRenderer, 1, 1);
	//	/*if (game_title == "testcase 4-0") {
	//		std::cout << "actors rendered: " << actorsNotNull << " / " << actorCount << "\n";
	//	}*/
	//}

	//void mapUpdate(Player* player, int x_resolution, int y_resolution) {
		//std::stringstream ss;
		////print map resolution
		////ss << "map resolution : " << x_resolution << " x " << y_resolution << "\n";
		////loop through each pos in camera (13 x 9)
		//for (int y = 0; y < y_resolution; y++) {
		//	for (int x = 0; x < x_resolution; x++) {
		//		//get the tile at the camera pos
		//		char tile_char = ' ';

		//		//get the pos of the camera
		//		int camera_x = player->actor->position.x - ((x_resolution / 2)) + x;
		//		int camera_y = player->actor->position.y - ((y_resolution / 2)) + y;

		//		if (this->posActors.count(packIvec2ToUint64({ camera_x, camera_y })) > 0) {
		//			//find the actor with highest id at the pos
		//			int highest_id = -1;
		//			for (int i = 0; i < this->posActors.at(packIvec2ToUint64({ camera_x, camera_y })).size(); i++) {
		//				if (this->posActors.at(packIvec2ToUint64({ camera_x, camera_y }))[i]->id > highest_id) {
		//					highest_id = this->posActors.at(packIvec2ToUint64({ camera_x, camera_y }))[i]->id;
		//					tile_char = this->posActors.at(packIvec2ToUint64({ camera_x, camera_y }))[i]->view;
		//				}
		//			}
		//		}
		//		//else if(camera_x >= 0 && camera_x < HARDCODED_MAP_WIDTH && camera_y >= 0 && camera_y < HARDCODED_MAP_HEIGHT) {
		//		//	  tile_char = hardcoded_map[camera_y][camera_x];
		//		///}
		//		ss << tile_char;
		//	}
		//	ss << "\n";
		//}
		//std::cout << ss.str();
	//}

	//int checkNearby(Player* player, std::string* level, Renderer* renderer, AudioDB* audioDB) {
	//	//clear messagesToShow
	//	renderer->messagesToShow.clear();
	//	int game_over = 0; // 0 = not over, 1 = win, -1 = lose, -2 = lose by health, 2 scene change
	//	std::stringstream ss;
	//	std::set<Actor*> messagingActors = {};
	//	//for all positions adjacent to player
	//	//for (int y = -1; y <= 1; y++) {
	//	//	for (int x = -1; x <= 1; x++) {
	//	//		std::pair<float, float> pos = player->actor->position;
	//	//		pos.first += x;
	//	//		pos.second += y;
	//	//		//if the position is in the map, add all to messagingActors
	//	//		if (posActors.count(pos) > 0) {
	//	//			for (int i = 0; i < posActors[pos].size(); i++) {
	//	//				messagingActors.push_back(posActors[pos][i]);
	//	//			}
	//	//		}
	//	//	}
	//	//}
	//	//for all actors in same posActors region as player
	//	glm::vec2 playerRegion = getCollisionRegion(player->actor->position, maxCollide);
	//	//look in adjacent regions (up, left, down, right)
	//	std::vector<glm::vec2> adjacentRegions = {
	//		{playerRegion.x, playerRegion.y},
	//		{playerRegion.x, playerRegion.y + 1},
	//		{playerRegion.x - 1, playerRegion.y},
	//		{playerRegion.x, playerRegion.y - 1},
	//		{playerRegion.x + 1, playerRegion.y},
	//		{playerRegion.x - 1, playerRegion.y + 1},
	//		{playerRegion.x + 1, playerRegion.y + 1},
	//		{playerRegion.x - 1, playerRegion.y - 1},
	//		{playerRegion.x + 1, playerRegion.y - 1}
	//	};

	//	for (int i = 0; i < adjacentRegions.size(); i++) {
	//		glm::vec2 region = adjacentRegions[i];
	//		if (trigActors.count(region) > 0) {
	//			for (int j = 0; j < trigActors[region].size(); j++) {
	//				Actor* currentActor = trigActors[region][j];
	//				if (player->actor->triggerHeight > 0 && player->actor->triggerWidth > 0 && currentActor->triggerHeight > 0 && currentActor-> triggerWidth > 0) {
	//					if (AABB_trig(player->actor, currentActor)) {
	//						messagingActors.insert(currentActor);
	//					}
	//				}
	//			}
	//		}
	//	}
	//	//sort messaging actors by id
	//	std::vector<Actor*> messagingActorsVec = {};
	//	for (auto& it : messagingActors) {
	//		messagingActorsVec.push_back(it);
	//	}
	//	std::sort(messagingActorsVec.begin(), messagingActorsVec.end(), [](Actor* a, Actor* b) {
	//		return a->id < b->id;
	//	});
	//	
	//	//check for messages
	//	for (Actor* currentActor : messagingActorsVec) {
	//		//if the position is equal to the player
	//		std::string dialogue = "";
	//		//if (player->actor->position == currentActor->position) {
	//			//dialogue = currentActor->contact_dialogue;
	//		//}
	//		//else if (distanceBetweenActors(currentActor, player->actor) <= 1.5f) {
	//		dialogue = currentActor->nearby_dialogue;
	//		//}
	//		if (dialogue != "") {
	//			//ss << dialogue << "\n";
	//			renderer->messagesToShow.push_back(dialogue);
	//			if (!currentActor->spoke && currentActor->nearbyDialogueSFX != "") {
	//				audioDB->Play(currentActor->nearbyDialogueSFX, Helper::GetFrameNumber() % 48 + 2, 0);
	//				currentActor->spoke = true;
	//			}
	//			std::smatch matches;
	//			//if the contact_dialogue matches the regex, do the thing
	//			if (std::regex_search(dialogue, health)) {
	//				if (Helper::GetFrameNumber() - player->lastFrameHit >= 180) {
	//					player->lastFrameHit = Helper::GetFrameNumber();
	//					currentActor->frameLastAttacked = Helper::GetFrameNumber();
	//					player->setHealth(player->getHealth() - 1);
	//					if(player->actor->damageSFX != "") {
	//						audioDB->Play(player->actor->damageSFX, Helper::GetFrameNumber() % 48 + 2, 0);
	//					}
	//					if (player->getHealth() <= 0) {
	//						game_over = -1;
	//					}
	//				}
	//			}
	//			else if (std::regex_search(dialogue, score)) {
	//				//check if the player hasn't already seen the actor
	//				if (seenMap.count(currentActor->id) <= 0) {
	//					player->setScore(player->getScore() + 1);
	//					if(scoreSFX != "") {
	//						audioDB->Play(scoreSFX, 1, 0);
	//					}
	//					seenMap[currentActor->id] = true;
	//				}
	//			}
	//			else if (std::regex_search(dialogue, win)) {
	//				game_over = 1;
	//			}
	//			else if (std::regex_search(dialogue, lose)) {
	//				if (Helper::GetFrameNumber() - player->lastFrameHit >= 180) {
	//					player->lastFrameHit = Helper::GetFrameNumber();
	//					game_over = -1;
	//				}
	//			}
	//			else if (std::regex_search(dialogue, matches, next_level)) {
	//				game_over = 2;
	//				*level = obtain_word_after_phrase(dialogue, "proceed to ");
	//			}
	//		}
	//	}
	//	//do the same for colliding actors, but don't display messages
	//	if (!player->actor->collidingActors.empty()) {
	//		//for each actor* in collidingActors
	//		for (auto& it : player->actor->collidingActors) {
	//			Actor* currentActor = it.second;
	//			std::string dialogue = currentActor->contact_dialogue;
	//			std::smatch matches;
	//			if (std::regex_search(dialogue, health)) {
	//				if (Helper::GetFrameNumber() - player->lastFrameHit >= 180) {
	//					player->lastFrameHit = Helper::GetFrameNumber();
	//					if (player->actor->damageSFX != "") {
	//						audioDB->Play(player->actor->damageSFX, Helper::GetFrameNumber() % 48 + 2, 0);
	//					}
	//					currentActor->frameLastAttacked = Helper::GetFrameNumber();
	//					player->setHealth(player->getHealth() - 1);
	//					if (player->getHealth() <= 0) {
	//						game_over = -1;
	//					}
	//				}
	//			}
	//			else if (std::regex_search(dialogue, score)) {
	//				//check if the player hasn't already seen the actor
	//				if (seenMap.count(currentActor->id) <= 0) {
	//					player->setScore(player->getScore() + 1);
	//					if(scoreSFX != "") {
	//						audioDB->Play(scoreSFX, 1, 0);
	//					}
	//					seenMap[currentActor->id] = true;
	//				}
	//			}
	//			else if (std::regex_search(dialogue, win)) {
	//				game_over = 1;
	//			}
	//			else if (std::regex_search(dialogue, lose)) {
	//				if (Helper::GetFrameNumber() - player->lastFrameHit >= 180) {
	//					player->lastFrameHit = Helper::GetFrameNumber();
	//					game_over = -1;
	//				}
	//			}
	//			else if (std::regex_search(dialogue, matches, next_level)) {
	//				game_over = 2;
	//				*level = obtain_word_after_phrase(dialogue, "proceed to ");
	//			}
	//		}
	//	}
	//	std::cout << ss.str();
	//	player->actor->collidingActors.clear();
	//	return game_over;
	//}

//	void PlayerMovement(Player* player, AudioDB* audioDB) {
//		
//		bool moving = true;
//		//get all regions 
//		std::vector<glm::vec2> regions = {};
//		if (player->actor->colliderHeight > 0 && player->actor->colliderWidth > 0) {
//			regions = getCollisionRegions(player->actor, player->new_pos, maxCollide);
//			//for all regions
//			for (int i = 0; i < regions.size(); i++) {
//				//if the region exists in posActors
//				if (colActors.count(regions[i]) > 0) {
//					//for all actors in the region
//					for (int j = 0; j < colActors[regions[i]].size(); j++) {
//						Actor* otherActor = colActors[regions[i]][j];
//						//AABB collision detection
//						if (otherActor->id != player->actor->id && AABB_col(player->actor, otherActor, player->new_pos)) {
//							moving = false;
//							player->actor->collidingActors[otherActor->id] = (otherActor);
//							otherActor->collidingActors[player->actor->id] = (player->actor);
//						}
//						
//					}
//				}
//			}
//		}
//	
//		if (moving && !player->actor->collidingActors.size()) {
//			//remove player from all old regions, and add to new ones
//			if(player->actor->stepSFX != "" && Helper::GetFrameNumber() % 20 == 0 && player->new_pos != player->actor->position) {
//				audioDB->Play(player->actor->stepSFX, Helper::GetFrameNumber() % 48 + 2, 0);
//			}
//
//			if (player->actor->colliderHeight > 0 && player->actor->colliderWidth > 0) {
//				std::vector<glm::vec2> oldRegions = getCollisionRegions(player->actor, player->actor->position, maxCollide);
//				for (int i = 0; i < oldRegions.size(); i++) {
//					auto& currentPosActors =(colActors)[oldRegions[i]];
//					currentPosActors.erase(std::remove_if(currentPosActors.begin(), currentPosActors.end(),
//						[player](Actor* actor) { return actor->id == player->actor->id; }),
//						currentPosActors.end());
//					if (currentPosActors.empty()) {
//						colActors.erase(oldRegions[i]);
//					}
//				}
//				for(int i = 0; i < regions.size(); i++) {
//					colActors[regions[i]].push_back(player->actor);
//				}
//			}
//			player->actor->position = player->new_pos;
//		}
//		
//	}
//
//	void npcCollision(Player* player, AudioDB* audioDB) {
//		//clear all actors ' collidingActors
//		for (int i = 0; i < collidingActors.size(); i++) {
//			collidingActors[i]->collidingActors.clear();
//		}
//
//
//		//create copy of hardcoded actors with only actors that have a velocity, and the player
//		std::vector<Actor*> movingActors = hardcoded_actors;
//		movingActors.erase(std::remove_if(movingActors.begin(), movingActors.end(),
//			[](Actor* actor) { return (actor->velocity.x == 0 && actor->velocity.y == 0) && actor->actor_name != "player"; }),
//			movingActors.end());
//
//		//for all positions adjacent to player
//		//collision detection
//		for (int i = 0; i < movingActors.size(); i++) {
//			Actor* currentActor = movingActors.at(i);
//			if (currentActor->actor_name == "player") {
//				PlayerMovement(player, audioDB);
//				continue;
//			}
//			glm::vec2 newActorPos = glm::vec2(currentActor->position.x + currentActor->velocity.x, currentActor->position.y + currentActor->velocity.y);
//			//do as above with player
//			bool moving = true;
//			std::vector<glm::vec2> regions = {};
//			if (currentActor->colliderHeight > 0 && currentActor->colliderWidth > 0) {
//				regions = getCollisionRegions(currentActor, newActorPos, maxCollide);
//				for (int i = 0; i < regions.size(); i++) {
//					if (colActors.count(regions[i]) > 0) {
//						for (int j = 0; j < colActors[regions[i]].size(); j++) {
//							Actor* otherActor = colActors[regions[i]][j];
//							//DEBUG
//							/*if(otherActor->actor_name == "player") {
//								std::cout << "player pos: " << otherActor->position.first << ", " << otherActor->position.second << "\n";
//							}*/
//							if (otherActor->id != currentActor->id && AABB_col(currentActor, otherActor, newActorPos)) {
//								moving = false;
//								currentActor->collidingActors[otherActor->id] = (otherActor);
//								otherActor->collidingActors[currentActor->id] = (currentActor);
//							}
//
//						}
//					}
//				}
//			}
//			if (currentActor->collidingActors.empty() && moving) {
//				if (currentActor->colliderHeight > 0 && currentActor->colliderWidth > 0) {
//					std::vector<glm::vec2> oldRegions = getCollisionRegions(currentActor, currentActor->position, maxCollide);
//					for (int i = 0; i < oldRegions.size(); i++) {
//						auto& currentPosActors = (colActors)[oldRegions[i]];
//						currentPosActors.erase(std::remove_if(currentPosActors.begin(), currentPosActors.end(),
//							[currentActor](Actor* actor) { return actor->id == currentActor->id; }),
//							currentPosActors.end());
//						if (currentPosActors.empty()) {
//							colActors.erase(oldRegions[i]);
//						}
//					}
//					for (int i = 0; i < regions.size(); i++) {
//						colActors[regions[i]].push_back(currentActor);
//					}
//				}
//				currentActor->position = newActorPos;
//			}
//			else if(!currentActor->collidingActors.empty()){
//				//if the actor is blocked, reverse the velocity
//				currentActor->turned = true;
//			}
//		}
//	}
};
