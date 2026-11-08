#pragma once
#include <windows.h>
// This first probe deliberately displays only bundled, trusted test content.
bool OpenPreviewProbe(HINSTANCE module, HWND owner = nullptr, bool quitOnClose = false);
void ClosePreviewProbes();
