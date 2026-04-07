#include "../include/Game.h"
#include <string>
#include <thread>

const std::string Game::DEFAULT_SAVE_FILE = "snake_save.txt";
const std::chrono::milliseconds Game::RENDER_FRAME_TIME(16);
const std::chrono::milliseconds Game::BASE_MOVE_INTERVAL(120);
const std::chrono::milliseconds Game::MIN_MOVE_INTERVAL(60);

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
	  accumulated_update_time(std::chrono::milliseconds::zero()),
	  current_move_interval(BASE_MOVE_INTERVAL) {
	
	renderer.initialize();
	food.spawn(snake);
}

Game::~Game() {
}

void Game::run() {
	auto previous_time = std::chrono::steady_clock::now();

	while (!should_exit) {
		auto frame_start = std::chrono::steady_clock::now();
		auto frame_delta = std::chrono::duration_cast<std::chrono::milliseconds>(
			frame_start - previous_time);
		previous_time = frame_start;

		if (frame_delta > std::chrono::milliseconds(250)) {
			frame_delta = std::chrono::milliseconds(250);
		}
		accumulated_update_time += frame_delta;

		handle_input();
		update_game_logic();
		render_frame();

		auto elapsed = std::chrono::steady_clock::now() - frame_start;
		if (elapsed < RENDER_FRAME_TIME) {
			std::this_thread::sleep_for(RENDER_FRAME_TIME - elapsed);
		}
	}
}

void Game::handle_input() {
	const std::vector<int> keys = input.poll_keys();
	if (keys.empty()) {
		return;
	}

	if (state == GameState::Running) {
		Direction resolved_direction = current_direction;

		for (int key : keys) {
			handle_running_state(key);

			if (state != GameState::Running) {
				break;
			}

			if (input.is_direction_key(key)) {
				Direction candidate = input.key_to_direction(key, current_direction);
				if (!input.is_opposite(candidate, current_direction)) {
					resolved_direction = candidate;
				}
			}
		}

		if (state == GameState::Running) {
			next_direction = resolved_direction;
		}
		return;
	}

	for (int key : keys) {
		switch (state) {
			case GameState::Start:
				handle_start_state(key);
				break;
			case GameState::Paused:
				handle_paused_state(key);
				break;
			case GameState::Ended:
				handle_ended_state(key);
				break;
			case GameState::Running:
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
			accumulated_update_time = std::chrono::milliseconds::zero();
			update_speed();
			state = GameState::Running;
		}
	} else if (key == 'q' || key == 'Q' || key == 27) {
		should_exit = true;
	}
}

void Game::handle_running_state(int key) {
	if (key == 'p' || key == 'P') {
		state = GameState::Paused;
		accumulated_update_time = std::chrono::milliseconds::zero();
	} else if (key == 'x' || key == 'X' || key == 27) {
		state = GameState::Ended;
		accumulated_update_time = std::chrono::milliseconds::zero();
	} else if (key == 'k' || key == 'K') {
		save_system.save_game(snake, current_direction, food.get_position(), 
							  score, width, height);
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
			accumulated_update_time = std::chrono::milliseconds::zero();
			update_speed();
			state = GameState::Running;
		}
	} else if (key == 'q' || key == 'Q' || key == 27) {
		should_exit = true;
	}
}

void Game::update_game_logic() {
	if (state != GameState::Running) {
		accumulated_update_time = std::chrono::milliseconds::zero();
		return;
	}

	while (accumulated_update_time >= current_move_interval) {
		current_direction = next_direction;
		Point food_pos = food.get_position();

		bool move_success = snake.move(current_direction, food_pos);
		accumulated_update_time -= current_move_interval;

		if (!move_success) {
			state = GameState::Ended;
			accumulated_update_time = std::chrono::milliseconds::zero();
			break;
		}

		Point new_head = snake.get_head();
		if (new_head.x == food_pos.x && new_head.y == food_pos.y) {
			score += 10;
			food.spawn(snake);
			update_speed();
		}
	}
}

void Game::update_speed() {
	const int difficulty = get_difficulty();
	const int speedup = (difficulty - 1) * MOVE_INTERVAL_DECREMENT;
	const int interval = static_cast<int>(BASE_MOVE_INTERVAL.count()) - speedup;
	const int clamped_interval = (interval < static_cast<int>(MIN_MOVE_INTERVAL.count()))
		? static_cast<int>(MIN_MOVE_INTERVAL.count())
		: interval;

	current_move_interval = std::chrono::milliseconds(clamped_interval);
}

int Game::get_difficulty() const {
	return (score / SCORE_PER_DIFFICULTY) + 1;
}

void Game::reset_game() {
	snake.reset();
	current_direction = Direction::Right;
	next_direction = Direction::Right;
	food.spawn(snake);
	score = 0;
	accumulated_update_time = std::chrono::milliseconds::zero();
	update_speed();
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
	const std::string score_text = "Score: " + std::to_string(score);
	const std::string difficulty_text = "Difficulty: " + std::to_string(get_difficulty());

	if (state == GameState::Start) {
		return score_text +
			"\n" + difficulty_text +
			"\nFood: +10" +
			"\nN: new game" +
			"\nL: load" +
			"\nQ: quit";
	} else if (state == GameState::Running) {
		return score_text +
			"\n" + difficulty_text +
			"\nFood: +10" +
			"\nWASD: move" +
			"\nP: pause" +
			"\nK: save" +
			"\nX: end";
	} else if (state == GameState::Paused) {
		return score_text +
			"\n" + difficulty_text +
			"\nPAUSED" +
			"\nP: resume" +
			"\nK: save" +
			"\nX: end";
	} else {
		return score_text +
			"\n" + difficulty_text +
			"\nENDED" +
			"\nR: new game" +
			"\nL: load" +
			"\nQ: quit";
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
