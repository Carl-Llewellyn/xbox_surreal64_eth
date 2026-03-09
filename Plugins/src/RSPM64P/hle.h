//#pragma once

#ifndef HLE_H
#define HLE_H

//#include "..\api\m64p_plugin.h"
#include "Rsp.h"

#define RSP_HLE_VERSION 0x016303

#ifdef _BIG_ENDIAN
#define S 0
#define S16 0
#define S8 0
#else
#define S 1
#define S16 2
#define S8 3
#endif

// types
typedef unsigned char       u8;
typedef unsigned short      u16;
typedef unsigned int        u32;
typedef unsigned long long  u64;

typedef signed char         s8;
typedef signed short        s16;
typedef signed int          s32;
typedef signed long long    s64;

//#define ACMD_SIZE               32
/*
 * Audio flags
 */

#define A_INIT          0x01
#define A_CONTINUE      0x00
#define A_LOOP          0x02
#define A_OUT           0x02
#define A_LEFT          0x02
#define A_RIGHT         0x00
#define A_VOL           0x04
#define A_RATE          0x00
#define A_AUX           0x08
#define A_NOAUX         0x00
#define A_MAIN          0x00
#define A_MIX           0x10

extern RSP_INFO rsp;

typedef struct
{
   unsigned int type;
   unsigned int flags;
   
   unsigned int ucode_boot;
   unsigned int ucode_boot_size;

   unsigned int ucode;
   unsigned int ucode_size;
   
   unsigned int ucode_data;
   unsigned int ucode_data_size;
   
   unsigned int dram_stack;
   unsigned int dram_stack_size;
   
   unsigned int output_buff;
   unsigned int output_buff_size;
   
   unsigned int data_ptr;
   unsigned int data_size;
   
   unsigned int yield_data_ptr;
   unsigned int yield_data_size;
} OSTask_t;

void DebugMessage(int level, const char *message, ...);

void jpg_uncompress(OSTask_t *task);
/*void ucode1(OSTask_t *task);
void ucode2(OSTask_t *task);
void ucode3(OSTask_t *task);
void init_ucode1();*/
void init_ucode2();

extern u32 inst1, inst2;
extern u16 AudioInBuffer, AudioOutBuffer, AudioCount;
extern u16 AudioAuxA, AudioAuxC, AudioAuxE;
extern u32 loopval; // Value set by A_SETLOOP : Possible conflict with SETVOLUME???
//extern u32 UCData, UDataLen;

#endif