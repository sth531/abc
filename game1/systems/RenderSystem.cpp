#include "../include/Renderer.h"
#include <algorithm>
#include <string>

namespace {
void set_square_console_font(HANDLE buffer) {
	CONSOLE_FONT_INFOEX font_info{};
	font_info.cbSize = sizeof(CONSOLE_FONT_INFOEX);
	font_info.nFont = 0;
	font_info.dwFontSize.X = 16;
	font_info.dwFontSize.Y = 16;
	font_info.FontFamily = FF_DONTCARE;
	font_info.FontWeight = FW_NORMAL;
	wcscpy_s(font_info.FaceName, L"Consolas");
	SetCurrentConsoleFontEx(buffer, FALSE, &font_info);
}
}

Renderer::Renderer(int width, int height)
	: width(width), height(height),
	playfield_cols((width + 2) * 2),
	sidebar_cols(28),
	frame_cols(playfield_cols + sidebar_cols),
	frame_rows(height + 2),
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
		set_square_console_font(buffer);
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
		cell.Char.UnicodeChar = L' ';
		cell.Attributes = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
	}
}

void Renderer::put(int x, int y, wchar_t ch, WORD attr) {
	if (x < 0 || x >= frame_cols || y < 0 || y >= frame_rows) {
		return;
	}
	auto& cell = frame[y * frame_cols + x];
	cell.Char.UnicodeChar = ch;
	cell.Attributes = attr;
}

void Renderer::put_cell(int logical_x, int y, wchar_t ch, WORD attr) {
	const int x = logical_x * 2;
	put(x, y, ch, attr);
	put(x + 1, y, L' ', attr);
}

void Renderer::draw_border() {
	WORD color = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
	constexpr wchar_t kTopLeft = L'┌';
	constexpr wchar_t kTopRight = L'┐';
	constexpr wchar_t kBottomLeft = L'└';
	constexpr wchar_t kBottomRight = L'┘';
	constexpr wchar_t kHorizontal = L'—';
	constexpr wchar_t kVertical = L'│';

	for (int x = 1; x < playfield_cols - 1; ++x) {
		put(x, 0, kHorizontal, color);
		put(x, height + 1, kHorizontal, color);
	}

	for (int y = 1; y < height + 1; ++y) {
		put(0, y, kVertical, color);
		put(playfield_cols - 1, y, kVertical, color);
	}

	put(0, 0, kTopLeft, color);
	put(playfield_cols - 1, 0, kTopRight, color);
	put(0, height + 1, kBottomLeft, color);
	put(playfield_cols - 1, height + 1, kBottomRight, color);
}

void Renderer::draw_snake(const Snake& snake) {
	bool first = true;
	for (const auto& part : snake.get_body()) {
		wchar_t body = first ? L'■' : L'□';
		WORD attr = first ? (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY)
						  : (FOREGROUND_GREEN | FOREGROUND_INTENSITY);
		put_cell(part.x + 1, part.y + 1, body, attr);
		first = false;
	}
}

void Renderer::draw_food(const Point& food) {
	put_cell(food.x + 1, food.y + 1, L'◆', FOREGROUND_RED | FOREGROUND_INTENSITY);
}

void Renderer::draw_info_bar(const std::string& info) {
	const int panel_x = playfield_cols + 2;
	const int panel_width = frame_cols - panel_x;
	const int panel_y_start = 1;

	int line_y = panel_y_start;
	int char_x = panel_x;
	for (char ch : info) {
		if (ch == '\n') {
			++line_y;
			char_x = panel_x;
			if (line_y >= frame_rows) {
				break;
			}
			continue;
		}

		if (char_x - panel_x >= panel_width) {
			++line_y;
			char_x = panel_x;
		}

		if (line_y >= frame_rows) {
			break;
		}

		put(char_x, line_y, ch, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
		++char_x;
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
	WriteConsoleOutputW(buffers[back], frame.data(), buffer_size, buffer_coord, &write_rect);

	SetConsoleActiveScreenBuffer(buffers[back]);
	std::swap(front, back);
}
