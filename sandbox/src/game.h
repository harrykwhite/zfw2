#pragma once

#include <zfw2/game.h>

class SandboxGame : public zfw2::Game
{
public:
    SandboxGame(const std::string &windowTitle);

protected:
    void on_init() override;
    void on_tick() override;
};
