// openciv4: BSD sockets under the Winsock names the engine uses.
#pragma once
#include <windows.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
typedef int SOCKET;
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)
#define SD_BOTH SHUT_RDWR
#define closesocket close
struct WSADATA {};
#define MAKEWORD(a, b) 0
inline int WSAStartup(int, WSADATA*) { return 0; }
