#pragma once
#include "Common.h"

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK DlgProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

void UpdateSettings(WPARAM, HMENU);
void SetupMenu(HWND);
