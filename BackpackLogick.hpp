#pragma once
#include <vector>
#include <cmath>
#include <iostream>
#include <algorithm>
#include "Item.hpp"


class BackpackLogick {
private:
	int Length;
	int Width;
	int Capacity;
	std::vector<Item> Items_vec; // вектор предметов (каждый элемент это обьект структуры Item)
	std::vector<std::vector<bool>> grid; // сетка состоянии ячеек рюкзака

public:
	BackpackLogick(int length, int width, int capacity) : 
	Length(length), Width(width), Capacity(capacity), grid(length, std::vector<bool>(width, false))
		{}

	// Получить размеры рюкзака
	int GetWidth() const { return Width; }
	int GetLength() const { return Length; }

	// Получить список предметов (только для чтения)
	const std::vector<Item>& GetItems() const { return Items_vec; }

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

	bool RotateItem(int x, int y) {
		Item original;
		bool found = false;

		// 1. Ищем предмет
		for (const auto& item : Items_vec) {
			if (item.startX != -1 && (x >= item.startX && x < item.startX + item.wth) &&
				(y >= item.startY && y < item.startY + item.len)) {
				original = item;
				found = true;
				break;
			}
		}

		// 1.1 Проверяем, что предмет возможно вращать или не был найден
		if (!found) { return false; }

		if (!original.rotatable) { return false; }

		// 2. Вращаем предмет за счет обмена полей
		Item rotated = original;
		RemoveItem(original.startX, original.startY);

		std::swap(rotated.wth, rotated.len);
		
		// 3. Проверяем, что при вращении предмет не вылез за границы рюкзака
		if (CanPlaceItem(rotated, rotated.startX, rotated.startY)) {
			AddItem(rotated, rotated.startX, rotated.startY);
			return true;
		}

		else {
			AddItem(original, original.startX, original.startY);
			return false;
		}


	}

	std::vector<Item> AutoSortBackpack() {
		// Жадный алгоритм

		// 1. Очищаем вектор и матрицу предметов для освобождения места в рюкзаке перед сортировкой
		std::vector<Item> tmp = std::move(Items_vec); // Не копируем весь вектор
		Items_vec.clear();
		grid = std::vector<std::vector<bool>>(Length, std::vector<bool>(Width, false));

		// 2. Сортируем промежуточный вектор tmp для дальнейшей сортировки
		auto comp = [](const Item& item1, const Item& item2) {
			int maxItem1 = std::max(item1.wth, item1.len);
			int minItem1 = std::min(item1.wth, item1.len);
			int maxItem2 = std::max(item2.wth, item2.len);
			int minItem2 = std::min(item2.wth, item2.len);

			if (maxItem1 != maxItem2) {
				return maxItem1 > maxItem2;
			}
			return minItem1 > minItem2;
			};

		std::sort(tmp.begin(), tmp.end(), comp);

		std::vector<Item> NotPlacedItems;
		for (const auto& item : tmp) {
			bool placed = false;
			for (int y = 0; y <= (Length - item.len) && !placed; ++y) {
				for (int x = 0; x <= (Width - item.wth) && !placed; ++x) {
					if (CanPlaceItem(item, x, y)) {
						AddItem(item, x, y);
						placed = true;
					}
				}
			}
			if (!placed) {
				NotPlacedItems.push_back(item);
			}
		}

		return NotPlacedItems;
	}

	bool CanPlaceItem(const Item& item, int x, int y) {

		// Блок проверки на перевес
		//-------------------------
		int currentWeight = 0;
		for (const auto& existingItem : Items_vec) {
			currentWeight += existingItem.weight;
		}

		if (currentWeight + item.weight > Capacity) { return false; }

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

		// 1. Центр тяжести исходного рюкзака (считаем, что рюкзак однородный)
		float c_x = Width / 2.0f;
		float c_y = Length / 2.0f;
		float newc_x;
		float newc_y;
		float sumx = 0.0f;
		float sumy = 0.0f;
		int total_mass = 0;
		
		// 2. Считаем новый центр тяжести с учетом новых предметов
		for (const auto& item : Items_vec) {
			float item_center_x = item.startX + item.wth / 2.0f;
			float item_center_y = item.startY + item.len / 2.0f;
			
			sumx += item_center_x * item.weight;
			sumy += item_center_y * item.weight;

			total_mass += item.weight;
		}

		if (total_mass == 0) { return true; }

		newc_x = sumx / total_mass;
		newc_y = sumy / total_mass;

		// 3.На основе (1) и (2) вычисляем разницу:
		// | (2) - (1) | < 0.1 (10 % - максимальна допустимая величина погрешности)
		// Интерпретируем выводы
		return std::abs(newc_x - c_x) <= 0.1f*Width &&
			   std::abs(newc_y - c_y) <= 0.1f*Length;
	}

};
