#ifndef SUPPLY_H_
#define SUPPLY_H_

class Supply
{
public:
	static void StartSleepingMode(bool forced);
    static void AwakeFromSleepingMode();
	
    static bool SleepingModeIsActive();
    static bool SleepingModeIsForced();

    static void CheckSleepingModeButton();

private:
    static bool sleepingMode;
    static bool forcedSleepingMode;

    static unsigned char forceSleepButtonState;
    static unsigned char forceSleepButtonLastState;
    static unsigned int forceSleepButtonLastPressTime;
    static unsigned int forceSleepButtonLastReleaseTime;
    static bool forceSleepButtonFired;
};

#endif
