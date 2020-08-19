#include "dll.h"
#pragma comment(lib, "ws2_32.lib")

using namespace std;


DWORD WINAPI MainThread(LPVOID param) {
	QueryPerformanceFrequency(&LOOTtcounter);
	LOOTfreq = LOOTtcounter.QuadPart;
	//HANDLE wow = file_kbd;
	short accnum = 0, passnum = 0, char_class = 0;
	char is_healbot = '0';
	//Sleep(2000);
	dinput_main();
	initialize();
	initializeDriver();
	initializeSkills();
	initializeSkills_Crus();
	initializeSkills_Arch();
	initializeSkills_Mage();
	InitializeSkills_Healbot();
	DWORD hookAddressDrops = fiestaBase + 0x28D461; // done, find X or Y of item, find what accesses, then go to ecx+000000C4
	jmpBackAddyDrops = hookAddressDrops + 6;
	HookFunctionAddy((void*)hookAddressDrops, dropsHook, 6);

	entityPointer = (fiestaBase + 0x700C58);
	DWORD playerEntity = *(DWORD*)(entityPointer + 0x3C);
	DWORD player = *(DWORD*)(playerEntity + 0x08);

	char talkToCC[5] = { 0x04, 0x0a, 0x20, 0x03, 0x43 };
	cout << "SLOW??";
	cin >> SLOW;
	getchar();

	//while (1){ Test function for trying to inject input via Windows SendMessage/PostMessage functions.
		//HWND fiesta = FindWindowW(NULL, L"FiestaOnline");
		//HWND innerfiesta = GetWindow(fiesta, GW_CHILD);
		//cout << hex << fiesta << "\t" << innerfiesta << endl;
		//getchar();
		//SendMessage(fiesta, WM_PARENTNOTIFY, WM_LBUTTONDOWN, 0x02630325);
		//EnumChildWindows(innerfiesta, EnumChildProc, 0);
		/*while (1){
			SendMessage(fiesta, WM_PARENTNOTIFY, WM_LBUTTONDOWN, 0x00C301A7);
			SendMessage(fiesta, WM_MOUSEACTIVATE, (WPARAM)fiesta, MAKELPARAM(HTCLIENT, WM_LBUTTONDOWN));
			SendMessage(innerfiesta, WM_MOUSEACTIVATE, (WPARAM)fiesta, MAKELPARAM(HTCLIENT, WM_LBUTTONDOWN));
			SendMessage(innerfiesta, WM_SETCURSOR, (WPARAM)innerfiesta, MAKELPARAM(HTCLIENT, WM_LBUTTONDOWN));
			PostMessage(innerfiesta, WM_LBUTTONDOWN, MK_LBUTTON, 0x00C301A7);
			//SendMessage(innerfiesta, WM_MOUSEACTIVATE, (WPARAM)innerfiesta, MAKELPARAM(HTCLIENT, WM_LBUTTONDOWN));
			SendMessage(innerfiesta, WM_SETCURSOR, (WPARAM)innerfiesta, MAKELPARAM(HTCLIENT, WM_LBUTTONUP));
			PostMessage(innerfiesta, WM_LBUTTONUP, 0, 0x00C301A7);
			//SendMessage(innerfiesta, WM_SETCURSOR, (WPARAM)innerfiesta, MAKELPARAM(HTCLIENT, WM_MOUSEMOVE));
			//PostMessage(innerfiesta, WM_MOUSEMOVE, 0, 0x02630325);
			//Sleep(500);
			//LOLCLICKDINPUTSHIT = true;
			Sleep(100);
		}*/
		//SendMessage()
		/*MoveMouseRelative(1, 0, file_mou);
		Sleep(500);
		MoveMouseRelative(0, -1, file_mou);
		Sleep(500);
		MoveMouseRelative(-1, 0, file_mou);
		Sleep(500);
		MoveMouseRelative(0, 1, file_mou);
		getchar();
		Sleep(3000);*/
	//}

	cout << "healbot?";
	cin >> is_healbot;
	getchar();
	if (is_healbot == '1'){
		healBot();
	}
	POINT wow;
	while (1){
		GetCursorPos(&wow);
		//cout << "X: " << *reinterpret_cast<float*>(playerX) << "\tY: " << *reinterpret_cast<float*>(playerY) << "\tZ: " << *reinterpret_cast<float*>(playerZ) << endl;
		//cout << "X: " << (*playerX) << "\tY: " << (*playerY) << "\tZ: " << (*playerZ) << endl;
		cout << "Mouse Coordinates:\tX:" << wow.x << "\t" << wow.y << endl;
		/*if (mapSwitch == true){
			cout << "Map switched successfully!\n";
			mapSwitch = false;
		}*/
		Sleep(1000);
	}

	cout << "CHAR CLASS?? (0 = cleric, 1 = crus)";
	cin >> char_class;
	getchar();

	cout << "Jump in game first to get socket\n";
	while (sendSocket_r == 0) Sleep(1000);
	//cout << "Invite Socket is : " << hex << sendSocket_Invite << endl;
	//getchar();
	Sleep(2000);

	/*while (1){
		cout << "TargetofTargetID is: " << hex << *(WORD*)targetofTargetID << endl;
		Sleep(1000);
	}*/

	char kkpJoin[7] = { 0x06, 0x05, 0x58, 0x10, 0x07, 0x00, 0x00 };

	ZombieFarm();

	initialX = *playerX;
	initialY = *playerY;
	lastPosX = *playerX;
	lastPosY = *playerY;
	int mobID[20] = { 0 };
	int optionalMobID[20] = { 0 };

	//mage
	mobID[0] = 71;
	mobID[1] = 786;
	mobID[2] = 788;
	//mobID[3] = 2030;
	//mobID[4] = 4522;
	//mobID[5] = 4523;
	//mobID[6] = 4524;
	//mobID[7] = 4525;
	//mobID[8] = 4526;
	//mobID[9] = 5129;

	optionalMobID[0] = 83;
	optionalMobID[1] = 787;
	optionalMobID[2] = 789;
	optionalMobID[3] = 2031;
	//optionalMobID[4] = 779;
	//optionalMobID[5] = 781;
	//optionalMobID[6] = 2029;
	//optionalMobID[7] = 716;
	//optionalMobID[8] = 719;
	//optionalMobID[9] = 77;
	//optionalMobID[10] = 717;
	//optionalMobID[11] = 720;
	//optionalMobID[12] = 2024;

	/*//CRUS BOT 1
	mobID[0] = 4546;
	mobID[1] = 4547;
	mobID[2] = 4548;
	mobID[3] = 4549;
	mobID[4] = 4545;
	//mobID[5] = 4510;
	//mobID[6] = 4511;
	//mobID[7] = 4512;
	//mobID[8] = 4513;

	optionalMobID[0] = 4550;
	optionalMobID[1] = 4551;
	optionalMobID[2] = 4552;
	optionalMobID[3] = 4553;
	optionalMobID[4] = 765;
	//optionalMobID[5] = 765;
	//optionalMobID[6] = 731;
	//optionalMobID[7] = 716;
	//optionalMobID[8] = 719;
	//optionalMobID[9] = 77;
	//optionalMobID[10] = 717;
	//optionalMobID[11] = 720;
	//optionalMobID[12] = 2024;*/

	//CRUS BOT 2
	/*mobID[0] = 4500;
	mobID[1] = 4501;
	mobID[2] = 4502;
	mobID[3] = 4503;
	mobID[4] = 4504;
	mobID[5] = 4510;
	mobID[6] = 4511;
	mobID[7] = 4512;
	mobID[8] = 4513;

	//optionalMobID[0] = 4550;
	//optionalMobID[1] = 4551;
	//optionalMobID[2] = 4552;
	//optionalMobID[3] = 4553;
	//optionalMobID[4] = 765;
	//optionalMobID[5] = 765;
	//optionalMobID[6] = 731;
	//optionalMobID[7] = 716;
	//optionalMobID[8] = 719;
	//optionalMobID[9] = 77;
	//optionalMobID[10] = 717;
	//optionalMobID[11] = 720;
	//optionalMobID[12] = 2024;*/

	//CLERIC
	/*mobID[0] = 561;
	mobID[1] = 875;
	mobID[2] = 876;
	//mobID[3] = 827;
	//mobID[4] = 4558;
	//mobID[5] = 4559;
	//mobID[6] = 4560;
	//mobID[7] = 4561;
	//mobID[8] = 4562;

	optionalMobID[0] = 544;
	optionalMobID[1] = 847;
	optionalMobID[2] = 848;
	optionalMobID[3] = 541;
	optionalMobID[4] = 840;
	optionalMobID[5] = 842;
	//optionalMobID[6] = 65;
	//optionalMobID[7] = 772;
	//optionalMobID[8] = 776;
	//optionalMobID[9] = 330;
	//optionalMobID[10] = 744;
	//optionalMobID[11] = 745;*/

	//Console();
	cout << "char class is " << char_class << endl;
	enableDropTrack = true;
	while (1){
		if (*(BYTE*)battleState == 0){
			char packet[4] = { 0x03, 0x08, 0x20, 0x02 };
			//03 08 20 02
			sendCrypt(sendSocket_r, packet, 4, 0);
			//delete packet;
		}
		if (*health == 0){
			while (1){
				if (GetAsyncKeyState(VK_F3)){
					playerID[0] = 0xFF;
					playerID[1] = 0xFF;
					break;
				}
				Sleep(1000);
			}
		}

		if (*health <= *maxhealth * 0.50 && char_class == 0){
			useSkill(&Heal, playerID, 0, 0);
		}
		if (lastPosX != *playerX || lastPosY != *playerY || stolenAggro == true || monsterTooFar == true){
			lastPosX = *playerX;
			lastPosY = *playerY;

			if (injectKey == false){
				injectDInput(DIK_S, .1);
			}
			
			Sleep(1000);

			cout << "DEBUG:\n playerX: " << *playerX << "\nplayerY: " << *playerY << "\n lastPosX: " << lastPosX << "\nlastPosY: " << lastPosY << "\nstolenAggro: " << stolenAggro << "\nmonsterTooFar: " << monsterTooFar << endl;
			if (monsterTooFar == true) monsterTooFar = false;
			if (stolenAggro == true) stolenAggro = false;

			Sleep(500);
			lastPosX = *playerX;
			lastPosY = *playerY;
		}
		if (char_class == 0) checkBuff(false);
		checkQuests();
		//if(char_class == 0) mobESPGrind(mobID, optionalMobID);
		//cout << "weee";
		if (char_class == 1){
			mobESPGrind_Mage(mobID, optionalMobID);//mobESPGrind_Crus(mobID, optionalMobID);
			if (LOOTTRAVEL == true){
				TravelTo(TRAVELX, TRAVELY);
				LOOTTRAVEL = false;
			}
		}
		//if (char_class == 1) mobESPGrind_Crus(mobID, optionalMobID);
		if (GetAsyncKeyState(VK_F1)){
			Sleep(1000);
			while (1){
				if (char_class == 0){
					checkBuff(0);
					if (*mana <= *maxmana * 0.3){
						useSPStone();
					}
				}

				if (GetAsyncKeyState(VK_F1)){
					playerID[0] = 0xFF;
					playerID[1] = 0xFF;
					Sleep(1000);
					break;
				}

				if (GetAsyncKeyState(VK_F2)){

					for (int i = 0; i < 20; i++){
						cout << "Enter new Mob ID " << i << ": ";
						cin >> mobID[i];
						getchar();

						if (mobID[i] == 0){
							for (int j = i + 1; j < 20; j++){
								mobID[j] = 0;
							}
							break;
						}
					}

					break;
				}

				if (GetAsyncKeyState(VK_F3)){

					for (int i = 0; i < 20; i++){
						cout << "Enter new optional Mob ID " << i << ": ";
						cin >> optionalMobID[i];
						getchar();

						if (optionalMobID[i] == 0){
							for (int j = i + 1; j < 20; j++){
								optionalMobID[j] = 0;
							}
							break;
						}
					}

					break;
				}

				if (GetAsyncKeyState(VK_F4)){

					cout << "X: " << *playerX << "\tY: " << *playerY << endl;
					Sleep(300);
				}

				if (GetAsyncKeyState(VK_F5)){

					cout << "Type new kill radius: ";
					cin >> killRadius;
					getchar();
					Sleep(300);
				}

				if (GetAsyncKeyState(VK_F6)){

					for (int i = 0; i < 20; i++){
						cout << "Enter new loot table" << i << ": ";
						cin >> lootTable[i];
						getchar();

						if (lootTable[i] == 0){
							for (int j = i + 1; j < 20; j++){
								lootTable[j] = 0;
							}
							break;
						}
					}

					break;
				}
			}

		}
		//Sleep(1000);
	}

	entityPointer = *(DWORD*)(fiestaBase + 0x703B10) + 0x04;
	DWORD entity = *(DWORD*)entityPointer;
	DWORD entities[1024] = { 0 };
	DWORD entityName, entityID;
	char name[30] = { 0 };
	DWORD lastEntity = 0;

	FreeLibraryAndExitThread((HMODULE)param, 0);

	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		AllocConsole();
		//freopen("CONOUT$", "w", stdout);
		freopen_s(&pFile_o, "CONOUT$", "w", stdout);
		freopen_s(&pFile_o, "CONIN$", "r", stdin);
		HookFunctionName("ws2_32.dll", "send", (LPVOID*)nSend, hook);
		HookFunctionName2("ws2_32.dll", "recv", (LPVOID*)nRecv, hook2);
		CreateThread(0, 0, MainThread, hModule, 0, 0);
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
