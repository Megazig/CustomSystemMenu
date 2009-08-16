#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ogcsys.h>
#include <gccore.h>
#include <ogc/ios.h>

#include "elf.h"
#include "dol.h"
#include "asm.h"
#include "processor.h"

extern void __exception_closeall();
extern s32 __IOS_ShutdownSubsystems();

void LaunchExecutable(char filename) {
	u8 *data = (u8 *)0x92000000;
	s32 res;
	void (*ep)();
	u32 level;
	
	snprintf(filename, 4+13, "elf/%s", filelist[selection]);
	if(f_stat(filename, &finfo) != FR_OK)
	{
		printf("error: f_stat() failed :/\n");
		goto redo;
	}
	if(f_open(&fp, filename, FA_READ) != FR_OK)
	{
		printf("error: f_open() failed\n");
		goto redo;
	}
	printf("Reading %u bytes\n", (unsigned int)finfo.fsize);
	bytes_read = bytes_read_total = 0;
	while(bytes_read_total < finfo.fsize)
	{
		if(f_read(&fp, data + bytes_read_total, 0x200, &bytes_read) != FR_OK)
		{
			printf("error: f_read failed.\n");
			goto redo;
		}
		if(bytes_read == 0)
			break;
		bytes_read_total += bytes_read;
//					printf("%d/%d bytes read (%d this time)\n", bytes_read_total, (unsigned int)finfo.fsize, bytes_read);
	}
	if(bytes_read_total < finfo.fsize)
	{
		printf("error: read %u of %u bytes.\n", bytes_read_total, (unsigned int)finfo.fsize);
		goto redo;
	}

	
    fread (buffer, 1, lsize, fd);
	
		res = valid_elf_image(data);
		if(res == 1)
		{
			//printf("ELF image found.\n");
			ep = (void(*)())load_elf_image(data);
		}
		else
		{
			//printf("Assuming that this is a DOL file. Your Wii will crash if it isn't...\n");
			ep = (void(*)())load_dol_image(data);
		}
		__IOS_ShutdownSubsystems ();
		//printf("IOS_ShutdownSubsystems() done\n");
		_CPU_ISR_Disable (level);
		//printf("_CPU_ISR_Disable() done\n");
		__exception_closeall ();
		//printf("__exception_closeall() done. Jumping to ep now...\n");
		ep();
		_CPU_ISR_Restore (level);
}

