#include <Windows.h>
#include <TlHelp32.h>
#include <iostream>

int getProcessId(const char* &processName) {
	int processId = -1;
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 sSnapshot;
	sSnapshot.dwSize = sizeof(sSnapshot);
	if (hSnapshot == INVALID_HANDLE_VALUE) {
		return -2;
	}
	if (Process32First(hSnapshot, &sSnapshot)) {
		do {
			printf("%s\t%d\n", sSnapshot.szExeFile, sSnapshot.th32ProcessID);
			if (!strcmp(sSnapshot.szExeFile, processName)) {
				processId = sSnapshot.th32ProcessID;
			}
		} while (processId == -1 && Process32Next(hSnapshot, &sSnapshot));
	}
	CloseHandle(hSnapshot);
	return processId;
}

bool inject(const int &processId, const char* &dllPath) {

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
	if (hProcess == NULL) {
		printf("Couldn't open target process\n");
		return false;
	}
	printf("Opened target process\n");

	LPVOID AllocAddress = VirtualAllocEx(hProcess, NULL, strlen(dllPath) + 1, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (AllocAddress == NULL) {
		printf("Couldn't allocate memory\n");
		return false;
	}
	printf("Allocated memory\n");
	BOOL WroteMemory = WriteProcessMemory(hProcess, AllocAddress, dllPath, strlen(dllPath) + 1, NULL);
	if (!WroteMemory) {
		printf("Couldn't write memory\n");
		return false;
	}
	printf("Wrote process memory\n");
	LPTHREAD_START_ROUTINE LoadLibraryAddress = (LPTHREAD_START_ROUTINE)GetProcAddress(LoadLibrary("kernel32"), "LoadLibraryA");
	HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, LoadLibraryAddress, AllocAddress, 0, NULL);
	if (hThread == NULL) {
		printf("Couldn't create thread\n");
		return false;
	}
	printf("Created Thread\n");
	printf("DLL injected\n");
	return true;
}

void usage(const char* name) {
	printf("usage: %s [-n process_name | -p process_id] dll_path", name);
}

int main(const int argc, const char** argv) {
	if (argc == 4) {
		if (!strcmp(argv[1], "-n")) {
			inject(getProcessId(argv[2]), argv[3]);
		}
		else if (!strcmp(argv[1], "-i")) {
			inject(atoi(argv[2]), argv[3]);
		}
		else {
			usage(argv[0]);
		}
	}
	else {
		usage(argv[0]);
	}
}
