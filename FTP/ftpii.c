/*

ftpii -- an FTP server for the Wii

Copyright (C) 2008 Joseph Jordan <joe.ftpii@psychlaw.com.au>

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from
the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1.The origin of this software must not be misrepresented; you must not
claim that you wrote the original software. If you use this software in a
product, an acknowledgment in the product documentation would be
appreciated but is not required.

2.Altered source versions must be plainly marked as such, and must not be
misrepresented as being the original software.

3.This notice may not be removed or altered from any source distribution.

*/
#include <di/di.h>
#include <fst/fst.h>
#include <iso/iso.h>
#include <string.h>
#include <unistd.h>
#include <wiiuse/wpad.h>
#include <wod/wod.h>

#include "common.h"
#include "ftp.h"

static const u16 PORT = 21;
static const char *APP_DIR_PREFIX = "ftpii_";
static s32 server=create_server(PORT);

static void FTP_Init() {
    DI_Init();
    //initialise_reset_buttons();
    //printf("To exit, hold A on controller #1 or press the reset button.\n");
    initialise_network();
    initialise_fat();
    //printf("To remount a device, hold B on controller #1.\n");
}

/*static void set_password_from_executable(char *executable) {
    char *dir = basename(dirname(executable));
    if (strncasecmp(APP_DIR_PREFIX, dir, strlen(APP_DIR_PREFIX)) == 0) {
        set_ftp_password(dir + strlen(APP_DIR_PREFIX));
    }
}*/


static void process_dvd_events() {
    if (dvd_mountWait() && DI_GetStatus() & DVD_READY) {
        set_dvd_mountWait(false);
        bool wod = false, fst = false, iso = false;

        fst = FST_Mount();
		wod = WOD_Mount();
		iso = ISO9660_Mount();
		
		if (!(wod || fst || iso)) dvd_stop();
    }
}

void FTP_RUN() {
    //printf("Listening on TCP port %u...\n", PORT);
   
    process_dvd_events();
    process_ftp_events(server);
    process_timer_events();

}

void FTP_Destroy() {
    //printf("\nKTHXBYE\n");
    cleanup_ftp();
    net_close(server);
    //if (dvd_mountWait()) printf("NOTE: Due to a known bug in libdi, ftpii is unable to exit until a DVD is inserted.\n");
    dvd_stop();
    DI_Close();

    if (power()) SYS_ResetSystem(SYS_POWEROFF, 0, 0);
    else if (!hbc_stub()) SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);

}
