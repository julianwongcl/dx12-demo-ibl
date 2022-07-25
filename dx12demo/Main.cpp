#include <Windows.h>
#include "Dx12Demo.h"
#include "Win32Application.h"

_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    Dx12Demo demo;
    return Win32Application::Run(&demo, hInstance, nCmdShow);
}
