#pragma once

#include <windows.h>
#include "DataTypes.h"

LRESULT CALLBACK DisplayWndProc(HWND displayWindow, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK BitDisplayWndProc(HWND bitWindow, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK MainWndProc(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam);

extern const wchar_t MAIN_CLASS_NAME[];
extern const wchar_t DISPLAY_CLASS_NAME[];
extern const wchar_t BIT_DISPLAY_CLASS_NAME[];


constexpr int MIN_CLIENT_WIDTH = 320;
constexpr int MIN_CLIENT_HEIGHT = 400;
constexpr int UI_PADDING = 5;
