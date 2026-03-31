#pragma once

#include "Common.h"
#include "Snake.h"
#include <string>
#include <deque>

class SaveSystem {
public:
	SaveSystem(const std::string& save_file_path = "snake_save.txt");
	
	// 保存游戏进度
	bool save_game(const Snake& snake,
				   Direction current_direction,
				   const Point& food,
				   int score,
				   int width,
				   int height);
	
	// 加载游戏进度
	bool load_game(std::deque<Point>& snake,
				   Direction& direction,
				   Point& food,
				   int& score,
				   int width,
				   int height);
	
	// 获取保存文件路径
	const std::string& get_save_path() const { return save_file_path; }
	
private:
	std::string save_file_path;
};
