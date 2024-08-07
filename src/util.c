#include "util.h"
#include <errno.h>

#ifdef _WIN32
#include <conio.h>
#include <shlobj.h>
#define _us_getchar _getch

errno_t get_cfg_path(_Out_writes_(_MAX_PATH) char* buffer, _In_z_ const char* name)
{
	if (!SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, buffer)))
	{
		return EIO;
	}
	strcat(buffer, "\\");
	strcat(buffer, name);
	strcat(buffer, "\\");

	if (CreateDirectoryA(buffer, NULL))
	{
		SetFileAttributesA(buffer, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_NOT_CONTENT_INDEXED);
		return EEXIST;
	}
	else
	{
		switch (GetLastError())
		{
		case ERROR_ALREADY_EXISTS:
			break;
		case ERROR_PATH_NOT_FOUND:
			return ENFILE;
		}
	}
	return 0;
}

void sleep(unsigned long ms)
{
	Sleep(ms);
}
#else
#define _us_getchar getch

#include <unistd.h>
void sleep(unsigned long ms)
{
	nanosleep((const struct timespec[]) {
		{
			0, ms * 1000000L
		}
	}, NULL);
}
#endif

errno_t create_directory(_In_z_ const char* path)
{
	return !CreateDirectoryA(path, NULL);
}

errno_t delete_directory(_Inout_z_ char* path)
{
	path[strlen(path) + 1] = 0;
	SHFILEOPSTRUCTA file_op = {
		NULL,
		FO_DELETE,
		path,
		NULL,
		FOF_NO_UI,
		FALSE,
		0,
		0
	};
	return SHFileOperationA(&file_op);
}

int get_directory_contents(_Inout_z_ char* path, _Out_writes_(max) char contents[][16], int max, bool is_dir)
{
	strcat(path, "\\*");
	WIN32_FIND_DATAA findData;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	hFind = FindFirstFileA(path, &findData);
	path[strlen(path) - 2] = 0;

	if (hFind == INVALID_HANDLE_VALUE) return -1;

	int i = 0;
	while (FindNextFileA(hFind, &findData) && i < max)
	{
		if (is_dir && !(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) continue;
		if (strcmp(findData.cFileName, "..") == 0) continue;

		strcpy_s(contents[i++], 16, findData.cFileName);
	}
	if (i < max)
	{
		contents[i][0] = 0;
	}

	FindClose(hFind);
	return 0;
}

USChar us_getchar(void)
{
	char c = _us_getchar();
	switch (c)
	{
	case 3:
	case 4:
		return USCharExit;

	case 0:
	case 224:
	case -32:
		char b = _us_getchar();
		switch (b)
		{
		case 72: return USCharUp;
		case 80: return USCharDown;
		case 77: return USCharRight;
		case 75: return USCharLeft;

		case 115: return USCharLeftWord;
		case 116: return USCharRightWord;
		case -115: return USCharUpWord;
		case -111: return USCharDownWord;

		case 71: return USCharHome;
		case 79: return USCharEnd;

		case 83: return USCharDelete;
		case -109: return USCharDeleteWord;

		case 73: return USCharPageUp;
		case 81: return USCharPageDown;
		case -122: return USCharPageUpWord;
		case 118: return USCharPageDownWord;
		case -108: return USCharDetails;

		default:
#if _DEBUG
			printf("%i", (int)b);
			fflush(stdout);
#endif
			return USCharUnknown;
		}
		break;

	default:
		return c;
	}
}

bool us_chrdy(void)
{
	return _kbhit();
}

bool strislwr(_In_z_ const char* str)
{
	int c = 0;
	while (str[c])
	{
		if (isalpha(str[c]) && !islower(str[c])) return false;
		c++;
	}
	return true;
}

static char word_delimiters[] = " /\\()\"'-.,:;<>~!@#$%^&*|+=[]{}?";
bool iswordend(const char c)
{
	return strchr(word_delimiters, c);
}

int strcatc(_Inout_z_ char* str, char c)
{
	size_t len = strlen(str);
	str[len] = c;
	str[len + 1] = 0;
	return 0;
}

void delete_self(void)
{
#ifdef _WIN32
	STARTUPINFOA si = { sizeof(STARTUPINFOA) };
	PROCESS_INFORMATION pi;
	char cmd_fmt[] = "cmd.exe /C " // start seperate cmd process
		"FOR / L % %L IN(0, 1, 10) DO " // 10 attempts to
		"Del \"%s\" && IF NOT EXIST \"%s\" (exit /b) " // delete file
		"ELSE timeout /NOBREAK 1"; // otherwise wait
	char cmd[MAX_PATH * 2 + sizeof(cmd_fmt)];
	sprintf(cmd, cmd_fmt, __argv[0], __argv[0]);
	CreateProcessA(NULL, cmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
#else
	unlink(__argv[0]);
#endif
}