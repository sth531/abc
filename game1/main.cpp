#include <conio.h>
#include <windows.h>

#include <algorithm>
#include <chrono>
#include <deque>
#include <fstream>
#include <random>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

struct Point {
	int x;
	int y;
};

enum class Direction {
	Up,
	Down,
	Left,
	Right
};

enum class GameState {
	Start,
	Running,
	Paused,
	Ended
};

bool is_opposite(Direction a, Direction b) {
	return (a == Direction::Up && b == Direction::Down) ||
		   (a == Direction::Down && b == Direction::Up) ||
		   (a == Direction::Left && b == Direction::Right) ||
		   (a == Direction::Right && b == Direction::Left);
}

bool is_direction_key(int key) {
	return key == 'w' || key == 'W' ||
		   key == 's' || key == 'S' ||
		   key == 'a' || key == 'A' ||
		   key == 'd' || key == 'D';
}

Direction key_to_direction(int key, Direction fallback) {
	if (key == 'w' || key == 'W') {
		return Direction::Up;
	}
	if (key == 's' || key == 'S') {
		return Direction::Down;
	}
	if (key == 'a' || key == 'A') {
		return Direction::Left;
	}
	if (key == 'd' || key == 'D') {
		return Direction::Right;
	}
	return fallback;
}

bool save_game(const std::string& path,
			   const std::deque<Point>& snake,
			   Direction dir,
			   const Point& food,
			   int score,
			   int width,
			   int height) {
	std::ofstream out(path, std::ios::trunc);
	if (!out) {
		return false;
	}

	out << "SNAKE_SAVE_V1\n";
	out << "width " << width << "\n";
	out << "height " << height << "\n";
	out << "score " << score << "\n";
	out << "dir " << static_cast<int>(dir) << "\n";
	out << "food " << food.x << ' ' << food.y << "\n";
	out << "length " << snake.size() << "\n";
	for (const auto& part : snake) {
		out << part.x << ' ' << part.y << "\n";
	}

	return out.good();
}

bool load_game(const std::string& path,
			  std::deque<Point>& snake,
			  Direction& dir,
			  Point& food,
			  int& score,
			  int width,
			  int height) {
	std::ifstream in(path);
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
	dir = static_cast<Direction>(file_dir);
	score = file_score;
	return true;
}

void hide_cursor(HANDLE handle) {
	CONSOLE_CURSOR_INFO cursor_info{};
	cursor_info.dwSize = 1;
	cursor_info.bVisible = FALSE;
	SetConsoleCursorInfo(handle, &cursor_info);
}

int main() {
	constexpr int kWidth = 30;
	constexpr int kHeight = 20;
	constexpr int kFrameCols = kWidth + 2;
	constexpr int kFrameRows = kHeight + 3;
	const std::string kSaveFile = "snake_save.txt";

	HANDLE default_out = GetStdHandle(STD_OUTPUT_HANDLE);
	HANDLE buffers[2] = {
		CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE,
								  FILE_SHARE_READ | FILE_SHARE_WRITE,
								  nullptr,
								  CONSOLE_TEXTMODE_BUFFER,
								  nullptr),
		CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE,
								  FILE_SHARE_READ | FILE_SHARE_WRITE,
								  nullptr,
								  CONSOLE_TEXTMODE_BUFFER,
								  nullptr)
	};

	if (buffers[0] == INVALID_HANDLE_VALUE || buffers[1] == INVALID_HANDLE_VALUE) {
		return 1;
	}

	COORD size{static_cast<SHORT>(kFrameCols), static_cast<SHORT>(kFrameRows)};
	SMALL_RECT window{0, 0, static_cast<SHORT>(kFrameCols - 1), static_cast<SHORT>(kFrameRows - 1)};

	for (HANDLE buffer : buffers) {
		SetConsoleScreenBufferSize(buffer, size);
		SetConsoleWindowInfo(buffer, TRUE, &window);
		hide_cursor(buffer);
	}

	int front = 0;
	int back = 1;
	SetConsoleActiveScreenBuffer(buffers[front]);

	std::deque<Point> snake{{kWidth / 2, kHeight / 2}, {kWidth / 2 - 1, kHeight / 2}, {kWidth / 2 - 2, kHeight / 2}};
	Direction dir = Direction::Right;
	Direction next_dir = dir;
	GameState state = GameState::Start;

	std::mt19937 rng(static_cast<unsigned int>(GetTickCount()));
	std::uniform_int_distribution<int> dist_x(0, kWidth - 1);
	std::uniform_int_distribution<int> dist_y(0, kHeight - 1);

	auto spawn_food = [&]() {
		Point food{};
		while (true) {
			food = {dist_x(rng), dist_y(rng)};
			bool on_snake = std::any_of(snake.begin(), snake.end(), [&](const Point& p) {
				return p.x == food.x && p.y == food.y;
			});
			if (!on_snake) {
				return food;
			}
		}
	};

	Point food = spawn_food();
	int score = 0;
	bool should_exit = false;
	std::string status_line = "N: New Game  L: Load Save  Q: Quit";

	auto reset_game = [&]() {
		snake = {{kWidth / 2, kHeight / 2}, {kWidth / 2 - 1, kHeight / 2}, {kWidth / 2 - 2, kHeight / 2}};
		dir = Direction::Right;
		next_dir = dir;
		food = spawn_food();
		score = 0;
	};

	const auto frame_time = std::chrono::milliseconds(120);

	while (!should_exit) {
		auto frame_start = std::chrono::steady_clock::now();

		while (_kbhit()) {
			int key = _getch();

			if (state == GameState::Start) {
				if (key == 'n' || key == 'N') {
					reset_game();
					state = GameState::Running;
					status_line = "Game started";
				} else if (key == 'l' || key == 'L') {
					if (load_game(kSaveFile, snake, dir, food, score, kWidth, kHeight)) {
						next_dir = dir;
						state = GameState::Running;
						status_line = "Loaded save file";
					} else {
						status_line = "Load failed: save missing or invalid";
					}
				} else if (key == 'q' || key == 'Q' || key == 27) {
					should_exit = true;
				}
				continue;
			}

			if (state == GameState::Running) {
				if (key == 'p' || key == 'P') {
					state = GameState::Paused;
					status_line = "Paused";
					continue;
				}
				if (key == 'x' || key == 'X' || key == 27) {
					state = GameState::Ended;
					status_line = "Ended by user";
					continue;
				}
				if (key == 'k' || key == 'K') {
					status_line = save_game(kSaveFile, snake, dir, food, score, kWidth, kHeight)
						? "Saved to snake_save.txt"
						: "Save failed";
					continue;
				}
				if (is_direction_key(key)) {
					Direction candidate = key_to_direction(key, dir);
					if (!is_opposite(dir, candidate)) {
						next_dir = candidate;
					}
				}
				continue;
			}

			if (state == GameState::Paused) {
				if (key == 'p' || key == 'P') {
					state = GameState::Running;
					status_line = "Resumed";
				} else if (key == 'k' || key == 'K') {
					status_line = save_game(kSaveFile, snake, dir, food, score, kWidth, kHeight)
						? "Saved to snake_save.txt"
						: "Save failed";
				} else if (key == 'x' || key == 'X' || key == 27) {
					state = GameState::Ended;
					status_line = "Ended by user";
				}
				continue;
			}

			if (state == GameState::Ended) {
				if (key == 'r' || key == 'R' || key == 'n' || key == 'N') {
					reset_game();
					state = GameState::Running;
					status_line = "New game started";
				} else if (key == 'l' || key == 'L') {
					if (load_game(kSaveFile, snake, dir, food, score, kWidth, kHeight)) {
						next_dir = dir;
						state = GameState::Running;
						status_line = "Loaded save file";
					} else {
						status_line = "Load failed: save missing or invalid";
					}
				} else if (key == 'q' || key == 'Q' || key == 27) {
					should_exit = true;
				}
			}
		}

		if (state == GameState::Running) {
			dir = next_dir;
			Point head = snake.front();
			if (dir == Direction::Up) {
				--head.y;
			} else if (dir == Direction::Down) {
				++head.y;
			} else if (dir == Direction::Left) {
				--head.x;
			} else {
				++head.x;
			}

			bool hit = head.x < 0 || head.x >= kWidth || head.y < 0 || head.y >= kHeight;

			if (!hit) {
				for (const auto& part : snake) {
					if (part.x == head.x && part.y == head.y) {
						hit = true;
						break;
					}
				}
			}

			if (!hit) {
				snake.push_front(head);
				if (head.x == food.x && head.y == food.y) {
					++score;
					food = spawn_food();
				} else {
					snake.pop_back();
				}
			} else {
				state = GameState::Ended;
				status_line = "Game over";
			}
		}

		std::vector<CHAR_INFO> frame(kFrameCols * kFrameRows);
		for (auto& cell : frame) {
			cell.Char.AsciiChar = ' ';
			cell.Attributes = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
		}

		auto put = [&](int x, int y, char ch, WORD attr) {
			if (x < 0 || x >= kFrameCols || y < 0 || y >= kFrameRows) {
				return;
			}
			auto& cell = frame[y * kFrameCols + x];
			cell.Char.AsciiChar = ch;
			cell.Attributes = attr;
		};

		for (int x = 0; x < kFrameCols; ++x) {
			put(x, 0, '#', FOREGROUND_GREEN | FOREGROUND_INTENSITY);
			put(x, kHeight + 1, '#', FOREGROUND_GREEN | FOREGROUND_INTENSITY);
		}
		for (int y = 0; y <= kHeight + 1; ++y) {
			put(0, y, '#', FOREGROUND_GREEN | FOREGROUND_INTENSITY);
			put(kWidth + 1, y, '#', FOREGROUND_GREEN | FOREGROUND_INTENSITY);
		}

		put(food.x + 1, food.y + 1, '*', FOREGROUND_RED | FOREGROUND_INTENSITY);

		bool first = true;
		for (const auto& part : snake) {
			char body = first ? '@' : 'o';
			WORD attr = first ? (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY)
							  : (FOREGROUND_GREEN | FOREGROUND_INTENSITY);
			put(part.x + 1, part.y + 1, body, attr);
			first = false;
		}

		std::string info;
		if (state == GameState::Start) {
			info = "START  N:new  L:load  Q:quit";
		} else if (state == GameState::Running) {
			info = "Score: " + std::to_string(score) + "  WASD  P:pause  K:save  X:end";
		} else if (state == GameState::Paused) {
			info = "PAUSED  P:resume  K:save  X:end";
		} else {
			info = "ENDED  R:new  L:load  Q:quit";
		}
		for (int i = 0; i < static_cast<int>(info.size()) && i < kFrameCols; ++i) {
			put(i, kHeight + 2, info[i], FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
		}

		if (!status_line.empty()) {
			for (int i = 0; i < static_cast<int>(status_line.size()) && i < kFrameCols; ++i) {
				put(i, 0, status_line[i], FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
			}
		}

		if (state == GameState::Start || state == GameState::Paused || state == GameState::Ended) {
			std::string title;
			if (state == GameState::Start) {
				title = "SNAKE";
			} else if (state == GameState::Paused) {
				title = "PAUSED";
			} else {
				title = "GAME OVER";
			}
			int title_x = (kFrameCols - static_cast<int>(title.size())) / 2;
			for (int i = 0; i < static_cast<int>(title.size()); ++i) {
				put(title_x + i, kFrameRows / 2 - 1, title[i], FOREGROUND_RED | FOREGROUND_INTENSITY);
			}

			std::string score_text = "Score: " + std::to_string(score);
			int score_x = (kFrameCols - static_cast<int>(score_text.size())) / 2;
			for (int i = 0; i < static_cast<int>(score_text.size()); ++i) {
				put(score_x + i, kFrameRows / 2 + 1, score_text[i], FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
			}
		}

		COORD buffer_size{static_cast<SHORT>(kFrameCols), static_cast<SHORT>(kFrameRows)};
		COORD buffer_coord{0, 0};
		SMALL_RECT write_rect{0, 0, static_cast<SHORT>(kFrameCols - 1), static_cast<SHORT>(kFrameRows - 1)};
		WriteConsoleOutputA(buffers[back], frame.data(), buffer_size, buffer_coord, &write_rect);

		SetConsoleActiveScreenBuffer(buffers[back]);
		std::swap(front, back);

		auto elapsed = std::chrono::steady_clock::now() - frame_start;
		if (elapsed < frame_time) {
			std::this_thread::sleep_for(frame_time - elapsed);
		}
	}

	SetConsoleActiveScreenBuffer(default_out);
	CloseHandle(buffers[0]);
	CloseHandle(buffers[1]);

	return 0;
}
