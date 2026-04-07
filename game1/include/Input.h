#pragma once

#include "Common.h"
#include <vector>

class Input {
public:
	Input();
	
	// 读取当前帧所有输入（按采样顺序）
	std::vector<int> poll_keys();
	
	// 检查是否有按键输入
	bool has_input() const;
	
	// 获取下一个按键
	int get_key();
	
	// 检查是否是方向键
	static bool is_direction_key(int key);
	
	// 将键转换为方向
	static Direction key_to_direction(int key, Direction fallback);
	
	// 检查两个方向是否相反
	static bool is_opposite(Direction a, Direction b);
};
