#pragma once

#include <string>
#include <thread>

#include "WebSocketServer.hpp"
#include "Content/index.html.h"

struct RemoteDebug {
	static const int WEBSERVER_PORT = 80;
	static const int WEBSOCKETSERVER_PORT = 8126;

	static inline void Plot(const std::string& graph, float value) {
		if (!IsRunning())
			Startup();

		// Send message to clients
		m_server.BroadcastMessage(graph + ";" + std::to_string(value));
	}

	//static inline void Shutdown() {
	//	if (IsRunning()) {
	//		m_server.Stop();
	//	}
	//}

private:
	static bool IsRunning() {
		return m_server.IsRunning();
	}

	static void Startup() {
		m_websocketServerThread = std::thread([]() {
			m_server.SetMessageHandler([](SOCKET client_socket, const std::string& message) {
				std::cout << "Received message from client " << client_socket << ": " << message << std::endl;
				// Echo back the message
				m_server.SendMessage(client_socket, message);
			});

			m_server.Start(WEBSOCKETSERVER_PORT);
		});

		m_webServerThread = std::thread([]() {
			InitWebServer();
		});

		m_websocketServerThread.detach();
		m_webServerThread.detach();
	}

	static void InitWebServer() {
		httplib::Server svr;

		svr.Get("/", [](const httplib::Request&, httplib::Response& res) {
			res.set_content(Content::index_html, "text/html");
		});

		svr.listen("0.0.0.0", 80);
	}

	static inline std::thread m_websocketServerThread;
	static inline std::thread m_webServerThread;	

	static inline Ext::WebSocketServer m_server;
};
