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
	bool rotatable = true;
};
