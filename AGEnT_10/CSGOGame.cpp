/*
 * Copyright (c) 2017 Georges Troulis
 *
 * CSGOGame.cpp
 *
 * Defines behavior for CSGO Game Mode
 *
 * Game Mode Summary:
 * The Terrorists have a plantable Bomb in their possession (this device) when the game starts, and can carry it with them.
 * The terrorists must plant this bomb at some designated bomb site, arm it, and have it count down and explode to win.
 * The Conuter Terrorists must either eliminate the entire Terrorist team before a bomb is planted, or defuse the bomb
 * to win. To arm or defuse the bomb, both RED and BLU buttons must be held for some amount of time
 *
 * Controls:
 * -> BLU Button: nothing by itself
 * -> RED Button: nothing by itself
 * -> BLU and RED Buttons together: Hold to Arm the bomb (if not armed) or to defuse the bomb (if armed)
 * -> Encoder Button: Pause the game (TODO WIP)
 *
 * Tweakable Game Settings:
 *  -> Arm Time: The amount of time that the buttons must be held down by for the bomb to become armed
 *  -> Defuse Time: The amount of time that the buttons must be held down by for the bomb to become defused
 *  -> TimeTilBoom: The amount of time that it takes for the bomb to explode once armed
 *
*/

#include <Arduino.h>

#include "CSGOGame.h"
#include "Game.h"
#include "HardwareMap.h"
#include "TimeManager.h"

CSGOGame::CSGOGame(int csgoID)
    : Game("CS:GO Competitive", "CS:GO", csgoID, CSGO_NUM_SETTINGS),
      armTime(5),
      defuseTime(7),
      password("7355608"),
      timeUntilBoom(35),
      prevButtonState(0),
      gameOver(false),
      arming(false),
      armed(false),
      timeSinceArmStart(0L),
      timeSpentArming(0L),
      timeArmComplete(0L)
{
    // Initialize Tweakable Game Settings
    GameOption armTimeOpt  = {"Arm Time",       &armTimeOpts[0],  &armTime};
    GameOption defTimeOpt  = {"Defuse Time",    &defTimeOpts[0],  &defuseTime};
    GameOption boomTimeOpt = {"TimeTillBoom",   &boomTimeOpts[0], &timeUntilBoom};

    gameSettings[0] = armTimeOpt;
    gameSettings[1] = defTimeOpt;
    gameSettings[2] = boomTimeOpt;
}

////////////////////////////////////////////////////////////
// Public interface methods
////////////////////////////////////////////////////////////

void CSGOGame::init()
{
    lcd = HardwareMap::getLCD();
}

bool CSGOGame::isPlaying()
{
    return !gameOver;
}

void CSGOGame::doGameLoop()
{
    unsigned long globalTime = TimeManager::getTime();
    updateDisplay(globalTime);
    updateArmStatus(globalTime);
    countDown(globalTime);
}

void CSGOGame::doEndGame()
{
    lcd->clear();

    // Bomb was defused
    if (!armed)
    {
        lcd->print("Bomb Defused");
        lcd->setCursor(0, 1);
        lcd->print("Counter Terrorists");
        lcd->setCursor(0, 2);
        lcd->print("win");

        digitalWrite(HardwareMap::ledBLU, HIGH);
        digitalWrite(HardwareMap::ledRED, LOW);
    }
    else
    {
        lcd->print("Bomb Blew Up");
        lcd->setCursor(0, 1);
        lcd->print("Terrorists win");

        digitalWrite(HardwareMap::ledBLU, LOW);
        digitalWrite(HardwareMap::ledRED, HIGH);
    }
}

GameOption* CSGOGame::getGameOptions()
{
    return gameSettings;
}

////////////////////////////////////////////////////////////
// Private Gameplay methods
////////////////////////////////////////////////////////////

void CSGOGame::updateDisplay(unsigned long globalTime)
{
    lcd->clear();

    // Phase 1: Waiting for bomb to become armed
    if (!arming)
    {
        lcd->setCursor(7, 1);
        lcd->print(formatTime());

        // Blink the lights if the bomb is armed
        if (armed)
        {
            int blinkPeriod;
            if (timeUntilBoom >= 30)         blinkPeriod = 1000;
            else if (timeUntilBoom >= 10)    blinkPeriod =  500;
            else if (timeUntilBoom >=  5)    blinkPeriod =  250;
            else                             blinkPeriod =  150;

            int brightness = 1 + sin((globalTime - timeArmComplete) * 2.0 * PI / blinkPeriod);
            digitalWrite(HardwareMap::ledRED, brightness);
            digitalWrite(HardwareMap::ledBLU, LOW);
        }
    }
    else if (arming)
    {
        // Displays the status of arming/disarming
        int armProgress = 0;

        // Display status message
        if (!armed)
        {
            lcd->setCursor(5, 1);
            lcd->print("Arming...");

            armProgress = ((password.length())/((double) armTime * 1000)) * timeSpentArming + 1;
            lcd->setCursor(5, 2);
            lcd->print("|");
            for(int i = 0; i < armProgress; i++)
                lcd->print(password[i]);
            for(int i = 0; i < 7 - armProgress; i++)
                lcd->print("*");
            lcd->print("|");

        }
        else
        {
            lcd->setCursor(4, 0);
            lcd->print("Defusing...");

            lcd->setCursor(7, 1);
            lcd->print(formatTime());

            armProgress = ((password.length())/((double) defuseTime * 1000)) * timeSpentArming;
            lcd->setCursor(5, 2);
            lcd->print("|");
            for(int i = 0; i < armProgress; i++)
                lcd->print("#");
            for(int i = 0; i < 7 - armProgress; i++)
                lcd->print("*");
            lcd->print("|");


            // Blink the Blue LEDs to indicate defusing
            int brightness = 1 + sin((globalTime - timeArmComplete) * 2.0 * PI / 500);
            digitalWrite(HardwareMap::ledBLU, brightness);
            digitalWrite(HardwareMap::ledRED, LOW);
        }
    }
}

void CSGOGame::updateArmStatus(unsigned long globalTime)
{
    int currentButtonState = digitalRead(HardwareMap::buttonRED) & digitalRead(HardwareMap::buttonBLU);

    // Commence the arming/defusing process if it's the first time pushing the button OR if we've already started
    if (currentButtonState == HIGH && (arming || prevButtonState == LOW))
    {
        arming = true;

        // Update the timeSinceArmStart only in the beginning
        if (prevButtonState == LOW && currentButtonState == HIGH)
            timeSinceArmStart = globalTime;

        timeSpentArming = globalTime - timeSinceArmStart;

        if (!armed && (timeSpentArming > armTime * 1000))
        {
            armed  = true;
            arming = false;

            timeSpentArming      = 0;
            timeArmComplete = globalTime;
        }
        else if (armed && (timeSpentArming > defuseTime * 1000))
        {
            arming = false;
            armed  = false;
            gameOver = true;
        }
    }
    else
    {
        arming = false;
    }

    prevButtonState = currentButtonState;
}

void CSGOGame::countDown(unsigned long globalTime)
{
    if (!armed) return;
    if (timeUntilBoom <= 0) gameOver = true;

    unsigned long timeSinceArmComplete = globalTime - timeArmComplete;
    if (timeSinceArmComplete > 1000)
    {
        timeArmComplete += 1000;
        timeUntilBoom --;
    }

}

// Returns the bomb time formatted as a mm:ss string
String CSGOGame::formatTime()
{
    String result = "";
    int min = timeUntilBoom / 60;
    int sec = timeUntilBoom % 60;

    // add the minutes, and a leading 0 if necessary
    if(min < 10)
        result += '0';
    result += min;

    result += ':';

    // add the timeUntilBoom, and a leading 0 if necessary
    if(sec < 10)
        result += '0';
    result += sec;

    return result;
}
