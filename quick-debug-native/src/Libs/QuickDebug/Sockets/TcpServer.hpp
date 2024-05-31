#pragma once

#include "Tcp.hpp"

namespace Ext::Sockets {
    Tcp TcpAccept(const char* host, short port) {
        Tcp socket(host, port);

        // Listen for a connection, exiting on failure
        socket.Listen();

        return socket;
    }
}
