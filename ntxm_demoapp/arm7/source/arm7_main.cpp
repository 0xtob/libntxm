#include <nds.h>
#include <stdlib.h>

#include <ntxm/ntxm7.h>

NTXM7 *ntxm7 = 0;

int vcount;
touchPosition first,tempPos;

//---------------------------------------------------------------------------------
void VcountHandler() {
//---------------------------------------------------------------------------------
    inputGetAndSend();
}

void ntxmTimerHandler(void)
{
	ntxm7->timerHandler();
}

volatile bool exitflag = false;

//---------------------------------------------------------------------------------
void powerButtonHandler() {
//---------------------------------------------------------------------------------
	exitflag = true;
}

//---------------------------------------------------------------------------------
void enableSound() {
//---------------------------------------------------------------------------------
    powerOn(POWER_SOUND);
    writePowerManagement(PM_CONTROL_REG, ( readPowerManagement(PM_CONTROL_REG) & ~PM_SOUND_MUTE ) | PM_SOUND_AMP );
    REG_SOUNDCNT = SOUND_ENABLE;
    REG_MASTER_VOLUME = 127;
}

//---------------------------------------------------------------------------------
int main(int argc, char ** argv) {
//---------------------------------------------------------------------------------
	readUserSettings();

	irqInit();
	// Start the RTC tracking IRQ
	initClockIRQ();
	
	fifoInit();
	installSystemFIFO();

	enableSound();
	
	SetYtrigger(80);
	irqSet(IRQ_VCOUNT, VcountHandler);
	irqSetAUX(IRQ_POWER, powerButtonHandler);
	irqEnableAUX(IRQ_POWER);
	irqEnable(IRQ_VBLANK);
	irqEnable(IRQ_VCOUNT);
	
	// Initialize NTXM update timer
	irqSet(IRQ_TIMER0, ntxmTimerHandler);
	irqEnable(IRQ_TIMER0);
	


	// Create ntxm player
	ntxm7 = new NTXM7(ntxmTimerHandler);
	
	// Keep the ARM7 idle
	while (!exitflag)
	{
		if ( 0 == (REG_KEYINPUT & (KEY_SELECT | KEY_START | KEY_L | KEY_R))) {
			exitflag = true;
		}
		swiWaitForVBlank();
	}
	return 0;
}
