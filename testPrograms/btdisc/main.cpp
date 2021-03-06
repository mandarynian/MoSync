/* Copyright 2013 David Axmark

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include <ma.h>
#include <mautil/mauuid.h>
#include <maassert.h>
#include <conprint.h>

void scan();

#define SCAN_DEVICES

extern "C" int MAMain() {
	InitConsole();
	gConsoleLogging = 1;
	scan();
	Freeze(0);
	return 0;
}

const char* btaddr2string(const MABtAddr& a) {
	static char buffer[16];
	sprintf(buffer, "%02x%02x%02x%02x%02x%02x", a.a[0], a.a[1], a.a[2], a.a[3], a.a[4], a.a[5]);
	return buffer;
}

void scan() {
	int nserv=0;
	MAEvent event;
#ifdef SCAN_DEVICES
scan_devices:
	int ndev=0;
	MABtDevice d;
	char namebuf[256];
	int res;

	printf("DevDisc...\n");
	int startTime = maGetMilliSecondCount();
	maBtStartDeviceDiscovery(1);
	//printf("Started\n");

	while(true) {
		d.name = namebuf;
		d.nameBufSize = sizeof(namebuf);

		do {
			strcpy(namebuf, "Hello Debugger!");
			res = maBtGetNewDevice(&d);
			if(res) {
				printf("d%i: %i %s\n", ndev++, res, d.name);
				if(res < 0) {
					break;
				}
				printf("%s\n", btaddr2string(d.address));
			} else {
				printf("res %i\n", res);
			}
		} while(res > 0);

		maWait(0);
		maGetEvent(&event);	//this oughta work, cause maWait(0) guarantees that there's at least one event.
		if(event.type == EVENT_TYPE_BT && event.state != 0)
			break;
		if(event.type == EVENT_TYPE_CLOSE ||
			(event.type == EVENT_TYPE_KEY_PRESSED && (event.key == MAK_0 || event.key == MAK_KP0)))
		{
			maExit(0);
		}
	}
	printf("Done %i, %i ms\n", event.state, maGetMilliSecondCount() - startTime);

	printf("Press 1 or 3\n");
	while(true) {
		maWait(0);
		maGetEvent(&event);
		if(event.type == EVENT_TYPE_CLOSE ||
			(event.type == EVENT_TYPE_KEY_PRESSED && (event.key == MAK_0 || event.key == MAK_KP0)))
		{
			maExit(0);
		}
		if(event.type == EVENT_TYPE_KEY_PRESSED && (event.key == MAK_1 || event.key == MAK_KP1)) {
			goto scan_devices;
		}
		if(event.type == EVENT_TYPE_KEY_PRESSED && (event.key == MAK_3 || event.key == MAK_KP3)) {
			break;
		}
	}


	const MABtAddr& address = d.address;
#else
	//const MABtAddr address = { { 0x00, 0x0f, 0xde, 0xa1, 0x4b, 0x70 } };	//K700i Jacob
	//const MABtAddr address = { { 0x00, 0x0b, 0x0d, 0x14, 0x99, 0x0f } };	//Holux
	//const MABtAddr address = { { 0x00, 0x11, 0x9f, 0xcb, 0x74, 0x10 } };	//6630 v1.0
	//const MABtAddr address = { { 0x00, 0x18, 0xc5, 0x3f, 0x74, 0x7e } };	//N73
	//const MABtAddr address = { { 0x00, 0x80, 0x98, 0x44, 0x74, 0xc8 } };	//MS-FREDRIK
	//const MABtAddr address = { { 0x00, 0x11, 0x67, 0x9c, 0xd9, 0x3c } };	//MS-FREDRIK, new dongle
	const MABtAddr address = { { 0x00, 0x23, 0x3a, 0xb3, 0xc7, 0x82 } };	//Samsung SGH-i900
#endif	//SCAN_DEVICES

	printf("ServDisc %s\n", btaddr2string(address));
	int servStartTime = maGetMilliSecondCount();
	maBtStartServiceDiscovery(&address, &L2CAP_PROTOCOL_MAUUID);

	while(true) {
		MABtService s;
		char sNameBuf[256];
		static const int MAX_UUIDS = 32;
		MAUUID uuidBuf[MAX_UUIDS];
		int res;
		s.name = sNameBuf;
		s.nameBufSize = sizeof(sNameBuf);
		s.uuids = uuidBuf;

		//printfln("state: %i\n", maBtDiscoveryState2());
		do {
			strcpy(sNameBuf, "Hello Sebugger!");
			MABtServiceSize ss;
			res = maBtGetNextServiceSize(&ss);
			if(res) {
				MAASSERT(ss.nUuids <= MAX_UUIDS);
				if(ss.nameBufSize < 0) {
					printf("%i\n", ss.nameBufSize);
				}
				res = maBtGetNewService(&s);
				if(res) {
					if(ss.nameBufSize < 0) {
						printf("s%i: %i %i No name\n", nserv++, res, s.port);
					} else {
						printf("s%i: %i %i %i \"%s\"\n", nserv++, res, s.port, ss.nameBufSize, s.name);
					}
					/*for(int j=0; j<ss.nUuids; j++) {
						int* u = s.uuids[j].i;
						printf("%08X-%08X-%08X-%08X\n", u[0], u[1], u[2], u[3]);
					}*/
					if(res < 0) {
						//printf("res1 %i\n", res);
						break;
					}
				} else {
					//printf("res2 %i\n", res);
				}
			} else {
				//printf("res3 %i\n", res);
			}
		} while(res > 0);

		maWait(0);
		maGetEvent(&event);
		if(event.type == EVENT_TYPE_BT) {
			//printf("event %i\n", event.state);
			if(event.state != 0)
				break;
		}
		if(event.type == EVENT_TYPE_CLOSE ||
			(event.type == EVENT_TYPE_KEY_PRESSED && (event.key == MAK_0 || event.key == MAK_KP0)))
		{
			maExit(0);
		}
	}
	printf("Done %i, %i ms\n", event.state, maGetMilliSecondCount() - servStartTime);
#ifdef SCAN_DEVICES
	printf("Done, total %i ms\n", maGetMilliSecondCount() - startTime);
#endif	//SCAN_DEVICES
}
