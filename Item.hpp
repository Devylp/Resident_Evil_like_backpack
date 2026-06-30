#pragma once
#include <string>

struct Item {
	int ID; // уникальный идентификатор предмета
	std::string texturePath;
	std::string NameItem;
	int weight;
	int len;
	int wth;
	int startX = -1;
	int startY = -1;
	int rotation = 0; // 0 = 0°, 1 = 90°, 2 = 180°, 3 = 270°
	bool rotatable = true;
};
