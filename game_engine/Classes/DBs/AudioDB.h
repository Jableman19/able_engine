#pragma once
#ifndef AUD_DB_H
#define AUD_DB_H
#include <unordered_map>
#include <string>
#include "AudioHelper.h"
namespace AudioDB {
	extern 	std::unordered_map<std::string, Mix_Chunk*> audioMap;
	extern bool subtitlesOn;


	void Subtitles(bool on);

	void loadAudio(std::string audioName);
	void LoadAllSubtitles();
	Mix_Chunk* getAudio(std::string audioName);
	void Play(int channel, std::string title, int loops);
	void Halt(int channel);
	void SetVolume(int channel, int volume);
	void UpdateSubtitles();
}
#endif // !AUD_DB_H