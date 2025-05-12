#include <errno.h>
#include <stddef.h>
#include <limits.h>
#include <stdbool.h>
#include <signal.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <setjmp.h>
#include <float.h>
#include <time.h>
#include <sys/types.h>
#ifndef _WIN32
#include <unistd.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/ipc.h>
#ifndef __HAIKU__
#include <sys/shm.h>
#endif
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <SDL2/SDL.h>
#else
#include <winsock2.h>
#include <ws2tcpip.h>
#ifndef _WINCROSS
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif
#include <direct.h>
#include <io.h>
#endif
