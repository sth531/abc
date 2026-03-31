#pragma once

#include <windows.h>
#include <vector>
#include <string>
#include "Common.h"
#include "Snake.h"

class Renderer {
public:
	Renderer(int width, int height);
	~Renderer();
	
	// 初始化渲染器
	void initialize();
	
	// 清空帧缓冲
	void clear();
	
	// 绘制蛇
	void draw_snake(const Snake& snake);
	
	// 绘制食物
	void draw_food(const Point& food);
	
	// 绘制边框
	void draw_border();
	
	// 绘制状态信息栏
	void draw_info_bar(const std::string& info);
	
	// 绘制标题和分数（用于菜单）
	void draw_menu_title(const std::string& title);
	void draw_score(int score);
	
	// 绘制状态行
	void draw_status_line(const std::string& status);
	
	// 交换缓冲区并显示
	void flush();
	
private:
	int width;
	int height;
	int frame_cols;
	int frame_rows;
	
	HANDLE default_out;
	HANDLE buffers[2];
	int front;
	int back;
	std::vector<CHAR_INFO> frame;
	
	// 在指定位置放置字符
	void put(int x, int y, char ch, WORD attr);
};
