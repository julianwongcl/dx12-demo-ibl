#pragma once

#include <vector>
#include <Windows.h>

class Graphics;
class Inputs;

class Dx12Demo
{
public:
    Dx12Demo();
    ~Dx12Demo();

    void Init(HWND hwnd, LONG width, LONG height);
    void HandleInputMessage(UINT message, WPARAM wParam, LPARAM lParam);
    void ResetInputs();
    void OnPaint();

private:
    double m_lastFrameTime = 0.016; // in milliseconds

    Graphics* m_graphics = nullptr;
    Inputs* m_inputs = nullptr;
};
