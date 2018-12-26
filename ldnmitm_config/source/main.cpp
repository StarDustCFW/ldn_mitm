#include <stdio.h>
#include <stdlib.h>
#include <switch.h>
#include <string.h>
#include "ldn.h"
#include "test.h"

#define MODULEID 0x233
static Service g_ldnSrv;
static LdnMitmConfigService g_ldnConfig;

Result saveLogToFile() {
    Result rc = 0;
    rc = ldnMitmSaveLogToFile(&g_ldnConfig);
    if (R_FAILED(rc)) {
        printf("Save log to file failed %x\n", rc);
        return rc;
    }

    return rc;
}

void cleanup() {
    serviceClose(&g_ldnSrv);
    serviceClose(&g_ldnConfig.s);
}

void die(const char *reason) {
    printf("fatal: %s\npress any key to exit.", reason);
    while(appletMainLoop()) {
        hidScanInput();
        u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);
        if (kDown) {
            break;
        }
        gfxFlushBuffers();
        gfxSwapBuffers();
    }
    gfxExit();
    cleanup();
    exit(1);
}

void printHeader() {
    char version[32];
    Result rc = ldnMitmGetVersion(&g_ldnConfig, version);
    if (R_FAILED(rc)) {
        strcpy(version, "Error");
    }

    printf("    ldnmitm_config " VERSION_STRING "\n          ldn_mitm %s\n\n", version);
}

const char * getOnOff(u32 enabled) {
    if (enabled) {
        return CONSOLE_GREEN "ON" CONSOLE_RESET;
    } else {
        return CONSOLE_RED "OFF" CONSOLE_RESET;
    }
}

void printStatus() {
    u32 enabled;
    Result rc = ldnMitmGetLogging(&g_ldnConfig, &enabled);
    if (R_FAILED(rc)) {
        die("failed to get logging status");
    }
    printf("Logging(X): %s\n", getOnOff(enabled));

    rc = ldnMitmGetEnabled(&g_ldnConfig, &enabled);
    if (R_FAILED(rc)) {
        die("failed to get enabled status");
    }
    printf("ldn_mitm(Y): %s\n", getOnOff(enabled));

    putchar('\n');
    puts("Press X: toggle logging (sd:/ldn_mitm.log)");
    puts("Press Y: toggle ldn_mitm");
    puts("Press B: exit");
}

void reprint() {
    gfxSetMode(GfxMode_TiledDouble);
    consoleClear();

    printHeader();
    printStatus();
}

void toggleLogging() {
    u32 enabled;
    Result rc = ldnMitmGetLogging(&g_ldnConfig, &enabled);
    if (R_FAILED(rc)) {
        die("failed to get logging status");
    }
    rc = ldnMitmSetLogging(&g_ldnConfig, !enabled);
    if (R_FAILED(rc)) {
        die("failed to set logging status");
    }
}

void toggleEnabled() {
    u32 enabled;
    Result rc = ldnMitmGetEnabled(&g_ldnConfig, &enabled);
    if (R_FAILED(rc)) {
        die("failed to get enabled status");
    }
    rc = ldnMitmSetEnabled(&g_ldnConfig, !enabled);
    if (R_FAILED(rc)) {
        die("failed to set enabled status");
    }
}

int main() {
    gfxInitDefault();
    consoleInit(NULL);

    Result rc = smGetService(&g_ldnSrv, "ldn:u");
    if (R_FAILED(rc)) {
        die("failed to get service ldn:u");
    }
    rc = ldnMitmGetConfig(&g_ldnSrv, &g_ldnConfig);
    if (R_FAILED(rc)) {
        die("ldn_mitm is not loaded or too old(requires ldn_mitm > v1.0)");
    }

    reprint();
    while(appletMainLoop()) {
        hidScanInput();
        u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);

        if (kDown & KEY_B) {
            break;
        }

        if (kDown & KEY_LSTICK) {
            Result rc = saveLogToFile();
            if (R_SUCCEEDED(rc)) {
                puts("Export complete");
            }
        }

        if (kDown & KEY_X) {
            toggleLogging();
            reprint();
        }

        if (kDown & KEY_Y) {
            toggleEnabled();
            reprint();
        }

        gfxFlushBuffers();
        gfxSwapBuffers();
    }

    gfxExit();
    cleanup();
    return 0;
}
