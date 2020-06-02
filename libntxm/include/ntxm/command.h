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

#if !defined(COMMAND_H)
#define COMMAND_H

#define DST_ARM7	0
#define DST_ARM9	1

#define DEBUGSTRSIZE 40

#include <nds.h>

#include "ntxm/sample.h"

/*
  Structures and functions to allow the ARM9 to send commands to the
  ARM7. Based on code from the MOD player example posted to the GBADEV
  forums.
*/

/* Enumeration of commands that the ARM9 can send to the ARM7 */
enum CommandType {
	PLAY_SAMPLE,
	STOP_SAMPLE,
	START_RECORDING,
	STOP_RECORDING,
	SET_SONG,
	START_PLAY,
	STOP_PLAY,
	DBG_OUT,
	UPDATE_ROW,
	UPDATE_POTPOS,
	PLAY_INST,
	STOP_INST,
	NOTIFY_STOP,
	MIC_ON,
	MIC_OFF,
	PATTERN_LOOP,
	SAMPLE_FINISH
};

struct PlaySampleCommand
{
	Sample *sample;
	u8 note;
	u8 volume;
	u8 channel;
};

/* Command parameters for stopping a sample */
struct StopSampleSoundCommand
{
	int channel;
};

/* Command parameters for starting to record from the microphone */
struct StartRecordingCommand
{
	u16* buffer;
	int length;
};

struct SetSongCommand {
	void *ptr;
};

struct StartPlayCommand {
	u8 potpos;
	u16 row;
	bool loop;
};

struct StopPlayCommand {
};

struct DbgOutCommand {
	char msg[DEBUGSTRSIZE];
};

struct UpdateRowCommand {
	u16 row;
};

struct UpdatePotPosCommand {
	u16 potpos;
};

struct PlayInstCommand {
	u8 inst;
	u8 note;
	u8 volume;
	u8 channel;
};

struct StopInstCommand {
	u8 channel;
};

struct PatternLoopCommand {
	bool state;
};

/* The ARM9 fills out values in this structure to tell the ARM7 what
   to do. */
struct Command {
	u8 destination;
	CommandType commandType;
	union {
		void* data;
		PlaySampleCommand      playSample;
		StopSampleSoundCommand stopSample;
		StartRecordingCommand  startRecording;
		SetSongCommand         setSong;
		StartPlayCommand       startPlay;
		StopPlayCommand        stopPlay;
		DbgOutCommand          dbgOut;
		UpdateRowCommand       updateRow;
		UpdatePotPosCommand    updatePotPos;
		PlayInstCommand        playInst;
		StopInstCommand        stopInst;
		PatternLoopCommand     ptnLoop;
	};
};

/* Maximum number of commands */
#define MAX_COMMANDS 40

/* A structure shared between the ARM7 and ARM9. The ARM9
   places commands here and the ARM7 reads and acts upon them.
*/
struct CommandControl {
	Command command[MAX_COMMANDS];
	int currentCommand;
	int return_data;
};

/* Address of the shared CommandControl structure */
#define commandControl ((CommandControl*)((uint32)(IPC) + sizeof(TransferRegion)))

#if defined(ARM9)
void CommandInit();
void CommandPlayOneShotSample(int channel, int frequency, const void* data, int length, int volume, int format, bool loop);
void CommandPlaySample(Sample *sample, u8 note, u8 volume, u8 channel);
void CommandPlaySample(Sample *sample);
void CommandStopSample(int channel);
void CommandStartRecording(u16* buffer, int length);
int CommandStopRecording(void);
void CommandSetSong(void *song);
void CommandStartPlay(u8 potpos, u16 row, bool loop);
void CommandStopPlay(void);
void CommandSetDebugStrPtr(char **arm7debugstrs, u16 debugstrsize, u8 n_debugbufs);
void CommandPlayInst(u8 inst, u8 note, u8 volume, u8 channel);
void CommandStopInst(u8 channel);
void CommandMicOn(void);
void CommandMicOff(void);
void CommandSetPatternLoop(bool state);

void RegisterRowCallback(void (*onUpdateRow_)(u16));
void RegisterStopCallback(void (*onStop_)(void));
void RegisterPlaySampleFinishedCallback(void (*onPlaySampleFinished_)(void));
void RegisterPotPosChangeCallback(void (*onPotPosChange_)(u16));
#endif

void CommandProcessCommands();

#if defined(ARM7)
void CommandDbgOut(const char *formatstr, ...); // Print text from the ARM7, syntax like printf
void CommandUpdateRow(u16 row);
void CommandUpdatePotPos(u16 potpos);
void CommandNotifyStop(void);
void CommandSampleFinish(void);
#endif

#endif
