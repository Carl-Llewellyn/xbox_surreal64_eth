/**
 * Mupen64 hle rsp - hle.
 * Copyright (C) 2002 Hacktarux
 *
 * Mupen64 homepage: http://mupen64.emulation64.com
 * email address: hacktarux@yahoo.fr
 * 
 * If you want to contribute to the project please contact
 * me first (maybe someone is already making what you are
 * planning to do).
 *
 *
 * This program is free software; you can redistribute it and/
 * or modify it under the terms of the GNU General Public Li-
 * cence as published by the Free Software Foundation; either
 * version 2 of the Licence, or any later version.
 *
 * This program is distributed in the hope that it will be use-
 * ful, but WITHOUT ANY WARRANTY; without even the implied war-
 * ranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public Licence for more details.
 *
 * You should have received a copy of the GNU General Public
 * Licence along with this program; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139,
 * USA.
 *
**/

#pragma once

#ifdef _BIG_ENDIAN
	#define S 0
	#define S8 0
#else
	#define S 1
	#define S8 3
#endif

extern RSP_INFO rsp;

typedef struct
{
   unsigned long type;
   unsigned long flags;   
   unsigned long ucode_boot;
   unsigned long ucode_boot_size;   
   unsigned long ucode;
   unsigned long ucode_size;   
   unsigned long ucode_data;
   unsigned long ucode_data_size;   
   unsigned long dram_stack;
   unsigned long dram_stack_size;   
   unsigned long output_buff;
   unsigned long output_buff_size;   
   unsigned long data_ptr;
   unsigned long data_size;   
   unsigned long yield_data_ptr;
   unsigned long yield_data_size;
} OSTask_t;
