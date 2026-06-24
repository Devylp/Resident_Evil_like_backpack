#pragma once
#include <vector>
#include <iostream>
#include "Item.hpp"

/*
Используем умные указатели 
 
Конструктор
Метод добавления элемента
Метод проверки на вместимость
Метод автосортировки
Метод удаления элемента из рюкзака
Метод перемещения

Метод проверки баланса
*/

class BackpackLogick {
private:
	int Length;
	int Width;
	int Weight;
	std::vector<Item> Items; // вектор предметов (каждый элемент это обьект структуры Item)
	std::vector<std::vector<bool>> grid; // сетка состоянии ячеек рюкзака

public:
	BackpackLogick(int length, int width, int weight) : 
	Length(length), Width(width), Weight(weight), grid(length, std::vector<bool>(width, false))
		{}

	void AddItem(const Item& item, int x, int y) {

	}

	void RemoveItem(int x, int y) {

	}

	void MoveItem() {

	}

	void AutoSortBackpack() {

	}

	bool CanPlaceItem(const Item& item, int x, int y) {
		if (x < 0 || y < 0 || item.wth + x > Width || item.len + y > Length) {
			return false;
		}
		
		for (int row = 0; row < item.len; ++row) {
			for (int col = 0; col < item.wth; ++col) {
				if (grid[y + row][x + col] == true) {
					return false;
				}
			}
		}

		return true;

	}

	bool IsBalance() {

	}

};