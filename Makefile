BOX2D_SRC := $(shell find ./game_engine/ThirdParty/box2d-2.4.1/src -name '*.cpp')
GAME_ENGINE_SRC := game_engine/Main.cpp \
	game_engine/Classes/SceneAPI.cpp \
	game_engine/Classes/DBs/AudioDB.cpp \
	game_engine/Classes/DBs/TextDB.cpp \
	game_engine/Classes/DBs/ImageDB.cpp \
	game_engine/Classes/Renderer.cpp \
	game_engine/Classes/ColliderListen.cpp \
	game_engine/Classes/Raycasts.cpp

main:
	clang++ -O3 -std=c++17 \
	-I./game_engine/ThirdParty -I./game_engine/ThirdParty/glm/ \
	-I./game_engine/Classes/ -I./game_engine/ThirdParty/rapidjson-1.1.0 \
	-I./game_engine/ThirdParty/rapidjson-1.1.0/include -I./game_engine/resources \
	-I./game_engine/resources/scenes -I./game_engine/Classes/DBs/ \
	-I./game_engine/ThirdParty/box2d-2.4.1 \
	-I./game_engine/ThirdParty/box2d-2.4.1/src \
	-I./game_engine/ThirdParty/box2d-2.4.1/include \
	-I./game_engine/ThirdParty/box2d-2.4.1/extern \
	-I./game_engine/ThirdParty/box2d-2.4.1/include/box2d \
	-I./game_engine/ThirdParty/SDL2 -I./game_engine/ThirdParty/resources/actor_templates \
	-L./game_engine/ThirdParty/SDL2 -I./game_engine/ThirdParty/LuaBridge \
	-I./game_engine/ThirdParty/LuaBridge/LuaBridge.h \
	-I./game_engine/ThirdParty/Lua \
	-llua5.4 -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf \
	-o game_engine_linux $(GAME_ENGINE_SRC) $(BOX2D_SRC)