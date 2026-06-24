#pragma once
#include <vector>
#include <iostream>
#include "Item.hpp"

/*
Используем умные указатели 
 
Конструктор (есть)
Метод добавления элемента (есть)
Метод проверки на вместимость (есть)
Метод автосортировки ()
Метод удаления элемента из рюкзака (есть)
Метод перемещения (есть)

Метод проверки баланса ()
*/

class BackpackLogick {
private:
	int Length;
	int Width;
	int Weight;
	std::vector<Item> Items_vec; // вектор предметов (каждый элемент это обьект структуры Item)
	std::vector<std::vector<bool>> grid; // сетка состоянии ячеек рюкзака

public:
	BackpackLogick(int length, int width, int weight) : 
	Length(length), Width(width), Weight(weight), grid(length, std::vector<bool>(width, false))
		{}

	bool AddItem(const Item& item, int x, int y) {

		if (!CanPlaceItem(item, x, y)) { return false; }
		
		// 1. Добавляем предмет
		for (int row = 0; row < item.len; ++row) {
			for (int col = 0; col < item.wth; ++col) {
				grid[y + row][x + col] = true;
			}
		}

		// 2. Добавляем предмет в вектор (через копию item)
		Item placedItem = item;
		placedItem.startX = x;
		placedItem.startY = y;

		Items_vec.push_back(placedItem);
		return true;
	}

	void RemoveItem(int x, int y) {
		for (size_t i = 0; i < Items_vec.size(); ++i) {
			const Item& item = Items_vec[i];

			if (item.startX != -1 &&
				x >= item.startX && x < item.startX + item.wth &&
				y >= item.startY && y < item.startY + item.len)
			{
				
				// 1. Удаляем предмет
				for (int row = 0; row < item.len; ++row) {
					for (int col = 0; col < item.wth; ++col) {
						grid[item.startY + row][item.startX + col] = false;
					}
				}

				// 2. Убираем предмет из вектора
				Items_vec.erase(Items_vec.begin() + i);

				return;
			}
		}
	}

	bool MoveItem(int oldX, int oldY, int newX, int newY) {
		Item tmp;
		bool found = false;

		for (const auto& item : Items_vec) {
			if (item.startX != -1 && (oldX >= item.startX && oldX < item.startX + item.wth) &&
				(oldY >= item.startY && oldY < item.startY + item.len)) {
				tmp = item;
				found = true;
				break;
			}
		}

		if (!found) { return false; }

		RemoveItem(oldX, oldY);

		if (CanPlaceItem(tmp, newX, newY)) {
			AddItem(tmp, newX, newY);
			return true;
		}

		else {
			AddItem(tmp, oldX, oldY);
			return false;
		}
	}

	void AutoSortBackpack() {

	}

	bool CanPlaceItem(const Item& item, int x, int y) {

		// Блок проверки на перевес
		//-------------------------
		int currentWeight = 0;
		for (const auto& item : Items_vec) {
			currentWeight += item.weight;
		}

		if (currentWeight > Weight) { return false; }

		if (x < 0 || y < 0 || item.wth + x > Width || item.len + y > Length) {
			return false;
		}
		//-------------------------

		// Проверка на вместимости предмета
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