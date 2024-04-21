#pragma once
#include <string>
#include <vector>
#include <optional>
#include <map>
#include "rapidjson/document.h"
#include "DBs/ComponentDB.h"
#include "ActorUtils.h"
#include "DBs/ImageDB.h"
#include "collision.h"
#include "Rigidbody.h"

class Actor
{
public:
	std::string actor_name;
	//map of components (luarefs)
	std::map<std::string, std::shared_ptr<luabridge::LuaRef>> component_tables;
	std::vector<std::string> addedComponents;
	std::vector<luabridge::LuaRef> removedComponents;
	std::map<std::string, std::shared_ptr<luabridge::LuaRef>> new_component_tables;
	bool dontDestroy = false;
	//bool turned = false;
	////SDL view
	//SDL_RendererFlip flip = SDL_FLIP_NONE;
	//std::unordered_map<int, Actor*> collidingActors;
	//SDL_Texture* targetTexture = nullptr;
	//SDL_Texture* view_texture = nullptr;
	//SDL_Texture* back_view_texture = nullptr;
	//SDL_Texture* image_damage = nullptr;
	//SDL_Texture* image_attack = nullptr;
	//glm::vec2 position;
	//
	//float transform_scale_x = 1.0;
	//float transform_scale_y = 1.0;
	//float transform_rotation_degrees = 0.0;
	//float view_pivot_offset_x = NAN;
	//float view_pivot_offset_y = NAN;
	//float width;
	//float height;
	//std::optional<int> render_order;
	//glm::vec2 velocity;
	//bool blocking;
	//bool bouncing = false;
	//bool spoke = false;
	//std::string nearby_dialogue;
	//std::string contact_dialogue;
	//std::string view_image;
	//std::string nearbyDialogueSFX = "";
	//std::string back_view_image;
	//std::string view_image_damage;
	//std::string damageSFX = "";
	//std::string view_image_attack;
	//std::string stepSFX = "";

	//float colliderWidth = 0;
	//float colliderHeight = 0;

	//float frameLastAttacked = -200;

	//float triggerWidth = 0;
	//float triggerHeight = 0;

	int id = -1;

	Actor(std::string name, int id) {
		////load view texture
		//if (view_image != "") {
		//	this->view_image = view_image;
		//	imageDB->addImage(view_image);
		//	this->view_texture = imageDB->GetImage(view_image);
		//}

		//if(back_view_image != "") {
		//	this->back_view_image = back_view_image;
		//	imageDB->addImage(back_view_image);
		//	this->back_view_texture = imageDB->GetImage(back_view_image);
		//}

		//if(view_image_damage != "") {
		//	this->view_image_damage = view_image_damage;
		//	imageDB->addImage(view_image_damage);
		//	this->image_damage = imageDB->GetImage(view_image_damage);
		//}

		//if(view_image_attack != "") {
		//	this->view_image_attack = view_image_attack;	
		//	imageDB->addImage(view_image_attack);
		//	this->image_attack = imageDB->GetImage(view_image_attack);
		//}

		//if(render_order.has_value()) {
		//	this->render_order = render_order.value();
		//}
		//
		actor_name = name;
		//this->position = position;
		//transform_scale_x = scale_x;
		//transform_scale_y = scale_y;
		//this->colliderWidth = colliderWidth;
		//this->colliderHeight = colliderHeight;
		//this->triggerWidth = triggerWidth;
		//this->triggerHeight = triggerHeight;
		//transform_rotation_degrees = rotation;
		//this->nearbyDialogueSFX = nearbyDialogueSFX;
		//this->damageSFX = damageSFX;
		//this->stepSFX = stepSFX;
		//int width, height;
		////if xscale is negative, change flip	
		//if (transform_scale_x < 0) {
		//	flip = SDL_FLIP_HORIZONTAL;
		//}
		//SDL_QueryTexture(this->view_texture, NULL, NULL, &width, &height);
		//if (!std::isnan(pivot_x)) {
		//	this->view_pivot_offset_x = pivot_x;
		//}
		//else {
		//	//half of view width
		//	this->view_pivot_offset_x = std::round(float(width) / 2);
		//}
		//if (!std::isnan(pivot_y)) {
		//	this->view_pivot_offset_y = pivot_y;
		//}
		//else {
		//	this->view_pivot_offset_y = std::round(float(height) / 2);
		//}
		//this->width = width;
		//this->height = height;
		//this->velocity = velocity;
		//this->blocking = blocking;
		//this->nearby_dialogue = nearby_dialogue;
		//this->contact_dialogue = contact_dialogue;
		//this->view_image = view_image;
		this->id = id;
		//this->bouncing = bouncing;
	}

	Actor() {
		actor_name = "";
		/*position = glm::vec2(0,0);
		transform_scale_x = 1.0;
		transform_scale_y = 1.0;
		transform_rotation_degrees = 0.0;
		view_pivot_offset_x = NAN;
		view_pivot_offset_y = NAN;
		velocity = glm::vec2(0, 0);
		blocking = false;
		nearby_dialogue = "";
		contact_dialogue = "";
		view_image = "";
		id = -1;
		height = 0;
		width = 0;*/
	}

	//GetName
	std::string GetName() const {
		return actor_name;
	}

	//GetID
	int GetID() const {
		return id;
	}

	//GetComponentByKey returns LuaRef
    luabridge::LuaRef GetComponentByKey(std::string key) {
		if(component_tables.find(key) != component_tables.end()) {
			return *component_tables[key];
		}
		return luabridge::LuaRef(ComponentDB::GetLuaState());
	}

	//GetComponent (by type_name) returns LuaRef
	luabridge::LuaRef GetComponent(std::string type_name) {
		for (auto& [key, value] : component_tables) {
			if ((*value)["type"] == type_name) {
				return *value;
			}
		}
		return luabridge::LuaRef(ComponentDB::GetLuaState());
	}

	//GetComponents (by type_name) builds and returns table of LuaRefs
	luabridge::LuaRef GetComponents(std::string type_name) {
		luabridge::LuaRef components = luabridge::newTable(ComponentDB::GetLuaState());
		int i = 1;
		for (auto& [key, value] : component_tables) {
			if ((*value)["type"] == type_name) {
				components[i] = *value;
				i++;
			}
		}
		return components;
	}

	void OnCollision(collision c){
		for (auto& [key, value] : component_tables) {
			if (value->operator[]("OnCollisionEnter").isFunction() && value->operator[]("enabled")) {
				try {
					value->operator[]("OnCollisionEnter")(*value, c);
				}
				catch (luabridge::LuaException const& e) {
					ReportError(actor_name, e);
				}
			}
		}
	}

	void OnTrigger(collision c) {
		for (auto& [key, value] : component_tables) {
			if (value->operator[]("OnTriggerEnter").isFunction() && value->operator[]("enabled")) {
				try {
					value->operator[]("OnTriggerEnter")(*value, c);
				}
				catch (luabridge::LuaException const& e) {
					ReportError(actor_name, e);
				}
			}
		}
	}

	void OnCollisionExit(collision c) {
		for (auto& [key, value] : component_tables) {
			if (value->operator[]("OnCollisionExit").isFunction() && value->operator[]("enabled")) {
				try {
					value->operator[]("OnCollisionExit")(*value, c);
				}
				catch (luabridge::LuaException const& e) {
					ReportError(actor_name, e);
				}
			}
		}
	}

	void OnTriggerExit(collision c) {
		for (auto& [key, value] : component_tables) {
			if (value->operator[]("OnTriggerExit").isFunction() && value->operator[]("enabled")) {
				try {
					value->operator[]("OnTriggerExit")(*value, c);
				}
				catch (luabridge::LuaException const& e) {
					ReportError(actor_name, e);
				}
			}
		}
	}

	void addRigidbodyComponent(std::string name, rapidjson::Value::ConstMemberIterator it) {
		Rigidbody* rigidbody = new Rigidbody();
		luabridge::LuaRef componentRB(ComponentDB::GetLuaState(), rigidbody);
		if (it) {
			//loop through it->value values and for any not called "type" inject the value into the instanceTable
			for (auto itr = it->value.MemberBegin(); itr != it->value.MemberEnd(); ++itr) {
				std::string compName = itr->name.GetString();
				/*check for all possible rigidbody types
				and set rigidbody value to them if present */
				if (compName == "x") {
					rigidbody->x = itr->value.GetFloat();
				}
				else if(compName == "y") {
					rigidbody->y = itr->value.GetFloat();
				}
				else if(compName == "width") {
					rigidbody->width = itr->value.GetFloat();
				}
				else if(compName == "height") {
					rigidbody->height = itr->value.GetFloat();
				}
				else if(compName == "body_type") {
					rigidbody->body_type = itr->value.GetString();
				}
				else if(compName == "enabled") {
					rigidbody->enabled = itr->value.GetBool();
				}
				else if(compName == "precise") {
					rigidbody->precise = itr->value.GetBool();
				}
				else if(compName == "gravity_scale") {
					rigidbody->gravity_scale = itr->value.GetFloat();
				}
				else if(compName == "density") {
					rigidbody->density = itr->value.GetFloat();
				}
				else if(compName == "angular_friction") {
					rigidbody->angular_friction = itr->value.GetFloat();
				}
				else if(compName == "rotation") {
					rigidbody->rotation = itr->value.GetFloat();
				}
				else if(compName == "has_collider") {
					rigidbody->has_collider = itr->value.GetBool();
				}
				else if(compName == "has_trigger") {
					rigidbody->has_trigger = itr->value.GetBool();
				}
				else if(compName == "collider_type") {
					rigidbody->collider_type = itr->value.GetString();
				}
				else if(compName == "radius") {
					rigidbody->radius = itr->value.GetFloat();
				}
				else if(compName == "friction") {
					rigidbody->friction = itr->value.GetFloat();
				}
				else if(compName == "bounciness") {
					rigidbody->bounciness = itr->value.GetFloat();
				}
				else if(compName == "trigger_type") {
					rigidbody->trigger_type = itr->value.GetString();
				}
			}
		}
		componentRB["key"] = name;
		component_tables[name] = std::make_shared<luabridge::LuaRef>(componentRB);
		InjectConvenienceReference(component_tables[name]);
		InjectEnabled(component_tables[name], true);
		addedComponents.push_back(name);
		//ComponentDB::component_tables["Rigidbody"] = std::make_shared<luabridge::LuaRef>(componentRB);
	}


	void InjectConvenienceReference(std::shared_ptr<luabridge::LuaRef> ref) {
		(*ref)["actor"] = this;
	}

	void InjectEnabled(std::shared_ptr<luabridge::LuaRef> ref, bool enabled) {
		(*ref)["enabled"] = enabled;
	}

	void ProcessAddedComponents() {
		//sort addedComponents
		std::sort(addedComponents.begin(), addedComponents.end());
		for (std::string name : addedComponents) {
			//get component from component_tables
			std::shared_ptr<luabridge::LuaRef> component = component_tables[name];
			//run OnStart function
			if (component->operator[]("OnStart").isFunction() && component->operator[]("enabled")) {
				try {
					component->operator[]("OnStart")(*component);
				}
				catch (luabridge::LuaException const& e) {
					ReportError(actor_name, e);
				}
			}
		}
		addedComponents.clear();
	}

	void RemoveAllComponents() {
		for (auto& [key, value] : component_tables) {
			//get component from component_tables
			value->operator[]("enabled") = false;
			removedComponents.push_back(*value);
		}
	
	}

	void RemoveComponent(luabridge::LuaRef componentRef) {
		try {
			componentRef["enabled"] = false;
			removedComponents.push_back(componentRef);
			//component_tables.erase(componentRef["key"].tostring());
		}
		catch (luabridge::LuaException const& e) {
			ReportError(actor_name, e);
		}
	}

	void AddAllComponents() {
		for (auto& [key, value] : new_component_tables) {
			//add component to component_tables
			component_tables[key] = value;
			//if Ready function exists, run it
			if (value->operator[]("Ready").isFunction() && value->operator[]("enabled")) {
				try {
					value->operator[]("Ready")(*value);
				}
				catch (luabridge::LuaException const& e) {
					ReportError(actor_name, e);
				}
			}
			addedComponents.push_back(key);
		}
		new_component_tables.clear();
	}

	void addRigidbody(std::string name) {
		Rigidbody* rigidbody = new Rigidbody();
		luabridge::LuaRef componentRB(ComponentDB::GetLuaState(), rigidbody);
		componentRB["key"] = name;
		new_component_tables[name] = std::make_shared<luabridge::LuaRef>(componentRB);
		InjectConvenienceReference(new_component_tables[name]);
		InjectEnabled(new_component_tables[name], true);
		//addedComponents.push_back(name);
	}

	void addRigidbodyIns(std::string name, Actor templateActor) {
		Rigidbody* rigidbody = new Rigidbody();
		//copy values from templateActor to rigidbody
		rigidbody->x = templateActor.GetComponent("Rigidbody")["x"];
		rigidbody->y = templateActor.GetComponent("Rigidbody")["y"];
		rigidbody->width = templateActor.GetComponent("Rigidbody")["width"];
		rigidbody->height = templateActor.GetComponent("Rigidbody")["height"];
		rigidbody->body_type = templateActor.GetComponent("Rigidbody")["body_type"].tostring();
		rigidbody->enabled = templateActor.GetComponent("Rigidbody")["enabled"];
		rigidbody->precise = templateActor.GetComponent("Rigidbody")["precise"];
		rigidbody->gravity_scale = templateActor.GetComponent("Rigidbody")["gravity_scale"];
		rigidbody->density = templateActor.GetComponent("Rigidbody")["density"];
		rigidbody->angular_friction = templateActor.GetComponent("Rigidbody")["angular_friction"];
		rigidbody->rotation = templateActor.GetComponent("Rigidbody")["rotation"];
		rigidbody->has_collider = templateActor.GetComponent("Rigidbody")["has_collider"];
		rigidbody->has_trigger = templateActor.GetComponent("Rigidbody")["has_trigger"];
		rigidbody->collider_type = templateActor.GetComponent("Rigidbody")["collider_type"].tostring();
		rigidbody->radius = templateActor.GetComponent("Rigidbody")["radius"];
		rigidbody->friction = templateActor.GetComponent("Rigidbody")["friction"];
		rigidbody->bounciness = templateActor.GetComponent("Rigidbody")["bounciness"];
		rigidbody->trigger_type = templateActor.GetComponent("Rigidbody")["trigger_type"].tostring();

		luabridge::LuaRef componentRB(ComponentDB::GetLuaState(), rigidbody);
		componentRB["key"] = name;
		component_tables[name] = std::make_shared<luabridge::LuaRef>(componentRB);
		InjectConvenienceReference(component_tables[name]);
		InjectEnabled(component_tables[name], true);
		addedComponents.push_back(name);
	}

	//add component function to add new function by type from lua script
	luabridge::LuaRef AddComponent(std::string type) {
		if(type == "Rigidbody") {
			std::string name = "r" + std::to_string(ComponentDB::numCreated);
			addRigidbody(name);
			ComponentDB::numCreated++;
			return *new_component_tables[name];
		}
		else {
			//check if type of component is already in global lua state, if not, register it
			if (ComponentDB::component_tables.count(type) <= 0) {
				//look for component lua script in resources/component_types
				std::string componentPath = "resources/component_types/" + type + ".lua";
				if (!std::filesystem::exists(componentPath)) {
					std::cout << "error: failed to locate component " << type;
					exit(0);
				}
				else {
					//register component with DB
					ComponentDB::RegisterComponent(componentPath);
				}
			}

			luabridge::LuaRef instanceTable = luabridge::newTable(ComponentDB::GetLuaState());
			//get global reference to component type
			luabridge::LuaRef globalRef = luabridge::getGlobal(ComponentDB::GetLuaState(), type.c_str());
			//establish inheritance
			ComponentDB::EstablishInheritance(instanceTable, globalRef);
			//set key to type
			std::string name = "r" + std::to_string(ComponentDB::numCreated);
			instanceTable["key"] = name;
			instanceTable["type"] = type;
			ComponentDB::numCreated++;
			//add component to component tables
			new_component_tables[name] = std::make_shared<luabridge::LuaRef>(instanceTable);
			//inject convenience reference
			this->InjectConvenienceReference(new_component_tables[name]);
			//inject enabled
			this->InjectEnabled(new_component_tables[name], true);
			return instanceTable;
		}
	}

	//addcomponent 
	void addComponent(std::string name, std::string type, rapidjson::Value::ConstMemberIterator it) {
		luabridge::LuaRef instanceTable = luabridge::newTable(ComponentDB::GetLuaState());
		luabridge::LuaRef globalRef = luabridge::getGlobal(ComponentDB::GetLuaState(), type.c_str());
		if (it) {
			//loop through it->value values and for any not called "type" inject the value into the instanceTable
			for (auto itr = it->value.MemberBegin(); itr != it->value.MemberEnd(); ++itr) {
				if (itr->name.GetString() != "type") {
					if (itr->value.IsString()) {
						instanceTable[itr->name.GetString()] = itr->value.GetString();
					}
					else if (itr->value.IsInt()) {
						instanceTable[itr->name.GetString()] = itr->value.GetInt();
					}
					else if (itr->value.IsBool()) {
						instanceTable[itr->name.GetString()] = itr->value.GetBool();
					}
					else if (itr->value.IsDouble()) {
						instanceTable[itr->name.GetString()] = itr->value.GetDouble();
					}
				}
			}
		}

		ComponentDB::EstablishInheritance(instanceTable, globalRef);
		instanceTable["key"] = name;
		component_tables[name] = std::make_shared<luabridge::LuaRef>(instanceTable);
		this->InjectConvenienceReference(component_tables[name]);
		this->InjectEnabled(component_tables[name], true);
		addedComponents.push_back(name);
	}

};