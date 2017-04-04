/*
 * Copyright (c) 2017 Georges Troulis
 *
 * MenuManager.h
 *
 * Handles displaying the main game menu and all settings submenus for games
 *
*/

#ifndef _AGENDA_MENUMANAGER_H
#define _AGENDA_MENUMANAGER_H

#include "HardwareMap.h"
#include "Game.h"

class MenuManager
{
    private:
        HardwareMap* hw;
        Game** gameList;
        int prevEncoderPinAState; // For detecting the direction of rotation from the rotary encoder

    public:
        MenuManager(HardwareMap* _hw);

        void initGameList(Game** _gameList);
        int displayMainMenu();
};

#endif