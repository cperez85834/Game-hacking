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

//Calculates distance between two objects in game. No Z coordinate because Fiesta Online doesn't really utilize it for anything... jumping in game is simpling a "jump animation",
//as opposed to actually increasing your Z value
float distanceFunc(float x1, float y1, float x2, float y2){
	return sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2));
}

//Gets base pointer of game program.
DWORD getBasePointer(){

	HWND WindowHandle = FindWindow(nullptr, L"FiestaOnline");
	DWORD PID;
	GetWindowThreadProcessId(WindowHandle, &PID);
	PVOID hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, 0, PID);
	HMODULE hMods[1024];
	DWORD cbNeeded;
	unsigned int i;

	//Gets list of processes and tries to match the window handle I specified
	if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded))
	{
		for (i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
		{
			cout << i << endl;
			TCHAR szModName[MAX_PATH];
			if (GetModuleFileNameEx(hProcess, hMods[i], szModName, sizeof(szModName) / sizeof(TCHAR)))
			{
				wstring wstrModName = szModName;
				//you will need to change this to the name of the exe of the foreign process
				wstring wstrModContain = L"Fiesta.exe";
				if (wstrModName.find(wstrModContain) != string::npos)
				{
					CloseHandle(WindowHandle);
					return (DWORD)hMods[i];
				}
			}
		}
	}

	return 0;
}

//Removes camera zoom limit so it can be as far out as you like
void toggleZoom(){
	//Backup bytes
	unsigned char backup[10] = { 0 };
	DWORD prevBP;
	
	//Reads current memory and stores it in backup
	ReadProcessMemory(GetCurrentProcess(), (LPVOID)zoom, backup, 1, 0);
	
	//If the first byte is 0x7A that means the patch has not been applied, so apply it
	if (backup[0] == 0x7A){
		VirtualProtect((void*)zoom, 1, PAGE_EXECUTE_READWRITE, &prevBP);
		memset((void*)zoom, (BYTE)0xEB, 1);
		VirtualProtect((void*)zoom, 1, prevBP, &prevBP);
	}
	//else reverse the patch
	else if (backup[0] == 0xEB){
		VirtualProtect((void*)zoom, 1, PAGE_EXECUTE_READWRITE, &prevBP);
		memset((void*)zoom, (BYTE)0x7A, 1);
		VirtualProtect((void*)zoom, 1, prevBP, &prevBP);
	}
}

//These functions specify what module and function to hook by name
//Can be consolidated into one function by just adding the jmp address as an argument
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


//Hooks the send function in ws2_32.dll; this is how the game client communicates with the server and hooking the function
//allows me to modify or inject packets to send as I see fit
int __stdcall nSend(SOCKET s, const char *buf, int len, int flags){
	UnHookFunction("ws2_32.dll", "send", hook);

	unsigned int x = 0;

	//This logs the packets to a text document
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

	//this setup function 
	if (setup == false){
		if (len == 0x43 && packetByte_socket[0] == 0x65){
			char* bytesToSend = new char[len];
			bool sixteenMult = true;
			int byteCounter = 1;
			bytesToSend[0] = len - 1;
			DWORD* crypterPointer = (DWORD*)lace;

			for (int i = 1; i < len; i++){
				bytesToSend[i] = binSpoof[i - 1];
			}
			int crypter = *(WORD*)lace;

			*crypter -= (len - 1);
			if (crypter < 0){
				crypter += 0x1F3;
			}

			for (int i = 1; i < len; i++){
				if (*crypter == 0x1F3) *crypter = 0;
				bytesToSend[i] = bytesToSend[i] ^ encryptionTable[*crypter];
				*crypter++;
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

	if (len == 0x05 && packetByte_socket[0] == 0x2d && packetByte_socket[1] == 0x20){ //send instant gather
		WORD* crypter = (WORD*)encryptCounterstatic;
		char* bytesToSend = new char[3];
		bytesToSend[0] = 0x02;
		bytesToSend[1] = 0x32;
		bytesToSend[2] = 0x20;
		cout << "WHY :(\n";
		for (int i = 1; i < 3; i++){
			if (*crypter == 0x1F3) crypter = 0;
			bytesToSend[i] = bytesToSend[i] ^ encryptionTable[*crypter];
			(*crypter)++;
		}

		send(s, bytesToSend, 3, 0);

		delete bytesToSend;
	}
	//f 01 20 00 0b 26 61 64 6d 69 6e 6c 65 76 65 6c
	//this packet is sent when map is successfully switched
	if (len == 0x10 && packetByte_socket[0] == 0x01 && packetByte_socket[1] == 0x20 && packetByte_socket[2] == 0x00 &&
		packetByte_socket[3] == 0x0b && packetByte_socket[4] == 0x26){
		mapSwitch = true;
	}

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


//Send a packet using the game's encryption method
void sendCrypt(SOCKET s, char* buf, int len, int flags){
	char* buffy = new char[len];

	for (int i = 0; i < len; i++){
		buffy[i] = buf[i];
	}
	while (sending == true);
	sending = true;
	WORD *crypter = NULL;
	//Changes the crypter pointer based on what type of packet i need to send
	if (flags == 0) crypter = (WORD*)encryptCounterstatic;
	else if (flags == 1) crypter = (WORD*)encryptCounterstatic_Invite;

	for (int i = 1; i < len; i++){
		packetByte_socket[i - 1] = buffy[i];
		//cout << hex << "Assman is: " << *assman << endl;
		buffy[i] = buffy[i] ^ encryptionTable[*crypter];
		(*crypter)++;
		if (*crypter == 0x1F3) *crypter = 0;

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

//This function hooks the recv function of the game.
int WINAPI nRecv(SOCKET s, char* buf, int len, int flags)
{
	UnHookFunction("ws2_32.dll", "recv", hook2);
	//DWORD tmp;

	len = recv(s, buf, len, flags);


	if (len == 0x314 && buf[1] == 0x11 && buf[2] == 0x03 && buf[3] == 0x14 && buf[4] == 0x0c) characterSelect = true;
	if (len > 0 && buf[0] == 0x16 && buf[1] == 0x03 && buf[2] == 0x38 && receivedPInvite == false){
		receivedPInvite = true;
	}
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

	if (len > 0){
		for (int w = 0; w < len - 23; w++){
			if (buf[w] == 0x07 && buf[w + 1] == 0x38 && buf[w + 22] == 0x1a){
				cout << "YEET" << endl;
				acceptedPInvite = true;
			}
		}
	}
	unsigned short x = 0;
	//The following code reads information from a packet to see if it contains the word "buff"
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
										
										cout << hex << static_cast<unsigned>(buf[j + 3]) << " " << static_cast<unsigned>(buf[j + 4]) << endl;
										for (int w = 0; w < 200; w++){
											if (buffEm[w] == 0){
												buffEm[w] = playerToBuff.id;
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

		//The following reads packets and stores them in the log if the else statement is executed. Due to the large amount of data received, 
		//I need to be able to turn it off.
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


	//find this by hmm debug on ws2.send, look in call stack for 2nd from top and press enter on it, and go to call previous to the one just called
	DWORD hookAddressCrypt = fiestaBase + 0x56073B; //encryption hook, postRNG function 53 56 57 8b 7d 08 0f b7 31
	//xor byte ptr ds:[edx+edi],bl
	//inc word ptr ds:[ecx]

	//DWORD hookAddressMove = fiestaBase + 0x288918;
	DWORD hookAddressCrypt2 = fiestaBase + 0x56072C; //encryption hook, preRNG function
	//push ebx
	//push esi
	//push edi

	jmpBackEncryptRNG = hookAddressCrypt2 + 0x6;
	jmpBackAddy = hookAddressCrypt + 0x6;


	HookFunctionAddy((void*)hookAddressCrypt2, preEncrypt, 6);
	HookFunctionAddy((void*)hookAddressCrypt, ourFunct, 6);

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
	//
	// Send an IOCTL to retrive the keyboard attributes
	// These are cached in the kbfiltr
	//
#pragma warning(disable:4127)
}

//Sends a packet to loot an item on a ground with the desired drop id
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

//Sends packet to turn in quest
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

//sends packet to progress quest text
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

//Sends packet to accept quest rewards
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

//sends packet to accept quest
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

//Checks quests in quest list for completion
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

//Uses a specified skill, on the specified mob, at the specified points
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

			QueryPerformanceCounter(&tcounter);
			toUse->tickAtCast = tcounter.QuadPart;
			globalCooldown = tcounter.QuadPart;
			used = true;

		}
		delete packet;
	}
	return used;
}

//Handles the automatic looting hook
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

	else if (currentDrop.distance < 600 && currentDrop.distance >= 85 && (currentDrop.itemID == 55000 || currentDrop.itemID == 55001)
	&& currentDrop.dropID != lastDrop.dropID && LOOTTRAVEL == false){
		TRAVELX = dropX;
		TRAVELY = dropY;
		LOOTTRAVEL = true;
	}
}
void __declspec(naked) dropsHook(){

	//The following assembly code checks to see if my patch is enabled,
	//then executes custom functions if it is
	__asm{ //start
		pushad
		pushfd
		lea ebx, [esi]
	}

	__asm{
		cmp enableDropTrack, 1
			je wow
			popfd
			popad
			mov edx, [eax + 0x000000C4]
			jmp jmpBackAddyDrops
	}
	__asm{
		wow : mov eax, [ebx + 0x58]
			  mov dropX, eax
	}
	__asm{
			mov eax, [ebx + 0x5c]
			mov dropY, eax
	}
	__asm{
			mov al, byte ptr[edi + 0x06]
			mov dropID[0], al
	}
	__asm{
			mov al, byte ptr[edi + 0x07]
			mov dropID[1], al
	}
	__asm{
			mov al, byte ptr[edi + 0x04]
				mov itemID[0], al
	}
	__asm{
			mov al, byte ptr[edi + 0x05]
				mov itemID[1], al
	}


	dropstuff.bytes[0] = itemID[0];
	dropstuff.bytes[1] = itemID[1];
	currentDrop.itemID = dropstuff.id;
	dropstuff.bytes[0] = dropID[0];
	dropstuff.bytes[1] = dropID[1];
	currentDrop.dropID = dropstuff.id;
	//printf("Yepp9\n");

	lootHandler();

	__asm{ // end
		popfd
		popad
		mov edx, [eax + 0x000000C4]
		jmp jmpBackAddyDrops
	}
}

//Checks if player has disconnected
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

//Sends party invite to specified player
void partyInvite(string nameToInvite){

	char *packet = new char[23];
	packet[0] = 0x16;
	packet[1] = 0x02;
	packet[2] = 0x38;
	for (int i = 3; i < 23; i++){
		if ((i - 3) < nameToInvite.length()) packet[i] = nameToInvite[i - 3];
		else packet[i] = 0x00;
	}

	sendCrypt(sendSocket_r_Invite, packet, 23, 1);
	delete packet;

}

//Accept a party invite
void partyAccept(string nameOfInviter){

	char *packet = new char[23];
	packet[0] = 0x16;
	packet[1] = 0x04;
	packet[2] = 0x38;
	for (int i = 3; i < 23; i++){
		if ((i - 3) < nameOfInviter.length()) packet[i] = nameOfInviter[i - 3];
		else packet[i] = 0x00;
	}


	sendCrypt(sendSocket_r_Invite, packet, 23, 1);
	delete packet;

}

//Quit party
void partyQuit(){
	char* packet = new char[3];
	packet[0] = 0x02;
	packet[1] = 0x0a;
	packet[2] = 0x38;

	sendCrypt(sendSocket_r_Invite, packet, 3, 1);
	delete packet;
}

//Gets distance from player to a specified entity
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
		DWORD entityBase = *(DWORD*)(entities[i] + 0x08);
		if (entityBase < fiestaBase){
			//i++;
			continue;
		}

		DWORD* entityDeref = (DWORD*)(entityBase + 0x2cc);
		if (entityDeref == 0) continue;

		DWORD entityDetails = *(DWORD*)entityDeref;
		if (entityDetails < fiestaBase){
			//i++;
			continue;
		}

		WORD currentMobID = *(WORD*)(entityBase + 0x3BE);

		WORD currentSelectionId = *(WORD*)(entityBase + 0x234);
		if (*(BYTE*)(entityBase + 0x234) != ID[0] || *(BYTE*)(entityBase + 0x235) != ID[1]) continue;
		cout << hex << static_cast<unsigned>(*(BYTE*)(entityBase + 0x234)) << " " << static_cast<unsigned>(*(BYTE*)(entityBase + 0x235)) << endl;
		float *currentEntityX = (float*)(entityDetails + 0x58);
		float *currentEntityY = (float*)(entityDetails + 0x5C);

		if (currentEntityX == NULL || currentEntityY == NULL) continue;

		distToEntity = sqrt(pow((*(playerX)-*currentEntityX), 2) + pow((*(playerY)-*currentEntityY), 2));

		break;
	}
	return distToEntity;
	//cout << "lol" << endl;
}

//An automatic trainer for a character class
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
		cout << hex << "entities[i]: " << entities[i] << endl;
		cout << "Getting Base: " << (DWORD*)(entities[i] + 0x08) << endl;
		DWORD entityBase = 0;

		if (isMemReadable((LPCVOID)(entities[i] + 0x08), sizeof(DWORD))) entityBase = *(DWORD*)(entities[i] + 0x08);
		else continue;

		if (entityBase < fiestaBase || entityBase > 0x7ffff000){
			//i++;
			continue;
		}

		cout << hex << "entityBase: " << entityBase << endl;
		cout << hex << "MobID address: " << (WORD*)(entityBase + 0x3BE) << endl;

		WORD currentMobID = 0;
		if (isMemReadable((LPCVOID)(entityBase + 0x3BE), sizeof(WORD))) currentMobID = *(WORD*)(entityBase + 0x3BE);
		else continue;
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
		if (foundMob == false) continue;
		cout << hex << "currentMobID: " << currentMobID << endl;

		cout << "entityDetails to dereference: " << (DWORD*)(entityBase + 0x2cc) << endl;
		DWORD* entityDeref = (DWORD*)(entityBase + 0x2cc);
		if (entityDeref == 0) continue;

		DWORD entityDetails = 0;
		if (isMemReadable((LPCVOID)entityDeref, sizeof(DWORD))) entityDetails = *(DWORD*)entityDeref;
		cout << hex << "entityDetails: " << entityDetails << endl;

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

		float *currentEntityX = (float*)(entityDetails + 0x58);
		float *currentEntityY = (float*)(entityDetails + 0x5C);

		if (currentEntityX == NULL || currentEntityY == NULL) continue;

		float distToPlayer = sqrt(pow((*(playerX)-*currentEntityX), 2) + pow((*(playerY)-*currentEntityY), 2));
		//distToCenter is the distance to a center point around which the player should kill. Anything greater than a distance of whatever won't be targetted
		float distToCenter = sqrt(pow((initialX - *currentEntityX), 2) + pow((initialY - *currentEntityY), 2));
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

		cout << "heh" << endl;
		DWORD shortestDetails = 0;
		if (isMemReadable((LPCVOID)(shortestBase + 0x2cc), sizeof(DWORD))) shortestDetails = *(DWORD*)(shortestBase + 0x2cc);
		else return;
		int *shortestHealth = (int*)(shortestBase + 0x288);


		lastEntity = shortestEntity;
		unsigned char id[2] = { *(BYTE*)(shortestBase + 0x234), *(BYTE*)(shortestBase + 0x235) };
		char* target = new char[5];

		target[0] = 0x04;
		target[1] = 0x01;
		target[2] = 0x24;
		target[3] = id[0];
		target[4] = id[1];

		sendCrypt(sendSocket_r, target, 5, 0);
		if(stolenAggro == true) Sleep(1000);

		IDtoBYTE playerstuff;
		playerstuff.bytes[0] = playerID[0];
		playerstuff.bytes[1] = playerID[1];
		int previousHealth = *shortestHealth;
		float distToCenter = sqrt(pow((initialX - *shortestEntityX), 2) + pow((initialY - *shortestEntityY), 2));
		float distToPlayer = sqrt(pow((*(playerX)-*shortestEntityX), 2) + pow((*(playerY)-*shortestEntityY), 2));
		bool useMoonlight = false;
		bool useSunlight = false;
		while (*shortestHealth > 0){
			if (distToCenter >= killRadius) break;
			cout << "shortestEntityX: " << shortestEntityX << "\tshortestEntityY: " << shortestEntityY << endl;
			distToCenter = sqrt(pow((initialX - *shortestEntityX), 2) + pow((initialY - *shortestEntityY), 2));
			distToPlayer = sqrt(pow((*(playerX)-*shortestEntityX), 2) + pow((*(playerY)-*shortestEntityY), 2));
			cout << "distToCenter: " << distToCenter << "\tdistToPlayer: " << distToPlayer << endl;
			lastPosX = *playerX;
			lastPosY = *playerY;

			if ((ticknow - tickAttack) / (freq / 1000) >= (10 * 1000)) break;

			if (GetAsyncKeyState(VK_F1)) break;

			if (distToPlayer <= 50){
				useSkill(&healbot_Bash, id, 0, 0);
			}

			cout << "Health: " << dec << *shortestHealth << endl;
			if (*health == 0) break;

			if (*health <= *maxhealth * 0.30){
				DownKey('=', file_kbd);
				Sleep(100);
				ReleaseKey('=', file_kbd);
			}

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

//Bot to automatically gather resources in the world
void gatherBot(int ids[20]){

		entityPointer = (fiestaBase + 0x700C58);  //base pointer/pointer to me
		cout << "hmmm" << endl;
		//DWORD entity = *(DWORD*)entityPointer;
		DWORD entities[1024] = { 0 };
		DWORD entityName, entityID;
		char name[30] = { 0 };

		entities[0] = *(DWORD*)(entityPointer + 0x3C);
		if (playerID[0] == 0xFF && playerID[1] == 0xFF){
			DWORD player = *(DWORD*)(entities[0] + 0x08);
			playerID[0] = *(BYTE*)(player + 0x234);
			playerID[1] = *(BYTE*)(player + 0x235);
		}

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
		for (int i = 0; entities[i] != 0; i++){
			WORD membuf = 0;
			float coordbuff = 0;
			cout << hex << "entities[i]: " << entities[i] << endl;
			cout << "Getting Base: " << (DWORD*)(entities[i] + 0x08) << endl;
			DWORD entityBase = 0;

			if (isMemReadable((LPCVOID)(entities[i] + 0x08), sizeof(DWORD))) entityBase = *(DWORD*)(entities[i] + 0x08);
			else continue;
			if (entityBase < fiestaBase || entityBase > 0x7ffff000){
				//i++;
				continue;
			}
			cout << hex << "entityBase: " << entityBase << endl;
			cout << hex << "MobID address: " << (WORD*)(entityBase + 0x3BE) << endl;

			WORD currentMobID = 0;
			if (isMemReadable((LPCVOID)(entityBase + 0x3BE), sizeof(WORD))) currentMobID = *(WORD*)(entityBase + 0x3BE);
			else continue;

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

			cout << "entityDetails to dereference: " << (DWORD*)(entityBase + 0x2cc) << endl;
			DWORD* entityDeref = (DWORD*)(entityBase + 0x2cc);
			if (entityDeref == 0) continue;

			DWORD entityDetails = 0;
			if (isMemReadable((LPCVOID)entityDeref, sizeof(DWORD))) entityDetails = *(DWORD*)entityDeref;
			//DWORD entityDetails = *(DWORD*)entityDeref;
			cout << hex << "entityDetails: " << entityDetails << endl;
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

			DWORD shortestDetails = 0;
			if (isMemReadable((LPCVOID)(shortestBase + 0x2cc), sizeof(DWORD))) shortestDetails = *(DWORD*)(shortestBase + 0x2cc);
			else return;

			int *shortestHealth = (int*)(shortestBase + 0x288);
			if (*shortestHealth == 0) return;

			WORD shortestMobID = *(WORD*)(shortestBase + 0x3BE);
			lastEntity = shortestEntity;
			unsigned char id[2] = { *(BYTE*)(shortestBase + 0x234), *(BYTE*)(shortestBase + 0x235) };
			char* target = new char[5];

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

				if ((ticknow - tickAttack) / (freq / 1000) >= (10 * 1000)){
					//TravelTo(*shortestEntityX, *shortestEntityY);
					break;
				}

				if (GetAsyncKeyState(VK_F1)) break;

				cout << "Health: " << dec << *shortestHealth << endl;
				if (*health == 0){
					//while (1) Sleep(1000);
					break;
				}

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

//Automatic login
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
