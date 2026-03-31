#include "../include/Game.h"
#include <string>
#include <thread>

const std::string Game::DEFAULT_SAVE_FILE = "snake_save.txt";
const std::chrono::milliseconds Game::FRAME_TIME(120);

Game::Game(int width, int height)
	: width(width), height(height),
	  snake(width, height),
	  food(width, height),
	  save_system(DEFAULT_SAVE_FILE),
	  renderer(width, height),
	  state(GameState::Start),
	  current_direction(Direction::Right),
	  next_direction(Direction::Right),
	  score(0),
	  should_exit(false),
	  rng(static_cast<unsigned int>(GetTickCount())),
	  dist_x(0, width - 1),
	  dist_y(0, height - 1) {
	
	renderer.initialize();
	food.spawn(snake);
}

Game::~Game() {
}

void Game::run() {
	while (!should_exit) {
		auto frame_start = std::chrono::steady_clock::now();

		handle_input();
		update_game_logic();
		render_frame();

		auto elapsed = std::chrono::steady_clock::now() - frame_start;
		if (elapsed < FRAME_TIME) {
			std::this_thread::sleep_for(FRAME_TIME - elapsed);
		}
	}
}

void Game::handle_input() {
	while (input.has_input()) {
		int key = input.get_key();

		switch (state) {
			case GameState::Start:
				handle_start_state(key);
				break;
			case GameState::Running:
				handle_running_state(key);
				break;
			case GameState::Paused:
				handle_paused_state(key);
				break;
			case GameState::Ended:
				handle_ended_state(key);
				break;
		}
	}
}

void Game::handle_start_state(int key) {
	if (key == 'n' || key == 'N') {
		reset_game();
		state = GameState::Running;
	} else if (key == 'l' || key == 'L') {
		std::deque<Point> loaded_snake;
		Direction loaded_dir;
		Point loaded_food;
		int loaded_score;

		if (save_system.load_game(loaded_snake, loaded_dir, loaded_food, 
								  loaded_score, width, height)) {
			snake.set_body(loaded_snake);
			current_direction = loaded_dir;
			next_direction = loaded_dir;
			food.set_position(loaded_food);
			score = loaded_score;
			state = GameState::Running;
		}
	} else if (key == 'q' || key == 'Q' || key == 27) {
		should_exit = true;
	}
}

void Game::handle_running_state(int key) {
	if (key == 'p' || key == 'P') {
		state = GameState::Paused;
	} else if (key == 'x' || key == 'X' || key == 27) {
		state = GameState::Ended;
	} else if (key == 'k' || key == 'K') {
		save_system.save_game(snake, current_direction, food.get_position(), 
							  score, width, height);
	} else if (input.is_direction_key(key)) {
		Direction candidate = input.key_to_direction(key, current_direction);
		if (!input.is_opposite(current_direction, candidate)) {
			next_direction = candidate;
		}
	}
}

void Game::handle_paused_state(int key) {
	if (key == 'p' || key == 'P') {
		state = GameState::Running;
	} else if (key == 'k' || key == 'K') {
		save_system.save_game(snake, current_direction, food.get_position(), 
							  score, width, height);
	} else if (key == 'x' || key == 'X' || key == 27) {
		state = GameState::Ended;
	}
}

void Game::handle_ended_state(int key) {
	if (key == 'r' || key == 'R' || key == 'n' || key == 'N') {
		reset_game();
		state = GameState::Running;
	} else if (key == 'l' || key == 'L') {
		std::deque<Point> loaded_snake;
		Direction loaded_dir;
		Point loaded_food;
		int loaded_score;

		if (save_system.load_game(loaded_snake, loaded_dir, loaded_food, 
								  loaded_score, width, height)) {
			snake.set_body(loaded_snake);
			current_direction = loaded_dir;
			next_direction = loaded_dir;
			food.set_position(loaded_food);
			score = loaded_score;
			state = GameState::Running;
		}
	} else if (key == 'q' || key == 'Q' || key == 27) {
		should_exit = true;
	}
}

void Game::update_game_logic() {
	if (state != GameState::Running) {
		return;
	}

	current_direction = next_direction;
	
	// 先保存当前蛇头位置以判断是否吃到食物
	Point old_head = snake.get_head();
	Point food_pos = food.get_position();
	
	// 尝试移动蛇
	bool move_success = snake.move(current_direction, food_pos);
	
	if (!move_success) {
		// 碰撞发生，游戏结束
		state = GameState::Ended;
	} else {
		// 检查是否吃到了食物（移动后蛇的长度是否增加）
		Point new_head = snake.get_head();
		if (new_head.x == food_pos.x && new_head.y == food_pos.y) {
			// 吃到了食物
			++score;
			food.spawn(snake);
		}
	}
}

void Game::reset_game() {
	snake.reset();
	current_direction = Direction::Right;
	next_direction = Direction::Right;
	food.spawn(snake);
	score = 0;
}

void Game::render_frame() {
	renderer.clear();
	renderer.draw_border();
	renderer.draw_food(food.get_position());
	renderer.draw_snake(snake);
	
	renderer.draw_info_bar(get_info_bar());
	
	if (state == GameState::Start || state == GameState::Paused || state == GameState::Ended) {
		renderer.draw_menu_title(get_status_title());
		renderer.draw_score(score);
	}
	
	renderer.flush();
}

std::string Game::get_info_bar() const {
	if (state == GameState::Start) {
		return "START  N:new  L:load  Q:quit";
	} else if (state == GameState::Running) {
		return "Score: " + std::to_string(score) + "  WASD  P:pause  K:save  X:end";
	} else if (state == GameState::Paused) {
		return "PAUSED  P:resume  K:save  X:end";
	} else {
		return "ENDED  R:new  L:load  Q:quit";
	}
}

std::string Game::get_status_title() const {
	if (state == GameState::Start) {
		return "SNAKE";
	} else if (state == GameState::Paused) {
		return "PAUSED";
	} else {
		return "GAME OVER";
	}
}
