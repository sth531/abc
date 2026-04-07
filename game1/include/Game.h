#pragma once

#include "Common.h"
#include "Snake.h"
#include "Food.h"
#include "Input.h"
#include "SaveSystem.h"
#include "Renderer.h"
#include <string>
#include <chrono>

class Game {
public:
	Game(int width = 20, int height = 20);
	~Game();
	
	// 运行游戏主循环
	void run();
	
private:
	static constexpr int DEFAULT_WIDTH = 20;
	static constexpr int DEFAULT_HEIGHT = 20;
	static const std::string DEFAULT_SAVE_FILE;
	static const std::chrono::milliseconds RENDER_FRAME_TIME;
	static const std::chrono::milliseconds BASE_MOVE_INTERVAL;
	static const std::chrono::milliseconds MIN_MOVE_INTERVAL;
	static constexpr int SCORE_PER_DIFFICULTY = 20;
	static constexpr int MOVE_INTERVAL_DECREMENT = 10;
	
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
	std::chrono::milliseconds accumulated_update_time;
	std::chrono::milliseconds current_move_interval;
	
	// 游戏逻辑方法
	void handle_input();
	void handle_start_state(int key);
	void handle_running_state(int key);
	void handle_paused_state(int key);
	void handle_ended_state(int key);
	
	void update_game_logic();
	void update_speed();
	void reset_game();
	void render_frame();
	int get_difficulty() const;
	
	std::string get_info_bar() const;
	std::string get_status_title() const;
};
