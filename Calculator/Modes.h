#pragma once

#include <windows.h>

void EnterBasicMode(HWND mainWindow);
void EnterProgrammerMode(HWND mainWindow);
bool HandleBasicChar(wchar_t key);
bool HandleProgrammerChar(wchar_t key);
