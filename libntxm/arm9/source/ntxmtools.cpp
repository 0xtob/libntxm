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

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <nds.h>

#include "ntxm/ntxmtools.h"

s32 unfreed_malloc_calls = 0;

#define WRITEBUFFER_SIZE	512

#ifdef ARM9

u8 writebuffer[WRITEBUFFER_SIZE] = {0};
u32 writebuffer_pos = 0;
u32 remaining_bytes = 0;

void *my_malloc(size_t size)
{
	void *ptr = malloc(size);
	if(ptr!=0) {
		unfreed_malloc_calls++;
	}
	return ptr;
}

void my_free(void *ptr)
{
	if(ptr!=0) {
		unfreed_malloc_calls--;
		free(ptr);
	} else {
		iprintf("Nullpointer free detected!\n");
	}
}

void my_start_malloc_invariant(void)
{
	unfreed_malloc_calls = 0;
}

void my_end_malloc_invariant(void)
{
	if(unfreed_malloc_calls != 0) {
		iprintf("Allocation error! Unfreed mallocs: %d\n", (int)unfreed_malloc_calls);
	}
}

void *my_memalign(size_t blocksize, size_t bytes)
{
	void *buf = memalign(blocksize, bytes);
	if((u32)buf & blocksize != 0) {
		iprintf("Memalign error! %p ist not %u-aligned\n", buf, (u16)blocksize);
		return 0;
	} else {
		unfreed_malloc_calls++;
		return buf;
	}
}

#endif

// Reinventing the wheel to save arm7 binary size
void *my_memset(void *s, int c, u32 n)
{
	u8 *t = (u8*)s;
	for(u32 i=0; i<n; ++i) {
		t[i] = c;
	}
	return s;
}

char *my_strncpy(char *dest, const char *src, u32 n)
{
	u32 i=0;
	while((src[i] != 0) && (i < n)) {
		dest[i] = src[i];
		i++;
	}
	if((i<n)&&(src[i]==0)) {
		dest[i] = 0;
	}
	return dest;
}

#ifdef ARM9

u32 my_fwrite_buffered(const void* buffer, u32 size, u32 count, FILE* file)
{
	u32 buffer_rest = WRITEBUFFER_SIZE - writebuffer_pos;
	u32 bytes = size * count;
	
	if(bytes < buffer_rest) {
		
		memcpy(writebuffer+writebuffer_pos, buffer, bytes);
		writebuffer_pos += bytes;
		
	} else {
		
		memcpy(writebuffer+writebuffer_pos, buffer, buffer_rest);
		buffer = (u8*)buffer + buffer_rest;
		
		// Flush
		fwrite(writebuffer, WRITEBUFFER_SIZE, 1, file);
		writebuffer_pos = 0;
		
		bytes -= buffer_rest;
		buffer_rest = WRITEBUFFER_SIZE;
		
		while(bytes >= WRITEBUFFER_SIZE) {
			
			memcpy(writebuffer, buffer, WRITEBUFFER_SIZE);
			buffer = (u8*)buffer + WRITEBUFFER_SIZE;
			bytes -= WRITEBUFFER_SIZE;
			
			// Flush
			fwrite(writebuffer, WRITEBUFFER_SIZE, 1, file);
			writebuffer_pos = 0;
			
		}
		
		memcpy(writebuffer, buffer, bytes);
		writebuffer_pos += bytes;
		
	}
	
	return size*count;
}


bool my_fclose_buffered(FILE* file)
{
	fwrite(writebuffer, writebuffer_pos, 1, file);
	writebuffer_pos = 0;
	
	return fclose(file);
}

bool my_file_exists(const char *filename)
{
	bool res;
	FILE* f = fopen(filename,"r");
	if(f == NULL) {
		res = false;
	} else {
		fclose(f);
		res = true;
	}
	
	return res;
}

#endif

s32 my_clamp(s32 val, s32 min, s32 max)
{
	if(val < min)
		return min;
	if(val > max)
		return max;
	return val;
}

// Borrowed from Infantile Paralyser
bool my_testmalloc(int size)
{
	if(size<=0) return(false);
	
	void *ptr;
	u32 adr;
	
	ptr=malloc(size+(64*1024)); // 64kb
	
	if(ptr==NULL) return(false);
	
	adr=(u32)ptr;
	free(ptr);
	
	if((adr&3)!=0){ // 4byte
		return(false);
	}
	
	if((adr+size)<0x02000000){
		return(false);
	}
	
	if((0x02000000+(4*1024*1024))<=adr){
		return(false);
	}
	
	return(true);
}

#define PrintFreeMem_Seg (10240)

// Borrowed from Infantile Paralyser
//TODO: This is NOT a clean way to get the free memory! Fix it!
u32 my_get_free_mem(void)
{
	s32 i;
	u32 FreeMemSize=0;
	
	for(i=1*PrintFreeMem_Seg;i<4096*1024;i+=PrintFreeMem_Seg){
		if(my_testmalloc(i)==false) break;
		FreeMemSize=i;
	}
	
	return FreeMemSize;
}
