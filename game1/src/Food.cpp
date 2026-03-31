#include "../include/Food.h"
#include <random>
#include <algorithm>

Food::Food(int width, int height) : width(width), height(height), position({0, 0}) {
}

void Food::spawn(const Snake& snake) {
	std::mt19937 rng(static_cast<unsigned int>(GetTickCount()));
	std::uniform_int_distribution<int> dist_x(0, width - 1);
	std::uniform_int_distribution<int> dist_y(0, height - 1);
	
	while (true) {
		position = {dist_x(rng), dist_y(rng)};
		
		// 检查食物是否与蛇重叠
		bool on_snake = std::any_of(
			snake.get_body().begin(), 
			snake.get_body().end(),
			[this](const Point& p) {
				return p.x == position.x && p.y == position.y;
			}
		);
		
		if (!on_snake) {
			break;
		}
	}
}
