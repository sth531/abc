#pragma once

#include "Common.h"
#include <deque>

class Snake {
public:
	Snake(int width, int height);
	
	// 初始化蛇到中心位置
	void reset();
	
	// 移动蛇（返回是否吃到食物）
	bool move(Direction direction, const Point& food);
	
	// 检查碰撞
	bool check_collision(const Point& head);
	
	// 获取蛇的身体
	const std::deque<Point>& get_body() const { return body; }
	
	// 获取蛇头
	Point get_head() const { return body.front(); }
	
	// 获取蛇的长度
	size_t get_length() const { return body.size(); }
	
	// 设置蛇的身体（用于加载存档）
	void set_body(const std::deque<Point>& new_body) { body = new_body; }
	
private:
	std::deque<Point> body;
	int width;
	int height;
};
