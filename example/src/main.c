#include <stdio.h>
#include <signal.h>

#include "game.h"

// generated assets can be found in "assets/." in the build directory (included at compile time)
// #include "assets/sprites/sprite.h"
// #include "assets/midi/song.h"

static void sigint_handler(int sg) {
    game_stop_loop();
}

int main(void) {
    signal(SIGINT, sigint_handler);

    if (!pal_init()) {
        printf("PAL failed to initialize.\n");
        return 1;
    }

    printf("PAL initialized successfully!\n");

    // infinite while loop
    game_run_loop();

    return 0;
}