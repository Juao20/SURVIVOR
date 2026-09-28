#include "include/Game.hpp"
#include <cstdlib>
#include <ctime>

int main()
{
    srand((unsigned int)time(nullptr));

#if defined(PLATFORM_ANDROID)
    // 0,0 tells raylib to use the device's native full-screen resolution
    // instead of a fixed size that would get letterboxed on the real display.
    Game game(0, 0, "Survivor — Top Down Shooter");
#else
    Game game(1280, 720, "Survivor — Top Down Shooter");
#endif
    game.init();
    game.run();

    return 0;
}