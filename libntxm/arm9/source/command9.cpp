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
  Structures and functions to allow the ARM9 to send commands to the
  ARM7. Based on code from the MOD player example posted to the GBADEV
  forums.
  Chris Double (chris.double@double.co.nz)
  http://www.double.co.nz/nintendo_ds
*/
#include <nds.h>
#include <string.h>
#include <stdio.h>

#include "ntxm/command.h"
#include "ntxm/song.h"

static int fifoChannel;

void (*onUpdateRow)(u16 row) = 0;
void (*onStop)(void) = 0;
void (*onPlaySampleFinished)(void) = 0;
void (*onPotPosChange)(u16 potpos) = 0;

void RegisterRowCallback(void (*onUpdateRow_)(u16))
{
	onUpdateRow = onUpdateRow_;
}

void RegisterStopCallback(void (*onStop_)(void))
{
	onStop = onStop_;
}

void RegisterPlaySampleFinishedCallback(void (*onPlaySampleFinished_)(void))
{
	onPlaySampleFinished = onPlaySampleFinished_;
}

void RegisterPotPosChangeCallback(void (*onPotPosChange_)(u16))
{
	onPotPosChange = onPotPosChange_;
}

void CommandInit(int channel)
{
	fifoChannel = channel;
}

static inline void CommandSend(Command* cmd, size_t len)
{
	fifoSendDatamsg(fifoChannel, len + 4, (u8*) cmd);
}

static inline int CommandReceive(void)
{
	while (!fifoCheckValue32(fifoChannel))
		swiIntrWait(1, IRQ_FIFO_NOT_EMPTY);
	return (int) fifoGetValue32(fifoChannel);
}

void CommandPlaySample(Sample *sample, u8 note, u8 volume, u8 channel)
{
	Command command;
	PlaySampleCommand* ps = &command.playSample;
	
	command.commandType = PLAY_SAMPLE;
	
	ps->sample = sample;
	ps->note = note;
	ps->volume = volume;
	ps->channel = channel;
	
	CommandSend(&command, sizeof(PlaySampleCommand));
}

void CommandStopSample(int channel)
{
	Command command;
	StopSampleSoundCommand *ss = &command.stopSample;

	command.commandType = STOP_SAMPLE; 
	ss->channel = channel;

	CommandSend(&command, sizeof(StopSampleSoundCommand));
}

void CommandStartRecording(u16* buffer, int length)
{
	Command command;
	StartRecordingCommand* sr = &command.startRecording;

	command.commandType = START_RECORDING; 
	sr->buffer = buffer;
	sr->length = length;

	CommandSend(&command, sizeof(StartRecordingCommand));
}

int CommandStopRecording(void)
{
	Command command;
	command.commandType = STOP_RECORDING;
	CommandSend(&command, 0);
	return CommandReceive();
}

void RecvCommandUpdateRow(UpdateRowCommand *c)
{
	if(onUpdateRow)
		onUpdateRow(c->row);
}

void RecvCommandUpdatePotPos(UpdatePotPosCommand *c)
{
	if(onPotPosChange)
		onPotPosChange(c->potpos);
}

void RecvCommandNotifyStop(void)
{
	if(onStop)
		onStop();
}

void RecvCommandSampleFinish(void) {
	if(onPlaySampleFinished)
		onPlaySampleFinished();
}

void CommandProcessCommands(void)
{
	Command command;

	while(fifoCheckDatamsg(fifoChannel)) {
		fifoGetDatamsg(fifoChannel, sizeof(Command), (u8*) &command);
		
		switch(command.commandType)
		{
			case DBG_OUT:
				iprintf("%s", command.dbgOut.msg);
				break;
			
			case UPDATE_ROW:
				RecvCommandUpdateRow(&command.updateRow);
				break;
			
			case UPDATE_POTPOS:
				RecvCommandUpdatePotPos(&command.updatePotPos);
				break;
			
			case NOTIFY_STOP:
				RecvCommandNotifyStop();
				break;
			
			case SAMPLE_FINISH:
				RecvCommandSampleFinish();
				break;
			
			default:
				break;
		}
	}
}

void CommandSetSong(void *song)
{
	Command command;
	SetSongCommand* c = &command.setSong;

	command.commandType = SET_SONG; 
	c->ptr = song;

	CommandSend(&command, sizeof(SetSongCommand));
}

void CommandStartPlay(u8 potpos, u16 row, bool loop)
{
	Command command;
	StartPlayCommand* c = &command.startPlay;

	command.commandType = START_PLAY; 
	c->potpos = potpos;
	c->row = row;
	c->loop = loop;
	
	CommandSend(&command, sizeof(StartPlayCommand));
}

void CommandStopPlay(void) {
	
	Command command;
	command.commandType = STOP_PLAY;

	CommandSend(&command, 0);
}

void CommandPlayInst(u8 inst, u8 note, u8 volume, u8 channel)
{
	Command command;
	command.commandType = PLAY_INST; 
	
	PlayInstCommand* c = &command.playInst;

	c->inst    = inst;
	c->note    = note;
	c->volume  = volume;
	c->channel = channel;

	CommandSend(&command, sizeof(PlayInstCommand));
}

void CommandStopInst(u8 channel)
{
	Command command;
	command.commandType = STOP_INST; 
	
	StopInstCommand* c = &command.stopInst;
	
	c->channel = channel;
	
	CommandSend(&command, sizeof(StopInstCommand));
}

void CommandMicOn(void)
{
	Command command;
	command.commandType = MIC_ON; 

	CommandSend(&command, 0);
}

void CommandMicOff(void)
{
	Command command;
	command.commandType = MIC_OFF;

	CommandSend(&command, 0);
}

void CommandSetPatternLoop(bool state)
{
	Command command;
	command.commandType = PATTERN_LOOP;
	
	PatternLoopCommand* c = &command.ptnLoop;
	c->state = state;
	
	CommandSend(&command, sizeof(PatternLoopCommand));
}
