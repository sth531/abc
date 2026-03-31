#include "../include/SaveSystem.h"
#include <fstream>
#include <algorithm>

SaveSystem::SaveSystem(const std::string& save_file_path) 
	: save_file_path(save_file_path) {
}

bool SaveSystem::save_game(const Snake& snake,
							Direction current_direction,
							const Point& food,
							int score,
							int width,
							int height) {
	std::ofstream out(save_file_path, std::ios::trunc);
	if (!out) {
		return false;
	}

	out << "SNAKE_SAVE_V1\n";
	out << "width " << width << "\n";
	out << "height " << height << "\n";
	out << "score " << score << "\n";
	out << "dir " << static_cast<int>(current_direction) << "\n";
	out << "food " << food.x << ' ' << food.y << "\n";
	out << "length " << snake.get_length() << "\n";
	
	for (const auto& part : snake.get_body()) {
		out << part.x << ' ' << part.y << "\n";
	}

	return out.good();
}

bool SaveSystem::load_game(std::deque<Point>& snake,
							Direction& direction,
							Point& food,
							int& score,
							int width,
							int height) {
	std::ifstream in(save_file_path);
	if (!in) {
		return false;
	}

	std::string magic;
	if (!std::getline(in, magic) || magic != "SNAKE_SAVE_V1") {
		return false;
	}

	auto read_key_value_int = [&](const std::string& expected_key, int& value) {
		std::string key;
		if (!(in >> key >> value) || key != expected_key) {
			return false;
		}
		return true;
	};

	int file_width = 0;
	int file_height = 0;
	if (!read_key_value_int("width", file_width) || !read_key_value_int("height", file_height)) {
		return false;
	}
	if (file_width != width || file_height != height) {
		return false;
	}

	int file_score = 0;
	int file_dir = 0;
	if (!read_key_value_int("score", file_score) || !read_key_value_int("dir", file_dir)) {
		return false;
	}
	if (file_dir < static_cast<int>(Direction::Up) || file_dir > static_cast<int>(Direction::Right)) {
		return false;
	}

	std::string food_key;
	if (!(in >> food_key >> food.x >> food.y) || food_key != "food") {
		return false;
	}

	std::string length_key;
	int length = 0;
	if (!(in >> length_key >> length) || length_key != "length" || length <= 0) {
		return false;
	}

	std::deque<Point> loaded_snake;
	loaded_snake.clear();
	for (int i = 0; i < length; ++i) {
		Point p{};
		if (!(in >> p.x >> p.y)) {
			return false;
		}
		loaded_snake.push_back(p);
	}

	auto in_bounds = [&](const Point& p) {
		return p.x >= 0 && p.x < width && p.y >= 0 && p.y < height;
	};

	if (!in_bounds(food)) {
		return false;
	}

	for (const auto& part : loaded_snake) {
		if (!in_bounds(part)) {
			return false;
		}
	}

	snake = loaded_snake;
	direction = static_cast<Direction>(file_dir);
	score = file_score;
	return true;
}
