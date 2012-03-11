﻿

#include "Settings.h"


#include "Controller.h"
/*  Pokopom - Input Plugin for PSX/PS2 Emulators
 *  Copyright (C) 2012  KrossX
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Main.h"
#include "ConfigDialog.h"

//HINSTANCE hInstance;
wchar_t settingsDirectory[1024] = {0}; // for PCSX2
extern _Settings settings[2];
bool bScrollLock; // backup to restore on exit

////////////////////////////////////////////////////////////////////////
// PPDK developer must change libraryName field and can change revision and build
////////////////////////////////////////////////////////////////////////

const unsigned int revision = 2;
const unsigned int build = 0;

const unsigned int versionPS1 = (emupro::PLUGIN_VERSION << 16) | (revision << 8) | build;
const unsigned int versionPS2 = (0x02 << 16) | (revision << 8) | build;

char* libraryName = "Pokopom XInput Pad Plugin"; // rewrite your plug-in name
char* PluginAuthor = "KrossX"; // rewrite your name

////////////////////////////////////////////////////////////////////////
// stuff to make this a true PDK module
////////////////////////////////////////////////////////////////////////

char* PSEgetLibName(void) {
    return libraryName;
}

unsigned int PSEgetLibType(void) {
    return emupro::LT_PAD;
}

unsigned int PSEgetLibVersion(void) {
    return versionPS1;
}

char* PS2EgetLibName(void) {
    isPs2Emulator = true;
    return PSEgetLibName();
}

unsigned int PS2EgetLibType(void) {
    isPs2Emulator = true;
    return 0x02;
}

unsigned int PS2EgetLibVersion2(unsigned int type) {
    isPs2Emulator = true;
    if (type == 0x02) return versionPS2;
    else return 0;
}

////////////////////////////////////////////////////////////////////////
// Init/shutdown, will be called just once on emu start/close
////////////////////////////////////////////////////////////////////////

extern "C" int pokomonPADinit(int flags) // PAD INIT
{
    //bScrollLock = GetKeyState(VK_SCROLL)&0x1;

    INI_LoadSettings();

    //printf("Pokopom -> PADinit [%X]\n", flags);
    controller[0] = new Controller(settings[0]);
    controller[1] = new Controller(settings[1]);

//    controller[0]->settings.xinputPort = 0;
//    controller[1]->settings.xinputPort = 1;

    return emupro::INIT_ERR_SUCCESS;
}

extern "C" void pokomonPADshutdown(void) // PAD SHUTDOWN
{
    //	if(bScrollLock != (GetKeyState(VK_SCROLL)&0x1))
    //	{
    //		keybd_event( VK_SCROLL, 0x45, KEYEVENTF_EXTENDEDKEY, 0 );
    //		keybd_event( VK_SCROLL, 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0 );
    //	}

    //printf("Pokopom -> PADshutdown\n");

    delete controller[0];
    delete controller[1];

    controller[0] = controller[1] = NULL;
}

////////////////////////////////////////////////////////////////////////
// Open/close will be called when a games starts/stops
////////////////////////////////////////////////////////////////////////
//
//LRESULT WINAPI PADwndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
//{
//	keyEvent newEvent;
//
//	switch(msg)
//	{
//	case WM_KEYDOWN:
//	case WM_SYSKEYDOWN:
//		newEvent.evt = 1;
//		newEvent.key = wParam;
//		keyEventList.push_back(newEvent);
//		break;
//
//	case WM_KEYUP:
//	case WM_SYSKEYUP:
//		newEvent.evt = 2;
//		newEvent.key = wParam;
//		keyEventList.push_back(newEvent);
//		break;
//
//	case WM_DESTROY:
//	case WM_QUIT:
//		newEvent.evt = 1;
//		newEvent.key = VK_ESCAPE;
//		keyEventList.push_back(newEvent);
//		break;
//	}
//
//	return CallWindowProcW(emuStuff.WndProc, hWnd, msg, wParam, lParam);
//}

extern void XInputPaused(bool pewpew);

extern "C" int pokomonPADopen(emupro::pad::DataS* ppis) // PAD OPEN
{
    //printf("Pokopom -> PADopen\n");
    XInputPaused(false);

    //	if(isPs2Emulator)
    //	{
    //		emuStuff.hWnd = *(HWND*)ppis;
    //		emuStuff.WndProc = (WNDPROC)SetWindowLongPtr(emuStuff.hWnd, GWLP_WNDPROC, (LPARAM)PADwndProc);
    //	}

    return emupro::pad::ERR_SUCCESS;
}

extern "C" int pokomonPADclose(void) // PAD CLOSE
{
    //printf("Pokopom -> PADclose\n");
    XInputPaused(true);

    //	if(isPs2Emulator)
    //		SetWindowLongPtr(emuStuff.hWnd, GWLP_WNDPROC, (LPARAM)emuStuff.WndProc);

    return emupro::pad::ERR_SUCCESS;
}

////////////////////////////////////////////////////////////////////////
// call config dialog
////////////////////////////////////////////////////////////////////////

extern "C" int pokomonPADconfigure(void) {
    INI_LoadSettings();
    //	CreateDialogs(hInstance);


    return emupro::pad::ERR_SUCCESS;
}

////////////////////////////////////////////////////////////////////////
// show about dialog
////////////////////////////////////////////////////////////////////////

extern "C" void pokomonPADabout(void) {
    //	MessageBox(NULL, L"Pokopom XInput pad plugin - KrossX © 2012", L"About...", MB_OK);
}

////////////////////////////////////////////////////////////////////////
// test... well, we are ever fine ;)
////////////////////////////////////////////////////////////////////////

extern "C" int pokomonPADtest(void) {
    return emupro::pad::ERR_SUCCESS;
}

////////////////////////////////////////////////////////////////////////
// tell the controller's port which can be used
////////////////////////////////////////////////////////////////////////

extern "C" int pokomonPADquery(void) {
    //printf("Pokopom -> PADquery\n");
    return emupro::pad::USE_PORT1 | emupro::pad::USE_PORT2;
}

////////////////////////////////////////////////////////////////////////
// tell the input of pad
// this function should be replaced with PADstartPoll and PADpoll
////////////////////////////////////////////////////////////////////////

extern "C" int pokomonPADreadPort(int port, emupro::pad::DataS* ppds) {
    //printf("Pokopom -> PADreadPort [%X]\n", port);

    controller[port]->command(0, 0x01);
    unsigned char cType = controller[port]->command(1, 0x42);
    ppds->controllerType = cType >> 4;

    controller[port]->command(2, 0x00);
    ppds->buttonStatus = controller[port]->command(3, 0x00) | (controller[port]->command(4, 0x00) << 8);

    cType = cType & 0xF;

    if (cType > 2) {
        ppds->rightJoyX = ppds->moveX = controller[port]->command(5, 0x00);
        ppds->rightJoyY = ppds->moveY = controller[port]->command(6, 0x00);

        if (cType >= 0x03) {
            ppds->leftJoyX = controller[port]->command(7, 0x00);
            ppds->leftJoyY = controller[port]->command(8, 0x00);
        }
    }


    return emupro::pad::ERR_SUCCESS;
}

extern "C" int pokomonPADreadPort1(emupro::pad::DataS* ppds) {
    return pokomonPADreadPort(0, ppds);
}

extern "C" int pokomonPADreadPort2(emupro::pad::DataS* ppds) {
    return pokomonPADreadPort(1, ppds);
}

////////////////////////////////////////////////////////////////////////
// input and output of pad
////////////////////////////////////////////////////////////////////////

extern "C" unsigned char pokomonPADstartPoll(int port) {
    curPort = port - 1;
    bufferCount = 0;

    unsigned char data = controller[curPort]->command(bufferCount, 0x01);

    //if(curPort == 0) printf("\n[%02d] [%02X|%02X]\n", bufferCount, 0x01, data);
    //printf("\n[%02d|%02d] [%02X|%02X]\n", bufferCount, curPort, 0x01, data);

    return data;
}

extern "C" unsigned char pokomonPADpoll(unsigned char data) {
    bufferCount++;

    unsigned char doto = controller[curPort]->command(bufferCount, data);

    //if(curPort == 0) printf("[%02d] [%02X|%02X]\n", bufferCount, data, doto);
    //printf("[%02d|%02d] [%02X|%02X]\n", bufferCount, curPort, data, doto);

    return doto;
}

////////////////////////////////////////////////////////////////////////
// other stuff
////////////////////////////////////////////////////////////////////////

extern "C" unsigned int pokomonPADfreeze(int mode, freezeData *data) {
    //printf("Pokopom -> PADfreeze [%X]\n", mode);
    return 0;
}

extern "C" keyEvent* pokomonPADkeyEvent() {
    //printf("Pokopom -> PADkeyEvent\n");

    static keyEvent pochy;

    if (!keyEventList.empty()) {
        pochy = keyEventList.front();
        keyEventList.pop_back();
        return &pochy;
    }

    return NULL;
}

extern "C" unsigned int pokomonPADqueryMtap(unsigned char port) {
    //printf("Pokopom -> PADqueryMtap [%X]\n", port);
    return 0;
}

extern "C" void pokomonPADsetSettingsDir(const char *dir) {
    //	size_t dirsize = strlen(dir) + 1;
    //    size_t convertedChars = 0;
    //    mbstowcs_s(&convertedChars, settingsDirectory, dirsize, dir, _TRUNCATE);
}

extern "C" unsigned int pokomonPADsetSlot(unsigned char port, unsigned char slot) {
    //printf("Pokopom -> PADsetSlot [%X|%X]\n", port, slot);
    return 0;
}

extern "C" void pokomonPADupdate(int port) {
    //printf("Pokopom -> PADupdate [%X]\n", port);
}
//
//BOOL APIENTRY DllMain(HMODULE hInst, DWORD dwReason, LPVOID lpReserved)
//{
//	hInstance = hInst;
//	return TRUE;
//}
//
//BOOL APIENTRY EntryPoint (HMODULE hInst, DWORD dwReason, LPVOID lpReserved)
//{
//	hInstance = hInst;
//	return TRUE;
//}
