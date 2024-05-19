#pragma once


// Windows
#ifdef _WIN32
#pragma comment(lib,"ws2_32.lib")
#define WIN32_LEAN_AND_MEAN
#undef TEXT
#include <winsock2.h>
#include <ws2tcpip.h>

// Linux
#else
#define sprintf_s sprintf
typedef int SOCKET;
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
static const int INVALID_SOCKET = -1;
static const int SOCKET_ERROR = -1;
#endif

#include <stdio.h>
#include <cstdint>

namespace Ext::Sockets {
	namespace Internal {
		static class SocketCompat {
		protected:
			bool InitWinsock(void)
			{
#ifdef _WIN32
				WSADATA wsaData;
				int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
				if (iResult != 0) {
					sprintf_s(_message, "WSAStartup() failed with error: %d\n", iResult);
					return false;
				}
#endif
				return true;
			}

			void Cleanup(void)
			{
#ifdef _WIN32
				WSACleanup();
#endif
			}

			void StringIpToBinary(const char* host, struct sockaddr_in& saddr_in)
			{
#ifdef _WIN32
				WCHAR wsz[64];
				swprintf_s(wsz, L"%S", host);
				InetPton(AF_INET, wsz, &(saddr_in.sin_addr.s_addr));
#else
				inet_pton(AF_INET, host, &(saddr_in.sin_addr));
#endif
			}

			void SetUdpTimeout(uint32_t msec)
			{
#ifdef _WIN32
				setsockopt(_sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&msec, sizeof(msec));

#else
				struct timeval timeout;
				timeout.tv_sec = msec / 1000;
				timeout.tv_usec = (msec * 1000) % 1000000;
				setsockopt(_sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
#endif
			}

		public:

			void CloseConnection(void)
			{
#ifdef _WIN32
				closesocket(_sock);
#else
				close(_sock);
#endif
			}

			int GetLastError() {
#ifdef _WIN32
				return WSAGetLastError();
#else
				throw "Last error not implemented for Linux";
#endif
			}

			char* GetErrorMessage(void)
			{
				return _message;
			}

		protected:
			SOCKET _sock;
			char _message[200];
		};
	}
}