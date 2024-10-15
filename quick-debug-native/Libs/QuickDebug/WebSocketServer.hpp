#ifndef WEBSOCKET_SERVER_HPP
#define WEBSOCKET_SERVER_HPP


#include <iostream>
#include <thread>
#include <atomic>
#include <string>
#include <functional>
#include "Hash/sha1.h"
#include "Common.hpp"


#ifdef _WIN32
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket close
#define SD_BOTH SHUT_RDWR
#endif




namespace Ext {
    class WebSocketServer {
        struct ClientConnection {
            SOCKET Socket;
            std::thread Thread;

            //This is set to true when the WebSocket protocol has established the connection
            //This is not the same as the raw socket establishment
            bool IsWebsocketConnectionEstablished;
        };

    public:
        explicit WebSocketServer() : m_port(0), m_isRunning(false) {
#ifdef _WIN32
            WSADATA wsaData;
            int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
            if (result != 0) {
                throw "WSA Startup failed, Error: " + std::to_string(result);
            }
#endif
        }

        ~WebSocketServer() {
            if (m_isRunning)
                Stop();
#ifdef _WIN32
            WSACleanup();
#endif
        }

        void Start(int port) {
            if (m_isRunning)
                throw "Server is already running!";

            m_port = port;
            m_isRunning = true;
            m_serverThread = std::thread(&WebSocketServer::ServerLoop, this);
            m_serverThread.detach();
        }

        void Stop() {
            if (!m_isRunning)
                throw "Server is not running!";

            m_isRunning = false;
            if (m_serverThread.joinable())
                m_serverThread.join();
        }

        bool IsRunning() const {
			return m_isRunning;
		}

        void SetMessageHandler(std::function<void(SOCKET, const std::string&)> handler) {
            m_messageHandler = std::move(handler);
        }

        void SetClientConnectedHandler(std::function<void(SOCKET)> handler) {
            m_onClientConnectedHandler = std::move(handler);
        }

        void SendWebMessage(SOCKET clientSocket, const std::string& message) {
            struct ControlData {
			    unsigned char opcode            : 4;
			    unsigned char rsv3              : 1;
			    unsigned char rsv2              : 1;
                unsigned char rsv1              : 1;
                unsigned char fin               : 1;

			    unsigned char payloadLength     : 7;
			    unsigned char mask              : 1;
		    } data;

            data.opcode = 1; // Text message
            data.fin = 1;    // Has all the data been sent in this frame?
            data.rsv1 = 0;
            data.rsv2 = 0;
            data.rsv3 = 0;

            data.mask = 0;
            if (message.size() < 126)
			    data.payloadLength = message.size();
            else if (message.size() < 65536) {
			    data.payloadLength = 126; //Indicates that the payload length is 16 bits
            }
            else {
			    data.payloadLength = 127; //Indicates that the payload length is 64 bits
            }

            SendRaw(clientSocket, (const char*) &data, sizeof(data));
		    if (data.payloadLength == 126) {
			    uint16_t length = static_cast<uint16_t>(message.size());
			    SendRaw(clientSocket, (const char*)&length, sizeof(length));
		    }
		    else if (data.payloadLength == 127) {
			    uint64_t length = message.size();
			    SendRaw(clientSocket, (const char*)&length, sizeof(length));
		    }

            SendRaw(clientSocket, message);
	    }

        void BroadcastMessage(const std::string& message) {
            for (auto& client : m_activeClients) {
                if (client.IsWebsocketConnectionEstablished)
                    SendWebMessage(client.Socket, message);
			}
        }


    private:
        void SendRaw(SOCKET client_socket, const std::string& message) {
             send(client_socket, message.c_str(), strlen(message.c_str()), 0);
        }

        void SendRaw(SOCKET client_socket, const std::vector<char>& data) {
            send(client_socket, data.data(), static_cast<int>(data.size()), 0);
        }

        void SendRaw(SOCKET client_socket, const char* data, int length) {
            send(client_socket, data, length, 0);
        }

        int GetLastErrror() {
#ifdef _WIN32
            return WSAGetLastError();
#else
            return errno;
#endif
        }

        bool IsError(int errorCode) {
#ifdef _WIN32
            return errorCode == WSAEWOULDBLOCK || errorCode == WSAECONNRESET || errorCode == WSAECONNABORTED;
#else
            return errorCode != 0;
#endif
        }

        void ServerLoop() {
            struct addrinfo hints, * result = nullptr;
            std::memset(&hints, 0, sizeof(hints));
            hints.ai_family = AF_INET; // IPv4
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_protocol = IPPROTO_TCP;
            hints.ai_flags = AI_PASSIVE;

            auto portStr = std::to_string(m_port);
            int result_val = getaddrinfo(nullptr, portStr.c_str(), &hints, &result);
            if (result_val != 0) {
                throw "[WebSocketServer] getaddrinfo failed: " + result_val;
            }

            SOCKET listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
            if (listenSocket == INVALID_SOCKET) {
                freeaddrinfo(result);
                throw "[WebSocketServer] Socket creation failed with error: " + GetLastErrror();
            }

            result_val = bind(listenSocket, result->ai_addr, (int)result->ai_addrlen);
            if (result_val == SOCKET_ERROR) {
                freeaddrinfo(result);
                closesocket(listenSocket);
                throw "[WebSocketServer] Socket bind failed with error: " + GetLastErrror();
            }

            freeaddrinfo(result);

            result_val = listen(listenSocket, 3);
            if (result_val == SOCKET_ERROR) {
                throw "[WebSocketServer] Socket listen failed with error: " + GetLastErrror();
                closesocket(listenSocket);
                return;
            }
            DEBUG_PRINT("[WebSocketServer] Listening on port " PRINT_INT64 PRINT_STRING, m_port, "\r\n");

            while (m_isRunning) {
                fd_set readfds;
                FD_ZERO(&readfds);
                FD_SET(listenSocket, &readfds);

                timeval timeout;
                timeout.tv_sec = 1;
                timeout.tv_usec = 0;

#ifdef _WIN32
                const int maxFd = 0;
#else
                const int maxFd = listenSocket + 1;
#endif

                result_val = select(maxFd, &readfds, nullptr, nullptr, &timeout);
                if (result_val > 0 && FD_ISSET(listenSocket, &readfds)) {
                    SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);
                    if (clientSocket == INVALID_SOCKET) {
                        closesocket(listenSocket);
                        throw "[WebSocketServer] Incoming connection accept failed: " + std::to_string(GetLastErrror());
                    }
                    DEBUG_PRINT("[WebSocketServer] Client connected on port %d\r\n", m_port);

                    auto& client = m_activeClients.emplace_back();
                    client.Socket = clientSocket;
                    client.IsWebsocketConnectionEstablished = false;

                    client.Thread = std::thread(&WebSocketServer::ClientLoop, this, &client);
                    client.Thread.detach(); // Detach the thread, let it run freely
                }
            }

            DEBUG_PRINT_NOARGS("[WebSocketServer] Listening stopped");
            closesocket(listenSocket);
        }

        bool IsSocketConnected(SOCKET socket) {
            char buffer[1];
            int result = recv(socket, buffer, sizeof(buffer), MSG_PEEK);
            if (result == SOCKET_ERROR) {
                int error = GetLastErrror();
                if (IsError(error))
                {
                    DEBUG_PRINT("[WebSocketServer] Socket closing, Error: %d \n", error);
                    return false; // Socket is not connected
                }
            }
            return true; // Socket is connected
        }


        void ClientLoop(ClientConnection* client) {
            char buffer[4096] = { 0 };
            int bytesRead;
            SOCKET clientSocket = client->Socket;

            DEBUG_PRINT("[WebSocketServer] Socket %llu: ClientLoop started. \n", static_cast<ui64>(clientSocket));
            while (m_isRunning) {
                if (!IsSocketConnected(clientSocket)) {
                    DEBUG_PRINT("[WebSocketServer] Socket %llu: Connection closed. \n", static_cast<ui64>(clientSocket));
                    break;
                }
                
                fd_set readfds;
                FD_ZERO(&readfds);
                FD_SET(clientSocket, &readfds);

                timeval timeout;
                timeout.tv_sec = 1;
                timeout.tv_usec = 0;

#ifdef _WIN32
                const int maxFd = 0;
#else
                const int maxFd = clientSocket + 1;
#endif
                auto result_val = select(maxFd, &readfds, nullptr, nullptr, &timeout);
                if (result_val <= 0 || !FD_ISSET(clientSocket, &readfds)) {
                    DEBUG_PRINT("[WebSocketServer] Socket %llu: Timeout", static_cast<ui64>(clientSocket));
                    continue;
                }

                DEBUG_PRINT("[WebSocketServer] Socket %llu: recv", static_cast<ui64>(clientSocket));
                if ((bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0)) <= 0)
                {
                    break;
                }

                if (!client->IsWebsocketConnectionEstablished) {
                    DEBUG_PRINT("[WebSocketServer] Socket %llu: Negotiating connection...\n", static_cast<ui64>(clientSocket));

                    NegotiateConnection(clientSocket, buffer, bytesRead);

                    client->IsWebsocketConnectionEstablished = true;
                    if (m_onClientConnectedHandler)
                        m_onClientConnectedHandler(clientSocket);
                    continue;
                }

                auto mask = (buffer[1] & 0b10000000) != 0;  // must be true, "All messages from the client to the server have this bit set"
                int opcode = buffer[0] & 0b00001111;        // expecting 1 - text message
                int offset = 2;
                uint64_t msglen = buffer[1] & (uint64_t)0b01111111;

                if (msglen == 126) {
                    // bytes are reversed because websocket will print them in Big-Endian, whereas
                    // BitConverter will want them arranged in little-endian on windows
                    char msgLengthReversed[2] = { buffer[3], buffer[2] };

                    msglen = *(uint16_t*)(msgLengthReversed);
                    offset = 4;
                }
                else if (msglen == 127) {
                    // To test the below code, we need to manually buffer larger messages — since the NIC's autobuffering
                    // may be too latency-friendly for this code to run (that is, we may have only some of the bytes in this
                    // websocket frame available through client.Available).
                    char msgLengthReversed[8] = { buffer[9], buffer[8], buffer[7], buffer[6], buffer[5], buffer[4], buffer[3], buffer[2] };
                    msglen = *(uint64_t*)(msgLengthReversed);
                    offset = 10;
                }

                if (msglen == 0) {
                    DEBUG_PRINT("[WebSocketServer] Socket %llu : Empty message received. Ignoring message.", static_cast<ui64>(clientSocket));
                    continue;
                }

                if (!mask) {
                    DEBUG_PRINT("[WebSocketServer] Socket %llu : Mask bit not set. Ignoring message.", static_cast<ui64>(clientSocket));
                    continue;
                }

                char* decoded = new char[msglen+1];
                char masks[4] = { buffer[offset], buffer[offset + 1], buffer[offset + 2], buffer[offset + 3] };
                offset += 4;

                //Decode message
                for (auto i = 0; i < msglen; ++i) {
                    decoded[i] = buffer[offset + i] ^ masks[i % 4];
                }
                decoded[msglen] = '\0';     //C++ string terminator

                std::string message(decoded);
                if (m_messageHandler)
                    m_messageHandler(clientSocket, message);

                delete[] decoded;
            }

            CleanupClientSocket(clientSocket);
        }

        inline void CleanupClientSocket(SOCKET clientSocket) {
            DEBUG_PRINT("[WebSocketServer] Socket %llu: Cleaning up data\n", static_cast<ui64>(clientSocket));

            m_activeClients.erase(std::remove_if(m_activeClients.begin(), m_activeClients.end(), [clientSocket](const ClientConnection& client) {
                return client.Socket == clientSocket;
            }), m_activeClients.end());

            DEBUG_PRINT("[WebSocketServer] Active clients: %llu\n", m_activeClients.size());

            auto result = shutdown(clientSocket, SD_BOTH);
            if (result == SOCKET_ERROR) {
                DEBUG_PRINT("[WebSocketServer] Socket %llu: shutdown failed\n", static_cast<ui64>(clientSocket));
            }

            result = closesocket(clientSocket);
            if (result == SOCKET_ERROR) {
                DEBUG_PRINT("[WebSocketServer] Socket %llu: closesocket failed\n", static_cast<ui64>(clientSocket));
            }
        }

        #pragma region MessageNegotiation
        inline void NegotiateConnection(const SOCKET clientSocket, const char* buffer, const int bufferLength) {
            std::string request(buffer, bufferLength);

            DEBUG_PRINT("[WebSocketServer] Socket %llu: NegotiateConnection: Received data %s\n", static_cast<ui64>(clientSocket), buffer);
            if (request.find("Upgrade: websocket") != std::string::npos) {
                const std::string attributeSec = "Sec-WebSocket-Key: ";
                auto startIdx = request.find(attributeSec) + attributeSec.size();
                auto endIdx = request.find_first_of("\r\n", startIdx);
                auto key = request.substr(startIdx, endIdx - startIdx);

                //https://developer.mozilla.org/en-US/docs/Web/API/WebSockets_API/Writing_WebSocket_server check the RFC
                std::string result;
                ComputeWebSocketSecKey(key, result);

                std::string response = "HTTP/1.1 101 Switching Protocols\r\n";
                response += "Connection: Upgrade\r\n";
                response += "Upgrade: websocket\r\n";
                response += "Sec-WebSocket-Accept: " + result + "\r\n";
                response += "\r\n";

                send(clientSocket, response.c_str(), static_cast<int>(response.size()), 0);
            }
        }

        inline std::string Trim(const std::string& str) {
            size_t first = str.find_first_not_of(' ');
            if (std::string::npos == first)
                return str;
            size_t last = str.find_last_not_of(' ');
            return str.substr(first, (last - first + 1));
        }

        inline std::string EncodeBase64(unsigned char const* bytes_to_encode, unsigned int in_len) {
            static const std::string base64_chars =
                "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                "abcdefghijklmnopqrstuvwxyz"
                "0123456789+/";

            std::string ret;
            int i = 0;
            int j = 0;
            unsigned char char_array_3[3];
            unsigned char char_array_4[4];

            while (in_len--) {
                char_array_3[i++] = *(bytes_to_encode++);
                if (i == 3) {
                    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                    char_array_4[3] = char_array_3[2] & 0x3f;

                    for (i = 0; (i < 4); i++)
                        ret += base64_chars[char_array_4[i]];
                    i = 0;
                }
            }

            if (i) {
                for (j = i; j < 3; j++)
                    char_array_3[j] = '\0';

                char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);

                for (j = 0; (j < i + 1); j++)
                    ret += base64_chars[char_array_4[j]];

                while ((i++ < 3))
                    ret += '=';

            }

            return ret;

        }
    public:
        inline void ComputeWebSocketSecKey(const std::string& secWebSocketKey, std::string& webSocketSecKey) {
            // Concatenate the Sec-WebSocket-Key with the special GUID
            std::string concatKey = secWebSocketKey + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

            // Compute SHA-1 hash
            Chocobo1::SHA1 sha1;
            sha1.toArray();
            sha1.addData(concatKey.c_str(), concatKey.length());
            sha1.finalize();
            auto sha1Hash = sha1.toArray();

            // Encode SHA-1 hash in Base64
            std::string base64Encoded = EncodeBase64((const unsigned char*)(sha1Hash.data()), static_cast<unsigned int>(sha1Hash.size()));
            webSocketSecKey = Trim(base64Encoded);
        }
        #pragma endregion


        int m_port;
        std::atomic<bool> m_isRunning;
        std::thread m_serverThread;
        std::vector<ClientConnection> m_activeClients;

        std::function<void(SOCKET, const std::string&)> m_messageHandler;
        std::function<void(SOCKET)> m_onClientConnectedHandler;
    };
}

#endif // WEBSOCKET_SERVER_HPP
