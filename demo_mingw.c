#ifdef __WINDOWS__
#include <windows.h>
#endif

#include "types_plus.h"
#include "MTF_io.h"
#include "music_play.h"

int main(int argc, char *argv[])
{
    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_VIDEO))
    {
        printf("Could not initialize SDL - %s\n", SDL_GetError());
        return -1;
    }

    printf("GOGOGO \n");
    MP3WAVplay("./aiwer_smile.wav");

    SDL_Quit();

#ifdef __WINDOWS__
    system("pause");
#endif

    return 0;
}
