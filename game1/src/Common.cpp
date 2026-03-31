#include "../include/Common.h"

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
