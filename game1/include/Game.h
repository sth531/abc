#pragma once

#include "Common.h"
#include "Snake.h"
#include "Food.h"
#include "Input.h"
#include "SaveSystem.h"
#include "Renderer.h"
#include <string>
#include <random>
#include <chrono>

class Game {
public:
	Game(int width = 30, int height = 20);
	~Game();
	
	// 运行游戏主循环
	void run();
	
private:
	static constexpr int DEFAULT_WIDTH = 30;
	static constexpr int DEFAULT_HEIGHT = 20;
	static const std::string DEFAULT_SAVE_FILE;
	static const std::chrono::milliseconds FRAME_TIME;
	
	int width;
	int height;
	
	// 游戏对象
	Snake snake;
	Food food;
	Input input;
	SaveSystem save_system;
	Renderer renderer;
	
	// 游戏状态
	GameState state;
	Direction current_direction;
	Direction next_direction;
	int score;
	bool should_exit;
	std::mt19937 rng;
	std::uniform_int_distribution<int> dist_x;
	std::uniform_int_distribution<int> dist_y;
	
	// 游戏逻辑方法
	void handle_input();
	void handle_start_state(int key);
	void handle_running_state(int key);
	void handle_paused_state(int key);
	void handle_ended_state(int key);
	
	void update_game_logic();
	void reset_game();
	void render_frame();
	
	std::string get_info_bar() const;
	std::string get_status_title() const;
};
