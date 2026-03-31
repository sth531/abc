#pragma once

#include "Common.h"
#include "Snake.h"

class Food {
public:
	Food(int width, int height);
	
	// 生成新的食物位置（不与蛇重叠）
	void spawn(const Snake& snake);
	
	// 获取食物位置
	Point get_position() const { return position; }
	
	// 设置食物位置（用于加载存档）
	void set_position(Point new_position) { position = new_position; }
	
private:
	Point position;
	int width;
	int height;
};
