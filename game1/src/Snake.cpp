#include "../include/Snake.h"
#include <algorithm>

Snake::Snake(int width, int height) : width(width), height(height) {
	reset();
}

void Snake::reset() {
	body.clear();
	body.push_back({width / 2, height / 2});
	body.push_back({width / 2 - 1, height / 2});
	body.push_back({width / 2 - 2, height / 2});
}

bool Snake::move(Direction direction, const Point& food) {
	Point head = body.front();
	
	if (direction == Direction::Up) {
		--head.y;
	} else if (direction == Direction::Down) {
		++head.y;
	} else if (direction == Direction::Left) {
		--head.x;
	} else {
		++head.x;
	}
	
	// 检查碰撞
	if (check_collision(head)) {
		return false; // 碰撞发生，游戏结束
	}
	
	body.push_front(head);
	
	// 检查是否吃到食物
	if (head.x == food.x && head.y == food.y) {
		return true; // 不删除尾部，因为吃到了食物
	}
	
	// 没有吃到食物，正常移动
	body.pop_back();
	return true;
}

bool Snake::check_collision(const Point& head) {
	// 检查边界碰撞
	if (head.x < 0 || head.x >= width || head.y < 0 || head.y >= height) {
		return true;
	}
	
	// 检查自身碰撞
	for (const auto& part : body) {
		if (part.x == head.x && part.y == head.y) {
			return true;
		}
	}
	
	return false;
}
