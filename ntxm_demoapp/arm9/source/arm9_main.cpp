#include <nds.h>
#include <nds/arm9/console.h> //basic print funcionality
#include <stdio.h>

#include <ntxm/ntxm9.h>

NTXM9 *ntxm9 = 0;

int main(void)
{
	powerON(POWER_ALL);
	irqInit();
	
	irqEnable(IRQ_VBLANK);
	
	videoSetMode(0);
	videoSetModeSub(MODE_0_2D | DISPLAY_BG0_ACTIVE);
	vramSetBankC(VRAM_C_SUB_BG);
	
	SUB_BG0_CR = BG_MAP_BASE(31);
	
	BG_PALETTE_SUB[255] = RGB15(31,31,31);
	
	consoleInitDefault((u16*)SCREEN_BASE_BLOCK_SUB(31), (u16*)CHAR_BASE_BLOCK_SUB(0), 16);
	
	iprintf("Demo XM player!\n\nLoading song...\n");
	
	ntxm9 = new NTXM9();
	u16 err = ntxm9->load("test.xm");
	
	if(err != 0)
	{
		iprintf(ntxm9->getError(err));
		while(1); // Stop execution
	}
	else
		iprintf("Song loaded successfuly.\nPress A to play\nPress B to stop.\n\n");
		
	while(1)
	{
		scanKeys();
		u16 keys = keysDown();
		
		if(keys & KEY_A)
		{
			iprintf("Playing.\n");
			ntxm9->play(true);
		}
		else if(keys & KEY_B)
		{
			iprintf("Stopped.\n");
			ntxm9->stop();
		}
		
		swiWaitForVBlank();
	}
	
	return 0;
}
