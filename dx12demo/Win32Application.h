#pragma once

#include <windows.h>

class Dx12Demo;
class Inputs;
class Graphics;

class Win32Application
{
public:
    static int Run(Dx12Demo* demo, HINSTANCE hInstance, int nCmdShow);
    static HWND GetHwnd() { return m_windowHandle; }

protected:
    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
    static HWND m_windowHandle;
    static const LONG m_width = 1000;
    static const LONG m_height = 1000;
};
