#include "Inputs.h"


Inputs::Inputs()
{
    ZeroMemory(m_keys, 128);
    ZeroMemory(m_keysLast, 128);
}

Inputs::~Inputs(void)
{
}

void Inputs::Reset()
{
    memcpy_s(m_keysLast, 128, m_keys, 128);
    mouseLDownLast = mouseLDown;
    mouseRDownLast = mouseRDown;
    mouseXLast = mouseX;
    mouseYLast = mouseY;
    mouseWheelChanged = false;
}

void Inputs::ProcessMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_KEYDOWN)
    {
        m_keys[wParam] = true;
    }
    if (message == WM_KEYUP)
    {
        m_keys[wParam] = false;
    }
    if (message == WM_LBUTTONDOWN)
    {
        this->mouseLDown = true;
    }
    if (message == WM_RBUTTONDOWN)
    {
        this->mouseRDown = true;
    }
    if (message == WM_LBUTTONUP)
    {
        this->mouseLDown = false;
    }
    if (message == WM_RBUTTONUP)
    {
        this->mouseRDown = false;
    }
    if (message == WM_MOUSEMOVE)
    {
        mouseX = GET_X_LPARAM(lParam);
        mouseY = GET_Y_LPARAM(lParam);
    }
    if (message == WM_MOUSEWHEEL)
    {
        mousewheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);
        mouseWheelChanged = true;
    }
}

bool Inputs::IsPressed(uint8_t key) const
{
    if (key == Inputs::MB_MOUSE_LEFT)
    {
        return mouseLDown && !mouseLDownLast;
    }
    if (key == Inputs::MB_MOUSE_RIGHT)
    {
        return mouseRDown && !mouseRDownLast;
    }

    return m_keys[key] && !m_keysLast[key];
}

bool Inputs::IsDown(uint8_t key) const
{
    if (key == Inputs::MB_MOUSE_LEFT)
    {
        return mouseLDown;
    }
    if (key == Inputs::MB_MOUSE_RIGHT)
    {
        return mouseRDown;
    }

    return m_keys[key];
}

bool Inputs::IsDragging(uint8_t mb) const
{
    if (mb == Inputs::MB_MOUSE_LEFT)
    {
        return mouseLDown && mouseLDownLast
            && (mouseX != mouseXLast
                || mouseY != mouseYLast);
    }
    // else MB_MOUSE_RIGHT
    return mouseRDown && mouseRDownLast
        && (mouseX != mouseXLast
            || mouseY != mouseYLast);
}

bool Inputs::MouseWheelChanged() const
{
    return mouseWheelChanged;
}
