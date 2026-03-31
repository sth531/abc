#include "../include/Renderer.h"
#include <algorithm>
#include <string>

Renderer::Renderer(int width, int height)
	: width(width), height(height),
	frame_cols(width + 2),
	frame_rows(height + 3),
	default_out(GetStdHandle(STD_OUTPUT_HANDLE)),
	front(0), back(1) {
	
	buffers[0] = INVALID_HANDLE_VALUE;
	buffers[1] = INVALID_HANDLE_VALUE;
	frame.clear();
}

Renderer::~Renderer() {
	if (buffers[0] != INVALID_HANDLE_VALUE) {
		CloseHandle(buffers[0]);
	}
	if (buffers[1] != INVALID_HANDLE_VALUE) {
		CloseHandle(buffers[1]);
	}
	SetConsoleActiveScreenBuffer(default_out);
}

void Renderer::initialize() {
	buffers[0] = CreateConsoleScreenBuffer(
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		nullptr,
		CONSOLE_TEXTMODE_BUFFER,
		nullptr);
	
	buffers[1] = CreateConsoleScreenBuffer(
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		nullptr,
		CONSOLE_TEXTMODE_BUFFER,
		nullptr);

	if (buffers[0] == INVALID_HANDLE_VALUE || buffers[1] == INVALID_HANDLE_VALUE) {
		return;
	}

	COORD size{static_cast<SHORT>(frame_cols), static_cast<SHORT>(frame_rows)};
	SMALL_RECT window{0, 0, static_cast<SHORT>(frame_cols - 1), static_cast<SHORT>(frame_rows - 1)};

	for (HANDLE buffer : buffers) {
		SetConsoleScreenBufferSize(buffer, size);
		SetConsoleWindowInfo(buffer, TRUE, &window);
		
		// 隐藏光标
		CONSOLE_CURSOR_INFO cursor_info{};
		cursor_info.dwSize = 1;
		cursor_info.bVisible = FALSE;
		SetConsoleCursorInfo(buffer, &cursor_info);
	}

	SetConsoleActiveScreenBuffer(buffers[front]);
}

void Renderer::clear() {
	frame.clear();
	frame.resize(frame_cols * frame_rows);
	for (auto& cell : frame) {
		cell.Char.AsciiChar = ' ';
		cell.Attributes = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
	}
}

void Renderer::put(int x, int y, char ch, WORD attr) {
	if (x < 0 || x >= frame_cols || y < 0 || y >= frame_rows) {
		return;
	}
	auto& cell = frame[y * frame_cols + x];
	cell.Char.AsciiChar = ch;
	cell.Attributes = attr;
}

void Renderer::draw_border() {
	WORD color = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
	
	// 上下边框
	for (int x = 0; x < frame_cols; ++x) {
		put(x, 0, '#', color);
		put(x, height + 1, '#', color);
	}
	
	// 左右边框
	for (int y = 0; y <= height + 1; ++y) {
		put(0, y, '#', color);
		put(width + 1, y, '#', color);
	}
}

void Renderer::draw_snake(const Snake& snake) {
	bool first = true;
	for (const auto& part : snake.get_body()) {
		char body = first ? '@' : 'o';
		WORD attr = first ? (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY)
						  : (FOREGROUND_GREEN | FOREGROUND_INTENSITY);
		put(part.x + 1, part.y + 1, body, attr);
		first = false;
	}
}

void Renderer::draw_food(const Point& food) {
	put(food.x + 1, food.y + 1, '*', FOREGROUND_RED | FOREGROUND_INTENSITY);
}

void Renderer::draw_info_bar(const std::string& info) {
	for (int i = 0; i < static_cast<int>(info.size()) && i < frame_cols; ++i) {
		put(i, height + 2, info[i], 
			FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
	}
}

void Renderer::draw_status_line(const std::string& status) {
	if (!status.empty()) {
		for (int i = 0; i < static_cast<int>(status.size()) && i < frame_cols; ++i) {
			put(i, 0, status[i],
				FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
		}
	}
}

void Renderer::draw_menu_title(const std::string& title) {
	int title_x = (frame_cols - static_cast<int>(title.size())) / 2;
	for (int i = 0; i < static_cast<int>(title.size()); ++i) {
		put(title_x + i, frame_rows / 2 - 1, title[i],
			FOREGROUND_RED | FOREGROUND_INTENSITY);
	}
}

void Renderer::draw_score(int score) {
	std::string score_text = "Score: " + std::to_string(score);
	int score_x = (frame_cols - static_cast<int>(score_text.size())) / 2;
	for (int i = 0; i < static_cast<int>(score_text.size()); ++i) {
		put(score_x + i, frame_rows / 2 + 1, score_text[i],
			FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
	}
}

void Renderer::flush() {
	COORD buffer_size{static_cast<SHORT>(frame_cols), static_cast<SHORT>(frame_rows)};
	COORD buffer_coord{0, 0};
	SMALL_RECT write_rect{0, 0, static_cast<SHORT>(frame_cols - 1), static_cast<SHORT>(frame_rows - 1)};
	WriteConsoleOutputA(buffers[back], frame.data(), buffer_size, buffer_coord, &write_rect);

	SetConsoleActiveScreenBuffer(buffers[back]);
	std::swap(front, back);
}
