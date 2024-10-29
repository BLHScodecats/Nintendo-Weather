#pragma once

#include <3ds.h>
#include <citro2d.h>
#include <curl/curl.h>
#include <functional>
#include <memory>
#include <string>
#include <string.h>
#include <sstream>
#include <stdlib.h>
#include <vector>
#include <thread>
#include <memory>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <map>
#include <malloc.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

static const std::string DATA_DIR = "/ku_engineering_project/";
static const unsigned int MAX_INPUT_SIZE = 256;

// like so many colors dude, just in case
namespace Color
{
	static const u32 BLACK = C2D_Color32(0x00, 0x00, 0x00, 0xFF);
	static const u32 WHITE = C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF);
	static const u32 RED = C2D_Color32(0xFF, 0x00, 0x00, 0xFF);
	static const u32 LIME = C2D_Color32(0x00, 0xFF, 0x00, 0xFF);
	static const u32 BLUE = C2D_Color32(0x00, 0x00, 0xFF, 0xFF);
	static const u32 YELLOW = C2D_Color32(0xFF, 0xFF, 0x00, 0xFF);
	static const u32 CYAN = C2D_Color32(0x00, 0xFF, 0xFF, 0xFF);
	static const u32 MAGENTA = C2D_Color32(0xFF, 0x00, 0xFF, 0xFF);
	static const u32 SILVER = C2D_Color32(0xC0, 0xC0, 0xC0, 0xFF);
	static const u32 GREY = C2D_Color32(0x80, 0x80, 0x80, 0xFF);
	static const u32 MAROON = C2D_Color32(0x80, 0x00, 0x00, 0xFF);
	static const u32 OLIVE = C2D_Color32(0x80, 0x80, 0x00, 0xFF);
	static const u32 GREEN = C2D_Color32(0x00, 0x80, 0x00, 0xFF);
	static const u32 PURPLE = C2D_Color32(0x80, 0x00, 0x80, 0xFF);
	static const u32 TEAL = C2D_Color32(0x00, 0x80, 0x80, 0xFF);
	static const u32 NAVY = C2D_Color32(0x00, 0x00, 0x80, 0xFF);

	static const u32 COLOR_ADJUST = C2D_Color32(0x20, 0x20, 0x20, 0x00);
}

inline std::vector<std::string> splitString(std::string s, char delimiter = ' ')
{
	std::vector<std::string> result;

	int frontIndex = 0;
	for (unsigned int i = 0; i < s.size(); i++)
	{
		if (s[i] == delimiter)
		{
			result.push_back(s.substr(frontIndex, i-frontIndex));
			frontIndex = i+1;
		}
		else if (i == s.size()-1)
			result.push_back(s.substr(frontIndex));
	}

	return result;
}