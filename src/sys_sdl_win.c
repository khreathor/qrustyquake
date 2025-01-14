#include "sys_sdl.c"
// CyanBun96: i ain't touching this shit anymore, if you want this to work on windoze fix it yourself
int Sys_FileOpenWrite(char *path)
{
	return 0;
}

int Sys_FileWrite(int handle, void *src, int count)
{
	return 0;
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
