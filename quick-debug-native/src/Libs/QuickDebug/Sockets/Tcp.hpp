#pragma once

#include "Ext/Common/Fmt.hpp"
#include "Ext/Sockets/SocketCompat.hpp"

namespace Ext::Sockets {
#ifndef _WIN32
    static void closesocket(int socket) { close(socket); }
#endif

    class Tcp : public Internal::SocketCompat {
    public:
        Tcp(const char* host, const short port)
        {
            sprintf_s(_host, "%s", host);
            sprintf_s(_port, "%d", port);

            // No connection yet
            _sock = INVALID_SOCKET;
            _connected = false;
            *_message = 0;

            // Initialize Winsock, returning on failure
            if (!InitWinsock()) return;

            // Set up client address info
            struct addrinfo hints = { 0 };
            hints.ai_family = AF_INET;
            hints.ai_socktype = SOCK_STREAM;

            // Resolve the server address and port, returning on failure
            _addressInfo = NULL;
            int iResult = getaddrinfo(_host, _port, &hints, &_addressInfo);
            if (iResult != 0) {
                sprintf_s(_message, "getaddrinfo() failed with error: %d", iResult);
                Cleanup();
                return;
            }

            // Create a SOCKET for connecting to server, returning on failure
            _sock = socket(_addressInfo->ai_family, _addressInfo->ai_socktype, _addressInfo->ai_protocol);
            if (_sock == INVALID_SOCKET) {
                sprintf_s(_message, "socket() failed");
                Cleanup();
                return;
            }
        }

        void Listen() {
            if (listen(_sock, 1) == -1) {
                sprintf_s(_message, "listen() failed");
                return;
            }

            // Accept connection, exiting on failure
            printf("Waiting for client to connect on %s:%s ... ", _host, _port);
            fflush(stdout);
            _conn = accept(_sock, (struct sockaddr*)NULL, NULL);
            if (_conn == SOCKET_ERROR) {
                sprintf_s(_message, "accept() failed");
                return;
            }
            printf("connected\n");
            fflush(stdout);
        }

        bool IsDataAvailable() {
            u_long availableBytes;
            if (ioctlsocket(_conn, FIONREAD, &availableBytes) != 0) {
                int error = WSAGetLastError();
                closesocket(_conn);
                throw "Error getting available bytes";
            }
        }

        bool SendData(void* buf, size_t len)
        {
            return (size_t)send(_conn, (const char*)buf, len, 0) == len;
        }

        bool ReceiveData(void* buf, size_t len)
        {
            return (size_t)recv(_conn, (char*)buf, len, 0) == len;
        }

        bool IsConnected()
        {
            return _connected;
        }

        void Bind() {
            if (bind(_sock, _addressInfo->ai_addr, (int)_addressInfo->ai_addrlen) == SOCKET_ERROR) {
                closesocket(_sock);
                _sock = INVALID_SOCKET;
                sprintf_s(_message, "bind() failed");
                return;
            }
        }

    protected:
        char _host[200];
        char _port[10];

        SOCKET _conn = NULL;
        struct addrinfo* _addressInfo = NULL;
        bool _connected;
    };
}