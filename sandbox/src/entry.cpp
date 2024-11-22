#include <zfw2/game.h>
#include "test_scene.h"

int main()
{
    zfw2::Game game("Sandbox");
    game.run(zfw2::create_scene_factory<TestScene>());
}
