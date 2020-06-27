#include <nds.h>
#include <stdlib.h>

#include <ntxm/ntxm7.h>

static volatile bool exitFlag = false;
NTXM7 *ntxm7 = 0;

void VcountHandler(void)
{
	inputGetAndSend();
}

void ntxmTimerHandler(void)
{
	ntxm7->timerHandler();
}

void powerButtonHandler(void)
{
	exitFlag = true;
}

int main()
{
	dmaFillWords(0, (void*) 0x04000400, 0x100);

	REG_SOUNDCNT |= SOUND_ENABLE;
	writePowerManagement(PM_CONTROL_REG, ( readPowerManagement(PM_CONTROL_REG) & ~PM_SOUND_MUTE ) | PM_SOUND_AMP );
	powerOn(POWER_SOUND);

	REG_SOUNDCNT = SOUND_ENABLE | SOUND_VOL(0x7F);

	readUserSettings();

	irqInit();
	initClockIRQ();
	fifoInit();
	touchInit();

	SetYtrigger(80);

	installSoundFIFO();
	installSystemFIFO();

	irqSet(IRQ_VCOUNT, VcountHandler);
	
	// Initialize NTXM update timer
	irqSet(IRQ_TIMER0, ntxmTimerHandler);
	irqEnable(IRQ_VCOUNT | IRQ_VBLANK | IRQ_TIMER0);
	
	// Create ntxm player
	ntxm7 = new NTXM7(FIFO_USER_01, ntxmTimerHandler);
	
	setPowerButtonCB(powerButtonHandler);

	// Keep the ARM7 idle
	while (!exitFlag)
	{
		ntxm7->updateCommands();
		swiIntrWait(1, IRQ_FIFO_NOT_EMPTY | IRQ_VBLANK);
	}

	return 0;
}
