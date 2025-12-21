#pragma once

#if defined(VULKRAFT_WINMAIN)
#    include <cstdlib>
#    include <windows.h>
int32_t main(int32_t argc, char** argv);
int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow)
{
    main(__argc, __argv);
}
#endif
