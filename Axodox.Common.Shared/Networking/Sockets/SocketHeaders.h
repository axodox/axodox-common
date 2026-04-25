#pragma once

#ifdef PLATFORM_LINUX
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/ioctl.h>
typedef int socket_t;
#define INVALID_SOCKET -1
#define closesocket close
#define ioctlsocket ioctl
#endif

#ifdef PLATFORM_WINDOWS
#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif
#define byte win_byte
#include <Winsock2.h>
#include <Ws2tcpip.h>
#undef byte
#pragma comment(lib, "Ws2_32.lib")
typedef int socklen_t;
typedef SOCKET socket_t;
#endif
