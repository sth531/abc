#include "../include/Input.h"
#include <conio.h>
#include <vector>

Input::Input() {
}

std::vector<int> Input::poll_keys() {
	std::vector<int> keys;
	while (has_input()) {
		keys.push_back(get_key());
	}
	return keys;
}

bool Input::has_input() const {
	return _kbhit() != 0;
}

int Input::get_key() {
	return _getch();
}

bool Input::is_direction_key(int key) {
	return key == 'w' || key == 'W' ||
		   key == 's' || key == 'S' ||
		   key == 'a' || key == 'A' ||
		   key == 'd' || key == 'D';
}

Direction Input::key_to_direction(int key, Direction fallback) {
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

bool Input::is_opposite(Direction a, Direction b) {
	return (a == Direction::Up && b == Direction::Down) ||
		   (a == Direction::Down && b == Direction::Up) ||
		   (a == Direction::Left && b == Direction::Right) ||
		   (a == Direction::Right && b == Direction::Left);
}
