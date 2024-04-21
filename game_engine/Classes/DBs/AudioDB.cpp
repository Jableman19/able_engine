#include "AudioDB.h"
#include "TextDB.h"
#include "EngineUtils.h"
#include <curl/curl.h>
#include "../key.h"
#include <iostream>
#include <string>
#include <sstream>
#include <cstdint>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

namespace AudioDB {
	std::unordered_map<std::string, Mix_Chunk*> audioMap;
    std::unordered_map<std::string, std::string> subtitleMap;
    std::unordered_map<std::string, double> activeSubMap;
    bool subtitlesOn = false;

    void Subtitles(bool on) {
		subtitlesOn = on;
	}

    size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
        std::ostringstream* stream = (std::ostringstream*)userp;
        size_t count = size * nmemb;
        stream->write((char*)contents, count);
        //print stream to console
        //std::cout << stream->str() << std::endl;
        return count;
    }

    std::string getSubtitle(std::string audioPath) {
        CURL* curl;
        CURLcode res;

        struct curl_httppost* formpost = NULL;
        struct curl_httppost* lastptr = NULL;
        struct curl_slist* headerlist = NULL;
        static const char buf[] = "Expect:";

        std::ostringstream response_data;

        curl_global_init(CURL_GLOBAL_ALL);

        std::string auth_header = "Authorization: Bearer " + Key::OPENAI_KEY;

        // Set up the header
        headerlist = curl_slist_append(headerlist, buf);
        headerlist = curl_slist_append(headerlist, auth_header.c_str());
        headerlist = curl_slist_append(headerlist, "Content-Type: multipart/form-data");

        // Set up the form post
        curl_formadd(&formpost,
            &lastptr,
            CURLFORM_COPYNAME, "file",
            CURLFORM_FILE, audioPath.c_str(),
            CURLFORM_END);
        curl_formadd(&formpost,
            &lastptr,
            CURLFORM_COPYNAME, "model",
            CURLFORM_COPYCONTENTS, "whisper-1",
            CURLFORM_END);

        // Initialize curl
        curl = curl_easy_init();

        // Set the attributes
        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, "https://api.openai.com/v1/audio/transcriptions");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
            curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);

            // Perform the request
            res = curl_easy_perform(curl);

            // Check for errors
            if (res != CURLE_OK) {
                std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            }
            /* else {
                 std::cout << "Response from server: " << response_data.str() << std::endl;
             }*/

             // cleanup
            curl_easy_cleanup(curl);

            // Clean the headers and forms
            curl_formfree(formpost);
            curl_slist_free_all(headerlist);
        }

        curl_global_cleanup();

        //go through response and get 'text' field in the json
        size_t start = response_data.str().find("text") + 8;
        size_t end = response_data.str().find("\"", start);
        if (start != std::string::npos && end != std::string::npos){
            return response_data.str().substr(start, end - start);
        }
        else {
            return response_data.str();
        }

    }
    

    struct WAVHeader {
        char riff_header[4]; // "RIFF"
        uint32_t wav_size;
        char wave_header[4]; // "WAVE"
        char fmt_header[4];  // "fmt "
        uint32_t fmt_chunk_size;
        uint16_t audio_format;
        uint16_t num_channels;
        uint32_t sample_rate;
        uint32_t byte_rate;
        uint16_t sample_alignment;
        uint16_t bit_depth;
    };

    struct ChunkHeader {
        char id[4];
        uint32_t size;
    };

    uint32_t findDataChunk(std::ifstream& file) {
        ChunkHeader chunkHeader;
        while (file.read((char*)&chunkHeader, sizeof(ChunkHeader))) {
            std::cout << "Chunk ID: " << std::string(chunkHeader.id, 4) << ", Size: " << chunkHeader.size << std::endl;
            if (std::string(chunkHeader.id, 4) == "data") {
                return chunkHeader.size;
            }
            else {
                file.seekg(chunkHeader.size + (chunkHeader.size % 2), std::ios::cur);
            }
        }
        return 0; // If no data chunk is found
    }

    double calculateWAVLength(const std::string& filepath) {
        std::ifstream file(filepath, std::ios::binary);
        WAVHeader header;

        if (file.is_open()) {
            file.read((char*)&header, sizeof(WAVHeader));

            std::cout << "Header read, searching for data chunk..." << std::endl;

            uint32_t dataChunkSize = findDataChunk(file);
            file.close();

            if (header.byte_rate == 0) return 0.0;
            return static_cast<double>(dataChunkSize) / static_cast<double>(header.byte_rate);
        }
        return 0.0;
    }

	void loadAudio(std::string title) {
		std::string path1 = "resources/audio/" + title + ".wav";
		std::string path2 = "resources/audio/" + title + ".ogg";
		std::string audioPath;
		//check if path exists
		if (std::filesystem::exists(path1)) {
			audioPath = path1;
		}
		else if (std::filesystem::exists(path2)) {
			audioPath = path2;
		}
		else {
			std::cout << "error: failed to play audio clip " << title;
			exit(0);
		}
        if (subtitlesOn && (subtitleMap.find(title) == subtitleMap.end())) {
            std::string sub = getSubtitle(audioPath);
            subtitleMap[title] = sub;
        }
		Mix_Chunk* audio = Mix_LoadWAV(audioPath.c_str());
        //get error from Mix
        if (audio == NULL) {
            std::cerr << "Error loading audio: " << Mix_GetError() << std::endl;
        }
		audioMap[title] = audio;
	}

	Mix_Chunk* getAudio(std::string title) {
		if (audioMap.find(title) == audioMap.end()) {
			loadAudio(title);
		}
		return audioMap[title];
	}

    void Play(int channel, std::string title, int loops)
	{
       Mix_Chunk* audio = getAudio(title);
       int res = Mix_PlayChannel(channel, audio, loops);
       if (res == -1) {
		   std::cerr << "Error playing audio: " << Mix_GetError() << std::endl;
	   }

       if(subtitleMap.find(title) != subtitleMap.end() && subtitlesOn) {
           activeSubMap[subtitleMap[title]] = 60 * calculateWAVLength("resources/audio/" + title + ".wav");
	   }

	}

    void UpdateSubtitles() {
		//iterate through active subtitles
        if(!subtitlesOn) {
			activeSubMap.clear();
		}
        std::vector<std::string> toDelete;
        for (auto& [content, length] : activeSubMap) {
            if(length > 0) {
				length--;
                TextDB::DrawSubtitle(content);
			}
			else {
				toDelete.push_back(content);
			}
        }
        for (auto& content : toDelete) {
			activeSubMap.erase(content);
		}
	}

    void LoadAllSubtitles() {
    	//look for subtitles.json in resources folder
        if (std::filesystem::exists("resources/subtitles.json")) {
            //use rapidjson to parse json file
            rapidjson::Document doc;
            EngineUtils::ReadJsonFile("resources/subtitles.json", doc);
            for(const auto& entry : doc.GetArray()) {
				std::string title = entry["title"].GetString();
				std::string subtitle = entry["subtitle"].GetString();
				subtitleMap[title] = subtitle;
			}
        }
        for (const auto& entry : std::filesystem::directory_iterator("resources/audio")) {
			std::string path = entry.path().string();
			std::string title = entry.path().filename().string();
			title = title.substr(0, title.find_last_of("."));
            if (subtitleMap.find(title) == subtitleMap.end()) {
                subtitleMap[title] = getSubtitle(path);
            }
		}

        //rewrite subtitles.json
        rapidjson::Document doc;
		doc.SetArray();
		for (const auto& [title, subtitle] : subtitleMap) {
			rapidjson::Value entry(rapidjson::kObjectType);
			entry.AddMember("title", rapidjson::Value(title.c_str(), doc.GetAllocator()), doc.GetAllocator());
			entry.AddMember("subtitle", rapidjson::Value(subtitle.c_str(), doc.GetAllocator()), doc.GetAllocator());
			doc.PushBack(entry, doc.GetAllocator());
		}
		std::ofstream file("resources/subtitles.json");
        //get string from rapidjson document
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        doc.Accept(writer);
        std::string str = buffer.GetString();
        file << str;
    }

	void Halt(int channel) {
		AudioHelper::Mix_HaltChannel498(channel);
	}

	void SetVolume(int channel, int volume) {
		AudioHelper::Mix_Volume498(channel, volume);
	}

};
