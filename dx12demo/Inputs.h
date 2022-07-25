#pragma once

#include <vector>
#include <iostream>
#include <Windows.h>
// mouse param retrieval
#include <windowsx.h>

class Inputs
{
public:
	enum MOUSEBUTTON
	{
		MB_MOUSE_LEFT = 129,
		MB_MOUSE_RIGHT
	};

	Inputs();
	~Inputs(void);

	void Reset();
	void ProcessMessage(UINT message, WPARAM wParam, LPARAM lParam);

	bool IsPressed(uint8_t key) const;
	bool IsDown(uint8_t key) const;
	bool IsDragging(uint8_t mb) const;

	bool MouseWheelChanged() const;

	bool mouseLDown = false;
	bool mouseLDownLast = false;
	bool mouseRDown = false;
	bool mouseRDownLast = false;

	int64_t mouseX = 0;
	int64_t mouseXLast = 0;
	int64_t mouseY = 0;
	int64_t mouseYLast = 0;

	int64_t mousewheelDelta = 0;
	bool mouseWheelChanged = false;

private:
	bool m_keys[128]{};
	bool m_keysLast[128]{};
};
