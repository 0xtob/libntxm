/*
 * libNTXM - XM Player Library for the Nintendo DS
 *
 *    Copyright (C) 2005-2008 Tobias Weyand (0xtob)
 *                         me@nitrotracker.tobw.net
 *
 */

/***** BEGIN LICENSE BLOCK *****
 * 
 * Version: Noncommercial zLib License / GPL 3.0
 * 
 * The contents of this file are subject to the Noncommercial zLib License 
 * (the "License"); you may not use this file except in compliance with
 * the License. You should have recieved a copy of the license with this package.
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either of the GNU General Public License Version 3 or later (the "GPL"),
 * in which case the provisions of the GPL are applicable instead of those above.
 * If you wish to allow use of your version of this file only under the terms of
 * either the GPL, and not to allow others to use your version of this file under
 * the terms of the Noncommercial zLib License, indicate your decision by
 * deleting the provisions above and replace them with the notice and other
 * provisions required by the GPL. If you do not delete the provisions above,
 * a recipient may use your version of this file under the terms of any one of
 * the GPL or the Noncommercial zLib License.
 * 
 ***** END LICENSE BLOCK *****/

/*
  Functions for the ARM7 to process the commands from the ARM9.Based
  on code from the MOD player example posted to the GBADEV forums.
  Chris Double (chris.double@double.co.nz)
  http://www.double.co.nz/nintendo_ds
*/
#include <nds.h>
#include <stdarg.h>

#include "ntxm/command.h"
#include "ntxm/linear_freq_table.h"

extern "C" {
  #include "xtoa.h"
  #include "ntxm/tobmic.h"
}

#include "ntxm/player.h"
#include "ntxm/ntxm7.h"

extern NTXM7 *ntxm7;
bool ntxm_recording = false;

static void RecvCommandPlaySample(PlaySampleCommand *ps)
{
	ntxm7->playSample(ps->sample, ps->note, ps->volume, ps->channel);
}

static void RecvCommandStopSample(StopSampleSoundCommand* ss) {
	ntxm7->stopChannel(ss->channel);
}

static void RecvCommandMicOn(void)
{
	tob_MIC_On();
}

static void RecvCommandMicOff(void)
{
	tob_MIC_Off();
}

static void RecvCommandStartRecording(StartRecordingCommand* sr)
{
	ntxm_recording = true;
	tob_StartRecording(sr->buffer, sr->length);
	commandControl->return_data = 0;
}

static void RecvCommandStopRecording()
{
	commandControl->return_data = tob_StopRecording();
	ntxm_recording = false;
}

static void RecvCommandSetSong(SetSongCommand *c) {
	ntxm7->setSong((Song*)c->ptr);
}

static void RecvCommandStartPlay(StartPlayCommand *c) {
	ntxm7->play(c->loop, c->potpos, c->row);
}

static void RecvCommandStopPlay(StopPlayCommand *c) {
	ntxm7->stop();
}

static void RecvCommandPlayInst(PlayInstCommand *c) {
	ntxm7->playNote(c->inst, c->note, c->volume, c->channel);
}

static void RecvCommandStopInst(StopInstCommand *c) {
	ntxm7->stopChannel(c->channel);
}

static void RecvCommandPatternLoop(PatternLoopCommand *c) {
	ntxm7->setPatternLoop(c->state);
}

void CommandDbgOut(const char *formatstr, ...)
{
	Command* command = &commandControl->command[commandControl->currentCommand];
	command->destination = DST_ARM9;
	command->commandType = DBG_OUT;
	
	DbgOutCommand *cmd = &command->dbgOut;
	
	va_list marker;
	va_start(marker, formatstr);
	
	char *debugstr = cmd->msg;
	
	for(u16 i=0;i<DEBUGSTRSIZE; ++i)
		debugstr[i] = 0;
	
	u16 strpos = 0;
	char *outptr = debugstr;
	char c;
	while((strpos < DEBUGSTRSIZE-1)&&(formatstr[strpos]!=0))
	{
		c=formatstr[strpos];
		
		if(c!='%') {
			*outptr = c;
			outptr++;
		} else {
			strpos++;
			c=formatstr[strpos];
			if(c=='d') {
				long l = va_arg(marker, long);
				outptr = ltoa(l, outptr, 10);
			} else if(c=='u'){
				unsigned long ul = va_arg(marker, unsigned long);
				outptr = ultoa(ul, outptr, 10);
			}
		}
		
		strpos++;
	}
	
	va_end(marker);
	
	commandControl->currentCommand++;
	commandControl->currentCommand %= MAX_COMMANDS;
}

void CommandUpdateRow(u16 row)
{
	Command* command = &commandControl->command[commandControl->currentCommand];
	command->destination = DST_ARM9;
	command->commandType = UPDATE_ROW;
	
	UpdateRowCommand *c = &command->updateRow;
	c->row = row;
	
	commandControl->currentCommand++;
	commandControl->currentCommand %= MAX_COMMANDS;
}

void CommandUpdatePotPos(u16 potpos)
{
	Command* command = &commandControl->command[commandControl->currentCommand];
	command->destination = DST_ARM9;
	command->commandType = UPDATE_POTPOS;
	
	UpdatePotPosCommand *c = &command->updatePotPos;
	c->potpos = potpos;
	
	commandControl->currentCommand++;
	commandControl->currentCommand %= MAX_COMMANDS;
}

void CommandNotifyStop(void)
{
	Command* command = &commandControl->command[commandControl->currentCommand];
	command->destination = DST_ARM9;
	command->commandType = NOTIFY_STOP;
	
	commandControl->currentCommand++;
	commandControl->currentCommand %= MAX_COMMANDS;
}

void CommandSampleFinish(void)
{
	Command* command = &commandControl->command[commandControl->currentCommand];
	command->destination = DST_ARM9;
	command->commandType = SAMPLE_FINISH;
	
	commandControl->currentCommand++;
	commandControl->currentCommand %= MAX_COMMANDS;
}

void CommandProcessCommands(void)
{
	static int currentCommand = 0;
	while(currentCommand != commandControl->currentCommand) {
		Command* command = &commandControl->command[currentCommand];
		
		if(command->destination == DST_ARM7) {
		
			switch(command->commandType) {
				case PLAY_SAMPLE:
					RecvCommandPlaySample(&command->playSample);
					break;
				case STOP_SAMPLE:
					RecvCommandStopSample(&command->stopSample);
					break;
				case START_RECORDING:
					RecvCommandStartRecording(&command->startRecording);
					break;
				case STOP_RECORDING:
					RecvCommandStopRecording();
					break;
				case SET_SONG:
					RecvCommandSetSong(&command->setSong);
					break;
				case START_PLAY:
					RecvCommandStartPlay(&command->startPlay);
					break;
				case STOP_PLAY:
					RecvCommandStopPlay(&command->stopPlay);
					break;
				case PLAY_INST:
					RecvCommandPlayInst(&command->playInst);
					break;
				case STOP_INST:
					RecvCommandStopInst(&command->stopInst);
					break;
				case MIC_ON:
					RecvCommandMicOn();
					break;
				case MIC_OFF:
					RecvCommandMicOff();
					break;
				case PATTERN_LOOP:
					RecvCommandPatternLoop(&command->ptnLoop);
					break;
				default:
					break;
			}
		
		}
		currentCommand++;
		currentCommand %= MAX_COMMANDS;
	}
}
