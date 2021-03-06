/*
 * Copyright (c) 2017 Georges Troulis
 *
 * CSGOGame.h
 *
 * Declares behavior for CSGO Game Mode
*/

#ifndef _AGENDA_CSGOGAME_H
#define _AGENDA_CSGOGAME_H

#include "HardwareMap.h"
#include "Game.h"

#define CSGO_NUM_SETTINGS 3

class CSGOGame : public Game
{
    public:
        CSGOGame(int CSGO_ID);

        // Inherited virtual methods from Game
        void init();
        bool isPlaying();
        void doGameLoop();
        void doEndGame();

        GameOption* getGameOptions();

    private:
        LiquidCrystal* lcd;         // Points to the lcd stored in HardwareMap for convenience

        // Game Options
        GameOption gameSettings[CSGO_NUM_SETTINGS];

        int armTimeOpts[5]  = {2, 3, 5, 10, 0};
        int defTimeOpts[5]  = {2, 3, 5, 10, 0};
        int boomTimeOpts[6] = {35, 45, 60, 90, 120, 0};

        // Game Option Variables
        int armTime;          // Time it takes to arm the bomb (seconds)
        int defuseTime;       // Time it takes to defuse the bomb (seconds)
        String password;      // The password that shows up on the screen while the bomb is arming

        // Game State Variables
        int timeUntilBoom;          // Time since armed until the bomb explodes (seconds)
        int prevButtonState;        // For arming/defusing: keeps track of if "button state" changes
        bool gameOver;              // Becomes true when the bomb is defused
        bool arming;                // True only while arming/defusing the bomb
        bool armed;

        unsigned long timeSinceArmStart;   // Time since we started arming (for arm/defuse countdown)
        unsigned long timeSpentArming;     // Time spent arming the bomb so far (for arm/defuse countdown)
        unsigned long timeArmComplete;     // Time since bomb was armed (for count down)

        // Gameplay Methods
        void updateDisplay(unsigned long globalTime);   // Updates the lcd with important game state stuff
        void updateArmStatus(unsigned long globalTime); // Handles real-time arming/defusing of the bomb
        void countDown(unsigned long globalTime);       // Counts down the bomb if it is armed
        String formatTime();                            // Formats the bomb time into mm:ss string
};

#endif
