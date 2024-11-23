#include <zfw2/game.h>
#include "main_menu_scene.h"

int main()
{
    zfw2::run_game(zfw2::create_scene_factory<MainMenuScene>());
}
