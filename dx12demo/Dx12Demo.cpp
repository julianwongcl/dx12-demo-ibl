#include "Dx12Demo.h"

#include "Graphics.h"
#include "Inputs.h"

#include <Windows.h>
#include <ctime>

Dx12Demo::Dx12Demo()
{
}

void Dx12Demo::Init(HWND hwnd, LONG width, LONG height)
{
	m_inputs = new Inputs();
	m_graphics = new Graphics(hwnd, m_inputs, width, height);
}

Dx12Demo::~Dx12Demo()
{
}

void Dx12Demo::OnPaint()
{
	LARGE_INTEGER StartingTime;
	QueryPerformanceCounter(&StartingTime);

	m_graphics->Update(m_lastFrameTime);

	LARGE_INTEGER EndingTime;
	QueryPerformanceCounter(&EndingTime);

	LARGE_INTEGER ElapsedMicroseconds;
	ElapsedMicroseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;
	ElapsedMicroseconds.QuadPart *= 1000000;

	LARGE_INTEGER Frequency;
	QueryPerformanceFrequency(&Frequency);
	ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;
	m_lastFrameTime = ElapsedMicroseconds.QuadPart / (1000.0 * 1000.0);
}

void Dx12Demo::HandleInputMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
	m_inputs->ProcessMessage(message, wParam, lParam);
}

void Dx12Demo::ResetInputs()
{
	m_inputs->Reset();
}
