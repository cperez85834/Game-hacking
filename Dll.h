#include "declarations.h"
#include "keyboard_functions.h"
#include "mouse_functions.h"
using namespace std;

//Start timer function, will start counting ticks until told to stop
void timerStart(){

	QueryPerformanceFrequency(&timerTcounter);
	timerFreq = timerTcounter.QuadPart;

	QueryPerformanceCounter(&timerTcounter);
	timerInitialTick = timerTcounter.QuadPart;
	timerElapsed = timerTcounter.QuadPart;
}

//Watch timer function, argument is the amount of time in seconds to wait after timerStart() is called before returning true.
//Useful for waiting for an event without using Sleep()
bool timerWatch(double duration){
	//LARGE_INTEGER timerTcounter;
	//cout << "bup\n";
	QueryPerformanceCounter(&timerTcounter);
	//cout << "bup\n";
	timerElapsed = timerTcounter.QuadPart;
	//cout << "bup\n";
	if ((timerElapsed - timerInitialTick) / (timerFreq / 1000) >= duration * 1000){
		return 1;
	}
	//cout << "bup\n";
	return 0;
}

//Checks to see if program memory is readable before accessing it, otherwise game crashes with ACCESS_VIOLATION
bool isMemReadable(LPCVOID memory, int bytes){
	
	BYTE membuf_byte;
	WORD membuf_word;
	DWORD membuf_dword;

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetCurrentProcessId());
	int return_status = 0;
	if (bytes == 1) return_status = ReadProcessMemory(hProcess, memory, &membuf_byte, sizeof(BYTE), NULL);
	else if (bytes == 2) return_status = ReadProcessMemory(hProcess, memory, &membuf_word, sizeof(WORD), NULL);
	else if (bytes == 4) return_status = ReadProcessMemory(hProcess, memory, &membuf_dword, sizeof(DWORD), NULL);
	CloseHandle(hProcess);

	if (return_status > 0) return true;
	else{
		cout << "RPM failed at " << hex << memory << " with " << GetLastError() << endl;
		return false;
	}
}

//Test function for finding child windows of game program, not really used
BOOL CALLBACK EnumChildProc(HWND hwnd, LPARAM lParam) {

		//cout << "hwnd_Child = " << hwnd << endl;
	for (int i = 0; i < 500; i++){
		if (hwndList[i] == 0){
			hwndList[i] = hwnd;
			break;
		}
	}
		return TRUE; // must return TRUE; If return is FALSE it stops the recursion
}

//Calculates distance between two objects in game. No Z coordinate because Fiesta Online doesn't really utilize it for anything... jumping in game is simpling a "jump animation",
//as opposed to actually increasing your Z value
float distanceFunc(float x1, float y1, float x2, float y2){
	return sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2));
}

//Gets base pointer of game program.
DWORD getBasePointer(){
	//FILE *pFile = nullptr;
	//AllocConsole();
	//freopen_s(&pFile, "CONOUT$", "w", stdout);
	//freopen_s(&pFile, "CONIN$", "r", stdin);

	HWND WindowHandle = FindWindow(nullptr, L"FiestaOnline");
	DWORD PID;
	GetWindowThreadProcessId(WindowHandle, &PID);
	PVOID hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, 0, PID);
	//cout << hProcess;
	HMODULE hMods[1024];
	//	HANDLE pHandle = GetHandle();
	DWORD cbNeeded;
	unsigned int i;

	if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded))
	{
		for (i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
		{
			cout << i << endl;
			TCHAR szModName[MAX_PATH];
			if (GetModuleFileNameEx(hProcess, hMods[i], szModName, sizeof(szModName) / sizeof(TCHAR)))
			{
				//cout << "HEY: " << szModName;
				wstring wstrModName = szModName;
				//you will need to change this to the name of the exe of the foreign process
				wstring wstrModContain = L"Fiesta.exe";
				if (wstrModName.find(wstrModContain) != string::npos)
				{
					CloseHandle(WindowHandle);
					//cout << "HEY: " << (DWORD)hMods[i];
					return (DWORD)hMods[i];
				}
			}
		}
	}

	return 0;
}

/*DllClass::DllClass()
{

}


DllClass::~DllClass()
{

}*/

//Removes camera zoom limit so it can be as far out as you like
void toggleZoom(){
	unsigned char backup[10] = { 0 };
	DWORD prevBP;

	ReadProcessMemory(GetCurrentProcess(), (LPVOID)zoom, backup, 1, 0);
	cout << "\nBACKUP: " << (short)backup[0] << endl;
	if (backup[0] == 0x7A){
		VirtualProtect((void*)zoom, 1, PAGE_EXECUTE_READWRITE, &prevBP);
		memset((void*)zoom, (BYTE)0xEB, 1);
		VirtualProtect((void*)zoom, 1, prevBP, &prevBP);
	}
	else if (backup[0] == 0xEB){
		VirtualProtect((void*)zoom, 1, PAGE_EXECUTE_READWRITE, &prevBP);
		memset((void*)zoom, (BYTE)0x7A, 1);
		VirtualProtect((void*)zoom, 1, prevBP, &prevBP);
	}
}

//Spams the last packet sent
void spamLast(){
	char *packet = new char[lastPacket[0]];
	char *sendPacket = new char[lastPacket[0]];

	packet[0] = lastPacket[0] - 1;
	for (int i = 1; i < lastPacket[0]; i++){
		packet[i] = lastPacket[i];
		cout << hex << (unsigned)(packet[i]) << endl;
	}
	sendPacket[0] = packet[0];
	WORD *assman = (WORD*)encryptCounterstatic;

	while (1){
		for (int i = 1; i < lastPacket[0]; i++){
			if (*assman == 0x1F3) *assman = 0;
			sendPacket[i] = packet[i] ^ encryptionTable[*assman];
			(*assman)++;
		}
		send(sendSocket_r, sendPacket, lastPacket[0], 0);
		//Sleep(1000);
	}
}

//Dummy bypass test that attempts to circumvent anticheat protection by patching game process before Xigncode can load.
void dummyBP(){
	DWORD fiestaBasey = (DWORD)GetModuleHandleA("Fiesta.exe");
	BYTE BPbytes[6] = { 0x74, 0x75, 0x74, 0x90, 0x75, 0x74 };
	DWORD prevBP;
	DWORD addys[6] = { fiestaBasey + 0x6050E, fiestaBasey + 0x6051E, fiestaBasey + 0x60527, fiestaBasey + 0x60552, fiestaBasey + 0x60556, fiestaBasey + 0x601FE };

	for (int i = 0; i < 6; i++){
		if (i == 3){
			VirtualProtect((void*)addys[i], 2, PAGE_EXECUTE_READWRITE, &prevBP);
			memset((void*)addys[i], BPbytes[i], 2);
			VirtualProtect((void*)addys[i], 2, prevBP, &prevBP);
		}
		else{
			VirtualProtect((void*)addys[i], 1, PAGE_EXECUTE_READWRITE, &prevBP);
			memset((void*)addys[i], BPbytes[i], 1);
			VirtualProtect((void*)addys[i], 1, prevBP, &prevBP);
		}
	}
}

//These functions specify what module and function to hook
DWORD HookFunctionName(LPCSTR lpModule, LPCSTR lpFuncName, LPVOID lpFunction, unsigned char *lpBackup)
{
	DWORD dwAddr = (DWORD)GetProcAddress(GetModuleHandleA(lpModule), lpFuncName);
	ReadProcessMemory(GetCurrentProcess(), (LPVOID)dwAddr, lpBackup, 6, 0);
	DWORD dwCalc = ((DWORD)lpFunction - dwAddr - 5);
	VirtualProtect((void*)dwAddr, 6, PAGE_EXECUTE_READWRITE, &pPrevious);
	memcpy(&jmp[1], &dwCalc, 4);
	WriteProcessMemory(GetCurrentProcess(), (LPVOID)dwAddr, jmp, 6, 0);
	VirtualProtect((void*)dwAddr, 6, pPrevious, &pPrevious);
	FlushInstructionCache(GetCurrentProcess(), 0, 0);
	return dwAddr;
}
DWORD HookFunctionName2(LPCSTR lpModule, LPCSTR lpFuncName, LPVOID lpFunction, unsigned char *lpBackup)
{
	DWORD dwAddr = (DWORD)GetProcAddress(GetModuleHandleA(lpModule), lpFuncName);
	ReadProcessMemory(GetCurrentProcess(), (LPVOID)dwAddr, lpBackup, 6, 0);
	DWORD dwCalc = ((DWORD)lpFunction - dwAddr - 5);
	VirtualProtect((void*)dwAddr, 6, PAGE_EXECUTE_READWRITE, &pPrevious);
	memcpy(&jmp2[1], &dwCalc, 4);
	WriteProcessMemory(GetCurrentProcess(), (LPVOID)dwAddr, jmp2, 6, 0);
	VirtualProtect((void*)dwAddr, 6, pPrevious, &pPrevious);
	FlushInstructionCache(GetCurrentProcess(), 0, 0);
	return dwAddr;
}
DWORD HookFunctionName3(LPCSTR lpModule, LPCSTR lpFuncName, LPVOID lpFunction, unsigned char *lpBackup)
{
	DWORD dwAddr = (DWORD)GetProcAddress(GetModuleHandleA(lpModule), lpFuncName);
	ReadProcessMemory(GetCurrentProcess(), (LPVOID)dwAddr, lpBackup, 6, 0);
	DWORD dwCalc = ((DWORD)lpFunction - dwAddr - 5);
	VirtualProtect((void*)dwAddr, 6, PAGE_EXECUTE_READWRITE, &pPrevious);
	memcpy(&jmp3[1], &dwCalc, 4);
	WriteProcessMemory(GetCurrentProcess(), (LPVOID)dwAddr, jmp3, 6, 0);
	VirtualProtect((void*)dwAddr, 6, pPrevious, &pPrevious);
	FlushInstructionCache(GetCurrentProcess(), 0, 0);
	return dwAddr;
}
bool HookFunctionAddy(void *toHook, void *ourFunct, int len)
{
	if (len < 5){
		return false;
	}

	DWORD curProtection;
	VirtualProtect(toHook, len, PAGE_EXECUTE_READWRITE, &curProtection);

	memset(toHook, 0x90, len);

	DWORD relativeAddress = ((DWORD)ourFunct - (DWORD)toHook) - 5;

	*(BYTE*)toHook = 0xE9;
	*(DWORD*)((DWORD)toHook + 1) = relativeAddress;

	DWORD temp;
	VirtualProtect(toHook, len, curProtection, &temp);
	return true;
}

//Unhook hooked functions
BOOL UnHookFunction(LPCSTR lpModule, LPCSTR lpFuncName, unsigned char *lpBackup)
{
	DWORD dwAddr = (DWORD)GetProcAddress(GetModuleHandleA(lpModule), lpFuncName);

	if (WriteProcessMemory(GetCurrentProcess(), (LPVOID)dwAddr, lpBackup, 6, 0))
		return TRUE;
	FlushInstructionCache(GetCurrentProcess(), 0, 0);

	return FALSE;
}

//Not used, but used to monitor the connect function in ws2_32.dll connect function
int __stdcall nConnect(SOCKET s, const sockaddr *name, int namelen){
	UnHookFunction("ws2_32.dll", "connect", hook3);
	int result = 0;

	result = connect(s, name, namelen);
	if (result){
		myfileSend.open("C:\\tmp\\log.txt", ios::app | ios::binary);
		myfileSend << "Connection established:\nSocket: " << s << "\nFamily: " << name->sa_family << "\nData: ";
		for (int i = 0; i < 14; i++){
			cout << hex << (short)name->sa_data[i] << ' ';
		}
		/*for (int i = 0; i < len; i++){
		if ((short)buf[i] <= 15 && (short)buf[i] >= 0) myfileSend << '0'; //print zero on numbers 0 - f

		if ((short)buf[i] < 0) x = 127 + (buf[i] + 129);
		else x = buf[i];
		myfileSend << hex << x << ' ';

		if (i != 0 && (i + 1) % 16 == 0 && i != len - 1){
		myfileSend << '\t';
		for (int y = i - 15; y <= i; y++){
		if (((short)buf[y] >= 0 && (short)buf[y] <= 31) || (short)buf[y] == 127) myfileSend << '.';
		else myfileSend << buf[y];
		}
		myfileSend << '\n';
		}
		}
		myfileSend << '\t';
		for (int i = len - (len % 16); i <= len; i++){
		myfileSend << buf[i];
		}*/
		myfileSend << '\n';
		myfileSend.close();


	}

	HookFunctionName("ws2_32.dll", "connect", (LPVOID*)nConnect, hook3);
	return result;
}


//Hooks the send function in ws2_32.dll; this is how the game client communicates with the server and hooking the function
//allows me to modify or inject packets to send as I see fit
int __stdcall nSend(SOCKET s, const char *buf, int len, int flags){
	UnHookFunction("ws2_32.dll", "send", hook);

	unsigned int x = 0;

	myfileSend.open("C:\\tmp\\log.txt", ios::app | ios::binary);
	myfileSend << "SEND ON SOCKET: " << s << "\n";
	if (len <= 15 && len >= 0) myfileSend << '0';
	myfileSend << hex << len - 1 << ' ';
	for (int i = 0; i < len - 1; i++){
		if ((short)packetByte_socket[i] <= 15 && (short)packetByte_socket[i] >= 0) myfileSend << '0'; //print zero on numbers 0 - f

		if ((short)packetByte_socket[i] < 0) x = 127 + (packetByte_socket[i] + 129);
		else x = packetByte_socket[i];
		myfileSend << hex << x << ' ';

		if (i != 0 && (i + 1) % 16 == 0 && i != len - 1){
			myfileSend << '\t';
			for (int y = i - 15; y <= i; y++){
				if (((short)packetByte_socket[y] >= 0 && (short)packetByte_socket[y] <= 31) || (short)packetByte_socket[y] == 127) myfileSend << '.';
				else myfileSend << packetByte_socket[y];
			}
			myfileSend << '\n';
		}
	}
	myfileSend << '\t';
	for (int i = len - (len % 16); i <= len; i++){
		myfileSend << packetByte_socket[i];
	}
	myfileSend << '\n';
	myfileSend.close();

	int result = 0;

	if (setup == false){
		if (len == 0x43 && packetByte_socket[0] == 0x65){
			char* bytesToSend = new char[len];
			bool sixteenMult = true;
			int byteCounter = 1;
			bytesToSend[0] = len - 1;
			DWORD* fucker = (DWORD*)lace;

			for (int i = 1; i < len; i++){
				bytesToSend[i] = binSpoof[i - 1];
			}
			int assman = *(WORD*)lace;

			assman -= (len - 1);
			if (assman < 0){
				assman += 0x1F3;
			}

			for (int i = 1; i < len; i++){
				if (assman == 0x1F3) assman = 0;
				bytesToSend[i] = bytesToSend[i] ^ encryptionTable[assman];
				assman++;
			}

			result = send(s, bytesToSend, len, 0);
			delete bytesToSend;
			newByte = 0;
			//setup = true;
			HookFunctionName("ws2_32.dll", "send", (LPVOID*)nSend, hook);
			sending = false;
			return result;
		}
	}

	result = send(s, buf, len, flags);
	//cout << len << "\t" << (short)packetByte_socket[0] << "\t" << (short)packetByte_socket[1] << "\t" << sendSocket_r << endl;
	if (len == 0x05 && packetByte_socket[0] == 0x2d && packetByte_socket[1] == 0x20){ //send instant gather
		WORD* assman = (WORD*)encryptCounterstatic;
		char* bytesToSend = new char[3];
		bytesToSend[0] = 0x02;
		bytesToSend[1] = 0x32;
		bytesToSend[2] = 0x20;
		cout << "WHY :(\n";
		for (int i = 1; i < 3; i++){
			if (*assman == 0x1F3) assman = 0;
			bytesToSend[i] = bytesToSend[i] ^ encryptionTable[*assman];
			(*assman)++;
		}
		cout << "HEY :)\n";
		//Sleep(1000);
		send(s, bytesToSend, 3, 0);

		delete bytesToSend;
	}
	//f 01 20 00 0b 26 61 64 6d 69 6e 6c 65 76 65 6c
	//this packet is sent when map is successfully switched
	if (len == 0x10 && packetByte_socket[0] == 0x01 && packetByte_socket[1] == 0x20 && packetByte_socket[2] == 0x00 &&
		packetByte_socket[3] == 0x0b && packetByte_socket[4] == 0x26){
		mapSwitch = true;
	}
	//cout << "Len: " << len << "\tPacketByte 0: " << packetByte_socket[0] << "\tPacketByte 1: " << packetByte_socket[1] << endl;
	//myfileSend.open("C:\\tmp\\log.txt", ios::app | ios::binary);
	//myfileSend << (unsigned short)packetByte_socket[0] << endl;
	if (len == 3){
		if (packetByte_socket[0] == 0x24 && packetByte_socket[1] == 0x20){
			sendSocket_r = s; //look for jump packet
			encryptCounterAddy = (DWORD*)lace;
			encryptCounterstatic = (DWORD*)lace;
			initialX = *playerX;
			initialY = *playerY;
		}
	}

	if (len == 5){ //get target id
		if (packetByte_socket[0] == 0x01 && packetByte_socket[1] == 0x24){
			partyMemberID[0] = packetByte_socket[2];
			partyMemberID[1] = packetByte_socket[3];
		}
	}

	if (len == 3 && sendSocket_r_Invite == 0){
		if (packetByte_socket[0] == 0x04 && packetByte_socket[1] == 0x70){
			sendSocket_r_Invite = s;
			encryptCounterAddy_Invite = (DWORD*)lace;
			encryptCounterstatic_Invite = (DWORD*)lace;
		}
	}

	lastPacket[0] = len;
	for (int i = 0; i < newByte; i++){
		lastPacket[i + 1] = packetByte_socket[i];
		packetByte_socket[i] = 0;
	}

	newByte = 0;


	HookFunctionName("ws2_32.dll", "send", (LPVOID*)nSend, hook);
	sending = false;
	return result;
}
//DirectInput

int WINAPI nDI8Create(HINSTANCE hinst,
	DWORD dwVersion,
	REFIID riidltf,
	LPVOID * ppvOut,
	LPUNKNOWN punkOuter){
	UnHookFunction("dinput8", "DirectInput8Create", hook3);

	cout << "nDI8Create is hooked!" << endl;
	HookFunctionName3("dinput8.dll", "DirectInput8Create", (LPVOID*)nDI8Create, hook3);
	return DirectInput8Create(hinst, dwVersion, riidltf, ppvOut, punkOuter);
}


//Send a packet using the game's encryption method
void sendCrypt(SOCKET s, char* buf, int len, int flags){
	char* buffy = new char[len];

	for (int i = 0; i < len; i++){
		buffy[i] = buf[i];
	}
	while (sending == true);
	sending = true;
	WORD *assman = NULL;
	if (flags == 0) assman = (WORD*)encryptCounterstatic;
	else if (flags == 1) assman = (WORD*)encryptCounterstatic_Invite;

	for (int i = 1; i < len; i++){
		packetByte_socket[i - 1] = buffy[i];
		//cout << hex << "Assman is: " << *assman << endl;
		buffy[i] = buffy[i] ^ encryptionTable[*assman];
		(*assman)++;
		if (*assman == 0x1F3) *assman = 0;

	}
	send(s, buffy, len, 0);
	//while (sending == true);
	if (SLOW == 1) Sleep(1);
	delete buffy;
}

//Use a mount
void toggleMount(){
	char packet[5] = { 0x04, 0x15, 0x30, 0x00, 0x09 };
	sendCrypt(sendSocket_r, packet, 5, 0);

	Sleep(4800);
}

//Use a mana stone
void useSPStone(){
	bool used = 0;
	LARGE_INTEGER tcounter;
	LONGLONG ticknow;
	LONGLONG freq;
	QueryPerformanceFrequency(&tcounter);
	freq = tcounter.QuadPart;

	QueryPerformanceCounter(&tcounter);
	ticknow = tcounter.QuadPart;

	if ((ticknow - spstoneCooldown) / (freq / 1000) <= 7500) return;

	char packet[3] = { 0x02, 0x09, 0x50 };

	sendCrypt(sendSocket_r, packet, 3, 0);

	QueryPerformanceCounter(&tcounter);
	spstoneCooldown = tcounter.QuadPart;
}

//Use a health potion
void useHPPotion(int slot){
	bool used = 0;
	LARGE_INTEGER tcounter;
	LONGLONG ticknow;
	LONGLONG freq;
	QueryPerformanceFrequency(&tcounter);
	freq = tcounter.QuadPart;

	QueryPerformanceCounter(&tcounter);
	ticknow = tcounter.QuadPart;

	if ((ticknow - potionCooldown) / (freq / 1000) <= 3200) return;

	char packet[5] = { 0x04, 0x15, 0x30, slot, 0x09 };
	sendCrypt(sendSocket_r, packet, 5, 0);
	QueryPerformanceCounter(&tcounter);
	potionCooldown = tcounter.QuadPart;
}

//use a health stone
void useHPStone(){
	bool used = 0;
	LARGE_INTEGER tcounter;
	LONGLONG ticknow;
	LONGLONG freq;
	QueryPerformanceFrequency(&tcounter);
	freq = tcounter.QuadPart;

	QueryPerformanceCounter(&tcounter);
	ticknow = tcounter.QuadPart;

	if ((ticknow - hpstoneCooldown) / (freq / 1000) <= 7500) return;

	char packet[3] = { 0x02, 0x07, 0x50 };

	sendCrypt(sendSocket_r, packet, 3, 0);
	QueryPerformanceCounter(&tcounter);
	hpstoneCooldown = tcounter.QuadPart;
}

DWORD WINAPI Console(){
	//char packetBuf[255] = { 0 };
	//	char testPacket[16] = { 0x0F, 0x01, 0x20, 0x00, 0x0B, 0x48, 0x65, 0x6C, 0x6C, 0x6F, 0x20, 0x57, 0x6F, 0x72, 0x6C, 0x64 };
	/*char testPacket[37] = { 0x24, 0x0C, 0x20, 0x00, 0x47, 0x61, 0x68, 0x68, 0x68, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x48, 0x65, 0x79, 0x20, 0x6C, 0x6F, 0x73, 0x65,
	0x72, 0x40, 0x40, 0x40 };*/
	int bufferLen = 0;
	bool sixteenMult = false, repeat = false;
	int byteCounter = 0;
	//char testPacket[bufferLen] = { 0x03, 0x1d, 0x20, 0x01 };
	//char encryptedPacket[bufferLen] = { 0 };
	FILE *pFile = nullptr;
	//unsigned int sendSocket_r = 0;
	unsigned int recvSocket_r = 0;
	WORD countercopy;

	//AllocConsole();
	//freopen("CONOUT$", "w", stdout);
	//freopen_s(&pFile, "CONOUT$", "w", stdout);
	//freopen_s(&pFile, "CONIN$", "r", stdin);
	cout << "Waiting for base pointer...\n";
	while (fiestaBase == 0) Sleep(1000);
	cout << "Base found! Address: " << hex << fiestaBase << endl << endl << endl;

	unsigned int recvSocket = 0;

	/*while (1){
	cin >> recvSocket_r;
	getchar();

	cout << "\n\nType a packet to receive:\n";
	getline(cin, packetToRecv);
	bufferLen = packetToRecv.length() / 2;

	char* bytesToRecv = new char[bufferLen];
	sixteenMult = true;
	cout << packetToRecv.length() << endl;
	for (int i = 0; i < packetToRecv.length(); i++){
	if (packetToRecv[i] >= 0x30 && packetToRecv[i] <= 0x39){ //numbers
	if (sixteenMult == true){
	bytesToRecv[byteCounter] = (packetToRecv[i] - 0x30) * 0x10;
	sixteenMult = false;
	}
	else{
	bytesToRecv[byteCounter] += (packetToRecv[i] - 0x30);
	sixteenMult = true;
	byteCounter++;
	}
	}
	if (packetToRecv[i] >= 0x41 && packetToRecv[i] <= 0x5A){ //caps
	if (sixteenMult == true){
	bytesToRecv[byteCounter] = (packetToRecv[i] - 0x37) * 0x10;
	sixteenMult = false;
	}
	else{
	bytesToRecv[byteCounter] += (packetToRecv[i] - 0x37);
	sixteenMult = true;
	byteCounter++;
	}
	}
	if (packetToRecv[i] >= 0x61 && packetToRecv[i] <= 0x7A){// lowers
	if (sixteenMult == true){
	bytesToRecv[byteCounter] = (packetToRecv[i] - 0x57) * 0x10;
	sixteenMult = false;
	}
	else{
	bytesToRecv[byteCounter] += (packetToRecv[i] - 0x57);
	sixteenMult = true;
	byteCounter++;
	}
	}
	}

	recv(recvSocket_r, bytesToRecv, bufferLen, 0);
	}*/



	//cout << "Jump in game first to get socket\n";
	while (sendSocket_r == 0) Sleep(1000);
	//cin >> sendSocket_r;
	//cout << "\nSocket is: " << sendSocket_r;
	//getchar();
	//encryptCounterAddy = (DWORD*)lace;
	//encryptCounterstatic = (DWORD*)lace;
	//cout << "SOCKET: " << sendSocket_r << endl << "Counter Addy: " << hex << encryptCounterAddy << endl << "Counter value: " << *(WORD*)encryptCounterAddy;
	//getchar();
	//*(WORD*)encryptCounterAddy = *(WORD*)encryptCounterstatic;
	countercopy = *(WORD*)encryptCounterAddy;
	bool socketType = 0;
	while (1){
		string packetToSend, packetToRecv;
		if (repeat == false){
			cout << "\n\nType a packet to send:\n";
			getline(cin, packetToSend);
			bufferLen = packetToSend.length() / 2;
			repeat = true;
			//cout << "Type of socket: ";
			//cin >> socketType;
			//getchar();
			//cin >> sendSocket_r;
			//getchar();
			//Sleep(500);
		}
		char* bytesToSend = new char[bufferLen];
		sixteenMult = true;
		cout << packetToSend.length() << endl;
		for (int i = 0; i < packetToSend.length(); i++){
			if (packetToSend[i] >= 0x30 && packetToSend[i] <= 0x39){ //numbers
				if (sixteenMult == true){
					bytesToSend[byteCounter] = (packetToSend[i] - 0x30) * 0x10;
					sixteenMult = false;
				}
				else{
					bytesToSend[byteCounter] += (packetToSend[i] - 0x30);
					sixteenMult = true;
					byteCounter++;
				}
			}
			if (packetToSend[i] >= 0x41 && packetToSend[i] <= 0x5A){ //caps
				if (sixteenMult == true){
					bytesToSend[byteCounter] = (packetToSend[i] - 0x37) * 0x10;
					sixteenMult = false;
				}
				else{
					bytesToSend[byteCounter] += (packetToSend[i] - 0x37);
					sixteenMult = true;
					byteCounter++;
				}
			}
			if (packetToSend[i] >= 0x61 && packetToSend[i] <= 0x7A){// lowers
				if (sixteenMult == true){
					bytesToSend[byteCounter] = (packetToSend[i] - 0x57) * 0x10;
					sixteenMult = false;
				}
				else{
					bytesToSend[byteCounter] += (packetToSend[i] - 0x57);
					sixteenMult = true;
					byteCounter++;
				}
			}
		}
		//Sleep(1000);
		//getchar();
		while (1){
			if (socketType == 0)
				sendCrypt(sendSocket_r, bytesToSend, bufferLen, 0);

			if (socketType == 1)
				sendCrypt(sendSocket_r_Invite, bytesToSend, bufferLen, 1);

			if (GetAsyncKeyState(VK_F5)){
				repeat = false;
				//cout << "Repeating stopped\n";
				//Sleep(500);
				break;
			}
			if (repeat == false) break;

			//Sleep(1000);
		}

		delete bytesToSend;

	}
	return 0;
}

//Packet encryption hooks
void __declspec(naked) preEncrypt(){

	__asm{
	loopydoop:  cmp sending, 1
				je loopydoop
				mov sending, 1
				push ebx
				push esi
				push edi
				mov edi, dword ptr ss : [ebp + 8]
				jmp jmpBackEncryptRNG
	}
}
void __declspec(naked) ourFunct(){

	__asm{
		//movzx esi, word ptr ds : [ecx]
		//mov bl, byte ptr ds : [esi+0x10B32E0]
		push eax
			push ebx
			push ecx
			push edx
			mov ecx, 0
			mov eax, [edx + edi - 1]
			cmp newByte, 0
			jne normal
			//add check to see if a packet is being sent by custom function, if it is make it wait
		county : cmp cl, al
				 je normal
				 mov ebx, dword ptr ds : [edx + edi]
				 mov[packetByte + ecx], bl
				 mov[packetByte_socket + ecx], bl
				 inc ecx
				 inc edx
				 inc newByte
				 jmp county
				 //mov ebx, [newByte]
				 //mov[packetByte + ebx], al
				 //inc newByte
				 ///mov eax, dword ptr ds : [edx + edi]
				 ///mov ebx, 0
				 ///mov [packetByte + ebx], al
				 ///inc newByte
			 normal: pop edx
					 pop ecx
					 lea ebx, dword ptr[ecx]
					 mov lace, ebx
					 mov bx, word ptr[ecx]
					 mov lace_counter, bx
					 pop ebx
					 pop eax
					 xor byte ptr ds : [edx + edi], bl
					 inc word ptr ds : [ecx]
					// mov ebx, 0x1F3
					 //cmp word ptr ds : [ecx],bx
					 //jb hehe
					 //xor esi, esi
					 //mov word ptr ds : [ecx],si
				hehe://inc edx
					 //cmp edx, eax
					 //jge hoho
					 //jmp
					// mov sending,0
			hoho:	 jmp[jmpBackAddy]
	}
}

//Not used. Would hook the movement function and allow me to specify my own coordinates to move to
void __declspec(naked) moveFunct(){

	__asm{
		cmp hijackMove, 0
			je normal
			mov edx, moveX
			mov[ecx + 0x00001038], edx
			mov edx, moveY
			mov[ecx + 0x0000103C], edx
			jmp jmpBackAddyMove
		normal : mov edx, [eax]
				 mov[ecx + 0x00001038], edx
				 mov edx, [eax + 04]
				 mov[ecx + 0x0000103C], edx
				 jmp jmpBackAddyMove
	}
}

int WINAPI nRecv(SOCKET s, char* buf, int len, int flags)
{
	UnHookFunction("ws2_32.dll", "recv", hook2);
	//DWORD tmp;

	len = recv(s, buf, len, flags);
	//16 03 38 42 61 6e 61 6e 61 57 61 6c 72 75 73 00 ..8BananaWalrus. recv it
	//00 00 00 00 00 00 00 x

	if (len == 0x314 && buf[1] == 0x11 && buf[2] == 0x03 && buf[3] == 0x14 && buf[4] == 0x0c) characterSelect = true;
	if (len > 0 && buf[0] == 0x16 && buf[1] == 0x03 && buf[2] == 0x38 && receivedPInvite == false){
		receivedPInvite = true;
	}
		//cout << "Hey got invite... see you..." << endl;
	if (len > 0 && buf[0] == 0x4b && buf[1] == 0x0a && buf[2] == 0x0c){
		serverPick = true;
	}

	if ( len > 0 && buf[1] == 0x0c && buf[2] == 0x6c){ //find dialogue packet...
		
		if (buf[0] == 0x3d){
			if (buf[39] == 0x70 && buf[40] == 0x6c && buf[41] == 0x61 && buf[42] == 0x79) coffinOrBox = true; //"play"
		}

		else if (buf[0] == 0x37){
			if (buf[34] == 0x74 && buf[35] == 0x68 && buf[36] == 0x72 && buf[37] == 0x65 && buf[38] == 0x61 && buf[39] == 0x74) coffinOrBox = true; //"threat"
		}

		receivedDialogue = true;
	}

	if (len > 0 && buf[1] == 0x11 && buf[2] == 0x20 && buf[26] == 0x70 && buf[27] == 0x61 && buf[28] == 0x72 && buf[29] == 0x74 && buf[30] == 0x79){ //[notice]
		for (int w = 0; w < len - 1; w++){
			if (buf[w] == 0x11 && buf[w + 1] == 0x20){
				if (w + 29 < len){
					if (buf[w + 3] == 0x24 && buf[w + 25] == 0x70 && buf[w + 26] == 0x61 && buf[w + 27] == 0x72 && buf[w + 28] == 0x74 && buf[w + 29] == 0x79) notInParty = true; //"must be in party" notice
				}

				if (w + 60 < len){
					if (buf[w + 3] == 0x3d && buf[w + 57] == 0x66 && buf[w + 58] == 0x75 && buf[w + 59] == 0x6c && buf[w + 60] == 0x6c) ccFull = true; // "instance is full"
				}
			}
		}
	}

	if (len > 30 && ccMessageBox == false){ //message box
		for (int w = 0; w < len - 30; w++){
			//if (buf[w] == 0xd2 && buf[w + 1] == 0x01 && buf[w + 2] == 0x3c){
			if (buf[w] == 0x01 && buf[w + 1] == 0x3c){
				//cout << "YEET" << endl;
				if (buf[w + 25] == 0x43 && buf[w + 26] == 0x72 && buf[w + 27] == 0x79 && buf[w + 28] == 0x73 && buf[w + 29] == 0x74){ //enter CC message box
					//cout << "FUCK" << endl;
					ccMessageBox = true;
					break;
				}

				if (buf[w + 2] == 0x54 && buf[w + 3] == 0x68 && buf[w + 4] == 0x65 && buf[w + 5] == 0x72 && buf[w + 6] == 0x65){ //theres no1 inside
					nooneMessageBox = true;
					break;
				}
				
			}
		}
	}

	//04 16 20 c0 07 1b 15 20 42 61 6c 6c 73 79 00 00 	..À... Ballsy..
	//00 00 00 00 00 00 00 00 00 00 00 00 00 03 6f 75
	//74 t 
//	if (len > 0 && buf[1] == 0x15 && buf[2] == 0x20){ //party message
	if (len > 0){
		for (int w = 0; w < len - 24; w++){
			if (buf[w] == 0x15 && buf[w + 1] == 0x20){
				int messageSize = buf[w + 23];
				int packetSize = buf[w - 1];

				if (w + packetSize <= len && packetSize - (messageSize + 1) == 23){ //checking packet is of approrpriate length so i don't read out of buffer
					if (messageSize == 0x02 && buf[w + 23 + 1] == 'i' && buf[w + 23 + 2] == 'n') goinCC = true;
					if (messageSize == 0x03 && buf[w + 23 + 1] == 'o' && buf[w + 23 + 2] == 'u' && buf[w + 23 + 3] == 't') gooutCC = true;
					if (messageSize == 0x02 && buf[w + 23 + 1] == 'u' && buf[w + 23 + 2] == 'p') moveUP = true;
					if (messageSize == 0x03 && buf[w + 23 + 1] == 'a' && buf[w + 23 + 2] == 'o' && buf[w + 23 + 3] == 'e') castAoe = true;
					if (messageSize == 0x02 && buf[w + 23 + 1] == 'g' && buf[w + 23 + 2] == 'o') startAttack = true;
					if (messageSize == 0x04 && buf[w + 23 + 1] == 'l' && buf[w + 23 + 2] == 'o' && buf[w + 23 + 3] == 'o'  && buf[w + 23 + 4] == 't') startLoot = true;
					if (messageSize == 0x07 && buf[w + 23 + 1] == 'l' && buf[w + 23 + 2] == 'o' && buf[w + 23 + 3] == 'o'  && buf[w + 23 + 4] == 't'   && buf[w + 23 + 5] == 'a'   && buf[w + 23 + 6] == 'l'   && buf[w + 23 + 7] == 'l'){
						restrictLoot = false;
						startLoot = true;
					}

				}
			}
		}

	}
	//if (len > 0 && buf[1] == 0x07 && buf[2] == 0x38 && buf[23] == 0x1a){
	if (len > 0){
		for (int w = 0; w < len - 23; w++){
			if (buf[w] == 0x07 && buf[w + 1] == 0x38 && buf[w + 22] == 0x1a){
				cout << "YEET" << endl;
				acceptedPInvite = true;
			}
		}
	}
	unsigned short x = 0;
	if (len > 0)
	{
		bool stop = false;
		for (int j = 0; j < len; j++){
			if (buf[j] == 0x02){ //normal chat message
				if (j + 8 >= len) break;
				if (buf[j + 1] == 0x20 && buf[j + 2] == 0x00 && buf[j + 7] == 0x00 && buf[j + 8] == 0x00){

					for (int i = 0x09; i < 0x09 + buf[j + 5]; i++){
						if (buf[j + i] == 0x42 || buf[j + i] == 0x62){
							if (j + i + 3 >= len) break;
							if (buf[j + i + 1] == 0x55 || buf[j + i + 1] == 0x75){
								if (buf[j + i + 2] == 0x46 || buf[j + i + 2] == 0x66){
									if (buf[j + i + 3] == 0x46 || buf[j + i + 3] == 0x66){
										IDtoBYTE playerToBuff;
										playerToBuff.bytes[0] = buf[j + 3];
										playerToBuff.bytes[1] = buf[j + 4];
										cout << "haha\n";
										
										cout << hex << static_cast<unsigned>(buf[j + 3]) << " " << static_cast<unsigned>(buf[j + 4]) << endl;
										for (int w = 0; w < 200; w++){
											if (buffEm[w] == 0){
												buffEm[w] = playerToBuff.id;
												cout << "hoho " << buffEm[w] << endl;
												stop = true;
												break;
											}
										}
										break;
									}
								}
							}
						}
					}

				}
			}
			if (stop == true) break;
		}
		//if (buf[1] == 0x18 || buf[1] == 0x1a || buf[1] == 0x2a || buf[1] == 0x27);
		if (0);
		else{
			myfileRecv.open("C:\\tmp\\log.txt", ios::app | ios::binary);
			myfileRecv << "RECV " << hex << len << " ON SOCKET: " << hex << s << endl;
			for (int i = 0; i < len; i++){
				if ((short)buf[i] <= 0x0f && (short)buf[i] >= 0x00) myfileRecv << '0'; //print zero on numbers 0 - f

				if ((short)buf[i] < 0) x = 127 + (buf[i] + 129);
				else x = buf[i];
				myfileRecv << hex << x << ' ';
				if (i != 0 && (i + 1) % 16 == 0 && i != len - 1){
					myfileRecv << '\t';
					for (int y = i - 15; y <= i; y++){
						if (((short)buf[y] >= 0 && (short)buf[y] <= 31) || (short)buf[y] == 127) myfileRecv << '.';
						else myfileRecv << buf[y];
					}
					myfileRecv << '\n';
				}
			}
			//myfileRecv << '\t';
			for (int i = len - (len % 16); i <= len; i++){
				myfileRecv << buf[i];
			}
			myfileRecv << '\n';
			myfileRecv.close();
		}
	}
	HookFunctionName2("ws2_32.dll", "recv", (LPVOID*)nRecv, hook2);
	return len;
}

//Initialize character pointers
void initialize(){
	fiestaBase = (DWORD)GetModuleHandleA("Fiesta.exe");

	playerBase = (DWORD*)(fiestaBase + 0x700E84); //done, simply look for mana writes to and ecx+0xXXXXXXX will be base pointer
	targetofTargetID = (DWORD*)(*playerBase + 0x1032); //this can change?? from patch to patch
	partyMemberHP = (DWORD*)(fiestaBase + 0x7C5F04);
	partyMemberMaxHP = (DWORD*)(fiestaBase + 0x7C5F0C);
	health = (DWORD*)(*playerBase + 0x288); // done
	battleState = (DWORD*)(*playerBase + 0x7A0);
	maxhealth =	 (DWORD*)(*playerBase + 0x290); // done
	mana = (DWORD*)(*playerBase + 0x294); // done
	maxmana = (DWORD*)(*playerBase + 0x298); // done
	//18098F70+ a80, float, idk what this is, speed modifier?
	//18098F70+ 0x79c, 2 bytes, 1 = unmounted, 6 = mounted
	mountStatus = reinterpret_cast<float*>((DWORD*)(*playerBase + 0xa80));
	lp = (DWORD*)(*playerBase + 0x2A4);
	name = (DWORD*)(*playerBase + 0x236); // done
	yaw = reinterpret_cast<float*>((DWORD*)(fiestaBase + 0x78F1F0));
	zoom = (DWORD)(fiestaBase + 0x68F6); // done, just look for zoom, find what writes to, should be a jp
	playerX = reinterpret_cast<float*>((DWORD*)(*playerBase + 0x758)); // done, actually camera
	playerY = reinterpret_cast<float*>((DWORD*)(*playerBase + 0x75C)); // done
	playerZ = (DWORD*)(*playerBase + 0x760); // done
	//To get quest pointers, do a quest that involves kills and search for byte increased. Scroll up to "test edi, edi" to find whwat writes to esi. EBX is base pointer
	//just find what writes to the byte, toggle breakpoint, search hex for the EBX address + 0x0c.
	//questPointer = (DWORD*)(*(DWORD*)(fiestaBase + 0x7C610C) + 0x0C); //Done, pointer to quest entity. word of this is quest ID, byte of this +2 is active or not(0x06 = active). add 0x20 for each quest. add 0x19 for number of kills
	questPointer = (DWORD*)(fiestaBase + 0x7C610C);
	//questNumberPointer = (DWORD*)(*(DWORD*)(fiestaBase + 0x7C6104) + 0x04); // Done. number of quest to iterate through. quest must not be "new" to count
	questNumberPointer = (DWORD*)(fiestaBase + 0x7C6104);
	dropNumber = *(BYTE*)(fiestaBase + 0x705204);
	dropPointer = (DWORD*)(fiestaBase + 0x7051F8 + 0x04);
	lastDrop.dropID = 0;
	lastDrop.itemID = 0;
	//entityPointer = *(DWORD*)(fiestaBase + 0x703B10) + 0x04; //
	//dummyBP();
	//while (1){
	//Sleep(20000);
	//}

	//find this by hmm debug on ws2.send, look in call stack for 2nd from top and press enter on it, and go to call previous to the one just called
	DWORD hookAddressCrypt = fiestaBase + 0x56073B; //encryption hook, postRNG function 53 56 57 8b 7d 08 0f b7 31
	//xor byte ptr ds:[edx+edi],bl
	//inc word ptr ds:[ecx]

	//DWORD hookAddressMove = fiestaBase + 0x288918;
	DWORD hookAddressCrypt2 = fiestaBase + 0x56072C; //encryption hook, preRNG function
	//push ebx
	//push esi
	//push edi

	//DWORD hookAddressDrops = fiestaBase + 0x2905E1;
	jmpBackEncryptRNG = hookAddressCrypt2 + 0x6;
	jmpBackAddy = hookAddressCrypt + 0x6;
	//jmpBackAddyMove = hookAddressMove + 17;
	//jmpBackAddyDrops = hookAddressDrops + 6;
	//int sizeofPacket = 0;

	HookFunctionAddy((void*)hookAddressCrypt2, preEncrypt, 6);
	HookFunctionAddy((void*)hookAddressCrypt, ourFunct, 6);
	//HookFunctionAddy((void*)hookAddressMove, moveFunct, 6);
	//HookFunctionAddy((void*)hookAddressDrops, dropsHook, 6);
}

//Initialize DINPUT8 hook so I can inject arbitrary input to game
bool initializeDInputHook(){
	HMODULE hModDInput8 = NULL;
	hModDInput8 = GetModuleHandle(L"dinput8.dll");
	if (!hModDInput8)
	{
		cout << "Direct Input Not Found!" << endl;
		return false;
	}


}

//Initialize and load drivers so I can use my custom keyboard/mouse drivers to send input to game
void initializeDriver(){
	HDEVINFO                            hardwareDeviceInfo_kbd, hardwareDeviceInfo_mou;
	SP_DEVICE_INTERFACE_DATA            deviceInterfaceData_kbd, deviceInterfaceData_mou;
	PSP_DEVICE_INTERFACE_DETAIL_DATA    deviceInterfaceDetailData_kbd = NULL, deviceInterfaceDetailData_mou = NULL;
	ULONG                               predictedLength_kbd = 0, predictedLength_mou = 0;
	ULONG                               requiredLength_kbd = 0, requiredLength_mou = 0;
	//HANDLE                              file_kbd, file_mou;
	ULONG                               i_kbd = 0, i_mou = 0;

	//chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();

	//KEYBOARD_ATTRIBUTES                 kbdattrib;

	//UNREFERENCED_PARAMETER(argc);
	//UNREFERENCED_PARAMETER(argv);

	//
	// Open a handle to the device interface information set of all
	// present toaster class interfaces.
	//

	hardwareDeviceInfo_kbd = SetupDiGetClassDevs(
		(LPGUID)&GUID_DEVINTERFACE_KBFILTER,
		NULL, // Define no enumerator (global)
		NULL, // Define no
		(DIGCF_PRESENT | // Only Devices present
		DIGCF_DEVICEINTERFACE)); // Function class devices.
	if (INVALID_HANDLE_VALUE == hardwareDeviceInfo_kbd)
	{
		printf("SetupDiGetClassDevs failed: %x\n", GetLastError());
		return;
	}

	deviceInterfaceData_kbd.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

	printf("\nList of KBFILTER Device Interfaces\n");
	printf("---------------------------------\n");

	i_kbd = 0;

	//
	// Enumerate devices of toaster class
	//

	do { //keyboard
		if (SetupDiEnumDeviceInterfaces(hardwareDeviceInfo_kbd,
			0, // No care about specific PDOs
			(LPGUID)&GUID_DEVINTERFACE_KBFILTER,
			i_kbd, //
			&deviceInterfaceData_kbd)) {

			if (deviceInterfaceDetailData_kbd) {
				free(deviceInterfaceDetailData_kbd);
				deviceInterfaceDetailData_kbd = NULL;
			}

			//
			// Allocate a function class device data structure to
			// receive the information about this particular device.
			//

			//
			// First find out required length of the buffer
			//

			if (!SetupDiGetDeviceInterfaceDetail(
				hardwareDeviceInfo_kbd,
				&deviceInterfaceData_kbd,
				NULL, // probing so no output buffer yet
				0, // probing so output buffer length of zero
				&requiredLength_kbd,
				NULL)) { // not interested in the specific dev-node
				if (ERROR_INSUFFICIENT_BUFFER != GetLastError()) {
					printf("SetupDiGetDeviceInterfaceDetail failed %d\n", GetLastError());
					SetupDiDestroyDeviceInfoList(hardwareDeviceInfo_kbd);
					return;
				}

			}

			predictedLength_kbd = requiredLength_kbd;

			deviceInterfaceDetailData_kbd = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(predictedLength_kbd);

			if (deviceInterfaceDetailData_kbd) {
				deviceInterfaceDetailData_kbd->cbSize =
					sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
			}
			else {
				printf("Couldn't allocate %d bytes for device interface details.\n", predictedLength_kbd);
				SetupDiDestroyDeviceInfoList(hardwareDeviceInfo_kbd);
				return;
			}


			if (!SetupDiGetDeviceInterfaceDetail(
				hardwareDeviceInfo_kbd,
				&deviceInterfaceData_kbd,
				deviceInterfaceDetailData_kbd,
				predictedLength_kbd,
				&requiredLength_kbd,
				NULL)) {
				printf("Error in SetupDiGetDeviceInterfaceDetail\n");
				SetupDiDestroyDeviceInfoList(hardwareDeviceInfo_kbd);
				free(deviceInterfaceDetailData_kbd);
				return;
			}
			printf("%d) %s\n", ++i_kbd,
				deviceInterfaceDetailData_kbd->DevicePath);
		}
		else if (ERROR_NO_MORE_ITEMS != GetLastError()) {
			free(deviceInterfaceDetailData_kbd);
			deviceInterfaceDetailData_kbd = NULL;
			continue;
		}
		else
			break;

	} WHILE(TRUE);

	SetupDiDestroyDeviceInfoList(hardwareDeviceInfo_kbd);

	if (!deviceInterfaceDetailData_kbd)
	{
		printf("No device interfaces present\n");
		return;
	}

	//MOUSE STUFF
	hardwareDeviceInfo_mou = SetupDiGetClassDevs(
		(LPGUID)&GUID_DEVINTERFACE_MOUFILTER,
		NULL, // Define no enumerator (global)
		NULL, // Define no
		(DIGCF_PRESENT | // Only Devices present
		DIGCF_DEVICEINTERFACE)); // Function class devices.
	if (INVALID_HANDLE_VALUE == hardwareDeviceInfo_mou)
	{
		printf("SetupDiGetClassDevs failed: %x\n", GetLastError());
		return;
	}

	deviceInterfaceData_mou.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

	printf("\nList of KBFILTER Device Interfaces\n");
	printf("---------------------------------\n");

	i_mou = 0;

	//
	// Enumerate devices of toaster class
	//

	do { //keyboard
		if (SetupDiEnumDeviceInterfaces(hardwareDeviceInfo_mou,
			0, // No care about specific PDOs
			(LPGUID)&GUID_DEVINTERFACE_MOUFILTER,
			i_mou, //
			&deviceInterfaceData_mou)) {

			if (deviceInterfaceDetailData_mou) {
				free(deviceInterfaceDetailData_mou);
				deviceInterfaceDetailData_mou = NULL;
			}

			//
			// Allocate a function class device data structure to
			// receive the information about this particular device.
			//

			//
			// First find out required length of the buffer
			//

			if (!SetupDiGetDeviceInterfaceDetail(
				hardwareDeviceInfo_mou,
				&deviceInterfaceData_mou,
				NULL, // probing so no output buffer yet
				0, // probing so output buffer length of zero
				&requiredLength_mou,
				NULL)) { // not interested in the specific dev-node
				if (ERROR_INSUFFICIENT_BUFFER != GetLastError()) {
					printf("SetupDiGetDeviceInterfaceDetail failed %d\n", GetLastError());
					SetupDiDestroyDeviceInfoList(hardwareDeviceInfo_mou);
					return;
				}

			}

			predictedLength_mou = requiredLength_mou;

			deviceInterfaceDetailData_mou = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(predictedLength_mou);

			if (deviceInterfaceDetailData_mou) {
				deviceInterfaceDetailData_mou->cbSize =
					sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
			}
			else {
				printf("Couldn't allocate %d bytes for device interface details.\n", predictedLength_mou);
				SetupDiDestroyDeviceInfoList(hardwareDeviceInfo_mou);
				return;
			}


			if (!SetupDiGetDeviceInterfaceDetail(
				hardwareDeviceInfo_mou,
				&deviceInterfaceData_mou,
				deviceInterfaceDetailData_mou,
				predictedLength_mou,
				&requiredLength_mou,
				NULL)) {
				printf("Error in SetupDiGetDeviceInterfaceDetail\n");
				SetupDiDestroyDeviceInfoList(hardwareDeviceInfo_mou);
				free(deviceInterfaceDetailData_mou);
				return;
			}
			printf("%d) %s\n", ++i_mou,
				deviceInterfaceDetailData_mou->DevicePath);
		}
		else if (ERROR_NO_MORE_ITEMS != GetLastError()) {
			free(deviceInterfaceDetailData_mou);
			deviceInterfaceDetailData_mou = NULL;
			continue;
		}
		else
			break;

	} WHILE(TRUE);

	SetupDiDestroyDeviceInfoList(hardwareDeviceInfo_mou);

	if (!deviceInterfaceDetailData_mou)
	{
		printf("No device interfaces present\n");
		return;
	}

	//
	// Open the last toaster device interface
	//

	//
	// Open the last toaster device interface
	//

	printf("\nOpening the last interface:\n %s\n",
		deviceInterfaceDetailData_kbd->DevicePath);

	file_kbd = CreateFile(deviceInterfaceDetailData_kbd->DevicePath,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL, // no SECURITY_ATTRIBUTES structure
		OPEN_EXISTING, // No special create flags
		FILE_FLAG_OVERLAPPED, // No special attributes
		NULL);

	file_mou = CreateFile(deviceInterfaceDetailData_mou->DevicePath,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL, // no SECURITY_ATTRIBUTES structure
		OPEN_EXISTING, // No special create flags
		FILE_FLAG_OVERLAPPED, // No special attributes
		NULL);


	if (INVALID_HANDLE_VALUE == file_kbd) {
		printf("Error in CreateFile: %x", GetLastError());
		free(deviceInterfaceDetailData_kbd);
		return;
	}

	file_mou = CreateFile(deviceInterfaceDetailData_mou->DevicePath,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL, // no SECURITY_ATTRIBUTES structure
		OPEN_EXISTING, // No special create flags
		0, // No special attributes
		NULL);

	if (INVALID_HANDLE_VALUE == file_mou) {
		printf("Error in CreateFile: %x", GetLastError());
		free(deviceInterfaceDetailData_mou);
		return;
	}
	printf("wit???");
	//
	// Send an IOCTL to retrive the keyboard attributes
	// These are cached in the kbfiltr
	//
	cout << "huh" << file_kbd << endl;
#pragma warning(disable:4127)
}

void lootItem(short dropID){
	IDtoBYTE wow;
	wow.id = dropID;
	char* packet = new char[5];
	packet[0] = 0x04;
	packet[1] = 0x09;
	packet[2] = 0x30;
	packet[3] = wow.bytes[0];
	packet[4] = wow.bytes[1];

	sendCrypt(sendSocket_r, packet, 5, 0);

	delete packet;
}

void turninQuest(char id[2]){
	char* packet = new char[5];
	packet[0] = 0x04;
	packet[1] = 0x14;
	packet[2] = 0x44;
	packet[3] = id[0];
	packet[4] = id[1];

	sendCrypt(sendSocket_r, packet, 5, 0);

	delete packet;
}

void progressQuestText(char id[2]){
	char* packet = new char[10];
	packet[0] = 0x09;
	packet[1] = 0x02;
	packet[2] = 0x44;
	packet[3] = id[0];
	packet[4] = id[1];
	packet[5] = 0x02;
	packet[6] = 0x01;
	packet[7] = 0x00;
	packet[8] = 0x00;
	packet[9] = 0x00;

	sendCrypt(sendSocket_r, packet, 10, 0);

	delete packet;
}

//08 11 44 5d 00 00 00 00 00 D]
void acceptQuestReward(char id[2]){
	char* packet = new char[9];
	packet[0] = 0x08;
	packet[1] = 0x11;
	packet[2] = 0x44;
	packet[3] = id[0];
	packet[4] = id[1];
	packet[5] = 0x00;
	packet[6] = 0x00;
	packet[7] = 0x00;
	packet[8] = 0x00;

	sendCrypt(sendSocket_r, packet, 9, 0);

	delete packet;
}

void acceptQuest(char id[2]){
	char* packet = new char[5];
	packet[0] = 0x04;
	packet[1] = 0x14;
	packet[2] = 0x44;
	packet[3] = id[0];
	packet[4] = id[1];

	sendCrypt(sendSocket_r, packet, 5, 0);

	delete packet;
}

bool checkQuests(){

	IDtoBYTE umm;
	bool questTurnedIn = false;

	for (int i = 0; i < *questNumberPointer; i++){
		//cout << *questNumberPointer << "\t" << static_cast<unsigned>(*(BYTE*)((*questPointer) + (0x20 * i) + 2)) << endl;
		if (*(BYTE*)((*questPointer) + (0x20 * i) + 2) == 0x08){
			cout << i << "\t" << *questNumberPointer << "\t" << *(BYTE*)((*questPointer) + (0x20 * i) + 2) << endl;
			//getchar();
			char *id = new char[2];
			id[0] = (*(BYTE*)((*questPointer) + (0x20 * i)));
			id[1] = (*(BYTE*)((*questPointer) + (0x20 * i) + 0x01));
			//getchar();
			turninQuest(id);
			//getchar();
			//Sleep(1000);
			cout << "Blip" << endl;
			int j = 0;
			while (*(BYTE*)((*questPointer) + (0x20 * i) + 2) == 0x08 && j < 20){
				//getchar();q
				acceptQuestReward(id);
				progressQuestText(id);
				j++;
				//cout << "3\n";
				Sleep(10);
			}
			//Sleep(1000);
			cout << "Blop" << endl;
			questTurnedIn = true;
			//return true;
			Sleep(250);
			umm.bytes[0] = id[0]; //need to reverse endianness
			umm.bytes[1] = id[1];
			for (int j = 0; j < *questNumberPointer; j++){ //turned in quest, check again from beginning in case of levelup
				if (*(BYTE*)((*questPointer) + (0x20 * i) + 2) != 0x06 && *(WORD*)(*questPointer + (0x20 * i)) == umm.id){
					for (int w = 0; w < 2; w++){
						if (*(BYTE*)((*questPointer) + (0x20 * i) + 2) == 0x06) break;
						acceptQuest(id);
						Sleep(10);
						cout << "Blap" << endl;
						int counter = 0;
						while (*(BYTE*)((*questPointer) + (0x20 * i) + 2) != 0x06 && counter < 20){
							progressQuestText(id);
							//cout << "3\n";
							Sleep(10);
							counter++;
						}
						cout << "Blup" << endl;
					}
				}
			}
			//Sleep(1000);
			delete id;
		}
	}
	return questTurnedIn;
}

bool useSkill(skill *toUse, unsigned char mobId[2], float X, float Y){

	//06 40 24 b4 0f 33 14
	bool used = 0;
	LARGE_INTEGER tcounter;
	LONGLONG ticknow;
	LONGLONG freq;
	QueryPerformanceFrequency(&tcounter);
	freq = tcounter.QuadPart;

	QueryPerformanceCounter(&tcounter);
	ticknow = tcounter.QuadPart;
	//float CDmod = 3.0; for zombies?
	float CDmod = 1.7;
	if ((ticknow - globalCooldown) / (freq / 1000) <= 500 * CDmod) return false;

	if (*mana >= lastMana && lastSkill != NULL){
		if (*mana - lastMana >= 15) lastSkill->tickAtCast = 1; //my sp recovery is 15...
	}

	if (X == 0 && Y == 0){
		char *packet = new char[7];
		//cout << "time: " << (ticknow - toUse->tickAtCast) / (freq / 1000) << "\tcooldown: " << toUse.cooldown << endl;
		if ((ticknow - toUse->tickAtCast) / (freq / 1000) >= (toUse->cooldown * 1000) || toUse->tickAtCast == 0) {
			lastMana = *mana;
			lastSkill = toUse;
			packet[0] = 0x06;
			packet[1] = 0x40;
			packet[2] = 0x24;
			packet[3] = toUse->id[0];
			packet[4] = toUse->id[1];
			packet[5] = mobId[0];
			packet[6] = mobId[1];

			sendCrypt(sendSocket_r, packet, 7, 0);

			if (toUse == &Lifeline || toUse == &LightStrike) Sleep(250);

			QueryPerformanceCounter(&tcounter);
			toUse->tickAtCast = tcounter.QuadPart;
			globalCooldown = tcounter.QuadPart;
			used = true;
		}
		delete packet;
	}
	//0c 41 24 b8 10 87 3e 00 00 ca 38 00 00
	else{
		char *packet = new char[13];
		IDtoBYTE xpos, ypos;
		xpos.id = (int)X;
		ypos.id = (int)Y;
		if ((ticknow - toUse->tickAtCast) / (freq / 1000) >= (toUse->cooldown * 1000) || toUse->tickAtCast == 0) {
			packet[0] = 0x0C;
			packet[1] = 0x41;
			packet[2] = 0x24;
			packet[3] = toUse->id[0];
			packet[4] = toUse->id[1];
			packet[5] = xpos.bytes[0];
			packet[6] = xpos.bytes[1];
			packet[7] = 0x00;
			packet[8] = 0x00;
			packet[9] = ypos.bytes[0];
			packet[10] = ypos.bytes[1];
			packet[11] = 0x00;
			packet[12] = 0x00;

			sendCrypt(sendSocket_r, packet, 13, 0);
			//Sleep(2000); NEED THIS FOR CRUSADER GRINDING!!!
			//sendCrypt(sendSocket_r, packet, 13, 0);
			//ReleaseKey('w', file_kbd);
			QueryPerformanceCounter(&tcounter);
			toUse->tickAtCast = tcounter.QuadPart;
			globalCooldown = tcounter.QuadPart;
			used = true;

		}
		delete packet;
	}
	return used;
}

/*void getDropList(){
DWORD *nextDrop = dropPointer;
DWORD droppyPointer1;
DWORD droppyPointer2;
float dropX = 0;
float dropY = 0;
BYTE dropID[2];
BYTE itemID[2];
IDtoBYTE dropstuff;

cout << hex << dropPointer << endl;
cout << hex << *(dropPointer) << endl;
//Sleep(1000);
while (*nextDrop != 0x0){
cout << "1 " << *nextDrop << endl;
droppyPointer1 = *(DWORD*)(*nextDrop + 0x08); // doing this to reduce the risk of the functions accessing invalid memory in the middle of call
cout << "2 " << droppyPointer1 << endl;
if (droppyPointer1 < fiestaBase) break;
droppyPointer2 = *(DWORD*)(droppyPointer1 + 0x0c);
if (droppyPointer2 < fiestaBase) break;
cout << "Or here? " << droppyPointer2 << endl;
if (dropReference == 0) dropReference = droppyPointer2;
//if (droppyPointer2 > dropReference + 0x2000000 || droppyPointer2 < dropReference - 0x2000000) break;
//if(nextDrop > 0)
cout << "3 " << (float*)(droppyPointer2 + 0x58) << endl;
dropX = *(float*)(droppyPointer2 + 0x58);
//if (nextDrop > 0)
cout << "4 " << (float*)(droppyPointer2 + 0x5C) << endl;
dropY = *(float*)(droppyPointer2 + 0x5C);
cout << "There is a drop!! X: " << dropX << "\tY: " << dropY << endl;
//if (nextDrop > 0)
dropID[0] = *(BYTE*)(droppyPointer1 + 0x06);
//if (nextDrop > 0)
dropID[1] = *(BYTE*)(droppyPointer1 + 0x07);

//if (nextDrop > 0)
itemID[0] = *(char*)(droppyPointer1 + 0x04);
//if (nextDrop > 0)
itemID[1] = *(char*)(droppyPointer1 + 0x05);

dropstuff.bytes[0] = itemID[0];
dropstuff.bytes[1] = itemID[1];

cout << "Drop ID: " << hex << static_cast<unsigned>(dropID[0]) << static_cast<unsigned>(dropID[1]) << endl;
cout << "Drop ID: " << dropstuff.id << endl;
cout << "Item ID: " << hex << static_cast<unsigned>(itemID[0]) << static_cast<unsigned>(itemID[1]) << endl;
float distance = sqrt(pow((*reinterpret_cast<float*>(playerX)-dropX), 2) + pow((*reinterpret_cast<float*>(playerY)-dropY), 2));
cout << "Distance: " << distance << endl;
for (int i = 0; i < sizeof(lootTable) / 2; i++){
if (dropstuff.id == lootTable[i] && distance <= 110) lootItem(dropID);
}
//	Sleep(1000);
cout << "Me crash here?" << endl;
if (*nextDrop != 0){
if (*(DWORD*)(*nextDrop) != 0)
nextDrop = (DWORD*)(*nextDrop);

else break;
}
}
//Sleep(1000);
}*/
/*dropPointer -> 8 -> c -> 58 float = X
dropPointer -> 8 -> c -> 5c float = Y
dropPointer -> 8 -> 4 -> 2 byte = item ID
dropPointer -> 8 -> 6 -> 2 byte = drop ID
*/

/*DWORD nextDrop = *dropPointer;
float *dropX;
float *dropY;
BYTE dropID[2];
BYTE itemID[2];
IDtoBYTE dropstuff;*/

void lootHandler(){
	currentDrop.distance = distanceFunc(*(playerX), *(playerY), dropX, dropY);

	if (currentDrop.distance < 85){
		for (int i = 0; i < sizeof(lootTable) / 2; i++){
			if (currentDrop.itemID == lootTable[i] &&
				currentDrop.dropID != lastDrop.dropID){
				lootItem(currentDrop.dropID);
				lastDrop.dropID = currentDrop.dropID;
				if (currentDrop.itemID == 9184) enableDropTrack = false;
				QueryPerformanceCounter(&LOOTtcounter);
				LOOTtickLoot = LOOTtcounter.QuadPart;
				//enableDropTrack = false;
				break;
			}
		}
	}
	//if item is slime jelly HQ
	else if (currentDrop.itemID == 9184 && currentDrop.dropID != lastDrop.dropID && LOOTTRAVEL == false){
		TRAVELX = dropX;
		TRAVELY = dropY;
		LOOTTRAVEL = true;
	}
	//for crusader slime farm?
	/*else if (currentDrop.distance < 300 && currentDrop.distance >= 90 && (currentDrop.itemID == 3061)
		&& currentDrop.dropID != lastDrop.dropID && LOOTTRAVEL == false){
		TRAVELX = dropX;
		TRAVELY = dropY;
		LOOTTRAVEL = true;
	}*/

	else if (currentDrop.distance < 600 && currentDrop.distance >= 85 && (currentDrop.itemID == 55000 || currentDrop.itemID == 55001)
	&& currentDrop.dropID != lastDrop.dropID && LOOTTRAVEL == false){
		TRAVELX = dropX;
		TRAVELY = dropY;
		LOOTTRAVEL = true;
	}
}
void __declspec(naked) dropsHook(){

	__asm{ //start
		pushad
		pushfd
		lea ebx, [esi]
	}
	//cout << "Yeppers!" << endl;
	//printf("Yepp1\n");
	__asm{
		cmp enableDropTrack, 1
			je wow
			popfd
			popad
			mov edx, [eax + 0x000000C4]
			jmp jmpBackAddyDrops
	}
	//printf("Yeppa\n");
	__asm{
		wow : mov eax, [ebx + 0x58]
			  mov dropX, eax
	}
	//printf("Yepp2\n");
	__asm{
			mov eax, [ebx + 0x5c]
			mov dropY, eax
	}
	//printf("Yepp3\n");
	__asm{
			mov al, byte ptr[edi + 0x06]
			mov dropID[0], al
	}
	//printf("Yepp4\n");
	__asm{
			mov al, byte ptr[edi + 0x07]
			mov dropID[1], al
	}
	//printf("Yepp5\n");
	__asm{
			mov al, byte ptr[edi + 0x04]
				mov itemID[0], al
	}
	//printf("Yepp6\n");
	__asm{
			mov al, byte ptr[edi + 0x05]
				mov itemID[1], al
	}

	//printf("Yepp8\n");
	//WORD lol;
	//cout << "yep" << endl;
	dropstuff.bytes[0] = itemID[0];
	dropstuff.bytes[1] = itemID[1];
	currentDrop.itemID = dropstuff.id;
	dropstuff.bytes[0] = dropID[0];
	dropstuff.bytes[1] = dropID[1];
	currentDrop.dropID = dropstuff.id;
	//printf("Yepp9\n");

	lootHandler();
	//cout << "Player X: " << *(playerX) << "\tPlayer Y: " << *(playerY) << endl
	//cout << "Drop X: " << dropX << "\tDrop Y: " << dropY << endl;
	//cout << "Drop ID: " << hex << static_cast<unsigned>(dropID[0]) << static_cast<unsigned>(dropID[1]) << endl;
	//cout << "item ID real: " << hex << currentDrop.itemID << endl;
	//cout << "Distance: " << currentDrop.distance << endl;


	//currentDrop.distance = distanceFunc(*(playerX), *(playerY), dropX, dropY);
	//for (int i = 0; i < 19; i++){
		/*if (currentDrop.itemID == lootTable[i] && currentDrop.distance <= 110 &&
			currentDrop.dropID != lastDrop.dropID){
			lootItem(currentDrop.dropID);
			lastDrop.dropID = currentDrop.dropID;
			break;
		}*/
	//}
	//printf("Yeppy\n");
	//cout << "yeppy" << endl;
	//cout << "Item ID: " << hex << static_cast<unsigned>(itemID[0]) << static_cast<unsigned>(itemID[1]) << endl;
	//cout << "X: " << dropX << "\tY: " << dropY << endl;
	//DWORD nextDrop = *dropPointer;
	/*cout << hex << dropPointer << endl;
	cout << hex << *(dropPointer) << endl;
	//Sleep(1000);
	while (nextDrop != 0x0){
		getchar();
		if (nextDrop > 0) dropX = (float*)(*(DWORD*)(*(DWORD*)(nextDrop + 0x08) + 0x0c) + 0x58);
		if (nextDrop > 0) dropY = (float*)(*(DWORD*)(*(DWORD*)(nextDrop + 0x08) + 0x0c) + 0x5C);
		cout << "There is a drop!! X: " << *dropX << "\tY: " << *dropY << endl;
		if (nextDrop > 0) dropID[0] = *(BYTE*)(*(DWORD*)(nextDrop + 0x08) + 0x06);
		if (nextDrop > 0) dropID[1] = *(BYTE*)(*(DWORD*)(nextDrop + 0x08) + 0x07);

		if (nextDrop > 0) itemID[0] = *(char*)(*(DWORD*)(nextDrop + 0x08) + 0x04);
		if (nextDrop > 0) itemID[1] = *(char*)(*(DWORD*)(nextDrop + 0x08) + 0x05);

		dropstuff.bytes[0] = itemID[0];
		dropstuff.bytes[1] = itemID[1];

		cout << "Drop ID: " << hex << static_cast<unsigned>(dropID[0]) << static_cast<unsigned>(dropID[1]) << endl;
		cout << "Drop ID: " << dropstuff.id << endl;
		cout << "Item ID: " << hex << static_cast<unsigned>(itemID[0]) << static_cast<unsigned>(itemID[1]) << endl;
		float distance = sqrt(pow((*reinterpret_cast<float*>(playerX)-*dropX), 2) + pow((*reinterpret_cast<float*>(playerY)-*dropY), 2));
		cout << "Distance: " << distance << endl;
		for (int i = 0; i < sizeof(lootTable) / 2; i++){
			if (dropstuff.id == lootTable[i] && distance <= 110) lootItem(dropID);
		}
		//Sleep(1000);
		nextDrop = *(DWORD*)nextDrop;
	}
	//Sleep(1000);*/

	__asm{ // end
		popfd
		popad
		mov edx, [eax + 0x000000C4]
		jmp jmpBackAddyDrops
	}
}

int disconnectCheck(int eventType, CHAR* account){
	//event types
	//1 = map check
	LARGE_INTEGER tcounter;
	LONGLONG tickynow;
	LONGLONG tickyEvent;
	LONGLONG freq;
	QueryPerformanceFrequency(&tcounter);
	freq = tcounter.QuadPart;

	QueryPerformanceCounter(&tcounter);
	tickynow = tcounter.QuadPart;
	tickyEvent = tcounter.QuadPart;

	mapSwitch = false;
	while (eventType == 1){
		if ((tickynow - tickyEvent) / (freq / 1000) > 16 * 1000){
			return 1;
		}

		if ((tickynow - tickyEvent) / (freq / 1000) > 1 * 1000){
			injectDInput(DIK_ESCAPE, .1);
			while (injectKey == true) Sleep(10);
			injectDInput(DIK_ESCAPE, .1);
			while (injectKey == true) Sleep(10);
		}

		if (mapSwitch == true){
			//Sleep(4000);
			mapSwitch = false;
			return 0;
		}

		if (characterSelect == true){ //do this when "disconnected from server" appears

			return 2;
		}

		QueryPerformanceCounter(&tcounter);
		tickynow = tcounter.QuadPart;
	}
}

void aoeGrind(){
	point points[4];

	for (int i = 0; i < 4; i++){
		cout << "Press enter to record point number " << i + 1 << endl;
		getchar();
		points[i].x = *reinterpret_cast<float*>(playerX);
		points[i].y = *reinterpret_cast<float*>(playerY);
	}
	cout << "Press enter to start..." << endl;
	getchar();
	Sleep(2000);

	while (1){
		for (int i = 0; i < 4; i++){
			moveX = points[i].x;
			moveY = points[i].y;
			hijackMove = true;

			DownKey('w', file_kbd);
			while (distanceFunc(*reinterpret_cast<float*>(playerX), *reinterpret_cast<float*>(playerY), moveX, moveY) >= 1){
				if (useSkill(&MultiShotr1, NULL, *reinterpret_cast<float*>(playerX), *reinterpret_cast<float*>(playerY))){
					//ReleaseKey('w', file_kbd);
					Sleep(400);
					DownKey('w', file_kbd);
					//Sleep(1000);
				}
				//getDropList();
				if (*health < 200) DownKey('q', file_kbd);
				if (*mana < 100) DownKey('e', file_kbd);
			}
			ReleaseKey('w', file_kbd);
			hijackMove = false;
			Sleep(100);
		}
	}


}
//Log on two accounts. Find a coordinate of the other player, search addresses until a change moves the player. Find what writes to.
//You will have to scroll up to see what writes to the registers, and weill have to go back a few functions (maybe 2 to 3) to get to the base pointer
//push ecx
//mov ecx, Fiesta.exe+?????? << this is address you want
//fstp dword ptr [esp]
//entityPointer = *(DWORD*)(fiestaBase + 0x704E28);
//to check if it is right, put address pointed to by fiesta.exe+?????? in cheat engine, with offsets 3c, 8, 236. should point to your name
void mobESP(){
	entityPointer = *(DWORD*)(fiestaBase + 0x703B10) + 0x04;
	DWORD entity = *(DWORD*)entityPointer;
	DWORD entities[1024] = { 0 };
	DWORD entityName, entityID;
	char name[30] = { 0 };
	DWORD lastEntity = 0;

	entityPointer = *(DWORD*)(fiestaBase + 0x703B10) + 0x04;
	entities[0] = *(DWORD*)entityPointer;

	int entityCounter = 1, entityBackup = 1;
	int i = 0;
	bool unique = true;
	while (1){
		if (*(DWORD*)entities[i] != entityPointer - 4 && *(DWORD*)entities[i] > fiestaBase){
			if (*(BYTE*)(*(DWORD*)entities[i] + 0x15) == 0){
				for (int i = 0; entities[i] != 0; i++){
					if (*(DWORD*)entities[i] == entities[i]){
						unique = false;
						break;
					}
				}

				if (unique == true){
					entities[entityCounter] = *(DWORD*)entities[i];
					entityCounter++;
				}
			}
		}
		if (*(DWORD*)(entities[i] + 0x08) != entityPointer - 4 && *(DWORD*)(entities[i] + 0x08) > fiestaBase){
			if (*(BYTE*)(*(DWORD*)(entities[i] + 0x08) + 0x15) == 0){
				for (int i = 0; entities[i] != 0; i++){
					if (*(DWORD*)entities[i] == entities[i]){
						unique = false;
						break;
					}
				}

				if (unique == true){
					entities[entityCounter] = *(DWORD*)(entities[i] + 0x08);
					entityCounter++;
				}
			}
		}
		//if (entityBackup == entityCounter) break;
		entityBackup = entityCounter;

		i++;
		if (entities[i] == 0) break;
		unique = true;
	}
	cout << "Number of Entities: " << entityCounter << endl;


	float shortestDistance = 0;
	DWORD shortestEntity = 0;
	DWORD entityHealth = 0;
	for (int i = 0; entities[i] != 0; i++){
		//cout << "sup ";
		cout << endl << entities[i] << endl;;
		//cout << "Getting Base" << endl;
		DWORD entityBase = *(DWORD*)(entities[i] + 0x10);
		if (entityBase < fiestaBase) continue;
		//cout << "Getting Details" << endl;
		DWORD entityDetails = *(DWORD*)(entityBase + 0x2cc);
		if (entityDetails < 40000000) continue;

		cout << hex << entityBase << endl;

		for (int i = 0; *(BYTE*)(entityBase + 0x236 + i) != '\0' && i < 30; i++){
			name[i] = *(BYTE*)(entityBase + 0x236 + i);
		}
		cout << "Address: " << entityDetails << endl;

		cout << "\tName: ";
		for (int i = 0; name[i] != 0 && i < 30; i++) cout << name[i];

		cout << "\tID: " << *(WORD*)(entities[i] + 0xC);
		cout << "\tHealth: " << *(int*)(entityBase + 0x288);

		cout << "\tX: ";
		cout << *(float*)(entityDetails + 0x58);

		cout << "\tY: ";
		cout << *(float*)(entityDetails + 0x5C);

		for (int i = 0; name[i] != 0 && i < 30; i++) name[i] = 0;

	}
	cout << "Done!\n";
}

void mobESPTest(){
	entityPointer = (fiestaBase + 0x700E84);  //base pointer/pointer to me
	//DWORD entity = *(DWORD*)entityPointer;
	DWORD entities[1024] = { 0 };
	DWORD entityName, entityID;
	char name[30] = { 0 };
	DWORD lastEntity = 0;

	//entityPointer = (DWORD)(fiestaBase + 0x704E28);
	entities[0] = *(DWORD*)(entityPointer + 0x3C);
	//cout << hex << entities[0] << endl;

	int entityCounter = 1, entityBackup = 1;
	int i = 0;
	bool unique = true;
	while (1){
		if (*(DWORD*)entities[i] != 0){
			for (int i = 0; entities[i] != 0; i++){
				if (*(DWORD*)entities[i] == entities[i]){
					unique = false;
					break;
				}
			}

			if (unique == true){
				entities[entityCounter] = *(DWORD*)entities[i];
				entityCounter++;
			}
		}

		i++;
		if (entities[i] == 0) break;
		unique = true;
	}
	cout << "Number of Entities: " << entityCounter << endl;
	

	float shortestDistance = 0;
	DWORD shortestEntity = 0;
//DWORD entityHealth = 0;	
	for (int i = 0; entities[i] != 0; i++){
		//cout << "sup ";
		cout << hex << "entities[i]: " << entities[i] << endl;
		//cout << "Getting Base" << endl;
		DWORD entityBase = *(DWORD*)(entities[i] + 0x08);
		if (entityBase == 0 || entityBase > entities[i] + 0x30000000){
			//i++;
			continue;
		}
		//if (entityBase < fiestaBase) continue;
		cout << hex << "entityBase: " << entityBase << endl;
		//cout << "Getting Details" << endl;
		DWORD entityDetails = *(DWORD*)(entityBase + 0x2cc);
		if (entityDetails == 0 || entityDetails > entities[i] + 0x30000000){
			//i++;
			continue;
		}
		cout << hex << "entityDetails: " << entityDetails << endl;
		//if (entityDetails < 40000000) continue;

			//cout << hex << entityBase << endl;

			for (int i = 0; *(BYTE*)(entityBase + 0x236 + i) != '\0' && i < 30; i++){
				name[i] = *(BYTE*)(entityBase + 0x236 + i);
			}
			//cout << "Address: " << hex << entityDetails << endl;

			cout << "\tName: ";
			for (int i = 0; name[i] != 0 && i < 30; i++) cout << name[i];

			cout << "\tSelection ID: " << *(WORD*)(entityBase + 0x234) << endl;
			cout << "Mob ID: " << dec << *(WORD*)(entityBase + 0x3BE);
			cout << "\tHealth: " << *(int*)(entityBase + 0x288);

			cout << "\tX: ";
			cout << *(float*)(entityDetails + 0x58);

			cout << "\tY: ";
			cout << *(float*)(entityDetails + 0x5C);

			float distance = distanceFunc(*playerX, *playerY, *(float*)(entityDetails + 0x58), *(float*)(entityDetails + 0x5C));
			cout << "\tDistance: " << distance << endl;
			cout << "\n";

			for (int i = 0; name[i] != 0 && i < 30; i++) name[i] = 0;

	}
	cout << "Done!\n";
}

void partyInvite(string nameToInvite){
	//16 02 38 50 65 6e 63 69 6c 76 65 73 74 65 72 00 00 .8Pencilvester.. send invite
	//00 00 00 00 00 00
	//16 03 38 42 61 6e 61 6e 61 57 61 6c 72 75 73 00 ..8BananaWalrus. recv it
	//00 00 00 00 00 00 00 x
	//16 04 38 42 61 6e 61 6e 61 57 61 6c 72 75 73 00 00 .8BananaWalrus.. accept invite
	//00 00 00 00 00 00
	char *packet = new char[23];
	packet[0] = 0x16;
	packet[1] = 0x02;
	packet[2] = 0x38;
	for (int i = 3; i < 23; i++){
		if ((i - 3) < nameToInvite.length()) packet[i] = nameToInvite[i - 3];
		else packet[i] = 0x00;
	}
	/*packet[3] = 0x42;
	packet[4] = 0x61;
	packet[5] = 0x6e;
	packet[6] = 0x61;
	packet[7] = 0x6e;
	packet[8] = 0x61;
	packet[9] = 0x57;
	packet[10] = 0x61;
	packet[11] = 0x6c;
	packet[12] = 0x72;
	packet[13] = 0x75;
	packet[14] = 0x73;
	packet[15] = 0x00;
	packet[16] = 0x00;
	packet[17] = 0x00;
	packet[18] = 0x00;
	packet[19] = 0x00;
	packet[20] = 0x00;
	packet[21] = 0x00;
	packet[22] = 0x00;*/

	sendCrypt(sendSocket_r_Invite, packet, 23, 1);
	delete packet;

}

void partyAccept(string nameOfInviter){
	//16 02 38 50 65 6e 63 69 6c 76 65 73 74 65 72 00 00 .8Pencilvester.. send invite
	//00 00 00 00 00 00
	//16 03 38 42 61 6e 61 6e 61 57 61 6c 72 75 73 00 ..8BananaWalrus. recv it
	//00 00 00 00 00 00 00 x
	//16 04 38 42 61 6e 61 6e 61 57 61 6c 72 75 73 00 00 .8BananaWalrus.. accept invite
	//00 00 00 00 00 00
	char *packet = new char[23];
	packet[0] = 0x16;
	packet[1] = 0x04;
	packet[2] = 0x38;
	for (int i = 3; i < 23; i++){
		if ((i - 3) < nameOfInviter.length()) packet[i] = nameOfInviter[i - 3];
		else packet[i] = 0x00;
	}
	/*packet[3] = 0x42;
	packet[4] = 0x61;
	packet[5] = 0x6e;
	packet[6] = 0x61;
	packet[7] = 0x6e;
	packet[8] = 0x61;
	packet[9] = 0x57;
	packet[10] = 0x61;
	packet[11] = 0x6c;
	packet[12] = 0x72;
	packet[13] = 0x75;
	packet[14] = 0x73;
	packet[15] = 0x00;
	packet[16] = 0x00;
	packet[17] = 0x00;
	packet[18] = 0x00;
	packet[19] = 0x00;
	packet[20] = 0x00;
	packet[21] = 0x00;
	packet[22] = 0x00;*/

	sendCrypt(sendSocket_r_Invite, packet, 23, 1);
	delete packet;

}

void partyQuit(){
	char* packet = new char[3];
	packet[0] = 0x02;
	packet[1] = 0x0a;
	packet[2] = 0x38;

	sendCrypt(sendSocket_r_Invite, packet, 3, 1);
	delete packet;
}

bool isTargetingMe(char id[2]){
	DWORD myEntity = *(DWORD*)(entityPointer + 0x3C);
	DWORD player = *(DWORD*)(myEntity + 0x08);
	char playerID[2] = { *(BYTE*)(player + 0x234), *(BYTE*)(player + 0x235) };

	char* memelol = new char[5];

	//cout << "FUCK: " << hex << *(WORD*)(memebase + 0x3BE) << "\tWHAT: " << *(int*)(memebase + 0x288) << endl;
	memelol[0] = 0x04;
	memelol[1] = 0x01;
	memelol[2] = 0x24;
	memelol[3] = id[0];
	memelol[4] = id[1];

	sendCrypt(sendSocket_r, memelol, 5, 0);



	//Sleep(1000);
	delete memelol;
}

float getEntityDistance(unsigned char ID[2]){
	entityPointer = (fiestaBase + 0x700C58);  //base pointer/pointer to me
	//DWORD entity = *(DWORD*)entityPointer;
	DWORD entities[1024] = { 0 };
	DWORD entityName, entityID;
	char name[30] = { 0 };

	//entityPointer = (DWORD)(fiestaBase + 0x704E28);
	entities[0] = *(DWORD*)(entityPointer + 0x3C);
	DWORD player = *(DWORD*)(entities[0] + 0x08);
	char playerID[2] = { *(BYTE*)(player + 0x234), *(BYTE*)(player + 0x235) };
	//cout << hex << entities[0] << endl;

	int entityCounter = 1, entityBackup = 1;
	int i = 0;
	bool unique = true;
	while (1){
		if (*(DWORD*)entities[i] != 0){
			for (int i = 0; entities[i] != 0; i++){
				if (*(DWORD*)entities[i] == entities[i]){
					unique = false;
					break;
				}
			}

			if (unique == true){
				entities[entityCounter] = *(DWORD*)entities[i];
				entityCounter++;
			}
		}

		i++;
		if (entities[i] == 0) break;
		unique = true;
	}
	Sleep(200);

	float distToEntity = 0;
	//DWORD entityHealth = 0;	
	for (int i = 0; entities[i] != 0; i++){
		//cout << "sup ";
		//cout << hex << "entities[i]: " << entities[i] << endl;
		//cout << "Getting Base" << endl;
		//cout << "umm1" << endl;
		DWORD entityBase = *(DWORD*)(entities[i] + 0x08);
		if (entityBase < fiestaBase){
			//i++;
			continue;
		}
		//cout << "umm2" << endl;
		//if (entityBase < fies	taBase) continue;
		//cout << hex << "entityBase: " << entityBase << endl;
		//cout << "Getting Details" << endl;
		//cout << "entityDetails to dereference: " << (DWORD*)(entityBase + 0x2cc) << endl;
		DWORD* entityDeref = (DWORD*)(entityBase + 0x2cc);
		if (entityDeref == 0) continue;
		/*BYTE entityDetails1 = static_cast<unsigned>(*(BYTE*)entityDeref);
		cout << "1: " << entityDetails1 << endl;
		BYTE entityDetails2 = static_cast<unsigned>(*(BYTE*)(entityDeref + 1));
		cout << "2: " << entityDetails2 << endl;
		BYTE entityDetails3 = static_cast<unsigned>(*(BYTE*)(entityDeref + 2));
		cout << "3: " << entityDetails3 << endl;
		BYTE entityDetails4 = static_cast<unsigned>(*(BYTE*)(entityDeref + 3));
		cout << "4: " << entityDetails4 << endl;*/
		DWORD entityDetails = *(DWORD*)entityDeref;
		//cout << hex << "entityDetails: " << entityDetails << endl;
		if (entityDetails < fiestaBase){
			//i++;
			continue;
		}

		//cout << "umm3" << endl;
		WORD currentMobID = *(WORD*)(entityBase + 0x3BE);
		//cout << hex << "currentMobID: " << *(WORD*)(entityBase + 0x3BE) << endl;

		WORD currentSelectionId = *(WORD*)(entityBase + 0x234);
		//cout << hex << "currentSelectionId: " << *(WORD*)(entityBase + 0x234) << endl;
		//cout << hex << static_cast<unsigned>(ID[0]) << " " << static_cast<unsigned>(ID[1]) << endl;
		if (*(BYTE*)(entityBase + 0x234) != ID[0] || *(BYTE*)(entityBase + 0x235) != ID[1]) continue;
		cout << hex << static_cast<unsigned>(*(BYTE*)(entityBase + 0x234)) << " " << static_cast<unsigned>(*(BYTE*)(entityBase + 0x235)) << endl;
		float *currentEntityX = (float*)(entityDetails + 0x58);
		float *currentEntityY = (float*)(entityDetails + 0x5C);

		if (currentEntityX == NULL || currentEntityY == NULL) continue;

		distToEntity = sqrt(pow((*(playerX)-*currentEntityX), 2) + pow((*(playerY)-*currentEntityY), 2));

		//cout << "Player X: " << *playerX << "\tPlayer Y: " << *playerY << endl;
		//cout << "Entity X: " << *currentEntityX << "\tEntity Y: " << *currentEntityY << endl;

		//cout << "\tDistance: " << distToEntity << endl;

		break;
	}
	return distToEntity;
	//cout << "lol" << endl;
}

void checkBuff(bool fighting){
	for (int j = 0; j < 5; j++){
		if (buffEm[j] != 0){
			cout << buffEm[j] << " look at this bullshit" << endl;
			IDtoBYTE playerToBuff;
			playerToBuff.id = buffEm[j];
			unsigned char buffID[2] = { playerToBuff.bytes[0], playerToBuff.bytes[1] };

			float entityDistance = getEntityDistance(buffID);
			if (entityDistance <= 480 && entityDistance != 0){
				if (fighting == true){
					DownKey('s', file_kbd);
					Sleep(50);
					ReleaseKey('s', file_kbd);
				}
				Sleep(600);
				while (useSkill(&Endure, buffID, 0, 0) == false) Sleep(100);
				Sleep(1000);
				//while (useSkill(&Immune, buffID, 0, 0) == false) Sleep(100);
			}
			buffEm[j] = 0;
		}
	}
}

bool applyBuffs(unsigned char *id){

	//06 40 24 b4 0f 33 14
	bool used = 0;
	LARGE_INTEGER tcounter;
	LONGLONG ticknow, tickCast;
	LONGLONG freq;
	QueryPerformanceFrequency(&tcounter);
	freq = tcounter.QuadPart;

	QueryPerformanceCounter(&tcounter);
	ticknow = tcounter.QuadPart;

	if ((ticknow - buffsCooldown) / (freq / 1) <= 3480 && buffsCooldown != 0) return false;
	cout << "me use buffs\n";
	/*SEND ON SOCKET : 8d0
	04 15 30 09 09 0
	SEND ON SOCKET : 8d0 use scrolls on slot 1 and 2 of backpack
	04 15 30 01 09 0 */
	char *packet = new char[5];
	//cout << "time: " << (ticknow - toUse->tickAtCast) / (freq / 1000) << "\tcooldown: " << toUse.cooldown << endl;
	for (int i = 0; i < 4; i++){ //use scrolls
		packet[0] = 0x04;
		packet[1] = 0x15;
		packet[2] = 0x30;
		packet[3] = i;
		packet[4] = 0x09;

		sendCrypt(sendSocket_r, packet, 5, 0);

		QueryPerformanceCounter(&tcounter);
		ticknow = tcounter.QuadPart;
		tickCast = tcounter.QuadPart;

		while ((ticknow - tickCast) / (freq / 1000) <= 4000){
			if (*health <= *maxhealth * 0.60){
				useSkill(&Heal, playerID, 0, 0);
			}
			QueryPerformanceCounter(&tcounter);
			ticknow = tcounter.QuadPart;
		}
	}
	delete packet;
	useSPStone();

	Sleep(600);
	while (useSkill(&Protect, playerID, 0, 0) == false){
		if (*health <= *maxhealth * 0.60){
			useHPStone();
		}
	}
	Sleep(2500);
	while (useSkill(&Heal, playerID, 0, 0) == false);
	Sleep(1500);
	while (useSkill(&Endure, playerID, 0, 0) == false){
		if (*health <= *maxhealth * 0.60){
			useHPStone();
		}
	}
	Sleep(2500);
	while (useSkill(&Heal, playerID, 0, 0) == false);
	Sleep(1500);
	while (useSkill(&Immune, playerID, 0, 0) == false){
		if (*health <= *maxhealth * 0.60){
			useHPStone();
		}
	}
	Sleep(2500);
	while(useSkill(&Heal, playerID, 0, 0) == false);
	Sleep(1500);
	while (useSkill(&Resist, playerID, 0, 0) == false){
		if (*health <= *maxhealth * 0.60){
			useHPStone();
		}
	}
	Sleep(2500);
	while (useSkill(&Heal, playerID, 0, 0) == false);
	Sleep(1500);
	while (useSkill(&Stoneskin, playerID, 0, 0) == false){
		if (*health <= *maxhealth * 0.60){
			useHPStone();
		}
	}
	Sleep(2500);
	while (useSkill(&Heal, playerID, 0, 0) == false);
	Sleep(1500);
	QueryPerformanceCounter(&tcounter);
	buffsCooldown = tcounter.QuadPart;

	return true;
}

bool applyBuffs_Crus(unsigned char *id){

	//06 40 24 b4 0f 33 14
	bool used = 0;
	LARGE_INTEGER tcounter;
	LONGLONG ticknow, tickCast;
	LONGLONG freq;
	QueryPerformanceFrequency(&tcounter);
	freq = tcounter.QuadPart;

	QueryPerformanceCounter(&tcounter);
	ticknow = tcounter.QuadPart;

	if ((ticknow - buffsCooldown) / (freq / 1) <= 3480 && buffsCooldown != 0) return false;

	cout << "me use buffs\n";
	/*SEND ON SOCKET : 8d0
	04 15 30 09 09 0
	SEND ON SOCKET : 8d0 use scrolls on slot 1 and 2 of backpack
	04 15 30 01 09 0 */
	char *packet = new char[5];
	int scroll_amount = 3;
	//cout << "time: " << (ticknow - toUse->tickAtCast) / (freq / 1000) << "\tcooldown: " << toUse.cooldown << endl;
	for (int i = 0; i < scroll_amount; i++){ //use scrolls
		packet[0] = 0x04;
		packet[1] = 0x15;
		packet[2] = 0x30;
		packet[3] = i;
		packet[4] = 0x09;

		sendCrypt(sendSocket_r, packet, 5, 0);

		QueryPerformanceCounter(&tcounter);
		ticknow = tcounter.QuadPart;
		tickCast = tcounter.QuadPart;

		while ((ticknow - tickCast) / (freq / 1000) <= 4000){
			if (*health <= *maxhealth * 0.45){
				useHPStone();
			}
			if (*health <= *maxhealth * 0.30){
					DownKey('=', file_kbd);
					Sleep(150);
					ReleaseKey('=', file_kbd);
			}
			QueryPerformanceCounter(&tcounter);
			ticknow = tcounter.QuadPart;
		}

		if (i == scroll_amount - 1) break;
	}
	delete packet;
	QueryPerformanceCounter(&tcounter);
	buffsCooldown = tcounter.QuadPart;

	return true;
}

void mobESPGrind(int ids[20], int optionalIDs[20]){
	cout << "hmm";
	entityPointer = (fiestaBase + 0x700C58);  //base pointer/pointer to me
	//DWORD entity = *(DWORD*)entityPointer;
	DWORD entities[1024] = { 0 };
	DWORD entityName, entityID;
	char name[30] = { 0 };

	//entityPointer = (DWORD)(fiestaBase + 0x704E28);
	entities[0] = *(DWORD*)(entityPointer + 0x3C);
	if (playerID[0] == 0xFF && playerID[1] == 0xFF){
		DWORD player = *(DWORD*)(entities[0] + 0x08);
		playerID[0] = *(BYTE*)(player + 0x234);
		playerID[1] = *(BYTE*)(player + 0x235);
	}
	//cout << hex << entities[0] << endl;
	int entityCounter = 1, entityBackup = 1;
	int i = 0;
	bool unique = true;
	while (1){
		if (*(DWORD*)entities[i] != 0){
			for (int i = 0; entities[i] != 0; i++){
				if (*(DWORD*)entities[i] == entities[i]){
					unique = false;
					break;
				}
			}

			if (unique == true){
				entities[entityCounter] = *(DWORD*)entities[i];
				entityCounter++;
			}
		}

		i++;
		if (entities[i] == 0) break;
		unique = true;
	}

	char* memelol = new char[5];
	DWORD memebase = 0;
	for (int i = 0; entities[i] != 0; i++){
		memebase = *(DWORD*)(entities[i] + 0x08);
		if (memebase == 0 || memebase > entities[i] + 0x30000000){
			//i++;
			continue;
		}
		bool found = false;
		for (int j = 0; j < 20 && ids[j] != 0; j++){
			if (*(WORD*)(memebase + 0x3BE) == ids[j]){
				found = true;
				break;
			}
		}
		if (found == false){
			for (int j = 0; j < 20 && optionalIDs[j] != 0; j++){
				if (*(WORD*)(memebase + 0x3BE) == optionalIDs[j]){
					found = true;
					break;
				}
			}
		}
		if (found == true){
			//cout << "FUCK: " << hex << *(WORD*)(memebase + 0x3BE) << "\tWHAT: " << *(int*)(memebase + 0x288) << endl;
			if (*(int*)(memebase + 0x288) == 0){
				memelol[0] = 0x04;
				memelol[1] = 0x01;
				memelol[2] = 0x24;
				memelol[3] = *(BYTE*)(memebase + 0x234);
				memelol[4] = *(BYTE*)(memebase + 0x235);

				sendCrypt(sendSocket_r, memelol, 5, 0);
				Sleep(1);
			}
		}	
	}
	//Sleep(1000);
	delete memelol;


	cout << "Number of Entities: " << entityCounter << endl;
	Sleep(200);

	float shortestDistance = 0;
	float *shortestEntityX = NULL;
	float *shortestEntityY = NULL;
	DWORD shortestEntity = 0;
	//DWORD entityHealth = 0;	
	for (int i = 0; entities[i] != 0; i++){
		//cout << "sup ";
		cout << hex << "entities[i]: " << entities[i] << endl;
		cout << "Getting Base: " << (DWORD*)(entities[i] + 0x08) << endl;
		//cout << "umm1" << endl;
		DWORD entityBase = *(DWORD*)(entities[i] + 0x08);
		if (entityBase < fiestaBase || entityBase > 0x7ffff000){
			//i++;
			continue;
		}
		//cout << "umm2" << endl;
		//if (entityBase < fies	taBase) continue;
		cout << hex << "entityBase: " << entityBase << endl;
		cout << hex << "MobID address: " << (WORD*)(entityBase + 0x3BE) << endl;
		WORD currentMobID = *(WORD*)(entityBase + 0x3BE);
		bool foundMob = false;
		for (int j = 0; j < 20; j++){
			if (ids[j] > 0){
				if (currentMobID == ids[j]){
					foundMob = true;
					break;
				}
			}

			if (optionalIDs[j] > 0){
				if (currentMobID == optionalIDs[j]){
					foundMob = true;
					break;
				}
			}
		}
		cout << "ugh sucky" << endl;
		if (foundMob == false) continue;
		cout << "wtf now dude" << endl;
		cout << hex << "currentMobID: " << *(WORD*)(entityBase + 0x3BE) << endl;

		//cout << "Getting Details" << endl;
		cout << "entityDetails to dereference: " << (DWORD*)(entityBase + 0x2cc) << endl;
		DWORD* entityDeref = (DWORD*)(entityBase + 0x2cc);
		if (entityDeref == 0) continue;
/*		BYTE entityDetails1 = static_cast<unsigned>(*(BYTE*)entityDeref);
		cout << "1: " << entityDetails1 << endl;
		BYTE entityDetails2 = static_cast<unsigned>(*(BYTE*)(entityDeref + 1));
		cout << "2: " << entityDetails2 << endl;
		BYTE entityDetails3 = static_cast<unsigned>(*(BYTE*)(entityDeref + 2));
		cout << "3: " << entityDetails3 << endl;
		BYTE entityDetails4 = static_cast<unsigned>(*(BYTE*)(entityDeref + 3));
		cout << "4: " << entityDetails4 << endl;*/
		DWORD entityDetails = *(DWORD*)entityDeref;
		cout << hex << "entityDetails: " << entityDetails << endl;
		/*if (entityDetails < fiestaBase || entityDetails > 0x7f000000 || entityDetails < 0x10000000){
			//i++;
			continue;
		}*/
		if (entityDetails < fiestaBase){
			//i++;
			continue;
		}
		//cout << "umm3" << endl;

		WORD currentSelectionId = *(WORD*)(entityBase + 0x234);
		cout << hex << "currentSelectionId: " << *(WORD*)(entityBase + 0x234) << endl;
		if (*(WORD*)(entityBase + 0x234) == 0) continue;

		int entityHealth = *(int*)(entityBase + 0x288);
		cout << hex << "entityHealth: " << *(int*)(entityBase + 0x288) << endl;
		cout << "entity X addy: " << (float*)(entityDetails + 0x58) << "\tentity Y addy: " << (float*)(entityDetails + 0x5C) << endl;
		//cout << "umm4" << endl;
/*		BYTE entityXbyte = static_cast<unsigned>(*(BYTE*)(entityDetails + 0x58 + 0));
		cout << hex << "1: " << entityXbyte << endl;
		BYTE entityXbyte2 = static_cast<unsigned>(*(BYTE*)(entityDetails + 0x58 + 1));
		cout << "2: " << entityXbyte2 << endl;
		BYTE entityXbyte3 = static_cast<unsigned>(*(BYTE*)(entityDetails + 0x58 + 2));
		cout << "3: " << entityXbyte3 << endl;
		BYTE entityXbyte4 = static_cast<unsigned>(*(BYTE*)(entityDetails + 0x58 + 3));
		cout << "4: " << entityXbyte4 << endl;

		BYTE entityYbyte = static_cast<unsigned>(*(BYTE*)(entityDetails + 0x5C + 0));
		cout << "1: " << entityYbyte << endl;
		BYTE entityYbyte2 = static_cast<unsigned>(*(BYTE*)(entityDetails + 0x5C + 1));
		cout << "2: " << entityYbyte2 << endl;
		BYTE entityYbyte3 = static_cast<unsigned>(*(BYTE*)(entityDetails + 0x5C + 2));
		cout << "3: " << entityYbyte3 << endl;
		BYTE entityYbyte4 = static_cast<unsigned>(*(BYTE*)(entityDetails + 0x5C + 3));
		cout << "4: " << entityYbyte4 << endl;*/

		float *currentEntityX = (float*)(entityDetails + 0x58);
		float *currentEntityY = (float*)(entityDetails + 0x5C);

		if (currentEntityX == NULL || currentEntityY == NULL) continue;

		float distToPlayer = sqrt(pow((*(playerX)-*currentEntityX), 2) + pow((*(playerY)-*currentEntityY), 2));
		cout << "umm1" << endl;
		//distToCenter is the distance to a center point around which the player should kill. Anything greater than a distance of whatever won't be targetted
		float distToCenter = sqrt(pow((initialX-*currentEntityX), 2) + pow((initialY-*currentEntityY), 2));
		cout << "umm2" << endl;
		bool found = false;
		for (int i = 0; i < 20 && ids[i] != 0; i++){
			if (currentMobID == ids[i]){
				found = true;
				break;
			}
		}

		if (found == false && distToPlayer <= 200){
			for (int i = 0; i < 20 && optionalIDs[i] != 0; i++){
				if (currentMobID == optionalIDs[i]){
					found = true;
					break;
				}
			}
		}
		if (found == true){
			if (distToCenter >= killRadius) continue;
			//cout << "Selection ID: " << *(WORD*)(entityBase + 0x234);
			//cout << "\tHealth: " << *(int*)(entityBase + 0x288);
			//cout << "\tDistance: " << distance << endl;

			if (shortestDistance == 0 && entityHealth > 0){
				shortestDistance = distToPlayer;
				shortestEntity = entities[i];
				shortestEntityX = (float*)(entityDetails + 0x58);
				shortestEntityY = (float*)(entityDetails + 0x5C);
			}
			else if (distToPlayer <= shortestDistance && entityHealth > 0){

				if (lastEntity != entities[i]){
					shortestDistance = distToPlayer;
					shortestEntity = entities[i];
					shortestEntityX = (float*)(entityDetails + 0x58);
					shortestEntityY = (float*)(entityDetails + 0x5C);
				}
			}
		}
		//cout << "umm5" << endl;
		//cout << hex << "entityDetails: " << entityDetails << endl;
		//if (entityDetails < 40000000) continue;

		//cout << hex << entityBase << endl;

		//for (int i = 0; *(BYTE*)(entityBase + 0x236 + i) != '\0' && i < 30; i++){
		//	name[i] = *(BYTE*)(entityBase + 0x236 + i);
		//}
		//cout << "Address: " << hex << entityDetails << endl;

		//cout << "\tName: ";
		//for (int i = 0; name[i] != 0 && i < 30; i++) cout << name[i];

		//cout << "Selection ID: " << *(WORD*)(entityBase + 0x234) << endl;
		//cout << "Mob ID: " << dec << *(WORD*)(entityBase + 0x3BE);
		//cout << "\tHealth: " << *(int*)(entityBase + 0x288);

		//cout << "\tX: ";
		//cout << *(float*)(entityDetails + 0x58);
		//cout << "\tY: ";
		//cout << *(float*)(entityDetails + 0x5C);

		//float distance = distanceFunc(*playerX, *playerY, *(float*)(entityDetails + 0x58), *(float*)(entityDetails + 0x5C));
		cout << "\tDistance: " << distToPlayer << endl;
		//cout << "\n";

		//for (int i = 0; name[i] != 0 && i < 30; i++) name[i] = 0;

	}
	//cout << "lol" << endl;

	if (shortestEntity > 0){


		LARGE_INTEGER tcounter;
		LONGLONG ticknow, tickAttack;
		LONGLONG freq;
		QueryPerformanceFrequency(&tcounter);
		freq = tcounter.QuadPart;

		QueryPerformanceCounter(&tcounter);
		ticknow = tcounter.QuadPart;
		tickAttack = tcounter.QuadPart;

		DWORD shortestBase = *(DWORD*)(shortestEntity + 0x08);
		DWORD shortestDetails = *(DWORD*)(shortestBase + 0x2cc);
		int *shortestHealth = (int*)(shortestBase + 0x288);
		//cout << "Yep!" << endl;
		//cout << "Shortest Address: " << shortestEntity << endl;
		lastEntity = shortestEntity;
		unsigned char id[2] = { *(BYTE*)(shortestBase + 0x234), *(BYTE*)(shortestBase + 0x235) };
		//cout << "\nKilling: " << hex << static_cast<unsigned>(id[1]) << static_cast<unsigned>(id[0]) << endl;
		//getchar();
		// for (int i = 0; name[i] != 0 && i < 30; i++) cout << name[i];
		char* target = new char[5];
		//char* attack = new char[3];
		//char* battleMode = new char[4];
		target[0] = 0x04;
		target[1] = 0x01;
		target[2] = 0x24;
		target[3] = id[0];
		target[4] = id[1];

		sendCrypt(sendSocket_r, target, 5, 0);
		Sleep(1000);
		//cout << "BEEP\n";
		IDtoBYTE playerstuff;
		playerstuff.bytes[0] = playerID[0];
		playerstuff.bytes[1] = playerID[1];
		int previousHealth = *shortestHealth;
		float distToCenter = sqrt(pow((initialX - *shortestEntityX), 2) + pow((initialY - *shortestEntityY), 2));
		//cout << "LALALAL: " << targetofTargetID << endl;
		while (*shortestHealth > 0){
			if (distToCenter >= killRadius) break;
			cout << "shortestEntityX: " << shortestEntityX << "\tshortestEntityY: " << shortestEntityY << endl;
			distToCenter = sqrt(pow((initialX - *shortestEntityX), 2) + pow((initialY - *shortestEntityY), 2));
			cout << "distToCenter: " << distToCenter << endl;
			lastPosX = *playerX;
			lastPosY = *playerY;


			if (*(WORD*)targetofTargetID != playerstuff.id && *(WORD*)targetofTargetID != 0xFFFF){
				Sleep(500);
				break;
			}

			if ((ticknow - tickAttack) / (freq / 1000) >= (10 * 1000)) break;

			if (GetAsyncKeyState(VK_F1)) break;

			if (*health > *maxhealth * 0.65){
				checkBuff(true);
				applyBuffs(playerID);

				useSkill(&MC, id, 0, 0);
				useSkill(&Bash, id, 0, 0);
				useSkill(&Bleed, id, 0, 0);
				useSkill(&DB, playerID, 0, 0);
			}

			cout << "Health: " << dec << *shortestHealth << endl;
			if (*health <= *maxhealth * 0.65){
				if (*health == 0) break;
				useSkill(&Heal, playerID, 0, 0);
			}
			if (*health <= *maxhealth * 0.35){
				useHPStone();
			}
			if (*mana < *maxmana * 0.40 && *mana > *maxmana * 0.30){
				useSPStone();
				Sleep(100);
			}
			if (*mana < *maxmana * 0.10){
				DownKey('6', file_kbd);
				Sleep(60);
				ReleaseKey('6', file_kbd);
				Sleep(500);
			}
			//Sleep(500);
			DownKey('1', file_kbd);
			Sleep(60);
			ReleaseKey('1', file_kbd);

			if (*shortestHealth == previousHealth){ // if health is the same, continue counting
				QueryPerformanceCounter(&tcounter);
				ticknow = tcounter.QuadPart;
			}
			else {
				QueryPerformanceCounter(&tcounter); //if health is different, reset timer
				ticknow = tcounter.QuadPart;
				tickAttack = tcounter.QuadPart;
				previousHealth = *shortestHealth;
			}

		}
	}
	//cout << "Done!\n";
}

void mobESPGrind_Crus(int ids[20], int optionalIDs[20]){
	entityPointer = (fiestaBase + 0x700C58);  //base pointer/pointer to me
	cout << "hmmm" << endl;
	//DWORD entity = *(DWORD*)entityPointer;
	DWORD entities[1024] = { 0 };
	DWORD entityName, entityID;
	char name[30] = { 0 };

	//entityPointer = (DWORD)(fiestaBase + 0x704E28);
	entities[0] = *(DWORD*)(entityPointer + 0x3C);
	if (playerID[0] == 0xFF && playerID[1] == 0xFF){
		DWORD player = *(DWORD*)(entities[0] + 0x08);
		playerID[0] = *(BYTE*)(player + 0x234);
		playerID[1] = *(BYTE*)(player + 0x235);
	}
	//cout << hex << entities[0] << endl;

	int entityCounter = 1, entityBackup = 1;
	int i = 0;
	bool unique = true;
	while (1){
		if (*(DWORD*)entities[i] != 0){
			for (int i = 0; entities[i] != 0; i++){
				if (*(DWORD*)entities[i] == entities[i]){
					unique = false;
					break;
				}
			}

			if (unique == true){
				entities[entityCounter] = *(DWORD*)entities[i];
				entityCounter++;
			}
		}

		i++;
		if (entities[i] == 0) break;
		unique = true;
	}

	char* memelol = new char[5];
	DWORD memebase = 0;
	for (int i = 0; entities[i] != 0; i++){
		memebase = *(DWORD*)(entities[i] + 0x08);
		if (memebase == 0 || memebase > entities[i] + 0x30000000){
			//i++;
			continue;
		}
		bool found = false;
		for (int j = 0; j < 20 && ids[j] != 0; j++){
			if (*(WORD*)(memebase + 0x3BE) == ids[j]){
				found = true;
				break;
			}
		}
		if (found == false){
			for (int j = 0; j < 20 && optionalIDs[j] != 0; j++){
				if (*(WORD*)(memebase + 0x3BE) == optionalIDs[j]){
					found = true;
					break;
				}
			}
		}
		if (found == true){
			//cout << "FUCK: " << hex << *(WORD*)(memebase + 0x3BE) << "\tWHAT: " << *(int*)(memebase + 0x288) << endl;
			if (*(int*)(memebase + 0x288) == 0){
				memelol[0] = 0x04;
				memelol[1] = 0x01;
				memelol[2] = 0x24;
				memelol[3] = *(BYTE*)(memebase + 0x234);
				memelol[4] = *(BYTE*)(memebase + 0x235);

				sendCrypt(sendSocket_r, memelol, 5, 0);
				Sleep(1);
			}
		}
	}
	//Sleep(1000);
	delete memelol;


	cout << "Number of Entities: " << entityCounter << endl;
	Sleep(200);
	float shortestDistance = 0;
	float *shortestEntityX = NULL;
	float *shortestEntityY = NULL;
	DWORD shortestEntity = 0;
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetCurrentProcessId());
	//DWORD entityHealth = 0;	
	for (int i = 0; entities[i] != 0; i++){
		WORD membuf = 0;
		float coordbuff = 0;
		//cout << "sup ";
		cout << hex << "entities[i]: " << entities[i] << endl;
		cout << "Getting Base: " << (DWORD*)(entities[i] + 0x08) << endl;
		//cout << "umm1" << endl;
		DWORD entityBase = 0;

		if (isMemReadable((LPCVOID)(entities[i] + 0x08), sizeof(DWORD))) entityBase = *(DWORD*)(entities[i] + 0x08);
		else continue;
		//DWORD entityBase = *(DWORD*)(entities[i] + 0x08);
		if (entityBase < fiestaBase || entityBase > 0x7ffff000){
			//i++;
			continue;
		}
		//cout << "umm2" << endl;
		//if (entityBase < fies	taBase) continue
		cout << hex << "entityBase: " << entityBase << endl;
		cout << hex << "MobID address: " << (WORD*)(entityBase + 0x3BE) << endl;

		WORD currentMobID = 0;
		if (isMemReadable((LPCVOID)(entityBase + 0x3BE), sizeof(WORD))) currentMobID = *(WORD*)(entityBase + 0x3BE);
		else continue;
		/*cout << hex << "address: " << GetCurrentProcessId() << endl;
		int read_result = ReadProcessMemory(hProcess, (LPCVOID)(entityBase + 0x3BE), &membuf, sizeof(WORD), NULL);
		cout << "ReadProcessMemory on MobID: " << read_result << endl;
		if (read_result == 0){
			cout << "RPM error: " << GetLastError() << endl;
			continue;
		}
		cout << "Read result: " << membuf << endl;*/
		//WORD currentMobID = *(WORD*)(entityBase + 0x3BE);
		bool foundMob = false;
		for (int j = 0; j < 20; j++){
			if (ids[j] > 0){
				if (currentMobID == ids[j]){
					foundMob = true;
					break;
				}
			}

			if (optionalIDs[j] > 0){
				if (currentMobID == optionalIDs[j]){
					foundMob = true;
					break;
				}
			}
		}
		cout << "ugh sucky" << endl;
		if (foundMob == false) continue;
		cout << "wtf now dude" << endl;
		cout << hex << "currentMobID: " << currentMobID << endl;

		//cout << "Getting Details" << endl;
		cout << "entityDetails to dereference: " << (DWORD*)(entityBase + 0x2cc) << endl;
		DWORD* entityDeref = (DWORD*)(entityBase + 0x2cc);
		if (entityDeref == 0) continue;
		/*		BYTE entityDetails1 = static_cast<unsigned>(*(BYTE*)entityDeref);
		cout << "1: " << entityDetails1 << endl;
		BYTE entityDetails2 = static_cast<unsigned>(*(BYTE*)(entityDeref + 1));
		cout << "2: " << entityDetails2 << endl;
		BYTE entityDetails3 = static_cast<unsigned>(*(BYTE*)(entityDeref + 2));
		cout << "3: " << entityDetails3 << endl;
		BYTE entityDetails4 = static_cast<unsigned>(*(BYTE*)(entityDeref + 3));
		cout << "4: " << entityDetails4 << endl;*/
		DWORD entityDetails = 0;
		if (isMemReadable((LPCVOID)entityDeref, sizeof(DWORD))) entityDetails = *(DWORD*)entityDeref;
		//DWORD entityDetails = *(DWORD*)entityDeref;
		cout << hex << "entityDetails: " << entityDetails << endl;
		/*if (entityDetails < fiestaBase || entityDetails > 0x7f000000 || entityDetails < 0x10000000){
		//i++;
		continue;
		}*/
		if (entityDetails < fiestaBase){
			//i++;
			continue;
		}
		//cout << "umm3" << endl;

		WORD currentSelectionId = *(WORD*)(entityBase + 0x234);
		cout << hex << "currentSelectionId: " << *(WORD*)(entityBase + 0x234) << endl;
		if (*(WORD*)(entityBase + 0x234) == 0) continue;

		int entityHealth = *(int*)(entityBase + 0x288);
		cout << hex << "entityHealth: " << *(int*)(entityBase + 0x288) << endl;
		cout << "entity X addy: " << (float*)(entityDetails + 0x58) << "\tentity Y addy: " << (float*)(entityDetails + 0x5C) << endl;
		//cout << "umm4" << endl;
		/*		BYTE entityXbyte = static_cast<unsigned>(*(BYTE*)(entityDetails + 0x58 + 0));
		cout << hex << "1: " << entityXbyte << endl;
		BYTE entityXbyte2 = static_cast<unsigned>(*(BYTE*)(entityDetails + 0x58 + 1));
		cout << "2: " << entityXbyte2 << endl;
		BYTE entityXbyte3 = static_cast<unsigned>(*(BYTE*)(entityDetails + 0x58 + 2));
		cout << "3: " << entityXbyte3 << endl;
		BYTE entityXbyte4 = static_cast<unsigned>(*(BYTE*)(entityDetails + 0x58 + 3));
		cout << "4: " << entityXbyte4 << endl;

		BYTE entityYbyte = static_cast<unsigned>(*(BYTE*)(entityDetails + 0x5C + 0));
		cout << "1: " << entityYbyte << endl;
		BYTE entityYbyte2 = static_cast<unsigned>(*(BYTE*)(entityDetails + 0x5C + 1));
		cout << "2: " << entityYbyte2 << endl;
		BYTE entityYbyte3 = static_cast<unsigned>(*(BYTE*)(entityDetails + 0x5C + 2));
		cout << "3: " << entityYbyte3 << endl;
		BYTE entityYbyte4 = static_cast<unsigned>(*(BYTE*)(entityDetails + 0x5C + 3));
		cout << "4: " << entityYbyte4 << endl;*/

		float *currentEntityX = (float*)(entityDetails + 0x58);
		float *currentEntityY = (float*)(entityDetails + 0x5C);

		if (currentEntityX == NULL || currentEntityY == NULL) continue;

		float distToPlayer = sqrt(pow((*(playerX)-*currentEntityX), 2) + pow((*(playerY)-*currentEntityY), 2));
		cout << "umm1" << endl;
		//distToCenter is the distance to a center point around which the player should kill. Anything greater than a distance of whatever won't be targetted
		float distToCenter = sqrt(pow((initialX - *currentEntityX), 2) + pow((initialY - *currentEntityY), 2));
		cout << "umm2" << endl;
		bool found = false;
		for (int i = 0; i < 20 && ids[i] != 0; i++){
			if (currentMobID == ids[i]){
				found = true;
				break;
			}
		}

		if (found == false && distToPlayer <= 300){
			for (int i = 0; i < 20 && optionalIDs[i] != 0; i++){
				if (currentMobID == optionalIDs[i]){
					found = true;
					break;
				}
			}
		}
		if (found == true){
			if (distToCenter >= killRadius) continue;
			//cout << "Selection ID: " << *(WORD*)(entityBase + 0x234);
			//cout << "\tHealth: " << *(int*)(entityBase + 0x288);
			//cout << "\tDistance: " << distance << endl;

			if (shortestDistance == 0 && entityHealth > 0){
				shortestDistance = distToPlayer;
				shortestEntity = entities[i];
				shortestEntityX = (float*)(entityDetails + 0x58);
				shortestEntityY = (float*)(entityDetails + 0x5C);
			}
			else if (distToPlayer <= shortestDistance && entityHealth > 0){

				if (lastEntity != entities[i]){
					shortestDistance = distToPlayer;
					shortestEntity = entities[i];
					shortestEntityX = (float*)(entityDetails + 0x58);
					shortestEntityY = (float*)(entityDetails + 0x5C);
				}
			}
		}
		//cout << "umm5" << endl;
		//cout << hex << "entityDetails: " << entityDetails << endl;
		//if (entityDetails < 40000000) continue;

		//cout << hex << entityBase << endl;

		//for (int i = 0; *(BYTE*)(entityBase + 0x236 + i) != '\0' && i < 30; i++){
		//	name[i] = *(BYTE*)(entityBase + 0x236 + i);
		//}
		//cout << "Address: " << hex << entityDetails << endl;

		//cout << "\tName: ";
		//for (int i = 0; name[i] != 0 && i < 30; i++) cout << name[i];

		//cout << "Selection ID: " << *(WORD*)(entityBase + 0x234) << endl;
		//cout << "Mob ID: " << dec << *(WORD*)(entityBase + 0x3BE);
		//cout << "\tHealth: " << *(int*)(entityBase + 0x288);

		//cout << "\tX: ";
		//cout << *(float*)(entityDetails + 0x58);
		//cout << "\tY: ";
		//cout << *(float*)(entityDetails + 0x5C);

		//float distance = distanceFunc(*playerX, *playerY, *(float*)(entityDetails + 0x58), *(float*)(entityDetails + 0x5C));
		cout << "\tDistance: " << distToPlayer << endl;
		//cout << "\n";

		//for (int i = 0; name[i] != 0 && i < 30; i++) name[i] = 0;

	}
	cout << "lollers" << endl;
	CloseHandle(hProcess);
	cout << "lol 1\n";
	if (shortestEntity > 0){

		LARGE_INTEGER tcounter;
		LONGLONG ticknow, tickAttack;
		LONGLONG freq;
		QueryPerformanceFrequency(&tcounter);
		freq = tcounter.QuadPart;

		QueryPerformanceCounter(&tcounter);
		ticknow = tcounter.QuadPart;
		tickAttack = tcounter.QuadPart;
		DWORD shortestBase = 0;
		if (isMemReadable((LPCVOID)(shortestEntity + 0x08), sizeof(DWORD))) shortestBase = *(DWORD*)(shortestEntity + 0x08);
		else return;

		//DWORD shortestBase = *(DWORD*)(shortestEntity + 0x08); 
		cout << "heh" << endl;
		DWORD shortestDetails = 0;
		if (isMemReadable((LPCVOID)(shortestBase + 0x2cc), sizeof(DWORD))) shortestDetails = *(DWORD*)(shortestBase + 0x2cc);
		else return;

		//DWORD shortestDetails = *(DWORD*)(shortestBase + 0x2cc);
		cout << "wow" << endl;
		int *shortestHealth = (int*)(shortestBase + 0x288);
		cout << "dope" << endl;
		//cout << "Yep!" << endl;
		//cout << "Shortest Address: " << shortestEntity << endl;
		lastEntity = shortestEntity;
		unsigned char id[2] = { *(BYTE*)(shortestBase + 0x234), *(BYTE*)(shortestBase + 0x235) };
		cout << "great" << endl;
		//while (LOOTTRAVEL == true) Sleep(200);
		//cout << "\nKilling: " << hex << static_cast<unsigned>(id[1]) << static_cast<unsigned>(id[0]) << endl;
		//getchar();
		// for (int i = 0; name[i] != 0 && i < 30; i++) cout << name[i];
		char* target = new char[5];
		//char* attack = new char[3];
		//char* battleMode = new char[4];
		target[0] = 0x04;
		target[1] = 0x01;
		target[2] = 0x24;
		target[3] = id[0];
		target[4] = id[1];

		sendCrypt(sendSocket_r, target, 5, 0);
		if(stolenAggro == true) Sleep(1000);
		//cout << "BEEP\n";
		IDtoBYTE playerstuff;
		playerstuff.bytes[0] = playerID[0];
		playerstuff.bytes[1] = playerID[1];
		int previousHealth = *shortestHealth;
		float distToCenter = sqrt(pow((initialX - *shortestEntityX), 2) + pow((initialY - *shortestEntityY), 2));
		float distToPlayer = sqrt(pow((*(playerX)-*shortestEntityX), 2) + pow((*(playerY)-*shortestEntityY), 2));
		cout << "LALALAL: " << endl;
		bool useMoonlight = false;
		bool useSunlight = false;
		while (*shortestHealth > 0){
			if (LOOTTRAVEL == true){
				if (useSkill(&Lightjump, id, TRAVELX, TRAVELY)) LOOTTRAVEL = false;
				//Sleep(1000);
				//Sleep(1500);
				/*while (LOOTTRAVEL == true){
					if (*playerX - TRAVELX > 63){ //X
						DownKey('a', file_kbd);
						while (*playerX - TRAVELX > 63) Sleep(10);
						ReleaseKey('a', file_kbd);
					}

					if (*playerX - TRAVELX < -63){
						DownKey('d', file_kbd);
						while (*playerX - TRAVELX < -63) Sleep(10);
						ReleaseKey('d', file_kbd);
					}

					if (*playerY - TRAVELY > 63){//Y
						DownKey('s', file_kbd);
						while (*playerY - TRAVELY > 63) Sleep(10);
						ReleaseKey('s', file_kbd);
					}

					if (*playerY - TRAVELY < -63){
						DownKey('w', file_kbd);
						while (*playerY - TRAVELY < -63) Sleep(10);
						ReleaseKey('w', file_kbd);
					}

					LOOTTRAVEL = false;
					Sleep(100);
			}*/
			}
			if (distToCenter >= killRadius) break;
			cout << "shortestEntityX: " << shortestEntityX << "\tshortestEntityY: " << shortestEntityY << endl;
			distToCenter = sqrt(pow((initialX - *shortestEntityX), 2) + pow((initialY - *shortestEntityY), 2));
			distToPlayer = sqrt(pow((*(playerX)-*shortestEntityX), 2) + pow((*(playerY)-*shortestEntityY), 2));
			cout << "distToCenter: " << distToCenter << "\tdistToPlayer: " << distToPlayer << endl;
			lastPosX = *playerX;
			lastPosY = *playerY;


			/*if (*(WORD*)targetofTargetID != playerstuff.id && *(WORD*)targetofTargetID != 0xFFFF){
				Sleep(500);
				stolenAggro = true;
				break;
			}*/

			//else stolenAggro = false;

			if ((ticknow - tickAttack) / (freq / 1000) >= (10 * 1000)) break;

			if (GetAsyncKeyState(VK_F1)) break;

			//if (*health > *maxhealth * 0.65){
				//checkBuff(true);
			//Crusader shit, using this as a bot for healbot
			/*applyBuffs_Crus(playerID);

			useSkill(&Lifeline, playerID, 0, 0);

			if (distToPlayer <= 70){
				if (useMoonlight == true) {
					if (useSkill(&Moonlight, playerID, 0, 0)) useMoonlight = false;
				}
				if (useSunlight == true) {
					if (useSkill(&Sunlight, playerID, 0, 0)) useSunlight = false;
				}
				useSkill(&LightTouch, playerID, 0, 0);
				useSkill(&Advent, id, *playerX, *playerY);
				useSkill(&LightStrike, id, 0, 0);
				useSkill(&HeroicStrike, id, 0, 0);
				useSkill(&LightBlast, id, 0, 0);
			}*/

			if (distToPlayer <= 50){
				useSkill(&healbot_Bash, id, 0, 0);
			}
			//}

			cout << "Health: " << dec << *shortestHealth << endl;
			if (*health == 0) break;

			if (*health <= *maxhealth * 0.45){
				//useHPStone();
				//useMoonlight = true;
				useSkill(&healbot_Heal, playerID, 0, 0);
			}

			if (*health <= *maxhealth * 0.30){
				DownKey('=', file_kbd);
				Sleep(100);
				ReleaseKey('=', file_kbd);
			}

			//if (*lp <= 30) useSunlight = true;
			/*if (*mana < *maxmana * 0.40 && *mana > *maxmana * 0.30){
				useSPStone();
				Sleep(100);
			}
			if (*mana < *maxmana * 0.10){
				DownKey('6', file_kbd);
				Sleep(60);
				ReleaseKey('6', file_kbd);
				Sleep(500);
			}*/
			//Sleep(500);
			if (distToPlayer > 50){
				DownKey('1', file_kbd);
				Sleep(60);
				ReleaseKey('1', file_kbd);
			}

			cout << "Attack timeout timer: " << (ticknow - tickAttack) / (freq / 1000) << endl;
			if (*shortestHealth == previousHealth){ // if health is the same, continue counting
				QueryPerformanceCounter(&tcounter);
				ticknow = tcounter.QuadPart;
			}
			else {
				QueryPerformanceCounter(&tcounter); //if health is different, reset timer
				ticknow = tcounter.QuadPart;
				tickAttack = tcounter.QuadPart;
				previousHealth = *shortestHealth;
			}

		}
	}
	//cout << "Done!\n";
}

bool healbot_applyBuffs(unsigned char *id, unsigned char *partyid){

	//06 40 24 b4 0f 33 14
	bool used = 0;
	LARGE_INTEGER tcounter;
	LONGLONG ticknow, tickCast;
	LONGLONG freq;
	QueryPerformanceFrequency(&tcounter);
	freq = tcounter.QuadPart;

	QueryPerformanceCounter(&tcounter);
	ticknow = tcounter.QuadPart;

	if ((ticknow - buffsCooldown) / (freq / 1) <= 3480 && buffsCooldown != 0) return false;
	cout << "me use buffs\n";
	/*SEND ON SOCKET : 8d0
	04 15 30 09 09 0
	SEND ON SOCKET : 8d0 use scrolls on slot 1 and 2 of backpack
	04 15 30 01 09 0 */
	useSPStone();

	Sleep(600);
	while (useSkill(&healbot_Protect, id, 0, 0) == false){
		if (*health <= *maxhealth * 0.60){
			useHPStone();
		}
	}

	Sleep(2500);
	while (useSkill(&healbot_Resist, id, 0, 0) == false){
		if (*health <= *maxhealth * 0.60){
			useHPStone();
		}
	}
	Sleep(2500);
	while (useSkill(&healbot_Endure, partyid, 0, 0) == false){
		if (*health <= *maxhealth * 0.60){
			useHPStone();
		}
	}
	Sleep(1000);

	QueryPerformanceCounter(&tcounter);
	buffsCooldown = tcounter.QuadPart;

	return true;
}

void healBot(){
	entityPointer = (fiestaBase + 0x700C58);
	DWORD playerEntity = *(DWORD*)(entityPointer + 0x3C);
	DWORD player = *(DWORD*)(playerEntity + 0x08);
	unsigned char playerIDyeet[2] = { *(BYTE*)(player + 0x234), *(BYTE*)(player + 0x235) };

	cout << "waiting to join party..." << endl;
	while (*partyMemberMaxHP == 0) Sleep(10);
	cout << "Get party member ID" << endl;
	partyMemberID[0] = 0;
	partyMemberID[1] = 0;
	while (partyMemberID[0] == 0 && partyMemberID[1] == 0) Sleep(10);

	if (sendSocket_r == 0){
		cout << "Jump to get send socket!\n";
		while (sendSocket_r == 0) Sleep(1000);
	}

	while (1){
		cout << "Party member's health is: " << *partyMemberHP << endl;
		playerEntity = *(DWORD*)(entityPointer + 0x3C);
		player = *(DWORD*)(playerEntity + 0x08);

		if (playerIDyeet[0] != *(BYTE*)(player + 0x234) || playerIDyeet[1] != *(BYTE*)(player + 0x235)){
			playerIDyeet[0] = *(BYTE*)(player + 0x234);
			playerIDyeet[1] = *(BYTE*)(player + 0x235);
		}

		if (getEntityDistance(partyMemberID) > 450){
			if (injectKey == false)
				injectDInput(DIK_5, .5);
		}

		if (*mana <= *maxmana * 0.4) useSPStone();

		healbot_applyBuffs(playerIDyeet, partyMemberID);
		checkBuff(0);
		if (castAoe == true){
			while (useSkill(&healbot_Cure, partyMemberID, 0, 0) == false) Sleep(10);
			castAoe = false;
		}
		if (*partyMemberHP <= *partyMemberMaxHP * 0.60){
			useSkill(&healbot_Heal, partyMemberID, 0, 0);
			if (injectKey == false)
				injectDInput(DIK_5, .5);
		}
		//if (*partyMemberHP >= *partyMemberMaxHP * 0.80 && *partyMemberHP <= *partyMemberMaxHP * 0.90) useSkill(&healbot_Restore, partyMemberID, 0, 0);
		if (*health <= *maxhealth * 0.70){
			if (useSkill(&healbot_Heal, playerIDyeet, 0, 0) == false){
				Sleep(300);
				if (*health <= *maxhealth * 0.50){
					useHPPotion(0);
					Sleep(350);
				}
				if (*health <= *maxhealth * 0.50) useHPStone();
			}
		}
		Sleep(10);
	}
}

void mobESPGrind_Mage(int ids[20], int optionalIDs[20]){
	entityPointer = (fiestaBase + 0x700C58);  //base pointer/pointer to me
	cout << "hmmm" << endl;
	//DWORD entity = *(DWORD*)entityPointer;
	DWORD entities[1024] = { 0 };
	DWORD entityName, entityID;
	char name[30] = { 0 };

	//entityPointer = (DWORD)(fiestaBase + 0x704E28);
	entities[0] = *(DWORD*)(entityPointer + 0x3C);
	if (playerID[0] == 0xFF && playerID[1] == 0xFF){
		DWORD player = *(DWORD*)(entities[0] + 0x08);
		playerID[0] = *(BYTE*)(player + 0x234);
		playerID[1] = *(BYTE*)(player + 0x235);
	}
	//cout << hex << entities[0] << endl;

	int entityCounter = 1, entityBackup = 1;
	int i = 0;
	bool unique = true;
	while (1){
		if (*(DWORD*)entities[i] != 0){
			for (int i = 0; entities[i] != 0; i++){
				if (*(DWORD*)entities[i] == entities[i]){
					unique = false;
					break;
				}
			}

			if (unique == true){
				entities[entityCounter] = *(DWORD*)entities[i];
				entityCounter++;
			}
		}

		i++;
		if (entities[i] == 0) break;
		unique = true;
	}

	char* memelol = new char[5];
	int memecounter = 0;
	DWORD memebase = 0;
	for (int i = 0; entities[i] != 0; i++){
		//memebase = *(DWORD*)(entities[i] + 0x08);
		if (isMemReadable((LPCVOID)(entities[i] + 0x08), sizeof(DWORD))) memebase = *(DWORD*)(entities[i] + 0x08);
		//if (memebase == 0 || memebase > entities[i] + 0x30000000){
		if (memebase == 0){
			//i++;
			continue;
		}
		bool found = false;
		for (int j = 0; j < 20 && ids[j] != 0; j++){
			if (*(WORD*)(memebase + 0x3BE) == ids[j]){
				found = true;
				break;
			}
		}
		if (found == false){
			for (int j = 0; j < 20 && optionalIDs[j] != 0; j++){
				if (*(WORD*)(memebase + 0x3BE) == optionalIDs[j]){
					found = true;
					break;
				}
			}
		}
		if (found == true){
			//cout << "FUCK: " << hex << *(WORD*)(memebase + 0x3BE) << "\tWHAT: " << *(int*)(memebase + 0x288) << endl;
			if (*(int*)(memebase + 0x288) == 0){
				memecounter++;
				memelol[0] = 0x04;
				memelol[1] = 0x01;
				memelol[2] = 0x24;
				memelol[3] = *(BYTE*)(memebase + 0x234);
				memelol[4] = *(BYTE*)(memebase + 0x235);

				sendCrypt(sendSocket_r, memelol, 5, 0);
				Sleep(1);
			}
		}
	}
	//Sleep(1000);
	delete memelol;


	cout << "Number of Entities: " << entityCounter << endl;
	cout << "Number of Entities targetted: " << memecounter << endl;
	Sleep(200);
	float shortestDistance = 0;
	float *shortestEntityX = NULL;
	float *shortestEntityY = NULL;
	DWORD shortestEntity = 0;
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetCurrentProcessId());
	//DWORD entityHealth = 0;	
	for (int i = 0; entities[i] != 0; i++){
		WORD membuf = 0;
		float coordbuff = 0;
		//cout << "sup ";
		cout << hex << "entities[i]: " << entities[i] << endl;
		cout << "Getting Base: " << (DWORD*)(entities[i] + 0x08) << endl;
		//cout << "umm1" << endl;
		DWORD entityBase = 0;

		if (isMemReadable((LPCVOID)(entities[i] + 0x08), sizeof(DWORD))) entityBase = *(DWORD*)(entities[i] + 0x08);
		else continue;
		//DWORD entityBase = *(DWORD*)(entities[i] + 0x08);
		if (entityBase < fiestaBase || entityBase > 0x7ffff000){
			//i++;
			continue;
		}
		//cout << "umm2" << endl;
		//if (entityBase < fies	taBase) continue
		cout << hex << "entityBase: " << entityBase << endl;
		cout << hex << "MobID address: " << (WORD*)(entityBase + 0x3BE) << endl;

		WORD currentMobID = 0;
		if (isMemReadable((LPCVOID)(entityBase + 0x3BE), sizeof(WORD))) currentMobID = *(WORD*)(entityBase + 0x3BE);
		else continue;
		/*cout << hex << "address: " << GetCurrentProcessId() << endl;
		int read_result = ReadProcessMemory(hProcess, (LPCVOID)(entityBase + 0x3BE), &membuf, sizeof(WORD), NULL);
		cout << "ReadProcessMemory on MobID: " << read_result << endl;
		if (read_result == 0){
		cout << "RPM error: " << GetLastError() << endl;
		continue;
		}
		cout << "Read result: " << membuf << endl;*/
		//WORD currentMobID = *(WORD*)(entityBase + 0x3BE);
		bool foundMob = false;
		for (int j = 0; j < 20; j++){
			if (ids[j] > 0){
				if (currentMobID == ids[j]){
					foundMob = true;
					break;
				}
			}

			if (optionalIDs[j] > 0){
				if (currentMobID == optionalIDs[j]){
					foundMob = true;
					break;
				}
			}
		}
		cout << "ugh sucky" << endl;
		if (foundMob == false) continue;
		cout << "wtf now dude" << endl;
		cout << hex << "currentMobID: " << currentMobID << endl;

		//cout << "Getting Details" << endl;
		cout << "entityDetails to dereference: " << (DWORD*)(entityBase + 0x2cc) << endl;
		DWORD* entityDeref = (DWORD*)(entityBase + 0x2cc);
		if (entityDeref == 0) continue;
		/*		BYTE entityDetails1 = static_cast<unsigned>(*(BYTE*)entityDeref);
		cout << "1: " << entityDetails1 << endl;
		BYTE entityDetails2 = static_cast<unsigned>(*(BYTE*)(entityDeref + 1));
		cout << "2: " << entityDetails2 << endl;
		BYTE entityDetails3 = static_cast<unsigned>(*(BYTE*)(entityDeref + 2));
		cout << "3: " << entityDetails3 << endl;
		BYTE entityDetails4 = static_cast<unsigned>(*(BYTE*)(entityDeref + 3));
		cout << "4: " << entityDetails4 << endl;*/
		DWORD entityDetails = 0;
		if (isMemReadable((LPCVOID)entityDeref, sizeof(DWORD))) entityDetails = *(DWORD*)entityDeref;
		//DWORD entityDetails = *(DWORD*)entityDeref;
		cout << hex << "entityDetails: " << entityDetails << endl;
		/*if (entityDetails < fiestaBase || entityDetails > 0x7f000000 || entityDetails < 0x10000000){
		//i++;
		continue;
		}*/
		if (entityDetails < fiestaBase){
			//i++;
			continue;
		}
		//cout << "umm3" << endl;

		WORD currentSelectionId = *(WORD*)(entityBase + 0x234);
		cout << hex << "currentSelectionId: " << *(WORD*)(entityBase + 0x234) << endl;
		if (*(WORD*)(entityBase + 0x234) == 0) continue;

		int entityHealth = *(int*)(entityBase + 0x288);
		cout << hex << "entityHealth: " << *(int*)(entityBase + 0x288) << endl;
		cout << "entity X addy: " << (float*)(entityDetails + 0x58) << "\tentity Y addy: " << (float*)(entityDetails + 0x5C) << endl;
		//cout << "umm4" << endl;
		/*		BYTE entityXbyte = static_cast<unsigned>(*(BYTE*)(entityDetails + 0x58 + 0));
		cout << hex << "1: " << entityXbyte << endl;
		BYTE entityXbyte2 = static_cast<unsigned>(*(BYTE*)(entityDetails + 0x58 + 1));
		cout << "2: " << entityXbyte2 << endl;
		BYTE entityXbyte3 = static_cast<unsigned>(*(BYTE*)(entityDetails + 0x58 + 2));
		cout << "3: " << entityXbyte3 << endl;
		BYTE entityXbyte4 = static_cast<unsigned>(*(BYTE*)(entityDetails + 0x58 + 3));
		cout << "4: " << entityXbyte4 << endl;

		BYTE entityYbyte = static_cast<unsigned>(*(BYTE*)(entityDetails + 0x5C + 0));
		cout << "1: " << entityYbyte << endl;
		BYTE entityYbyte2 = static_cast<unsigned>(*(BYTE*)(entityDetails + 0x5C + 1));
		cout << "2: " << entityYbyte2 << endl;
		BYTE entityYbyte3 = static_cast<unsigned>(*(BYTE*)(entityDetails + 0x5C + 2));
		cout << "3: " << entityYbyte3 << endl;
		BYTE entityYbyte4 = static_cast<unsigned>(*(BYTE*)(entityDetails + 0x5C + 3));
		cout << "4: " << entityYbyte4 << endl;*/

		float *currentEntityX = (float*)(entityDetails + 0x58);
		float *currentEntityY = (float*)(entityDetails + 0x5C);

		if (currentEntityX == NULL || currentEntityY == NULL) continue;

		float distToPlayer = sqrt(pow((*(playerX)-*currentEntityX), 2) + pow((*(playerY)-*currentEntityY), 2));
		cout << "umm1" << endl;
		//distToCenter is the distance to a center point around which the player should kill. Anything greater than a distance of whatever won't be targetted
		float distToCenter = sqrt(pow((initialX - *currentEntityX), 2) + pow((initialY - *currentEntityY), 2));
		cout << "umm2" << endl;
		bool found = false;
		for (int i = 0; i < 20 && ids[i] != 0; i++){
			if (currentMobID == ids[i]){
				found = true;
				break;
			}
		}

		if (found == false && distToPlayer <= 300){
			for (int i = 0; i < 20 && optionalIDs[i] != 0; i++){
				if (currentMobID == optionalIDs[i]){
					found = true;
					break;
				}
			}
		}
		if (found == true){
			if (distToCenter >= killRadius) continue;
			if (entities[i] == lastEntity) continue;
			//cout << "Selection ID: " << *(WORD*)(entityBase + 0x234);
			//cout << "\tHealth: " << *(int*)(entityBase + 0x288);
			//cout << "\tDistance: " << distance << endl;

			if (shortestDistance == 0 && entityHealth > 0){
				shortestDistance = distToPlayer;
				shortestEntity = entities[i];
				shortestEntityX = (float*)(entityDetails + 0x58);
				shortestEntityY = (float*)(entityDetails + 0x5C);
			}
			else if (distToPlayer <= shortestDistance && entityHealth > 0){

				if (lastEntity != entities[i]){
					shortestDistance = distToPlayer;
					shortestEntity = entities[i];
					shortestEntityX = (float*)(entityDetails + 0x58);
					shortestEntityY = (float*)(entityDetails + 0x5C);
				}
			}
		}
		//cout << "umm5" << endl;
		//cout << hex << "entityDetails: " << entityDetails << endl;
		//if (entityDetails < 40000000) continue;

		//cout << hex << entityBase << endl;

		//for (int i = 0; *(BYTE*)(entityBase + 0x236 + i) != '\0' && i < 30; i++){
		//	name[i] = *(BYTE*)(entityBase + 0x236 + i);
		//}
		//cout << "Address: " << hex << entityDetails << endl;

		//cout << "\tName: ";
		//for (int i = 0; name[i] != 0 && i < 30; i++) cout << name[i];

		//cout << "Selection ID: " << *(WORD*)(entityBase + 0x234) << endl;
		//cout << "Mob ID: " << dec << *(WORD*)(entityBase + 0x3BE);
		//cout << "\tHealth: " << *(int*)(entityBase + 0x288);

		//cout << "\tX: ";
		//cout << *(float*)(entityDetails + 0x58);
		//cout << "\tY: ";
		//cout << *(float*)(entityDetails + 0x5C);

		//float distance = distanceFunc(*playerX, *playerY, *(float*)(entityDetails + 0x58), *(float*)(entityDetails + 0x5C));
		cout << "\tDistance: " << distToPlayer << endl;
		//cout << "\n";

		//for (int i = 0; name[i] != 0 && i < 30; i++) name[i] = 0;

	}
	cout << "lollers" << endl;
	CloseHandle(hProcess);
	cout << "lol 1\n";
	bool stopMoving = false;
	bool moveToAttack = false;
	if (shortestEntity > 0){

		LARGE_INTEGER tcounter;
		LONGLONG ticknow, tickAttack;
		LONGLONG freq;
		QueryPerformanceFrequency(&tcounter);
		freq = tcounter.QuadPart;

		QueryPerformanceCounter(&tcounter);
		ticknow = tcounter.QuadPart;
		tickAttack = tcounter.QuadPart;
		DWORD shortestBase = 0;
		if (isMemReadable((LPCVOID)(shortestEntity + 0x08), sizeof(DWORD))) shortestBase = *(DWORD*)(shortestEntity + 0x08);
		else return;

		//DWORD shortestBase = *(DWORD*)(shortestEntity + 0x08); 
		cout << "heh" << endl;
		DWORD shortestDetails = 0;
		if (isMemReadable((LPCVOID)(shortestBase + 0x2cc), sizeof(DWORD))) shortestDetails = *(DWORD*)(shortestBase + 0x2cc);
		else return;

		//DWORD shortestDetails = *(DWORD*)(shortestBase + 0x2cc);
		cout << "wow" << endl;
		int *shortestHealth = (int*)(shortestBase + 0x288);
		WORD shortestMobID = *(WORD*)(shortestBase + 0x3BE);
		cout << "dope" << endl;
		//cout << "Yep!" << endl;
		//cout << "Shortest Address: " << shortestEntity << endl;
		lastEntity = shortestEntity;
		unsigned char id[2] = { *(BYTE*)(shortestBase + 0x234), *(BYTE*)(shortestBase + 0x235) };
		cout << "great" << endl;
		//while (LOOTTRAVEL == true) Sleep(200);
		//cout << "\nKilling: " << hex << static_cast<unsigned>(id[1]) << static_cast<unsigned>(id[0]) << endl;
		//getchar();
		// for (int i = 0; name[i] != 0 && i < 30; i++) cout << name[i];
		char* target = new char[5];
		//char* attack = new char[3];
		//char* battleMode = new char[4];
		target[0] = 0x04;
		target[1] = 0x01;
		target[2] = 0x24;
		target[3] = id[0];
		target[4] = id[1];

		sendCrypt(sendSocket_r, target, 5, 0);
		if (stolenAggro == true) Sleep(1000);
		//cout << "BEEP\n";
		IDtoBYTE playerstuff;
		playerstuff.bytes[0] = playerID[0];
		playerstuff.bytes[1] = playerID[1];
		int previousHealth = *shortestHealth;
		float distToCenter = sqrt(pow((initialX - *shortestEntityX), 2) + pow((initialY - *shortestEntityY), 2));
		float distToPlayer = sqrt(pow((*(playerX)-*shortestEntityX), 2) + pow((*(playerY)-*shortestEntityY), 2));
		cout << "LALALAL: " << endl;

		lastEntity = shortestEntity;
		while (*shortestHealth > 0){

			if (distToCenter >= killRadius){
				monsterTooFar = true;
				break;
			}
			cout << "shortestEntityX: " << shortestEntityX << "\tshortxestEntityY: " << shortestEntityY << endl;
			distToCenter = sqrt(pow((initialX - *shortestEntityX), 2) + pow((initialY - *shortestEntityY), 2));
			distToPlayer = sqrt(pow((*(playerX)-*shortestEntityX), 2) + pow((*(playerY)-*shortestEntityY), 2));
			cout << "distToCenter: " << distToCenter << "\tdistToPlayer: " << distToPlayer << endl;
			lastPosX = *playerX;
			lastPosY = *playerY;

			cout << "Target of target ID: " << hex << *(WORD*)targetofTargetID << endl;
			cout << "player ID: " << hex << playerstuff.id << endl;
			if (*(WORD*)targetofTargetID != playerstuff.id && *(WORD*)targetofTargetID != 0xFFFF){
				Sleep(500);
				stolenAggro = true;
				break;
			}
			
			else stolenAggro = false;

			if ((ticknow - tickAttack) / (freq / 1000) >= (14 * 1000)){
				//TravelTo(*shortestEntityX, *shortestEntityY);
				break;
			}

			if (GetAsyncKeyState(VK_F1)) break;

			//if (*health > *maxhealth * 0.65){
			//checkBuff(true);

			/*if (marloneCounter == -1){
			applyBuffs_Crus(playerID);
			marloneCounter = 0;
			}*/

			//applyBuffs_Crus(playerID);

			/*if (distToPlayer <= 80 && stopMoving == false){
				stopMoving = true;

				if (moveToAttack == true){
					injectDInput(DIK_W, .1);
					while (injectKey == true) Sleep(10);
					moveToAttack = false;
				}
			}*/

			if (distToPlayer <= 80){
				//useSkill(&IceBolt, id, 0, 0);
				//useSkill(&LightBolt, id, 0, 0);
				if (moveToAttack == true){
					injectDInput(DIK_W, .1);
					while (injectKey == true) Sleep(10);
					moveToAttack = false;
				}
				useSkill(&MagicBolt, id, 0, 0);
				useSkill(&FireBolt, id, 0, 0);
				useSkill(&MagicBurst, playerID, 0, 0);
			}
			/*if (distToPlayer <= 100 && stopMoving == true){
				useSkill(&MagicBurst, playerID, 0, 0);
			}*/
			//}

			cout << "Health: " << dec << *shortestHealth << endl;
			if (*health == 0){
				//while (1) Sleep(1000);
				break;
			}

			if (*health <= *maxhealth * 0.25){
				useHPStone();
			}

			if (*mana <= *maxmana * 0.25){
				useSPStone();
			}

			if (*mana <= *maxmana * 0.10){
				injectDInput(DIK_MINUS, .1);
				while (injectKey == true) Sleep(10);
			}

			if (*health <= *maxhealth * 0.10){
				injectDInput(DIK_EQUALS, .1);
				while (injectKey == true) Sleep(10);
				Sleep(50);
			}

			/*if (*mana < *maxmana * 0.40 && *mana > *maxmana * 0.30){
			useSPStone();
			Sleep(100);
			}
			if (*mana < *maxmana * 0.10){
			DownKey('6', file_kbd);
			Sleep(60);
			ReleaseKey('6', file_kbd);
			Sleep(500);
			}*/
			//Sleep(500);
			if (distToPlayer > 80){
				moveToAttack = true;

				if (stopMoving == true) stopMoving = false;

				if (injectKey == false){
					injectDInput(DIK_1, .1);
					while (injectKey == true) Sleep(10);
				}
			}

			cout << "Attack timeout timer: " << (ticknow - tickAttack) / (freq / 1000) << endl;
			if (*shortestHealth == previousHealth){ // if health is the same, continue counting
				QueryPerformanceCounter(&tcounter);
				ticknow = tcounter.QuadPart;
			}
			else {
				QueryPerformanceCounter(&tcounter); //if health is different, reset timer
				ticknow = tcounter.QuadPart;
				tickAttack = tcounter.QuadPart;
				previousHealth = *shortestHealth;
			}

		}
		if (shortestMobID == 4006){
			marloneCounter++;
			sendCrypt(sendSocket_r, aoeParty_chat, 8, 0);

			if (marloneCounter == 4){
				applyBuffs_Crus(playerID);
			}
		}
	}
	//cout << "Done!\n";
}

void gatherBot(int ids[20]){
	//lootTable[0] = 2601;
	//while (1){
		entityPointer = (fiestaBase + 0x700C58);  //base pointer/pointer to me
		cout << "hmmm" << endl;
		//DWORD entity = *(DWORD*)entityPointer;
		DWORD entities[1024] = { 0 };
		DWORD entityName, entityID;
		char name[30] = { 0 };

		//entityPointer = (DWORD)(fiestaBase + 0x704E28);
		entities[0] = *(DWORD*)(entityPointer + 0x3C);
		if (playerID[0] == 0xFF && playerID[1] == 0xFF){
			DWORD player = *(DWORD*)(entities[0] + 0x08);
			playerID[0] = *(BYTE*)(player + 0x234);
			playerID[1] = *(BYTE*)(player + 0x235);
		}
		//cout << hex << entities[0] << endl;

		int entityCounter = 1, entityBackup = 1;
		int i = 0;
		bool unique = true;
		while (1){
			if (*(DWORD*)entities[i] != 0){
				for (int i = 0; entities[i] != 0; i++){
					if (*(DWORD*)entities[i] == entities[i]){
						unique = false;
						break;
					}
				}

				if (unique == true){
					entities[entityCounter] = *(DWORD*)entities[i];
					entityCounter++;
				}
			}

			i++;
			if (entities[i] == 0) break;
			unique = true;
		}

		char* memelol = new char[5];
		DWORD memebase = 0;
		for (int i = 0; entities[i] != 0; i++){
			memebase = *(DWORD*)(entities[i] + 0x08);
			if (memebase == 0 || memebase > entities[i] + 0x30000000){
				//i++;
				continue;
			}
			bool found = false;
			for (int j = 0; j < 20 && ids[j] != 0; j++){
				if (*(WORD*)(memebase + 0x3BE) == ids[j]){
					found = true;
					break;
				}
			}
			if (found == true){
				//cout << "FUCK: " << hex << *(WORD*)(memebase + 0x3BE) << "\tWHAT: " << *(int*)(memebase + 0x288) << endl;
				if (*(int*)(memebase + 0x288) == 0){
					memelol[0] = 0x04;
					memelol[1] = 0x01;
					memelol[2] = 0x24;
					memelol[3] = *(BYTE*)(memebase + 0x234);
					memelol[4] = *(BYTE*)(memebase + 0x235);

					sendCrypt(sendSocket_r, memelol, 5, 0);
					Sleep(1);
				}
			}
		}
		//Sleep(1000);
		delete memelol;


		cout << "Number of Entities: " << entityCounter << endl;
		Sleep(200);
		float shortestDistance = 0;
		float *shortestEntityX = NULL;
		float *shortestEntityY = NULL;
		DWORD shortestEntity = 0;
		HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetCurrentProcessId());
		//DWORD entityHealth = 0;	
		for (int i = 0; entities[i] != 0; i++){
			WORD membuf = 0;
			float coordbuff = 0;
			//cout << "sup ";
			cout << hex << "entities[i]: " << entities[i] << endl;
			cout << "Getting Base: " << (DWORD*)(entities[i] + 0x08) << endl;
			//cout << "umm1" << endl;
			DWORD entityBase = 0;

			if (isMemReadable((LPCVOID)(entities[i] + 0x08), sizeof(DWORD))) entityBase = *(DWORD*)(entities[i] + 0x08);
			else continue;
			//DWORD entityBase = *(DWORD*)(entities[i] + 0x08);
			if (entityBase < fiestaBase || entityBase > 0x7ffff000){
				//i++;
				continue;
			}
			//cout << "umm2" << endl;
			//if (entityBase < fies	taBase) continue
			cout << hex << "entityBase: " << entityBase << endl;
			cout << hex << "MobID address: " << (WORD*)(entityBase + 0x3BE) << endl;

			WORD currentMobID = 0;
			if (isMemReadable((LPCVOID)(entityBase + 0x3BE), sizeof(WORD))) currentMobID = *(WORD*)(entityBase + 0x3BE);
			else continue;
			/*cout << hex << "address: " << GetCurrentProcessId() << endl;
			int read_result = ReadProcessMemory(hProcess, (LPCVOID)(entityBase + 0x3BE), &membuf, sizeof(WORD), NULL);
			cout << "ReadProcessMemory on MobID: " << read_result << endl;
			if (read_result == 0){
			cout << "RPM error: " << GetLastError() << endl;
			continue;
			}
			cout << "Read result: " << membuf << endl;*/
			//WORD currentMobID = *(WORD*)(entityBase + 0x3BE);
			bool foundMob = false;
			for (int j = 0; j < 20; j++){
				if (ids[j] > 0){
					if (currentMobID == ids[j]){
						foundMob = true;
						break;
					}
				}
			}
			cout << "ugh sucky" << endl;
			if (foundMob == false) continue;
			cout << "wtf now dude" << endl;
			cout << hex << "currentMobID: " << currentMobID << endl;

			//cout << "Getting Details" << endl;
			cout << "entityDetails to dereference: " << (DWORD*)(entityBase + 0x2cc) << endl;
			DWORD* entityDeref = (DWORD*)(entityBase + 0x2cc);
			if (entityDeref == 0) continue;
			/*		BYTE entityDetails1 = static_cast<unsigned>(*(BYTE*)entityDeref);
			cout << "1: " << entityDetails1 << endl;
			BYTE entityDetails2 = static_cast<unsigned>(*(BYTE*)(entityDeref + 1));
			cout << "2: " << entityDetails2 << endl;
			BYTE entityDetails3 = static_cast<unsigned>(*(BYTE*)(entityDeref + 2));
			cout << "3: " << entityDetails3 << endl;
			BYTE entityDetails4 = static_cast<unsigned>(*(BYTE*)(entityDeref + 3));
			cout << "4: " << entityDetails4 << endl;*/
			DWORD entityDetails = 0;
			if (isMemReadable((LPCVOID)entityDeref, sizeof(DWORD))) entityDetails = *(DWORD*)entityDeref;
			//DWORD entityDetails = *(DWORD*)entityDeref;
			cout << hex << "entityDetails: " << entityDetails << endl;
			/*if (entityDetails < fiestaBase || entityDetails > 0x7f000000 || entityDetails < 0x10000000){
			//i++;
			continue;
			}*/
			if (entityDetails < fiestaBase){
				//i++;
				continue;
			}
			//cout << "umm3" << endl;

			WORD currentSelectionId = *(WORD*)(entityBase + 0x234);
			cout << hex << "currentSelectionId: " << *(WORD*)(entityBase + 0x234) << endl;
			if (*(WORD*)(entityBase + 0x234) == 0) continue;

			int entityHealth = *(int*)(entityBase + 0x288);
			cout << hex << "entityHealth: " << *(int*)(entityBase + 0x288) << endl;
			cout << "entity X addy: " << (float*)(entityDetails + 0x58) << "\tentity Y addy: " << (float*)(entityDetails + 0x5C) << endl;
			//cout << "umm4" << endl;
			/*		BYTE entityXbyte = static_cast<unsigned>(*(BYTE*)(entityDetails + 0x58 + 0));
			cout << hex << "1: " << entityXbyte << endl;
			BYTE entityXbyte2 = static_cast<unsigned>(*(BYTE*)(entityDetails + 0x58 + 1));
			cout << "2: " << entityXbyte2 << endl;
			BYTE entityXbyte3 = static_cast<unsigned>(*(BYTE*)(entityDetails + 0x58 + 2));
			cout << "3: " << entityXbyte3 << endl;
			BYTE entityXbyte4 = static_cast<unsigned>(*(BYTE*)(entityDetails + 0x58 + 3));
			cout << "4: " << entityXbyte4 << endl;

			BYTE entityYbyte = static_cast<unsigned>(*(BYTE*)(entityDetails + 0x5C + 0));
			cout << "1: " << entityYbyte << endl;
			BYTE entityYbyte2 = static_cast<unsigned>(*(BYTE*)(entityDetails + 0x5C + 1));
			cout << "2: " << entityYbyte2 << endl;
			BYTE entityYbyte3 = static_cast<unsigned>(*(BYTE*)(entityDetails + 0x5C + 2));
			cout << "3: " << entityYbyte3 << endl;
			BYTE entityYbyte4 = static_cast<unsigned>(*(BYTE*)(entityDetails + 0x5C + 3));
			cout << "4: " << entityYbyte4 << endl;*/

			float *currentEntityX = (float*)(entityDetails + 0x58);
			float *currentEntityY = (float*)(entityDetails + 0x5C);

			if (currentEntityX == NULL || currentEntityY == NULL) continue;

			float distToPlayer = sqrt(pow((*(playerX)-*currentEntityX), 2) + pow((*(playerY)-*currentEntityY), 2));
			cout << "umm1" << endl;
			//distToCenter is the distance to a center point around which the player should kill. Anything greater than a distance of whatever won't be targetted
			float distToCenter = sqrt(pow((initialX - *currentEntityX), 2) + pow((initialY - *currentEntityY), 2));
			cout << "umm2" << endl;
			bool found = false;
			for (int i = 0; i < 20 && ids[i] != 0; i++){
				if (currentMobID == ids[i]){
					found = true;
					break;
				}
			}

			if (found == true){
				if (distToCenter >= killRadius) continue;
				if (entities[i] == lastEntity) continue;
				//cout << "Selection ID: " << *(WORD*)(entityBase + 0x234);
				//cout << "\tHealth: " << *(int*)(entityBase + 0x288);
				//cout << "\tDistance: " << distance << endl;

				if (shortestDistance == 0 && entityHealth > 0){
					shortestDistance = distToPlayer;
					shortestEntity = entities[i];
					shortestEntityX = (float*)(entityDetails + 0x58);
					shortestEntityY = (float*)(entityDetails + 0x5C);
				}
				else if (distToPlayer <= shortestDistance && entityHealth > 0){

					if (lastEntity != entities[i]){
						shortestDistance = distToPlayer;
						shortestEntity = entities[i];
						shortestEntityX = (float*)(entityDetails + 0x58);
						shortestEntityY = (float*)(entityDetails + 0x5C);
					}
				}
			}
			//cout << "umm5" << endl;
			//cout << hex << "entityDetails: " << entityDetails << endl;
			//if (entityDetails < 40000000) continue;

			//cout << hex << entityBase << endl;

			//for (int i = 0; *(BYTE*)(entityBase + 0x236 + i) != '\0' && i < 30; i++){
			//	name[i] = *(BYTE*)(entityBase + 0x236 + i);
			//}
			//cout << "Address: " << hex << entityDetails << endl;

			//cout << "\tName: ";
			//for (int i = 0; name[i] != 0 && i < 30; i++) cout << name[i];

			//cout << "Selection ID: " << *(WORD*)(entityBase + 0x234) << endl;
			//cout << "Mob ID: " << dec << *(WORD*)(entityBase + 0x3BE);
			//cout << "\tHealth: " << *(int*)(entityBase + 0x288);

			//cout << "\tX: ";
			//cout << *(float*)(entityDetails + 0x58);
			//cout << "\tY: ";
			//cout << *(float*)(entityDetails + 0x5C);

			//float distance = distanceFunc(*playerX, *playerY, *(float*)(entityDetails + 0x58), *(float*)(entityDetails + 0x5C));
			cout << "\tDistance: " << distToPlayer << endl;
			//cout << "\n";

			//for (int i = 0; name[i] != 0 && i < 30; i++) name[i] = 0;

		}
		cout << "lollers" << endl;
		CloseHandle(hProcess);
		cout << "lol 1\n";
		bool stopMoving = false;
		bool moveToAttack = false;
		if (shortestEntity > 0){

			LARGE_INTEGER tcounter;
			LONGLONG ticknow, tickAttack;
			LONGLONG freq;
			QueryPerformanceFrequency(&tcounter);
			freq = tcounter.QuadPart;

			QueryPerformanceCounter(&tcounter);
			ticknow = tcounter.QuadPart;
			tickAttack = tcounter.QuadPart;
			DWORD shortestBase = 0;
			if (isMemReadable((LPCVOID)(shortestEntity + 0x08), sizeof(DWORD))) shortestBase = *(DWORD*)(shortestEntity + 0x08);
			else return;

			//DWORD shortestBase = *(DWORD*)(shortestEntity + 0x08); 
			cout << "heh" << endl;
			DWORD shortestDetails = 0;
			if (isMemReadable((LPCVOID)(shortestBase + 0x2cc), sizeof(DWORD))) shortestDetails = *(DWORD*)(shortestBase + 0x2cc);
			else return;

			//DWORD shortestDetails = *(DWORD*)(shortestBase + 0x2cc);
			cout << "wow" << endl;
			int *shortestHealth = (int*)(shortestBase + 0x288);
			if (*shortestHealth == 0) return;

			WORD shortestMobID = *(WORD*)(shortestBase + 0x3BE);
			cout << "dope" << endl;
			//cout << "Yep!" << endl;
			//cout << "Shortest Address: " << shortestEntity << endl;
			lastEntity = shortestEntity;
			unsigned char id[2] = { *(BYTE*)(shortestBase + 0x234), *(BYTE*)(shortestBase + 0x235) };
			cout << "great" << endl;
			//while (LOOTTRAVEL == true) Sleep(200);
			//cout << "\nKilling: " << hex << static_cast<unsigned>(id[1]) << static_cast<unsigned>(id[0]) << endl;
			//getchar();
			// for (int i = 0; name[i] != 0 && i < 30; i++) cout << name[i];
			char* target = new char[5];
			//char* attack = new char[3];
			//char* battleMode = new char[4];
			target[0] = 0x04;
			target[1] = 0x01;
			target[2] = 0x24;
			target[3] = id[0];
			target[4] = id[1];

			sendCrypt(sendSocket_r, target, 5, 0);
			if (stolenAggro == true) Sleep(1000);
			//cout << "BEEP\n";
			IDtoBYTE playerstuff;
			playerstuff.bytes[0] = playerID[0];
			playerstuff.bytes[1] = playerID[1];
			int previousHealth = *shortestHealth;
			float distToCenter = sqrt(pow((initialX - *shortestEntityX), 2) + pow((initialY - *shortestEntityY), 2));
			float distToPlayer = sqrt(pow((*(playerX)-*shortestEntityX), 2) + pow((*(playerY)-*shortestEntityY), 2));
			cout << "LALALAL: " << endl;

			lastEntity = shortestEntity;
			while (*shortestHealth > 0){

				if (distToCenter >= killRadius) break;
				cout << "shortestEntityX: " << shortestEntityX << "\tshortestEntityY: " << shortestEntityY << endl;
				distToCenter = sqrt(pow((initialX - *shortestEntityX), 2) + pow((initialY - *shortestEntityY), 2));
				distToPlayer = sqrt(pow((*(playerX)-*shortestEntityX), 2) + pow((*(playerY)-*shortestEntityY), 2));
				cout << "distToCenter: " << distToCenter << "\tdistToPlayer: " << distToPlayer << endl;
				lastPosX = *playerX;
				lastPosY = *playerY;


				/*if (*(WORD*)targetofTargetID != playerstuff.id && *(WORD*)targetofTargetID != 0xFFFF){
				Sleep(500);
				stolenAggro = true;
				break;
				}*/

				//else stolenAggro = false;

				if ((ticknow - tickAttack) / (freq / 1000) >= (10 * 1000)){
					//TravelTo(*shortestEntityX, *shortestEntityY);
					break;
				}

				if (GetAsyncKeyState(VK_F1)) break;

				//if (*health > *maxhealth * 0.65){
				//checkBuff(true);

				/*if (marloneCounter == -1){
					applyBuffs_Crus(playerID);
					marloneCounter = 0;
					}*/

				//}

				cout << "Health: " << dec << *shortestHealth << endl;
				if (*health == 0){
					//while (1) Sleep(1000);
					break;
				}

				/*if (*mana < *maxmana * 0.40 && *mana > *maxmana * 0.30){
				useSPStone();
				Sleep(100);
				}
				if (*mana < *maxmana * 0.10){
				DownKey('6', file_kbd);
				Sleep(60);
				ReleaseKey('6', file_kbd);
				Sleep(500);
				}*/
				//Sleep(500);
				if (distToPlayer > 30){

					DownKey('1', file_kbd);
					Sleep(60);
					ReleaseKey('1', file_kbd);
					Sleep(200);
				}

				cout << "Attack timeout timer: " << (ticknow - tickAttack) / (freq / 1000) << endl;
				if (*shortestHealth == previousHealth){ // if health is the same, continue counting
					QueryPerformanceCounter(&tcounter);
					ticknow = tcounter.QuadPart;
				}
				else {
					QueryPerformanceCounter(&tcounter); //if health is different, reset timer
					ticknow = tcounter.QuadPart;
					tickAttack = tcounter.QuadPart;
					previousHealth = *shortestHealth;
				}

			}
		}
		//cout << "Done!\n";
	//}
}

void login(CHAR* account, CHAR* password, char charNum){
	LARGE_INTEGER tcounter;
	LONGLONG tickynow;
	LONGLONG tickyEvent;
	LONGLONG freq;
	QueryPerformanceFrequency(&tcounter);
	freq = tcounter.QuadPart;

	sendSocket_r_Invite = 0;
	serverPick = false;
	characterSelect = false;
	mapSwitch = false;
	SendString(account, file_kbd);
	Sleep(200);
	SendSpecial('t', file_kbd);
	Sleep(200);
	SendString(password, file_kbd);
	Sleep(700);
	DownKey('\n', file_kbd);
	Sleep(100);
	ReleaseKey('\n', file_kbd);
	QueryPerformanceCounter(&tcounter);
	tickynow = tcounter.QuadPart;
	tickyEvent = tcounter.QuadPart;

	while (serverPick == false){

		if ((tickynow - tickyEvent) / (freq / 1000) > 10 * 1000){
			DownSpecial('e', file_kbd);
			Sleep(100);
			ReleaseSpecial('e', file_kbd);
			Sleep(500);
			SendSpecial('t', file_kbd);
			Sleep(200);
			SendString(password, file_kbd);
			Sleep(200);
			DownKey('\n', file_kbd);
			Sleep(100);
			ReleaseKey('\n', file_kbd);

			QueryPerformanceCounter(&tcounter);
			tickynow = tcounter.QuadPart;
			tickyEvent = tcounter.QuadPart;
		}

		QueryPerformanceCounter(&tcounter);
		tickynow = tcounter.QuadPart;
		Sleep(1000);
	}
	Sleep(2000);
	DownKey('\n', file_kbd);
	Sleep(100);
	ReleaseKey('\n', file_kbd);
	while (characterSelect == false) Sleep(1000);
	Sleep(2000);
	MoveMouseAbsolute(characterClick[charNum].x, characterClick[charNum].y, file_mou);
	Sleep(100);

	for (int i = 0; i < 2; i++){
		HoldMouseLeft(file_mou);
		Sleep(100);
		ReleaseMouseLeft(file_mou);
		Sleep(100);
	}
	while (mapSwitch == false) Sleep(1000);
	Sleep(3000);
	injectDInput(DIK_SPACE, .1);
	while (injectKey == true) Sleep(10);
	Sleep(2000);
}

void mobESP_zombieGathers(){
	DWORD spawners[10] = { 0 };
	int ids[4] = { 8567, 8568, 8540, 8570 };
	entityPointer = (fiestaBase + 0x700C58);  //base pointer/pointer to me
	cout << "hmmm" << endl;
	//DWORD entity = *(DWORD*)entityPointer;
	DWORD entities[1024] = { 0 };
	DWORD entityName, entityID;
	char name[30] = { 0 };

	//entityPointer = (DWORD)(fiestaBase + 0x704E28);
	entities[0] = *(DWORD*)(entityPointer + 0x3C);
	if (playerID[0] == 0xFF && playerID[1] == 0xFF){
		DWORD player = *(DWORD*)(entities[0] + 0x08);
		playerID[0] = *(BYTE*)(player + 0x234);
		playerID[1] = *(BYTE*)(player + 0x235);
	}
	//cout << hex << entities[0] << endl;

	int entityCounter = 1, entityBackup = 1;
	int i = 0;
	bool unique = true;
	while (1){
		if (*(DWORD*)entities[i] != 0){
			for (int i = 0; entities[i] != 0; i++){
				if (*(DWORD*)entities[i] == entities[i]){
					unique = false;
					break;
				}
			}

			if (unique == true){
				entities[entityCounter] = *(DWORD*)entities[i];
				entityCounter++;
			}
		}

		i++;
		if (entities[i] == 0) break;
		unique = true;
	}

	char* memelol = new char[5];
	DWORD memebase = 0;
	int found_counter = 0;
	for (int i = 0; entities[i] != 0; i++){
		//bool found = false;
		memebase = *(DWORD*)(entities[i] + 0x08);
		if (memebase == 0 || memebase > entities[i] + 0x30000000){
			//i++;
			continue;
		}
		bool found = false;
		for (int j = 0; j < 4 && ids[j] != 0; j++){
			if (*(WORD*)(memebase + 0x3BE) == ids[j]){
				found = true;
				found_counter++;
				break;
			}
		}
		if (found == true){
			//cout << "FUCK: " << hex << *(WORD*)(memebase + 0x3BE) << "\tWHAT: " << *(int*)(memebase + 0x288) << endl;
			if (*(int*)(memebase + 0x288) == 0){
				memelol[0] = 0x04;
				memelol[1] = 0x01;
				memelol[2] = 0x24;
				memelol[3] = *(BYTE*)(memebase + 0x234);
				memelol[4] = *(BYTE*)(memebase + 0x235);

				sendCrypt(sendSocket_r, memelol, 5, 0);
				Sleep(10);
			}
		}
		//delete memelol;
		//if (found_counter == 0) break;
	}
	//Sleep(1000);
	delete memelol;
	if (found_counter == 0) return;
	cout << "Number of found entities: " << found_counter << endl;
	Sleep(200);
	float shortestDistance = 0;
	float *shortestEntityX = NULL;
	float *shortestEntityY = NULL;
	DWORD shortestEntity = 0;
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetCurrentProcessId());
	//DWORD entityHealth = 0;	
	for (int i = 0; entities[i] != 0; i++){
		WORD membuf = 0;
		float coordbuff = 0;
		//cout << "sup ";
		cout << hex << "entities[i]: " << entities[i] << endl;
		cout << "Getting Base: " << (DWORD*)(entities[i] + 0x08) << endl;
		//cout << "umm1" << endl;
		DWORD entityBase = 0;

		if (isMemReadable((LPCVOID)(entities[i] + 0x08), sizeof(DWORD))) entityBase = *(DWORD*)(entities[i] + 0x08);
		else continue;
		//DWORD entityBase = *(DWORD*)(entities[i] + 0x08);
		if (entityBase < fiestaBase || entityBase > 0x7ffff000){
			//i++;
			continue;
		}
		//cout << "umm2" << endl;
		//if (entityBase < fies	taBase) continue
		cout << hex << "entityBase: " << entityBase << endl;
		cout << hex << "MobID address: " << (WORD*)(entityBase + 0x3BE) << endl;

		WORD currentMobID = 0;
		if (isMemReadable((LPCVOID)(entityBase + 0x3BE), sizeof(WORD))) currentMobID = *(WORD*)(entityBase + 0x3BE);
		else continue;
		/*cout << hex << "address: " << GetCurrentProcessId() << endl;
		int read_result = ReadProcessMemory(hProcess, (LPCVOID)(entityBase + 0x3BE), &membuf, sizeof(WORD), NULL);
		cout << "ReadProcessMemory on MobID: " << read_result << endl;
		if (read_result == 0){
		cout << "RPM error: " << GetLastError() << endl;
		continue;
		}
		cout << "Read result: " << membuf << endl;*/
		//WORD currentMobID = *(WORD*)(entityBase + 0x3BE);
		bool foundMob = false;
		for (int j = 0; j < 4; j++){
			if (ids[j] > 0){
				if (currentMobID == ids[j]){
					foundMob = true;
					for (int w = 0; w < 10; w++){
						if (spawners[w] == 0){
							spawners[w] = entities[i];
							break;
						}
					}
					break;
				}
			}
		}
		cout << "ugh sucky" << endl;
		if (foundMob == false) continue;
		cout << "wtf now dude" << endl;
		cout << hex << "currentMobID: " << currentMobID << endl;

		//cout << "Getting Details" << endl;
		cout << "entityDetails to dereference: " << (DWORD*)(entityBase + 0x2cc) << endl;
		DWORD* entityDeref = (DWORD*)(entityBase + 0x2cc);
		if (entityDeref == 0) continue;
		DWORD entityDetails = 0;
		if (isMemReadable((LPCVOID)entityDeref, sizeof(DWORD))) entityDetails = *(DWORD*)entityDeref;
		//DWORD entityDetails = *(DWORD*)entityDeref;
		cout << hex << "entityDetails: " << entityDetails << endl;
		/*if (entityDetails < fiestaBase || entityDetails > 0x7f000000 || entityDetails < 0x10000000){
		//i++;
		continue;
		}*/
		if (entityDetails < fiestaBase){
			//i++;
			continue;
		}
		//cout << "umm3" << endl;

		WORD currentSelectionId = 0;
		if (isMemReadable((LPCVOID)(WORD*)(entityBase + 0x234), sizeof(WORD))) currentSelectionId = *(WORD*)(entityBase + 0x234);
		else continue;
		cout << hex << "currentSelectionId: " << *(WORD*)(entityBase + 0x234) << endl;
		if (*(WORD*)(entityBase + 0x234) == 0) continue;

		int entityHealth = *(int*)(entityBase + 0x288);
		cout << hex << "entityHealth: " << *(int*)(entityBase + 0x288) << endl;
		cout << "entity X addy: " << (float*)(entityDetails + 0x58) << "\tentity Y addy: " << (float*)(entityDetails + 0x5C) << endl;
		//cout << "umm4" << endl;
		/*		BYTE entityXbyte = static_cast<unsigned>(*(BYTE*)(entityDetails + 0x58 + 0));
		cout << hex << "1: " << entityXbyte << endl;
		BYTE entityXbyte2 = static_cast<unsigned>(*(BYTE*)(entityDetails + 0x58 + 1));
		cout << "2: " << entityXbyte2 << endl;
		BYTE entityXbyte3 = static_cast<unsigned>(*(BYTE*)(entityDetails + 0x58 + 2));
		cout << "3: " << entityXbyte3 << endl;
		BYTE entityXbyte4 = static_cast<unsigned>(*(BYTE*)(entityDetails + 0x58 + 3));
		cout << "4: " << entityXbyte4 << endl;

		BYTE entityYbyte = static_cast<unsigned>(*(BYTE*)(entityDetails + 0x5C + 0));
		cout << "1: " << entityYbyte << endl;
		BYTE entityYbyte2 = static_cast<unsigned>(*(BYTE*)(entityDetails + 0x5C + 1));
		cout << "2: " << entityYbyte2 << endl;
		BYTE entityYbyte3 = static_cast<unsigned>(*(BYTE*)(entityDetails + 0x5C + 2));
		cout << "3: " << entityYbyte3 << endl;
		BYTE entityYbyte4 = static_cast<unsigned>(*(BYTE*)(entityDetails + 0x5C + 3));
		cout << "4: " << entityYbyte4 << endl;*/

		float *currentEntityX = 0; //(float*)(entityDetails + 0x58);
		float *currentEntityY = 0; //(float*)(entityDetails + 0x5C);
		if (isMemReadable((LPCVOID)(entityDetails + 0x58), sizeof(DWORD))) currentEntityX = (float*)(entityDetails + 0x58);
		else continue;
		if (isMemReadable((LPCVOID)(entityDetails + 0x5C), sizeof(DWORD))) currentEntityX = (float*)(entityDetails + 0x5C);
		else continue;
		if (currentEntityX == NULL || currentEntityY == NULL) continue;

		float distToPlayer = sqrt(pow((*(playerX)-*currentEntityX), 2) + pow((*(playerY)-*currentEntityY), 2));
		cout << "umm1" << endl;
		//distToCenter is the distance to a center point around which the player should kill. Anything greater than a distance of whatever won't be targetted
		float distToCenter = sqrt(pow((initialX - *currentEntityX), 2) + pow((initialY - *currentEntityY), 2));
		cout << "umm2" << endl;

		bool found = false;
		for (int i = 0; i < 4 && ids[i] != 0; i++){
			if (currentMobID == ids[i]){
				found = true;
				break;
			}
		}

		if (found == true){
			//if (distToCenter >= killRadius) continue;
			//cout << "Selection ID: " << *(WORD*)(entityBase + 0x234);
			//cout << "\tHealth: " << *(int*)(entityBase + 0x288);
			//cout << "\tDistance: " << distance << endl;

			if (shortestDistance == 0 && entityHealth > 0){
				shortestDistance = distToPlayer;
				shortestEntity = entities[i];
				shortestEntityX = (float*)(entityDetails + 0x58);
				shortestEntityY = (float*)(entityDetails + 0x5C);
			}
			else if (distToPlayer <= shortestDistance && entityHealth > 0){

				if (lastEntity != entities[i]){
					shortestDistance = distToPlayer;
					shortestEntity = entities[i];
					shortestEntityX = (float*)(entityDetails + 0x58);
					shortestEntityY = (float*)(entityDetails + 0x5C);
				}
			}
		}
		cout << "\tDistance: " << distToPlayer << endl;

	}
	cout << "lollers" << endl;
	CloseHandle(hProcess);
	cout << "lol 1\n";
	//if (shortestEntity > 0){
	for (int w = 0; w < 10 && spawners[w] != 0; w++){
		shortestEntity = spawners[w];
		cout << "bupers\n";
		LARGE_INTEGER tcounter;
		LONGLONG ticknow, tickAttack;
		LONGLONG freq;
		QueryPerformanceFrequency(&tcounter);
		freq = tcounter.QuadPart;
		cout << "bupers\n";
		QueryPerformanceCounter(&tcounter);
		ticknow = tcounter.QuadPart;
		tickAttack = tcounter.QuadPart;
		DWORD shortestBase = 0;
		if (isMemReadable((LPCVOID)(shortestEntity + 0x08), sizeof(DWORD))) shortestBase = *(DWORD*)(shortestEntity + 0x08);
		else return;
		cout << "bupers\n";
		//DWORD shortestBase = *(DWORD*)(shortestEntity + 0x08); 
		cout << "heh" << endl;
		DWORD shortestDetails = 0;
		if (isMemReadable((LPCVOID)(shortestBase + 0x2cc), sizeof(DWORD))) shortestDetails = *(DWORD*)(shortestBase + 0x2cc);
		else return;
		cout << "bupers\n";
		//DWORD shortestDetails = *(DWORD*)(shortestBase + 0x2cc);
		cout << "wow" << endl;
		int *shortestHealth = (int*)(shortestBase + 0x288);
		cout << "bupers\n";
		cout << "dope" << endl;
		//cout << "Yep!" << endl;
		//cout << "Shortest Address: " << shortestEntity << endl;
		lastEntity = shortestEntity;
		unsigned char id[2] = { *(BYTE*)(shortestBase + 0x234), *(BYTE*)(shortestBase + 0x235) };
		cout << "bupers\n";
		if (id[0] == playerID[0] && id[1] == playerID[1]) continue;
		cout << "bupers\n";
		cout << "great" << endl;
		//while (LOOTTRAVEL == true) Sleep(200);
		//cout << "\nKilling: " << hex << static_cast<unsigned>(id[1]) << static_cast<unsigned>(id[0]) << endl;
		//getchar();
		// for (int i = 0; name[i] != 0 && i < 30; i++) cout << name[i];
		char* target = new char[5];
		//char* attack = new char[3];
		//char* battleMode = new char[4];
		target[0] = 0x04;
		target[1] = 0x01;
		target[2] = 0x24;
		target[3] = id[0];
		target[4] = id[1];

		sendCrypt(sendSocket_r, target, 5, 0);
		//if (stolenAggro == true) Sleep(1000);
		//cout << "BEEP\n";
		IDtoBYTE playerstuff;
		playerstuff.bytes[0] = playerID[0];
		playerstuff.bytes[1] = playerID[1];
		int previousHealth = *shortestHealth;
		//float distToCenter = sqrt(pow((initialX - *shortestEntityX), 2) + pow((initialY - *shortestEntityY), 2));
		float distToPlayer = sqrt(pow((*(playerX)-*shortestEntityX), 2) + pow((*(playerY)-*shortestEntityY), 2));
		cout << "bupers\n";
		cout << "LALALAL: " << endl;
		Sleep(800);
		DownKey('4', file_kbd);
		Sleep(100);
		ReleaseKey('4', file_kbd);
		int counter = 0;
		while (*shortestHealth > 0){
			cout << "breee\n";
			Sleep(1000);
			DownKey('4', file_kbd);
			Sleep(100);
			ReleaseKey('4', file_kbd);
			if (counter == 7){
				TravelTo(1186, 4311);
				Sleep(1000);
			}

			if (counter == 14){
				TravelTo(1186, 4311);
				Sleep(1000);
			}
			counter++;
			//distToPlayer = sqrt(pow((*(playerX)-*shortestEntityX), 2) + pow((*(playerY)-*shortestEntityY), 2));
			//X
			/*if (*playerX - *shortestEntityX > 0){
				DownKey('a', file_kbd);
				while (*playerX - *shortestEntityX > 2){
				Sleep(10);
				}
				ReleaseKey('a', file_kbd);
				}

				else if (*playerX - *shortestEntityX < 0){
				DownKey('d', file_kbd);
				while (*playerX - *shortestEntityX < -2){
				Sleep(10);
				}
				ReleaseKey('d', file_kbd);
				}
				//Y
				if (*playerY - *shortestEntityY > 0){
				DownKey('s', file_kbd);
				while (*playerY - *shortestEntityY > 2){
				Sleep(10);
				}
				ReleaseKey('s', file_kbd);
				}

				else if (*playerY- *shortestEntityY < 0){
				DownKey('w', file_kbd);
				while (*playerY - *shortestEntityY < -2){
				Sleep(10);
				}
				ReleaseKey('w', file_kbd);
				}
				}*/

			//DownKey('4', file_kbd); //gather
			//Sleep(100);
			//ReleaseKey('4', file_kbd);
		}
		//ReleaseKey('4', file_kbd);
	}
	//cout << "Done!\n";
}

void mobESP_zombies(){
	DWORD spawners[20] = { 0 };
	int ids[1] = { 8562 };
	entityPointer = (fiestaBase + 0x700C58);  //base pointer/pointer to me
	cout << "hmmm" << endl;
	//DWORD entity = *(DWORD*)entityPointer;
	DWORD entities[1024] = { 0 };
	DWORD entityName, entityID;
	char name[30] = { 0 };

	//entityPointer = (DWORD)(fiestaBase + 0x704E28);
	entities[0] = *(DWORD*)(entityPointer + 0x3C);
	if (playerID[0] == 0xFF && playerID[1] == 0xFF){
		DWORD player = *(DWORD*)(entities[0] + 0x08);
		playerID[0] = *(BYTE*)(player + 0x234);
		playerID[1] = *(BYTE*)(player + 0x235);
	}
	//cout << hex << entities[0] << endl;

	int entityCounter = 1, entityBackup = 1;
	int i = 0;
	bool unique = true;
	while (1){
		if (*(DWORD*)entities[i] != 0){
			for (int i = 0; entities[i] != 0; i++){
				if (*(DWORD*)entities[i] == entities[i]){
					unique = false;
					break;
				}
			}

			if (unique == true){
				entities[entityCounter] = *(DWORD*)entities[i];
				entityCounter++;
			}
		}

		i++;
		if (entities[i] == 0) break;
		unique = true;
	}

	char* memelol = new char[5];
	DWORD memebase = 0;
	int found_counter = 0;
	for (int i = 0; entities[i] != 0; i++){
		//bool found = false;
		memebase = *(DWORD*)(entities[i] + 0x08);
		if (memebase == 0 || memebase > entities[i] + 0x30000000){
			//i++;
			continue;
		}
		bool found = false;
		for (int j = 0; j < 1 && ids[j] != 0; j++){
			if (*(WORD*)(memebase + 0x3BE) == ids[j]){
				found = true;
				found_counter++;
				break;
			}
		}
		if (found == true){
			//cout << "FUCK: " << hex << *(WORD*)(memebase + 0x3BE) << "\tWHAT: " << *(int*)(memebase + 0x288) << endl;
			if (*(int*)(memebase + 0x288) == 0){
				memelol[0] = 0x04;
				memelol[1] = 0x01;
				memelol[2] = 0x24;
				memelol[3] = *(BYTE*)(memebase + 0x234);
				memelol[4] = *(BYTE*)(memebase + 0x235);

				sendCrypt(sendSocket_r, memelol, 5, 0);
				Sleep(10);
			}
		}
		//delete memelol;
		//if (found_counter == 0) break;
	}
	//Sleep(1000);
	delete memelol;
	if (found_counter == 0) return;
	cout << "Number of found entities: " << found_counter << endl;
	Sleep(200);
	float shortestDistance = 0;
	float *shortestEntityX = NULL;
	float *shortestEntityY = NULL;
	DWORD shortestEntity = 0;
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetCurrentProcessId());
	//DWORD entityHealth = 0;	
	for (int i = 0; entities[i] != 0; i++){
		WORD membuf = 0;
		float coordbuff = 0;
		//cout << "sup ";
		cout << hex << "entities[i]: " << entities[i] << endl;
		cout << "Getting Base: " << (DWORD*)(entities[i] + 0x08) << endl;
		//cout << "umm1" << endl;
		DWORD entityBase = 0;

		if (isMemReadable((LPCVOID)(entities[i] + 0x08), sizeof(DWORD))) entityBase = *(DWORD*)(entities[i] + 0x08);
		else continue;
		//DWORD entityBase = *(DWORD*)(entities[i] + 0x08);
		if (entityBase < fiestaBase || entityBase > 0x7ffff000){
			//i++;
			continue;
		}
		//cout << "umm2" << endl;
		//if (entityBase < fies	taBase) continue
		cout << hex << "entityBase: " << entityBase << endl;
		cout << hex << "MobID address: " << (WORD*)(entityBase + 0x3BE) << endl;

		WORD currentMobID = 0;
		if (isMemReadable((LPCVOID)(entityBase + 0x3BE), sizeof(WORD))) currentMobID = *(WORD*)(entityBase + 0x3BE);
		else continue;
		/*cout << hex << "address: " << GetCurrentProcessId() << endl;
		int read_result = ReadProcessMemory(hProcess, (LPCVOID)(entityBase + 0x3BE), &membuf, sizeof(WORD), NULL);
		cout << "ReadProcessMemory on MobID: " << read_result << endl;
		if (read_result == 0){
		cout << "RPM error: " << GetLastError() << endl;
		continue;
		}
		cout << "Read result: " << membuf << endl;*/
		//WORD currentMobID = *(WORD*)(entityBase + 0x3BE);
		int entityHealth = *(int*)(entityBase + 0x288);

		bool foundMob = false;
		for (int j = 0; j < 1; j++){
			if (ids[j] > 0){
				if (currentMobID == ids[j] && entityHealth > 0){
					foundMob = true;
					for (int w = 0; w < 20; w++){
						if (spawners[w] == 0){
							spawners[w] = entities[i];
							break;
						}
					}
					break;
				}
			}
		}
		cout << "ugh sucky" << endl;
		if (foundMob == false) continue;
		cout << "wtf now dude" << endl;
		cout << hex << "currentMobID: " << currentMobID << endl;

		//cout << "Getting Details" << endl;
		cout << "entityDetails to dereference: " << (DWORD*)(entityBase + 0x2cc) << endl;
		DWORD* entityDeref = (DWORD*)(entityBase + 0x2cc);
		if (entityDeref == 0) continue;
		DWORD entityDetails = 0;
		if (isMemReadable((LPCVOID)entityDeref, sizeof(DWORD))) entityDetails = *(DWORD*)entityDeref;
		//DWORD entityDetails = *(DWORD*)entityDeref;
		cout << hex << "entityDetails: " << entityDetails << endl;
		/*if (entityDetails < fiestaBase || entityDetails > 0x7f000000 || entityDetails < 0x10000000){
		//i++;
		continue;
		}*/
		if (entityDetails < fiestaBase){
			//i++;
			continue;
		}
		//cout << "umm3" << endl;

		WORD currentSelectionId = 0;
		if (isMemReadable((LPCVOID)(WORD*)(entityBase + 0x234), sizeof(WORD))) currentSelectionId = *(WORD*)(entityBase + 0x234);
		else continue;
		cout << hex << "currentSelectionId: " << *(WORD*)(entityBase + 0x234) << endl;
		if (*(WORD*)(entityBase + 0x234) == 0) continue;

		//int entityHealth = *(int*)(entityBase + 0x288);
		cout << hex << "entityHealth: " << *(int*)(entityBase + 0x288) << endl;
		cout << "entity X addy: " << (float*)(entityDetails + 0x58) << "\tentity Y addy: " << (float*)(entityDetails + 0x5C) << endl;
		//cout << "umm4" << endl;
		/*		BYTE entityXbyte = static_cast<unsigned>(*(BYTE*)(entityDetails + 0x58 + 0));
		cout << hex << "1: " << entityXbyte << endl;
		BYTE entityXbyte2 = static_cast<unsigned>(*(BYTE*)(entityDetails + 0x58 + 1));
		cout << "2: " << entityXbyte2 << endl;
		BYTE entityXbyte3 = static_cast<unsigned>(*(BYTE*)(entityDetails + 0x58 + 2));
		cout << "3: " << entityXbyte3 << endl;
		BYTE entityXbyte4 = static_cast<unsigned>(*(BYTE*)(entityDetails + 0x58 + 3));
		cout << "4: " << entityXbyte4 << endl;

		BYTE entityYbyte = static_cast<unsigned>(*(BYTE*)(entityDetails + 0x5C + 0));
		cout << "1: " << entityYbyte << endl;
		BYTE entityYbyte2 = static_cast<unsigned>(*(BYTE*)(entityDetails + 0x5C + 1));
		cout << "2: " << entityYbyte2 << endl;
		BYTE entityYbyte3 = static_cast<unsigned>(*(BYTE*)(entityDetails + 0x5C + 2));
		cout << "3: " << entityYbyte3 << endl;
		BYTE entityYbyte4 = static_cast<unsigned>(*(BYTE*)(entityDetails + 0x5C + 3));
		cout << "4: " << entityYbyte4 << endl;*/

		float *currentEntityX = 0; //(float*)(entityDetails + 0x58);
		float *currentEntityY = 0; //(float*)(entityDetails + 0x5C);
		if (isMemReadable((LPCVOID)(entityDetails + 0x58), sizeof(DWORD))) currentEntityX = (float*)(entityDetails + 0x58);
		else continue;
		if (isMemReadable((LPCVOID)(entityDetails + 0x5C), sizeof(DWORD))) currentEntityX = (float*)(entityDetails + 0x5C);
		else continue;
		if (currentEntityX == NULL || currentEntityY == NULL) continue;

		float distToPlayer = sqrt(pow((*(playerX)-*currentEntityX), 2) + pow((*(playerY)-*currentEntityY), 2));
		cout << "umm1" << endl;
		//distToCenter is the distance to a center point around which the player should kill. Anything greater than a distance of whatever won't be targetted
		float distToCenter = sqrt(pow((initialX - *currentEntityX), 2) + pow((initialY - *currentEntityY), 2));
		cout << "umm2" << endl;

		bool found = false;
		for (int i = 0; i < 4 && ids[i] != 0; i++){
			if (currentMobID == ids[i]){
				found = true;
				break;
			}
		}

		if (found == true){
			//if (distToCenter >= killRadius) continue;
			//cout << "Selection ID: " << *(WORD*)(entityBase + 0x234);
			//cout << "\tHealth: " << *(int*)(entityBase + 0x288);
			//cout << "\tDistance: " << distance << endl;

			if (shortestDistance == 0 && entityHealth > 0){
				shortestDistance = distToPlayer;
				shortestEntity = entities[i];
				shortestEntityX = (float*)(entityDetails + 0x58);
				shortestEntityY = (float*)(entityDetails + 0x5C);
			}
			else if (distToPlayer <= shortestDistance && entityHealth > 0){

				if (lastEntity != entities[i]){
					shortestDistance = distToPlayer;
					shortestEntity = entities[i];
					shortestEntityX = (float*)(entityDetails + 0x58);
					shortestEntityY = (float*)(entityDetails + 0x5C);
				}
			}
		}
		cout << "\tDistance: " << distToPlayer << endl;

	}
	cout << "lollers" << endl;
	CloseHandle(hProcess);
	cout << "lol 1\n";
	//if (shortestEntity > 0){
	for (int w = 0; w < 20 && spawners[w] != 0; w++){
		shortestEntity = spawners[w];
		LARGE_INTEGER tcounter;
		LONGLONG ticknow, tickAttack;
		LONGLONG freq;
		QueryPerformanceFrequency(&tcounter);
		freq = tcounter.QuadPart;

		QueryPerformanceCounter(&tcounter);
		ticknow = tcounter.QuadPart;
		tickAttack = tcounter.QuadPart;
		DWORD shortestBase = 0;
		if (isMemReadable((LPCVOID)(shortestEntity + 0x08), sizeof(DWORD))) shortestBase = *(DWORD*)(shortestEntity + 0x08);
		else return;

		//DWORD shortestBase = *(DWORD*)(shortestEntity + 0x08); 
		cout << "heh" << endl;
		DWORD shortestDetails = 0;
		if (isMemReadable((LPCVOID)(shortestBase + 0x2cc), sizeof(DWORD))) shortestDetails = *(DWORD*)(shortestBase + 0x2cc);
		else return;

		//DWORD shortestDetails = *(DWORD*)(shortestBase + 0x2cc);
		cout << "wow" << endl;
		int *shortestHealth = (int*)(shortestBase + 0x288);
		cout << "dope" << endl;
		//cout << "Yep!" << endl;
		//cout << "Shortest Address: " << shortestEntity << endl;
		lastEntity = shortestEntity;
		unsigned char id[2] = { *(BYTE*)(shortestBase + 0x234), *(BYTE*)(shortestBase + 0x235) };
		if (id[0] == playerID[0] && id[1] == playerID[1]) continue;
		cout << "great" << endl;
		//while (LOOTTRAVEL == true) Sleep(200);
		//cout << "\nKilling: " << hex << static_cast<unsigned>(id[1]) << static_cast<unsigned>(id[0]) << endl;
		//getchar();
		// for (int i = 0; name[i] != 0 && i < 30; i++) cout << name[i];
		char* target = new char[5];
		//char* attack = new char[3];
		//char* battleMode = new char[4];
		target[0] = 0x04;
		target[1] = 0x01;
		target[2] = 0x24;
		target[3] = id[0];
		target[4] = id[1];

		sendCrypt(sendSocket_r, target, 5, 0);
		//if (stolenAggro == true) Sleep(1000);
		//cout << "BEEP\n";
		IDtoBYTE playerstuff;
		playerstuff.bytes[0] = playerID[0];
		playerstuff.bytes[1] = playerID[1];
		int previousHealth = *shortestHealth;
		//float distToCenter = sqrt(pow((initialX - *shortestEntityX), 2) + pow((initialY - *shortestEntityY), 2));
		float distToPlayer = sqrt(pow((*(playerX)-*shortestEntityX), 2) + pow((*(playerY)-*shortestEntityY), 2));
		cout << "LALALAL: " << endl;
		Sleep(800);
		DownKey('2', file_kbd);
		Sleep(100);
		ReleaseKey('2', file_kbd);
		int counter = 0;
		while (*shortestHealth > 0){
			Sleep(1000);
			DownKey('2', file_kbd);
			Sleep(100);
			ReleaseKey('2', file_kbd);
			counter++;
			if (counter == 5){
				TravelTo(*playerX, *shortestEntityY);
				Sleep(1000);
			}
			//distToPlayer = sqrt(pow((*(playerX)-*shortestEntityX), 2) + pow((*(playerY)-*shortestEntityY), 2));
			//X
			/*if (*playerX - *shortestEntityX > 0){
			DownKey('a', file_kbd);
			while (*playerX - *shortestEntityX > 2){
			Sleep(10);
			}
			ReleaseKey('a', file_kbd);
			}

			else if (*playerX - *shortestEntityX < 0){
			DownKey('d', file_kbd);
			while (*playerX - *shortestEntityX < -2){
			Sleep(10);
			}
			ReleaseKey('d', file_kbd);
			}
			//Y
			if (*playerY - *shortestEntityY > 0){
			DownKey('s', file_kbd);
			while (*playerY - *shortestEntityY > 2){
			Sleep(10);
			}
			ReleaseKey('s', file_kbd);
			}

			else if (*playerY- *shortestEntityY < 0){
			DownKey('w', file_kbd);
			while (*playerY - *shortestEntityY < -2){
			Sleep(10);
			}
			ReleaseKey('w', file_kbd);
			}
			}*/

			//DownKey('4', file_kbd); //gather
			//Sleep(100);
			//ReleaseKey('4', file_kbd);
		}
		//ReleaseKey('4', file_kbd);
	}
	//cout << "Done!\n";
}

void attackZombies(){
	entityPointer = (fiestaBase + 0x700C58);
	DWORD playerEntity = *(DWORD*)(entityPointer + 0x3C);
	DWORD player = *(DWORD*)(playerEntity + 0x08);
	unsigned char playerID[2] = { *(BYTE*)(player + 0x234), *(BYTE*)(player + 0x235) };

	float aoeXPositions[4][3] = { {1377, 1177, 1030}, { 1334, 1107, 962 }, { 962, 1107, 1427 }, { 1427, 1148, 946 } };
	float aoeYpositions[4] = { 4738, 4631, 4462, 4232 };
	//float x = 1186, homeY = 5586, homeattackY = 5363;
	//float y_pos[8] = { 5114, 4954, 4832, 4654, 4982, 4982, 4717, 4717};
	//float y_pos[10] = { 5074, 4984, 4944, 4984, 4944, 4944, 4832, 4687, 4737, 4624 };
	//float y_posAttack[8] = { 4761, 4622, 4445, 4261, 4604, 4604, 4319, 4319 };
	//float y_posAttack[10] = { 4761, 4604, 4604, 4604, 4604, 4622, 4445, 4319, 4319, 4261 };
	//float x_posAttack[8] = { 1186, 1186, 1186, 1186, 1377, 951, 1385, 979 };
	//float x_posAttack[10] = { 1186, 1377, 951, 1377, 951, 1186, 1186, 1385, 979, 1186 };

	//1, 5, 6, rest, 2, 3, 7, 8, 4
	//5114, 4982, 4982, 4954, 4832, 4717, 4717, 4654
	//4761, 4604, 4604, 4622, 4445, 4319, 4319, 4261
	//1186, 1137, 951, 1186, 1186, 1385, 979, 1186
	//home position: 1186, 5586
	//home attack aoe location: 1186, 5403

	//first attack position: 1186, 5014
	//first attack aoe location: 1186, 4761

	//second attack position: 1186, 4854
	//second attack aoe location: 1186, 4622

	//third attack position: 1186, 4734
	//third attack aoe location: 1186, 4495

	//fourth attack position: 1186, 4494
	//fourth attack aoe location: 1186, 4311

	//fifth attack position: 1186, 4902
	//fifth attack aoe:	1377, 4604

	//6th attack aoe: 951, 4604

	//sevent attack position: 1186, 4617
	//seventh attack aoe: 1385, 4379

	//8th attack aoe:	979, 4379
	char packet[4] = { 0x03, 0x08, 0x20, 0x02 }; //set battlestate to use attacks
	//03 08 20 02
	sendCrypt(sendSocket_r, packet, 4, 0);
	for (int y = 0; y < 4; y++){
		if (y == 0 || y == 1){
			TravelTo(1187, aoeYpositions[y]);
			Sleep(500);
		}
		for (int x = 0; x < 3; x++){
			TravelTo(aoeXPositions[y][x], aoeYpositions[y]);
			while (useSkill(&MagicBurst, playerID, 0, 0) == false) Sleep(10);
			Sleep(1500);
		}

		if (y == 0 || y == 1){
			sendCrypt(sendSocket_r, upParty_chat, 7, 0);
		}
	}
	/*for (int i = 0; i < 10; i++){
		if (*mana < 1000) useSPStone();
		if (i == 5 || i == 7 || i == 9 ){
			sendCrypt(sendSocket_r, lootParty_chat, 9, 0);
			startAttack = false;
			while (startAttack == false) Sleep(100);
		}
		TravelTo(x, y_pos[i]);
		Sleep(700);
		useSkill(&NatureMist, playerID, x_posAttack[i], y_posAttack[i]);
		Sleep(300);
		TravelTo(x, homeY);
		sendCrypt(sendSocket_r, aoeParty_chat, 8, 0);
		Sleep(500);
		if (i == 3) useSkill(&MultiShot, playerID, x, homeattackY);
		else useSkill(&MultiShot, playerID, x, homeattackY);
		Sleep(1700);
		TravelTo(x, homeY + 100);
		Sleep(300);
		while (useSkill(&NatureMist, playerID, *playerX, *playerY - 200) == false) Sleep(10);
		TravelTo(x, homeY + 200);
		Sleep(300);
		while (useSkill(&PierceShot, playerID, *playerX, *playerY - 150) == false) Sleep(10);
		while (useSkill(&NatureMist, playerID, *playerX, *playerY - 100) == false) Sleep(10);
		Sleep(400);

		if (i == 9){
			sendCrypt(sendSocket_r, lootallParty_chat, 12, 0);
			startAttack = false;
			while (startAttack == false) Sleep(100);
		}
	}*/
}

int ccDisconnectHandler(int dcEvent, int situation, CHAR* account, CHAR* password, int botType, char charNum){
	int disconnectType = dcEvent;
	if (dcEvent == 1){ //kicked back to login screen
		if (situation == 1){ //dced teleporting from uru to cc
			//while (disconnectType == dcEvent){
				//dced = true;
				cout << "DC Event 1, situation 1\n;";
				login(account, password, charNum);

				//Sleep(1500);
				/*DownKey('a', file_kbd); // get to cc
				DownKey('w', file_kbd);
				Sleep(2000);
				ReleaseKey('w', file_kbd);
				ReleaseKey('a', file_kbd);
				sendCrypt(sendSocket_r, talkToCC, 5, 0);
				Sleep(1000);
				MoveMouseAbsolute(441, 222, file_mou);
				Sleep(100);
				LeftClick(file_mou);
				Sleep(100);
				disconnectType = disconnectCheck(1, account);*/
			//}
		}

		if (situation == 2){ //dced teleporting from cc to uru
			//while (disconnectType == dcEvent){
				//dced = true;
				cout << "DC Event 1, situation 2\n;";
				login(account, password, charNum);

				/*DownKey('a', file_kbd); // get to cc
				DownKey('w', file_kbd);
				Sleep(2000);
				ReleaseKey('w', file_kbd);
				ReleaseKey('a', file_kbd);
				sendCrypt(sendSocket_r, talkToCC, 5, 0);
				Sleep(1000);
				MoveMouseAbsolute(441, 222, file_mou);
				Sleep(100);
				LeftClick(file_mou);
				Sleep(100);
				disconnectType = disconnectCheck(1, account);*/
			//}
		}
	}

	if (dcEvent == 2){ //"disconnected from server" message and sent to character select
		if (situation == 1){ //dced going from uru to cc
			while (disconnectType == 2){
				characterSelect = false;
				Sleep(5000);
				MoveMouseAbsolute(630, 573, file_mou); //click bottom rightish character
				if (account == "beegeemang2") MoveMouseAbsolute(89, 449, file_mou); //click leftmost character
				Sleep(100);

				for (int i = 0; i < 2; i++){
					HoldMouseLeft(file_mou);
					Sleep(100);
					ReleaseMouseLeft(file_mou);
					Sleep(100);
				}
				mapSwitch = false;
				timerStart();
				while (mapSwitch == false){
					Sleep(10);
					if (timerWatch(20)){
						MoveMouseAbsolute(630, 573, file_mou); //click bottom rightish character
						if (account == "beegeemang2") MoveMouseAbsolute(89, 449, file_mou); //click leftmost character
						Sleep(100);

						for (int i = 0; i < 2; i++){
							HoldMouseLeft(file_mou);
							Sleep(100);
							ReleaseMouseLeft(file_mou);
							Sleep(100);
						}
						timerStart();
					}
				}
				disconnectType = 1;
				Sleep(2000);
				injectDInput(DIK_SPACE, .1);
				while (injectKey == true) Sleep(10);

				partyQuit();
				//while (disconnectType == 1){
					cout << "DC Event 2, situation 1\n;";

					/*DownKey('a', file_kbd); // get to cc
					DownKey('w', file_kbd);
					Sleep(2000);
					ReleaseKey('w', file_kbd);
					ReleaseKey('a', file_kbd);
					sendCrypt(sendSocket_r, talkToCC, 5, 0);
					Sleep(1000);
					MoveMouseAbsolute(441, 222, file_mou);
					Sleep(100);
					LeftClick(file_mou);
					Sleep(100);
					disconnectType = disconnectCheck(1, account);
					if (disconnectType == 2) break; //check if disconnected at character screen again...*/
				//}
			}
		}

		if (situation == 2){ //dced going from cc to zombies
			while (disconnectType == 2){
				Sleep(5000);
				characterSelect = false;
				MoveMouseAbsolute(630, 573, file_mou); //click bottom rightish character
				if (account == "beegeemang2") MoveMouseAbsolute(89, 449, file_mou); //click leftmost character
				Sleep(100);

				for (int i = 0; i < 2; i++){
					HoldMouseLeft(file_mou);
					Sleep(100);
					ReleaseMouseLeft(file_mou);
					Sleep(100);
				}
				mapSwitch = false;
				timerStart();
				while (mapSwitch == false){
					Sleep(10);
					if (timerWatch(10)){
						MoveMouseAbsolute(630, 573, file_mou); //click bottom rightish character
						if (account == "beegeemang2") MoveMouseAbsolute(89, 449, file_mou); //click leftmost character
						Sleep(100);

						for (int i = 0; i < 2; i++){
							HoldMouseLeft(file_mou);
							Sleep(100);
							ReleaseMouseLeft(file_mou);
							Sleep(100);
						}
						timerStart();
					}
				}
				disconnectType = 1;
				Sleep(2000);
				injectDInput(DIK_SPACE, .1);
				while (injectKey == true) Sleep(10);

				partyQuit();
				//while (disconnectType == 1){
					cout << "DC Event 2, situation 2\n;";

					/*DownKey('a', file_kbd); // get to cc
					DownKey('w', file_kbd);
					Sleep(2000);
					ReleaseKey('w', file_kbd);
					ReleaseKey('a', file_kbd);
					sendCrypt(sendSocket_r, talkToCC, 5, 0);
					Sleep(1000);
					MoveMouseAbsolute(441, 222, file_mou);
					Sleep(100);
					LeftClick(file_mou);
					Sleep(100);
					disconnectType = disconnectCheck(1, account);
					if (disconnectType == 2) break;*/
				//}
			}
		}
	}

	return 0;
}

//if Y > 2877 and mapswitch == true quickly after going into CC...
void walktoCC(){
	cout << "1" << endl;
	TravelTo(5852, 5842);
	cout << "2" << endl;
	TravelTo(7288, 5842);
	cout << "3" << endl;
	TravelTo(7288, 5468);
	cout << "4" << endl;
	TravelTo(7409, 5468);
	cout << "5" << endl;
	TravelTo(7409, 5336);
	cout << "6" << endl;
	TravelTo(7523, 5336);
	cout << "7" << endl;
	TravelTo(7523, 5193);
	cout << "8" << endl;
	TravelTo(7643, 5193);
	cout << "9" << endl;
	TravelTo(7643, 5042);
	cout << "10" << endl;
	TravelTo(7753, 5035);
	cout << "11" << endl;
	TravelTo(7753, 4982);
	cout << "12" << endl;
	TravelTo(7826, 4982);
	cout << "13" << endl;
	TravelTo(7826, 4803);
	cout << "14" << endl;
	TravelTo(7941, 4803);
	cout << "15" << endl;
	TravelTo(7941, 2542);
	cout << "16" << endl;
	TravelTo(8907, 2542);
	cout << "17" << endl;
	TravelTo(8907, 1323);
	cout << "18" << endl;
	TravelTo(9249, 1323);
	cout << "19" << endl;
	TravelTo(9249, 1182);
	cout << "20" << endl;
	TravelTo(9379, 1182);
	cout << "21" << endl;
	TravelTo(9378, 1067);
}

void ZombieFarm(){
	int event_result = 0;
	bool stopLoop = false;
	short accnum = 0, passnum = 0, char_class = 0;
	cout << "Account? (0 = wow, 1 = bee2, 2 = bee1, 3 = ant, 4 = mangos, 5 = fiesta1";
	cin >> accnum;
	//if (accnum == 1) login(accounts[accnum], passwords[0]);
	//else login(accounts[accnum], passwords[1]);
	char ASSFUCK = 0;
	cout << "Type of bot? (1 = attacker, 2 = looter, 3 = afker" << endl;
	cin >> ASSFUCK;
	getchar();
	char charNum = 0;
	cout << "Char to click?\n (0 = 6oclk, 1 = 9oclk, 2 = 12oclk, 3 = 2oclk, 4 = 3oclk, 5 = 5oclk" << endl;
	cin >> charNum;
	getchar();
	//Console();
	//getchar();
	Sleep(2000);
	if (ASSFUCK == '1'){
		enableDropTrack = false;
		bool goAgain = false, dced = false;
		*yaw = 180.0;
		//WORD *assman = (WORD*)encryptCounterstatic_Invite;
		//WORD sigh = *assman;
		characterSelect = false;
		while (1){
			//inviteParty();
			//Sleep(3000);
			//cout << questNumberPointer << "\t" << questPointer << endl;
			//if (checkQuests()) enableDropTrack = true;
			//cout << "Player X: " << *(playerX) << "\tPlayer Y: " << *(playerY) << endl;
			//useSkill(&MultiShotr1, NULL, *reinterpret_cast<float*>(playerX), *reinterpret_cast<float*>(playerY));
			//if(enableDropTrack) enableDropTrack = false;
			//else enableDropTrack = true;
			//aoeGrind();
			if (GetAsyncKeyState(VK_F1)){
				sendSocket_r_Invite = 0;
				serverPick = false;
				if (accnum == 1) login(accounts[accnum], passwords[0], charNum);
				else login(accounts[accnum], passwords[1], charNum);
				Sleep(500);
				DownKey('\n', file_kbd);
				Sleep(100);
				ReleaseKey('\n', file_kbd);
				while (serverPick == false) Sleep(1000);
				Sleep(2000);
				DownKey('\n', file_kbd);
				Sleep(100);
				ReleaseKey('\n', file_kbd);
				Sleep(5000);
				MoveMouseAbsolute(855, 396, file_mou);
				Sleep(100);
				HoldMouseLeft(file_mou);
				Sleep(100);
				ReleaseMouseLeft(file_mou);
				Sleep(100);
				HoldMouseLeft(file_mou);
				Sleep(100);
				ReleaseMouseLeft(file_mou);
			}

			if (GetAsyncKeyState(VK_F4)){
				if (unmountStatus == 0) unmountStatus = *mountStatus;

				notInParty = false;
				while (stopLoop == false){
					ccMessageBox = false;
					//cout << "buh\n";
					acceptedPInvite = false;
					partyInvite("Cashader");  //maybe change this to send party invite after im done loading?
					//cout << "buh\n";
					timerStart();
					while (acceptedPInvite == false){
						Sleep(1000);
						cout << "Waiting for party invite accept..." << endl;

						if (timerWatch(18)){
							partyQuit();
							Sleep(1000);
							partyInvite("Cashader");
							timerStart();
						}
						//cout << "buh\n";
						Sleep(100);
					}
					if (*mountStatus == 0) toggleMount();
					//cout << "buh\n";
					characterSelect = false;
					DownKey(' ', file_kbd);
					Sleep(100);
					DownKey(' ', file_kbd);
					Sleep(100);
					//cout << "buh\n";
					if (*playerX <= 9447) TravelTo(9380, 1086);
					//cout << "buh\n";
					//if (*mountStatus == unmountStatus) toggleMount();

					injectDInput(DIK_ESCAPE, .1);
					while (injectKey == true) Sleep(10);
					injectDInput(DIK_ESCAPE, .1);
					while (injectKey == true) Sleep(10);
					cout << "buh\n";

					float tempX = *playerX;
					float tempYaw = *yaw;

					*yaw = 135;
					Sleep(300);
					//DownKey('a', file_kbd); // get to cc
					//DownKey('w', file_kbd);
					injectDInput(DIK_A, -1);
					//Sleep(500);
					timerStart();
					do{
						//tempX = *playerX;
						Sleep(200);
						sendCrypt(sendSocket_r, talkToCC, 5, 0);
						if (notInParty == true) break;
						if (ccFull == true){
							Sleep(5000);
							timerStart();
							ccFull = false;
						}
						if (timerWatch(15) == true){
							ReleaseKey('w', file_kbd);
							ReleaseKey('a', file_kbd);

							injectDInput(DIK_ESCAPE, .1);
							while (injectKey == true) Sleep(10);
							injectDInput(DIK_ESCAPE, .1);
							while (injectKey == true) Sleep(10);

							while (characterSelect == false) Sleep(10);

							if (accnum == 0) ccDisconnectHandler(2, 1, accounts[accnum], passwords[1], 1, charNum);
							else ccDisconnectHandler(2, 1, accounts[accnum], passwords[0], 1, charNum);

							DownKey('a', file_kbd); // get to cc
							DownKey('w', file_kbd);
							Sleep(500);

							timerStart();
						}
					}
					//while (*playerX != tempX);
					while (ccMessageBox == false);
					releaseInjectedKey = true;
					while (injectKey == true) Sleep(10);
					*yaw = tempYaw;
					//ReleaseKey('w', file_kbd);
					//ReleaseKey('a', file_kbd);
					//sendCrypt(sendSocket_r, talkToCC, 5, 0);
					//Sleep(250);

					if (characterSelect == true){
						if (accnum == 0) ccDisconnectHandler(2, 1, accounts[accnum], passwords[1], 1, charNum);
						else ccDisconnectHandler(2, 1, accounts[accnum], passwords[0], 1, charNum);
						continue;
					}

					while (notInParty == true || *partyMemberHP == 0){
						notInParty = false;
						partyQuit();
						Sleep(1000);
						injectDInput(DIK_ESCAPE, .1);
						while (injectKey == true) Sleep(10);

						partyInvite("Cashader");
						acceptedPInvite = false;
						timerStart();
						while (acceptedPInvite == false){
							if (characterSelect == true){
								if (accnum == 0) ccDisconnectHandler(2, 1, accounts[accnum], passwords[1], 1, charNum);
								else ccDisconnectHandler(2, 1, accounts[accnum], passwords[0], 1, charNum);
								characterSelect = false;
								dced = true;
								break;
							}
							if (timerWatch(18)){
								partyQuit();
								Sleep(1000);
								injectDInput(DIK_ESCAPE, .1);
								while (injectKey == true) Sleep(10);

								Sleep(500);
								partyInvite("Cashader");
								acceptedPInvite = false;
	
								timerStart();
							}

							Sleep(1000);
							cout << "Waiting for party invite accept...1" << endl;
						}
						if (dced == true) break;
						sendCrypt(sendSocket_r, talkToCC, 5, 0);
						Sleep(1000);
					}

					if (dced == true){
						dced = false;
						continue;
					}
					/*MoveMouseAbsolute(441, 222, file_mou);
					Sleep(1000);
					LeftClick(file_mou);
					Sleep(100);*/
					sendCrypt(sendSocket_r, acceptMessageBox, 4, 0);
					injectDInput(DIK_ESCAPE, .1);
					while (injectKey == true) Sleep(10);
					ccMessageBox = false;
					//int event_result = 0;
					event_result = disconnectCheck(1, accounts[accnum]);
					cout << event_result << endl;
					if (event_result == 1){ //uru-cc disconnect check
						dced = true;
						cout << "DC Event 1\n;";
						if (accnum == 0) ccDisconnectHandler(1, 1, accounts[accnum], passwords[1], 1, charNum);
						else ccDisconnectHandler(1, 1, accounts[accnum], passwords[0], 1, charNum);
						//continue;
						//if (accnum == 0) login(accounts[accnum], passwords[1]);
						//else login(accounts[accnum], passwords[0]);
						/*partyInvite("Cashader");  //maybe change this to send party invite after im done loading?
						acceptedPInvite = false;
						while (acceptedPInvite == false){
							Sleep(1000);
							cout << "Waiting for party invite accept..." << endl;
						}
						Sleep(1000);
						DownKey('6', file_kbd);
						Sleep(200);
						ReleaseKey('6', file_kbd);
						Sleep(3000);
						DownKey('a', file_kbd); // get to cc
						DownKey('w', file_kbd);
						Sleep(2000);
						ReleaseKey('w', file_kbd);
						ReleaseKey('a', file_kbd);
						sendCrypt(sendSocket_r, talkToCC, 5, 0);
						Sleep(1000);
						MoveMouseAbsolute(441, 222, file_mou);
						Sleep(100);
						LeftClick(file_mou);
						Sleep(100);
						event_result = disconnectCheck(1, accounts[accnum]);*/
					}
					if (event_result == 2){ //uru-cc disconnect check
						dced = true;
						cout << "DC Event 1\n;";
						if (accnum == 0) ccDisconnectHandler(2, 1, accounts[accnum], passwords[1], 1, charNum);
						else ccDisconnectHandler(2, 1, accounts[accnum], passwords[0], 1, charNum);
						//continue;
					}
					//dced = false;
					if (dced == true){
						dced = false;
						continue;
					}
					receivedDialogue = false;
					coffinOrBox = false;

					LARGE_INTEGER tcounter;
					LONGLONG ticknow, tickAttack;
					LONGLONG freq;
					QueryPerformanceFrequency(&tcounter);
					freq = tcounter.QuadPart;

					QueryPerformanceCounter(&tcounter);
					ticknow = tcounter.QuadPart;
					tickAttack = tcounter.QuadPart;

					Sleep(500);
					injectDInput(DIK_SPACE, .1);
					while (injectKey == true) Sleep(10);

					while (1){ // in cc
						Sleep(100);
						if (mapSwitch == true){
							Sleep(1000);
							partyQuit();
							Sleep(500);
							walktoCC();
							mapSwitch = false;
							/*partyInvite("Cashsader");
							acceptedPInvite = false;

							QueryPerformanceCounter(&tcounter);
							ticknow = tcounter.QuadPart;
							tickAttack = tcounter.QuadPart;

							while (acceptedPInvite == false || *partyMemberHP == 0){
								if ((ticknow - tickAttack) / (freq / 1000) >= (18 * 1000)){
									partyInvite("Cashsader");

									QueryPerformanceCounter(&tcounter);
									ticknow = tcounter.QuadPart;
									tickAttack = tcounter.QuadPart;
								}

								QueryPerformanceCounter(&tcounter);
								ticknow = tcounter.QuadPart;
							}
							mapSwitch = false;*/
							break;
						}
						if ((ticknow - tickAttack) / (freq / 1000) >= (15 * 1000)){ // timeout
							mapSwitch = false;
							partyQuit();
							//acceptedPInvite = false;
							//partyInvite("Cashader");
							Sleep(200);
							event_result = disconnectCheck(1, accounts[accnum]);
							if(event_result == 1){ //cc-uru dc
								cout << "DC Event 2\n;";
								dced = true;
								if (accnum == 0) ccDisconnectHandler(1, 2, accounts[accnum], passwords[1], 1, charNum);
								else ccDisconnectHandler(1, 2, accounts[accnum], passwords[0], 1, charNum);
								dced = false;
								break;
								/*if (accnum == 0) login(accounts[accnum], passwords[1]);
								else login(accounts[accnum], passwords[0]);
								partyInvite("Cashader");
								acceptedPInvite = false;
								while (acceptedPInvite == false){
								Sleep(1000);
								cout << "Waiting for party invite accept..." << endl;
								}
								DownKey('6', file_kbd);
								Sleep(100);
								ReleaseKey('6', file_kbd);
								Sleep(1500)*/
							}
							if (event_result == 0 && dced == false){
								/*partyInvite("Cashader");
								acceptedPInvite = false;
								while (acceptedPInvite == false){
									Sleep(1000);
									cout << "Waiting for party invite accept..." << endl;
								}*/
							}
							dced = false;
							break;
						} // timeout
						if (GetAsyncKeyState(VK_F4)){
							stopLoop = true;
							Sleep(1000);
						}
						if (receivedDialogue == true){ //got dialogue
							if (coffinOrBox == true){ //got zombies
								//partyInvite("N_I_C_E");
								Sleep(300);
								//partyInvite("TurtleEater");
								Sleep(300);
								//partyInvite("hahaweee");
								Sleep(300);

								sendCrypt(sendSocket_r, inParty_chat, 7, 0);

								//DownKey('w', file_kbd);
								//Sleep(500);
								//ReleaseKey('w', file_kbd);
								injectDInput(DIK_ESCAPE, .1);
								while (injectKey == true) Sleep(10);
								injectDInput(DIK_ESCAPE, .1);
								while (injectKey == true) Sleep(10);
								injectDInput(DIK_ESCAPE, .1);
								while (injectKey == true) Sleep(10);

								Sleep(300);
								TravelTo(1186, 4311);
								cout << "FUG\n";
								if (characterSelect == true){
									receivedDialogue = false;
									coffinOrBox = false;
									if (accnum == 0) ccDisconnectHandler(2, 2, accounts[accnum], passwords[1], 1, charNum);
									else ccDisconnectHandler(2, 2, accounts[accnum], passwords[0], 1, charNum);

									break;

								}
								cout << "FUG\n";
								if (*mountStatus >= 44) toggleMount();
								cout << "FUG\n";
								//DownKey('6', file_kbd);
								//Sleep(100);
								//ReleaseKey('6', file_kbd);
								//Sleep(1500);
								//getchar();
								//Sleep(3000);
								//for (int i = 0; i < 10; i++){
								mobESP_zombieGathers(); //open all boxes/coffins
								cout << "FUG\n";
								Sleep(100);
								attackZombies();
								cout << "FUG\n";
								Sleep(500);
								mobESP_zombies(); //kill remaining zombies
								cout << "FUG\n";
								Sleep(500);
								sendCrypt(sendSocket_r, lootParty_chat, 9, 0);
								//getchar();
								startAttack = false;
								while (startAttack == false) Sleep(100);
								/*while (1){
									//make a loot command?
									//cout << "X: " << *reinterpret_cast<float*>(playerX) << "\tY: " << *reinterpret_cast<float*>(playerY) << "\tZ: " << *reinterpret_cast<float*>(playerZ) << endl;
									cout << "X: " << (*playerX) << "\tY: " << (*playerY) << "\tZ: " << (*playerZ) << endl;
									//cout << "Mouse Coordinates:\tX:" << wow.x << "\t" << wow.y << endl;
									Sleep(1000);
									}*/
								//getchar();
								sendCrypt(sendSocket_r, outParty_chat, 8, 0);
								Sleep(1000);
								event_result = disconnectCheck(1, accounts[accnum]);
								while (event_result == 1){
									cout << "DC Event 3\n;";
									dced = true;
									if (accnum == 0) ccDisconnectHandler(1, 2, accounts[accnum], passwords[1], 1, charNum);
									else ccDisconnectHandler(1, 2, accounts[accnum], passwords[0], 1, charNum);
									dced = false;
									break;
									//if (accnum == 0) login(accounts[accnum], passwords[1]);
									//else login(accounts[accnum], passwords[0]);
									/*partyInvite("Cashader");
									acceptedPInvite = false;
									while (acceptedPInvite == false){
									Sleep(1000);
									cout << "Waiting for party invite accept..." << endl;
									}
									DownKey('6', file_kbd);
									Sleep(100);
									ReleaseKey('6', file_kbd);
									Sleep(1500);
									event_result = 0;*/
								}
								if (event_result == 0 && dced == false){
									Sleep(2000);
									/*partyInvite("Cashader");
									acceptedPInvite = false;

									QueryPerformanceFrequency(&tcounter);
									freq = tcounter.QuadPart;

									QueryPerformanceCounter(&tcounter);
									ticknow = tcounter.QuadPart;
									tickAttack = tcounter.QuadPart;
									while (acceptedPInvite == false){

										Sleep(1000);
										cout << "Waiting for party invite accept..." << endl;

										if ((ticknow - tickAttack) / (freq / 1000) >= (18 * 1000)){ // timeout
											partyQuit();
											Sleep(500);
											partyInvite("Cashader");
											QueryPerformanceCounter(&tcounter);
											ticknow = tcounter.QuadPart;
											tickAttack = tcounter.QuadPart;
										}

										QueryPerformanceCounter(&tcounter);
										ticknow = tcounter.QuadPart;
									}
									Sleep(500);
									DownKey('6', file_kbd);
									Sleep(100);
									ReleaseKey('6', file_kbd);
									Sleep(1800);*/
								}
								dced = false;
								coffinOrBox = false;
								break;
							} // got zombies

							else{ //if not zombies
								mapSwitch = false;
								partyQuit();
								Sleep(200);
								//acceptedPInvite = false;
								//partyInvite("Cashader");
								event_result = disconnectCheck(1, accounts[accnum]);
								if (event_result == 1){
									cout << "DC Event 4\n;";
									dced = true;
									if (accnum == 0) ccDisconnectHandler(1, 2, accounts[accnum], passwords[1], 1, charNum);
									else ccDisconnectHandler(1, 2, accounts[accnum], passwords[0], 1, charNum);
									dced = false;
									break;
									//if (accnum == 0) login(accounts[accnum], passwords[1]);
									//else login(accounts[accnum], passwords[0]);
									/*partyInvite("Cashader");
									acceptedPInvite = false;
									while (acceptedPInvite == false){
										Sleep(1000);
										cout << "Waiting for party invite accept..." << endl;
									}
									Sleep(1000);
									DownKey('6', file_kbd);
									Sleep(200);
									ReleaseKey('6', file_kbd);
									event_result = 0;*/
								}
								//Sleep(2000);
								if (event_result == 0 && dced == false){
									/*partyInvite("Cashader");
									acceptedPInvite = false;

									LARGE_INTEGER tcounter;
									LONGLONG tocknow;
									LONGLONG tockInvite;
									LONGLONG freq;
									QueryPerformanceFrequency(&tcounter);
									freq = tcounter.QuadPart;

									QueryPerformanceCounter(&tcounter);
									tocknow = tcounter.QuadPart;
									tockInvite = tcounter.QuadPart;

									while (acceptedPInvite == false){

									if ((tocknow - tockInvite) / (freq / 1000) > 18 * 1000){
									partyQuit();
									Sleep(1000);
									partyInvite("Cashader");
									acceptedPInvite = false;
									QueryPerformanceCounter(&tcounter);
									tocknow = tcounter.QuadPart;
									tockInvite = tcounter.QuadPart;
									}

									Sleep(1000);
									cout << "Waiting for party invite accept..." << endl;

									QueryPerformanceCounter(&tcounter);
									tocknow = tcounter.QuadPart;
									}*/
								}
								dced = false;
								break;
							} // if not zombies
						} // got dialogue

						QueryPerformanceCounter(&tcounter);
						ticknow = tcounter.QuadPart;
					} // in cc
				}
			}

			if (GetAsyncKeyState(VK_F5)){
				cout << "X: " << *playerX << "\tY: " << *playerY << endl;
				Sleep(1000);
			}

			if (GetAsyncKeyState(VK_F6)){
				partyQuit();
				Sleep(200);
				partyInvite("Cashader");
				Sleep(500);
			}

			if (GetAsyncKeyState(VK_F7)){
				walktoCC();
			}
			//Sleep(1000);
		}
	}

	if (ASSFUCK == '2'){
		while (1){
			characterSelect = false;
			if (receivedPInvite == true){
				/*partyAccept("Ballsy");
				Sleep(100);
				injectDInput(DIK_ESCAPE, .1);
				while (injectKey == true) Sleep(10);*/
				Sleep(200);
				MoveMouseAbsolute(441, 222, file_mou);
				Sleep(100);
				LeftClick(file_mou);
				receivedPInvite = false;
			}
			if (goinCC == true){
				int upCounter = 0;
				goinCC = false;
				TravelTo(9380, 1086);
				if (nooneMessageBox == true){
					injectDInput(DIK_ESCAPE, .1);
					while (injectKey == true) Sleep(10);
					nooneMessageBox = false;
				}
				Sleep(200);
				DownKey('d', file_kbd);
				DownKey('s', file_kbd);
				Sleep(4000);
				ReleaseKey('s', file_kbd);
				ReleaseKey('d', file_kbd);
				Sleep(200);

				while (ccMessageBox == false){
					sendCrypt(sendSocket_r, talkToCC, 5, 0);
					Sleep(1000);
				}
				ccMessageBox = false;
				Sleep(1000);
				MoveMouseAbsolute(441, 222, file_mou);
				Sleep(100);
				LeftClick(file_mou);
				event_result = disconnectCheck(1, accounts[accnum]);
				while (event_result == 1){
					if (accnum == 0) ccDisconnectHandler(1, 1, accounts[accnum], passwords[1], 1, charNum);
					else ccDisconnectHandler(1, 1, accounts[accnum], passwords[1], 1, charNum);
					sendCrypt(sendSocket_r, talkToCC, 5, 0);
					Sleep(1000);
					MoveMouseAbsolute(441, 222, file_mou);
					Sleep(100);
					LeftClick(file_mou);
					Sleep(100);
					event_result = disconnectCheck(1, accounts[accnum]);
				}

				Sleep(300);
				DownKey(' ', file_kbd);
				Sleep(100);
				ReleaseKey(' ', file_kbd);
				Sleep(200);
				TravelTo(1185, 5550);


				//getchar();
				enableDropTrack = true;
				gooutCC = false;

				LARGE_INTEGER tcounter;
				LONGLONG ticknow;
				LONGLONG tickLoot;
				LONGLONG freq;
				QueryPerformanceFrequency(&tcounter);
				freq = tcounter.QuadPart;

				QueryPerformanceCounter(&tcounter);
				ticknow = tcounter.QuadPart;
				tickLoot = tcounter.QuadPart;
				bool firstLoot = true;
				char packet[4] = { 0x03, 0x08, 0x20, 0x02 }; //set battlestate to use attacks
				//03 08 20 02
				sendCrypt(sendSocket_r, packet, 4, 0);
				while (1){
					if (gooutCC == true){
						partyQuit();
						restrictLoot = true;
						gooutCC = false;
						event_result = disconnectCheck(1, accounts[accnum]);
						while (event_result == 1){
							if (accnum == 0) ccDisconnectHandler(1, 1, accounts[accnum], passwords[1], 1, charNum);
							else ccDisconnectHandler(1, 1, accounts[accnum], passwords[1], 1, charNum);
						}
						goinCC = false;
						break;
					}

					if (moveUP == true){
						if (upCounter == 0) TravelTo(1185, 5350);
						if (upCounter == 1) TravelTo(*playerX, 5050);
						upCounter++;
						moveUP = false;
					}

					if (startLoot == true){
						float aoeXPositions[7][10] = {
							{ 1377, 1377, 1277, 1277, 1177, 1177, 1100, 1100, 1030, 1030 },
							{ 962, 962, 1007, 1007, 1107, 1107, 1234, 1234, 1334, 1334 },
							{ 1397, 1335, 1273, 1211, 1149, 1087, 1025, 963, 901, 882 },
							{ 962, 1019, 1076, 1133, 1190, 1247, 1304, 1361, 1418, 1477 },
							{ 1477, 1406, 1335, 1264, 1193, 1122, 1051, 980, 909, 832 },
							{ 962, 1019, 1076, 1133, 1190, 1247, 1304, 1361, 1418, 1457 },
							{ 1477, 1418, 1361, 1304, 1247, 1190, 1133, 1076, 1030, 1000 } };

						float aoeYpositions[7] = { 4738, 4631, 4531, 4462, 4362, 4232, 4132 };

						for (int y = 0; y < 7; y++){
							if (y == 0 || y == 1){
								TravelTo(1187, aoeYpositions[y]);
								Sleep(500);

							}
							for (int x = 0; x < 10; x++){
								TravelTo(aoeXPositions[y][x], aoeYpositions[y]);

								if (firstLoot == true){
									QueryPerformanceCounter(&tcounter);
									ticknow = tcounter.QuadPart;
									tickLoot = tcounter.QuadPart;
									firstLoot = false;
								}

								while ((ticknow - tickLoot) / (freq / 1000) < 1 * 200){
									//startLoot = false;
									if (checkQuests()){
										Sleep(200);
										enableDropTrack = true;
										QueryPerformanceCounter(&tcounter);
										ticknow = tcounter.QuadPart;
										tickLoot = tcounter.QuadPart;
									}

									else{
										Sleep(200);
										if (checkQuests()){
											Sleep(200);
											enableDropTrack = true;
											QueryPerformanceCounter(&tcounter);
											ticknow = tcounter.QuadPart;
											tickLoot = tcounter.QuadPart;
										}

										else enableDropTrack = true;
									}
									QueryPerformanceCounter(&tcounter);
									ticknow = tcounter.QuadPart;

								}

								firstLoot = true;
							}

						}

						startLoot = false;
						sendCrypt(sendSocket_r, goParty_chat, 7, 0);
					}

					if (GetAsyncKeyState(VK_F9)){
						enableDropTrack = false;
						break;
					}
				}
				//break;
			}
			//Sleep(1000);
		}

		getchar();

		enableDropTrack = true;
		/*LARGE_INTEGER tcounter;
		LONGLONG ticknow, tickLoot;
		LONGLONG freq;
		QueryPerformanceFrequency(&tcounter);
		freq = tcounter.QuadPart;

		QueryPerformanceCounter(&tcounter);
		ticknow = tcounter.QuadPart;
		tickLoot = tcounter.QuadPart;*/
		//bool timerStarted = false;
		while (1){
			if (checkQuests()){
				Sleep(500);
				enableDropTrack = true;
				//timerStarted = false;
			}

			/*if (checkQuests() == false){
				DownKey('2', file_kbd);
				Sleep(100);
				ReleaseKey('2', file_kbd);
				Sleep(600);
				}*/

			/*if (enableDropTrack == false){
				QueryPerformanceCounter(&LOOTtcounter);
				LOOTticknow = LOOTtcounter.QuadPart;

				if ((LOOTticknow - LOOTtickLoot) / (LOOTfreq / 1000) >= (4 * 1000)){
				//enableDropTrack = true;
				//timerStarted = false;
				}
				}*/

			if (LOOTTRAVEL == true){
				if (*playerX - TRAVELX > 0){
					DownKey('a', file_kbd);
					while (*playerX - TRAVELX > 75){
						//if (checkQuests()) enableDropTrack = true;
						Sleep(10);
					}
					ReleaseKey('a', file_kbd);
				}

				else if (*playerX - TRAVELX < 0){
					DownKey('d', file_kbd);
					while (*playerX - TRAVELX < -75){
						//if (checkQuests()) enableDropTrack = true;
						Sleep(10);
					}
					ReleaseKey('d', file_kbd);
				}
				//Y
				if (checkQuests()){
					Sleep(500);
					enableDropTrack = true;
					//timerStarted = false;
				}

				if (*playerY - TRAVELY > 0){
					DownKey('s', file_kbd);
					while (*playerY - TRAVELY > 10){
						//if (checkQuests()) enableDropTrack = true;
						Sleep(10);
					}
					ReleaseKey('s', file_kbd);
				}

				else if (*playerY - TRAVELY < 0){
					DownKey('w', file_kbd);
					while (*playerY - TRAVELY < -10){
						//if (checkQuests()) enableDropTrack = true;
						Sleep(10);
					}
					ReleaseKey('w', file_kbd);
				}

				LOOTTRAVEL = false;
			}

			Sleep(100);
		}
	}

	if (ASSFUCK == '3'){
		while (1){
			if (receivedPInvite == true){
				Sleep(200);
				MoveMouseAbsolute(441, 222, file_mou);
				Sleep(100);
				LeftClick(file_mou);
				receivedPInvite = false;
				TravelTo(9380, 1086);
				Sleep(200);
				DownKey('d', file_kbd);
				DownKey('s', file_kbd);
				Sleep(4000);
				ReleaseKey('s', file_kbd);
				ReleaseKey('d', file_kbd);
				Sleep(200);
				MoveMouseAbsolute(441, 222, file_mou);
				Sleep(100);
				LeftClick(file_mou);
				event_result = disconnectCheck(1, accounts[accnum]);
				while (event_result == 1){
					if (accnum == 0) login(accounts[accnum], passwords[1], charNum);
					else login(accounts[accnum], passwords[0], charNum);
					sendCrypt(sendSocket_r, talkToCC, 5, 0);
					Sleep(1000);
					MoveMouseAbsolute(441, 222, file_mou);
					Sleep(100);
					LeftClick(file_mou);
					Sleep(100);
					event_result = disconnectCheck(1, accounts[accnum]);
				}
				TravelTo(1185, 5641);
				receivedPInvite = false;
			}

			if (gooutCC == true){
				partyQuit();
				gooutCC = false;
				event_result = disconnectCheck(1, accounts[accnum]);
				while (event_result == 1){
					if (accnum == 0) login(accounts[accnum], passwords[1], charNum);
					else login(accounts[accnum], passwords[0], charNum);
					event_result = disconnectCheck(1, accounts[accnum]);
				}
				//break;
			}
			Sleep(1000);
		}
	}

}