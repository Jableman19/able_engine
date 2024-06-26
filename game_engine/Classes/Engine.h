#pragma once
#include "Scene.h"
#include "DBs/ActorDB.h"
#include "Input.h"
#include "Application.h"
#include "Renderer.h"
#include "DBs/TextDB.h"
#include "DBs/AudioDB.h"
#include "SceneAPI.h"
#include "Rigidbody.h"
#include "Physics.h"
#include "ColliderListen.h"
#include "EventBus.h"


class Engine{
	public:
		int x_resolution; //default 640
		int y_resolution; //default 360

		

		std::string game_start_message;
		std::string game_over_good_message;
		std::string game_over_bad_message;

		std::string levelPath;
		std::string levelName;
		const char* game_title;
		rapidjson::Document jsonDoc;
		EngineUtils engineUtils;
		ColliderListen* col;

		//window
		SDL_Window* window;

		Scene scene;
		Engine() {
			x_resolution = 640;
			y_resolution = 360;
			levelPath = "resources/scenes/";
			levelName = "";
			game_title = "";

			window = nullptr;
			scene = Scene();
			SceneAPI::scene = &scene;
		}

		//get dimensions of the window
		static glm::vec2 GetDimensions() {
			//use x_resolution and y_resolution to make glm vec2
			return glm::vec2(Renderer::window_width, Renderer::window_height);
		}

		void start() {

			SDL_Init(SDL_INIT_VIDEO);
			TTF_Init();
			Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
			Mix_AllocateChannels(50);
			//init lua
			lua_State* L = luaL_newstate();
			luaL_openlibs(L);
			
			ComponentDB::SetLuaState(L);
			Input::Init();


			AudioDB::LoadAllSubtitles();

			//DEBUG
			luabridge::getGlobalNamespace(L)
				.beginNamespace("Debug")
				.addFunction("Log", ComponentDB::CppLog)
				.addFunction("LogError", ComponentDB::CppLogError)
				.endNamespace();

			//ACTOR
			luabridge::getGlobalNamespace(L)
				.beginClass<Actor>("Actor")
				.addFunction("GetName", &Actor::GetName)
				.addFunction("GetID", &Actor::GetID)
				.addFunction("GetComponentByKey", &Actor::GetComponentByKey)
				.addFunction("GetComponent", &Actor::GetComponent)
				.addFunction("GetComponents", &Actor::GetComponents)
				.addFunction("AddComponent", &Actor::AddComponent)
				.addFunction("RemoveComponent", &Actor::RemoveComponent)
				.endClass();
				
			//ACTORS
			luabridge::getGlobalNamespace(L)
				.beginNamespace("Actor")
					.addFunction("Find", &ActorDB::Find)
					.addFunction("FindAll", &ActorDB::FindAll)
				    .addFunction("Instantiate", &ActorDB::Instantiate)
				    .addFunction("Destroy", &ActorDB::Destroy)
				.endNamespace();

			//APPLICATION
			luabridge::getGlobalNamespace(L)
				.beginNamespace("Application")
					.addFunction("Quit", Application::Quit)
				    .addFunction("GetFrame", Application::GetFrame)
				    .addFunction("Sleep", Application::Sleep)
					.addFunction("OpenURL", Application::OpenURL)
					.addFunction("GetDimensions", GetDimensions)
				.endNamespace();

			//GLM
			luabridge::getGlobalNamespace(L)
				.beginClass<glm::vec2>("vec2")
				.addConstructor<void(*)(float, float)>()
				.addProperty("x", &glm::vec2::x)
				.addProperty("y", &glm::vec2::y)
				.endClass();

			//INPUT
			luabridge::getGlobalNamespace(L)
				.beginNamespace("Input")
					.addFunction("GetKey", Input::GetKey)
					.addFunction("GetKeyDown", Input::GetKeyDown)
					.addFunction("GetKeyUp", Input::GetKeyUp)
					.addFunction("GetMousePosition", Input::GetMousePosition)
					.addFunction("GetMousePositionWorld", Input::GetMousePositionWorld)
					.addFunction("GetMouseButtonDown", Input::GetMouseButtonDown)
					.addFunction("GetMouseButtonUp", Input::GetMouseButtonUp)
					.addFunction("GetMouseButton", Input::GetMouseButton)
					.addFunction("GetMouseScrollDelta", Input::GetMouseScrollDelta)
				.endNamespace();

			//TEXT API
			luabridge::getGlobalNamespace(L)
				.beginNamespace("Text")
					.addFunction("Draw", &TextDB::Draw)
				.endNamespace();

			//AUDIO API
			luabridge::getGlobalNamespace(L)
				.beginNamespace("Audio")
					.addFunction("Play", &AudioDB::Play)
					.addFunction("Halt", &AudioDB::Halt)
					.addFunction("SetVolume", &AudioDB::SetVolume)
				.endNamespace();

			//IMAGE API
			luabridge::getGlobalNamespace(L)
				.beginNamespace("Image")
					.addFunction("DrawUI", &ImageDB::DrawUI)
					.addFunction("DrawUIEx", &ImageDB::DrawUIEx)
					.addFunction("Draw", &ImageDB::Draw)
					.addFunction("DrawEx", &ImageDB::DrawEx)
					.addFunction("DrawPixel", &ImageDB::DrawPixel)
				.endNamespace();

			//CAMERA API
			luabridge::getGlobalNamespace(L)
				.beginNamespace("Camera")
					.addFunction("SetPosition", &Renderer::SetPosition)
					.addFunction("SetZoom", &Renderer::SetZoom)
					.addFunction("GetZoom", &Renderer::GetZoom)
					.addFunction("GetPositionX", &Renderer::GetPositionX)
					.addFunction("GetPositionY", &Renderer::GetPositionY)
					.addFunction("ScreenToWorld", &Renderer::ScreenToWorld)
				.endNamespace();

			//SCENE API
			luabridge::getGlobalNamespace(L)
				.beginNamespace("Scene")
					.addFunction("Load", &SceneAPI::Load)
					.addFunction("GetCurrent", &SceneAPI::GetCurrent)
					.addFunction("DontDestroy", &SceneAPI::DontDestroy)
				.endNamespace();

			//VECTOR2 IMP
			luabridge::getGlobalNamespace(L)
				.beginClass<b2Vec2>("Vector2")
				.addConstructor<void(*)(float, float)>()
				.addData("x", &b2Vec2::x)
				.addData("y", &b2Vec2::y)
				.addFunction("Normalize", &b2Vec2::Normalize)
				.addFunction("Length", &b2Vec2::Length)
				.addFunction("__add", &b2Vec2::operator_add)
				.addFunction("__sub", &b2Vec2::operator_sub)
				.addFunction("__mul", &b2Vec2::operator_mul)
				.addStaticFunction("Distance", &b2Distance)
				.addStaticFunction("Dot", static_cast<float (*)(const b2Vec2&, const b2Vec2&)>(&b2Dot))
				.endClass();

			//RIGIDBODY
			luabridge::getGlobalNamespace(L)
				.beginClass<Rigidbody>("Rigidbody")
				.addConstructor<void(*)()>()
				.addData("enabled", &Rigidbody::enabled)
				.addData("key", &Rigidbody::key)
				.addData("type", &Rigidbody::type)
				.addData("actor", &Rigidbody::actor)
				.addData("body", &Rigidbody::body)
				.addData("body_type", &Rigidbody::body_type)
				.addData("x", &Rigidbody::x)
				.addData("y", &Rigidbody::y)
				.addData("precise", &Rigidbody::precise)
				.addData("gravity_scale", &Rigidbody::gravity_scale)
				.addData("angular_friction", &Rigidbody::angular_friction)
				.addData("density", &Rigidbody::density)
				.addData("rotation", &Rigidbody::rotation)
				.addData("has_collider", &Rigidbody::has_collider)
				.addData("has_trigger", &Rigidbody::has_trigger)
				.addData("width", &Rigidbody::width)
				.addData("height", &Rigidbody::height)
				.addData("radius", &Rigidbody::radius)
				.addData("friction", &Rigidbody::friction)
				.addData("bounciness", &Rigidbody::bounciness)
				.addData("collider_type", &Rigidbody::collider_type)
				.addData("trigger_type", &Rigidbody::trigger_type)
				.addData("trigger_width", &Rigidbody::trigger_width)
				.addData("trigger_height", &Rigidbody::trigger_height)
				.addData("trigger_radius", &Rigidbody::trigger_radius)
				.addFunction("OnStart", &Rigidbody::OnStart)
				.addFunction("GetPosition", &Rigidbody::GetPosition)
				.addFunction("GetRotation", &Rigidbody::GetRotation)
				.addFunction("Ready", &Rigidbody::Ready)
				.addFunction("AddForce", &Rigidbody::AddForce)
				.addFunction("SetVelocity", &Rigidbody::SetVelocity)
				.addFunction("SetAngularVelocity", &Rigidbody::SetAngularVelocity)
				.addFunction("SetGravityScale", &Rigidbody::SetGravityScale)
				.addFunction("SetPosition", &Rigidbody::SetPosition)
				.addFunction("SetRotation", &Rigidbody::SetRotation)
				.addFunction("SetUpDirection", &Rigidbody::SetUpDirection)
				.addFunction("SetRightDirection", &Rigidbody::SetRightDirection)
				.addFunction("GetVelocity", &Rigidbody::GetVelocity)
				.addFunction("GetAngularVelocity", &Rigidbody::GetAngularVelocity)
				.addFunction("GetGravityScale", &Rigidbody::GetGravityScale)
				.addFunction("GetUpDirection", &Rigidbody::GetUpDirection)
				.addFunction("GetRightDirection", &Rigidbody::GetRightDirection)
				.addFunction("OnDestroy", &Rigidbody::OnDestroy)
				.endClass();

			//COLLISION CLASS
			luabridge::getGlobalNamespace(L)
				.beginClass<collision>("Collision")
				.addData("other", &collision::other)
				.addData("point", &collision::point)
				.addData("normal", &collision::normal)
				.addData("relative_velocity", &collision::relative_velocity)
				.endClass();

			//PHYSICS
			luabridge::getGlobalNamespace(L)
				.beginNamespace("Physics")
				.addFunction("Raycast", &Physics::Raycast)
				.addFunction("RaycastAll", &Physics::RaycastAll)
				.endNamespace();

			//HITRESULT
			luabridge::getGlobalNamespace(L)
				.beginClass<HitResult>("HitResult")
				.addData("actor", &HitResult::actor)
				.addData("point", &HitResult::point)
				.addData("normal", &HitResult::normal)
				.addData("is_trigger", &HitResult::is_trigger)
				.endClass();

			//EVENTBUS
			luabridge::getGlobalNamespace(L)
				.beginNamespace("Event")
				.addFunction("Subscribe", &EventBus::Subscribe)
				.addFunction("Unsubscribe", &EventBus::Unsubscribe)
				.addFunction("Publish", &EventBus::Publish)
				.endNamespace();

			//ACCESSIBILITY
			luabridge::getGlobalNamespace(L)
				.beginNamespace("Accessibility")
				.addFunction("Subtitles", &AudioDB::Subtitles)
				.addFunction("SetColorMode", &ImageDB::SetColorMode)
				.endNamespace();

			//check for resources folder
			if (!std::filesystem::exists("resources")) {
				std::cout << "error: resources/ missing";
				exit(0);
			}

			//define rgb clear colors
			int r = 0;
			int g = 0;
			int b = 0;
			//Rendering Startup
			//if rendering.config exists, override x_resolution and y_resolution
			if (std::filesystem::exists("resources/rendering.config")) {
				engineUtils.ReadJsonFile("resources/rendering.config", jsonDoc);
				if (jsonDoc.HasMember("x_resolution")) {
					this->x_resolution = jsonDoc["x_resolution"].GetInt();
				}
				if (jsonDoc.HasMember("y_resolution")) {
					this->y_resolution = jsonDoc["y_resolution"].GetInt();
				}
				if(jsonDoc.HasMember("cam_offset_x")){
					Renderer::camera_offset_x = jsonDoc["cam_offset_x"].GetFloat();
				}
				if(jsonDoc.HasMember("cam_offset_y")){
					Renderer::camera_offset_y = jsonDoc["cam_offset_y"].GetFloat();
				}
				if(jsonDoc.HasMember("zoom_factor")){
					Renderer::zoom_factor = jsonDoc["zoom_factor"].GetFloat();
				}
				if(jsonDoc.HasMember("cam_ease_factor")){
					scene.cam_ease_factor = jsonDoc["cam_ease_factor"].GetFloat();
				}
				if(jsonDoc.HasMember("x_scale_actor_flipping_on_movement")){
					scene.xFlip = jsonDoc["x_scale_actor_flipping_on_movement"].GetBool();
				}
				if (jsonDoc.HasMember("colorMode")) {
					ImageDB::SetColorModeI(jsonDoc["colorMode"].GetInt());
				}
				if (jsonDoc.HasMember("clear_color_r")) {
					r = jsonDoc["clear_color_r"].GetInt();
				}
				if (jsonDoc.HasMember("clear_color_g")) {
					g = jsonDoc["clear_color_g"].GetInt();
				}
				if (jsonDoc.HasMember("clear_color_b")) {
					b = jsonDoc["clear_color_b"].GetInt();
				}
				
			}

			//check for game.config json in resources folder
			if (!std::filesystem::exists("resources/game.config")) {
				std::cout << "error: resources/game.config missing";
				exit(0);
			}

			
			//get game_start_message, game_over_good_message, and game_over_bad_message from game.config using rapidjson
			//define a document
			

			engineUtils.ReadJsonFile("resources/game.config", jsonDoc);
			if(jsonDoc.HasMember("game_title")){
				this->game_title = jsonDoc["game_title"].GetString();
			}
			if (jsonDoc.HasMember("initial_scene")) {
				this->levelName = jsonDoc["initial_scene"].GetString();
				this->levelPath += levelName + ".scene";
			}
			else {
				std::cout << "error: initial_scene unspecified";
				exit(0);
			}
			

			Renderer::window_height = y_resolution;
			Renderer::window_width = x_resolution;

			window = Helper::SDL_CreateWindow498(game_title, 0, 40, x_resolution, y_resolution, SDL_WINDOW_SHOWN);

			TextDB::LoadBuiltInFonts();
			TextDB::windowWidth = x_resolution;
			TextDB::windowHeight = y_resolution;

			Renderer::start(&window, r,g,b);

			//renderer.player = &player;
			//define a scene
			this->scene.LoadScene(levelPath, levelName, engineUtils, jsonDoc, L);
			Input::Init();
			this->MainLoop();
		}

		//engine run function
		void MainLoop() {

			//create timer to time each frame
			float start_time = SDL_GetTicks() * .0001;
			while (true) {
				if(WorldManager::rbs && !WorldManager::contactListener){
					col = new ColliderListen();
					WorldManager::world->SetContactListener(col);
					WorldManager::contactListener = true;
				}
				//get time since last frame
				float current_time = SDL_GetTicks() * .001;
				start_time = current_time;

				//game loop
				Renderer::clear();
				//Update
				int endState = Renderer::getEvents();
				if (scene.nextScene != "") {
					EventBus::reset();
				}
				scene.Update();
				//LateUpdate
				Input::LateUpdate();
				scene.LateUpdate();
				EventBus::ProcessRequests();

				if (WorldManager::rbs) {
					WorldManager::world->Step(1.0f / 60.0f, 8, 3);
				}
				Renderer::Render();
				if (endState == 1) {
					exit(0);
				}
			}
		}
};
