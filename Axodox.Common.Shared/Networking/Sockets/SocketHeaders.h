#pragma once

#ifdef PLATFORM_LINUX
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/ioctl.h>
using socket_t = int;
using socklen_t = size_t;
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
using socket_t = SOCKET;
using socklen_t = int;
#endif
