#pragma once

#include <string>
#include <thread>
#include <cstdlib>	
#include "Helper.h"

namespace Application
{
	void Quit(){
		exit(0);
	}

	void Sleep(int dur_ms){
		std::this_thread::sleep_for(std::chrono::milliseconds(dur_ms));
	}

	int GetFrame() {
		return Helper::GetFrameNumber();
	}

	void OpenURL(std::string url) {
		//openURL based on operating system

		//windows - use start
#ifdef _WIN32
		std::string command = "start " + url;
		system(command.c_str());
#endif
//mac - use open
#ifdef __APPLE__
		std::string command = "open " + url;
		system(command.c_str());
#endif
//linux - use xdg-open
#ifdef __linux__
		std::string command = "xdg-open " + url;
		system(command.c_str());
#endif
	}
}