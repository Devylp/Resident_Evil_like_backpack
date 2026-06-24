#pragma once
#include <string>

struct Item {
	int ID; // уникальный идентификатор предмета
	std::string texturePath;
	std::string NameItem;
	int weight;
	int len;
	int wth;
};
