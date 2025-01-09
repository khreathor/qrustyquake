#include "quakedef.h"

FILE *sys_handles[MAX_HANDLES];

// =============================================================================
// General routines
// =============================================================================

void Sys_Printf(char *fmt, ...)
{
	va_list argptr;
	char text[1024];
	va_start(argptr, fmt);
	vsprintf(text, fmt, argptr);
	va_end(argptr);
	fprintf(stderr, "%s", text);
}

void Sys_Quit()
{
	Host_Shutdown();
	exit(0);
}

void Sys_Error(char *error, ...)
{
	va_list argptr;
	char string[1024];
	va_start(argptr, error);
	vsprintf(string, error, argptr);
	va_end(argptr);
	fprintf(stderr, "Error: %s\n", string);
	Host_Shutdown();
	exit(1);
}

// =============================================================================
// File IO
// =============================================================================

int findhandle()
{
	for (int i = 1; i < MAX_HANDLES; i++)
		if (!sys_handles[i])
			return i;
	Sys_Error("out of handles");
	return -1;
}

static int Qfilelength(FILE *f)
{
	int pos = ftell(f);
	fseek(f, 0, SEEK_END);
	int end = ftell(f);
	fseek(f, pos, SEEK_SET);
	return end;
}

int Sys_FileOpenRead(char *path, int *hndl)
{
	int i = findhandle();
	FILE *f = fopen(path, "rb");
	if (!f) {
		*hndl = -1;
		return -1;
	}
	sys_handles[i] = f;
	*hndl = i;
	return Qfilelength(f);
}

int Sys_FileOpenWrite(char *path)
{
	int i = findhandle();
	FILE *f = fopen(path, "wb");
	if (!f)
		Sys_Error("Error opening %s: %s", path, strerror(errno));
	sys_handles[i] = f;
	return i;
}

void Sys_FileClose(int handle)
{
	fclose(sys_handles[handle]);
	sys_handles[handle] = NULL;
}

void Sys_FileSeek(int handle, int position)
{
	fseek(sys_handles[handle], position, SEEK_SET);
}

int Sys_FileRead(int handle, void *dst, int count)
{
	return fread(dst, 1, count, sys_handles[handle]);
}

int Sys_FileWrite(int handle, void *src, int count)
{
	return fwrite(src, 1, count, sys_handles[handle]);
}

int Sys_FileTime(char *path)
{
	FILE *f = fopen(path, "rb");
	if (f) {
		fclose(f);
		return 1;
	}
	return -1;
}

void Sys_mkdir(char *path)
{
#ifdef _WIN32
	_mkdir(path);
#else
	mkdir(path, 0777);
#endif
}

double Sys_FloatTime()
{
#ifdef _WIN32
	static int starttime = 0;
	if (!starttime)
		starttime = clock();
	return (clock() - starttime) * 1.0 / 1024;
#else
	struct timeval tp;
	struct timezone tzp;
	static int secbase;
	gettimeofday(&tp, &tzp);
	if (!secbase) {
		secbase = tp.tv_sec;
		return tp.tv_usec / 1000000.0;
	}
	return (tp.tv_sec - secbase) + tp.tv_usec / 1000000.0;
#endif
}

#ifdef _WIN32
// Function to parse lpCmdLine into argc and argv
void CommandLineToArgv(const char *lpCmdLine, int *argc, char ***argv)
{
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
#endif

int main(int c, char **v)
{
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
		Sys_Error("SDL_Init failed: %s", SDL_GetError());
	quakeparms_t parms;
	parms.memsize = DEFAULT_MEMORY;
	if (COM_CheckParm("-heapsize")) {
		int t = COM_CheckParm("-heapsize") + 1;
		if (t < c)
			parms.memsize = Q_atoi(v[t]) * 1024;
	}
	parms.membase = malloc(parms.memsize);
	parms.basedir = ".";
	COM_InitArgv(c, v);
	parms.argc = com_argc;
	parms.argv = com_argv;
	Host_Init(&parms);
	double oldtime = Sys_FloatTime() - 0.1;
	while (1) {
		double newtime = Sys_FloatTime();
		double deltatime = newtime - oldtime;
		if (cls.state == ca_dedicated)
			deltatime = sys_ticrate.value;
		if (deltatime > sys_ticrate.value * 2)
			oldtime = newtime;
		else
			oldtime += deltatime;
		Host_Frame(deltatime);
	}
}
