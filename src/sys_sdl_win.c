#include "sys_sdl.c"
int Sys_FileOpenWrite (char *path) // CyanBun96: pasta from Quakespasm
{
	int i = findhandle ();
	FILE *f = fopen(path, "wb");
	if (!f) Sys_Error ("Error opening %s: %s", path, strerror(errno));
	sys_handles[i] = f;
	return i;
}

int Sys_FileWrite (int handle, void *data, int count) // ditto
{
	return fwrite (data, 1, count, sys_handles[handle]);
}

void Sys_mkdir(char *path)
{
	_mkdir(path);
}

double Sys_FloatTime()
{
	static int starttime = 0;
	if (!starttime)
		starttime = clock();
	return (clock() - starttime) * 1.0 / 1024;
}

double Sys_DoubleTime() { return Sys_FloatTime(); }

#ifndef INVALID_FILE_ATTRIBUTES
#define INVALID_FILE_ATTRIBUTES ((DWORD)-1)
#endif
int Sys_FileType (const char *path)
{
        DWORD result = GetFileAttributes(path);

        if (result == INVALID_FILE_ATTRIBUTES)
                return FS_ENT_NONE;
        if (result & FILE_ATTRIBUTE_DIRECTORY)
                return FS_ENT_DIRECTORY;

        return FS_ENT_FILE;
}

void CommandLineToArgv(const char *lpCmdLine, int *argc, char ***argv)
{ // Function to parse lpCmdLine into argc and argv
	*argc = 0;
	char *cmdLine = strdup(lpCmdLine);
	char *token = strtok(cmdLine, " ");
	while (token) {
		(*argc)++;
		token = strtok(NULL, " ");
	}
	*argv = (char **)malloc((*argc + 1) * sizeof(char *));
	strcpy(cmdLine, lpCmdLine);
	token = strtok(cmdLine, " ");
	for (int i = 0; i < *argc; i++) {
		(*argv)[i] = strdup(token);
		token = strtok(NULL, " ");
	}
	(*argv)[*argc] = NULL;
	free(cmdLine);
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine,
	    int nCmdShow)
{
	int argc;
	char **argv;
	CommandLineToArgv(lpCmdLine, &argc, &argv);
	int retCode = main(argc, argv);
	for (int i = 0; i < argc; i++) {
		free(argv[i]);
	}
	free(argv);
	return retCode;
}
