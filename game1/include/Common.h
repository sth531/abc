#pragma once

#include <windows.h>

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

// 枚举和方向相关的实用函数
bool is_opposite(Direction a, Direction b);
bool is_direction_key(int key);
Direction key_to_direction(int key, Direction fallback);
