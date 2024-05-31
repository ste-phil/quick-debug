#ifndef QUICKDEBUG_HPP
#define QUICKDEBUG_HPP

#include <string>
#include <thread>
#include <mutex>
#include <chrono>

#include "Common.hpp"
#include "WebSocketServer.hpp"
#include "Content/index.html.h"
#include "httplib.h"

struct QuickDebugConfig {
	bool UseWebserver = false;
};

struct QuickDebugData {
	FixedString<32> graph;
	float value;
};

struct QuickDebug {
	static const int WEBSERVER_PORT = 80;
	static const int WEBSOCKETSERVER_PORT = 8126;

	static inline void Plot(const std::string& graph, float value) {
		std::lock_guard<std::mutex> lock(counter_mutex);

		if (!IsRunning())
			Startup();

		// Send message to clients
		m_server.BroadcastMessage(graph + ";" + std::to_string(value));
	}

	/// @brief This has to be called before the first QuickDebug::Plot() is called, to be able to take effect.
	/// @param cfg
	static inline void Configure(QuickDebugConfig cfg) {
		m_cfg = cfg;
	}

	static inline void Startup() {
		QuickDebugConfig cfg = {};
		Startup(cfg);
	}

	static inline void Startup(const QuickDebugConfig& cfg) {
		m_cfg = cfg;

		m_server.Start(WEBSOCKETSERVER_PORT);
		m_messageQueue = std::queue<QuickDebugData>();
		m_publishPlotMessageThread = std::thread([&]() {
			while (m_server.IsRunning())
			{
				{
					std::lock_guard<std::mutex> lock(counter_mutex);
					for (int i = 0; i < m_messageQueue.size(); i++) {
						auto data = m_messageQueue.front();
						m_messageQueue.pop();

						std::string graphMsg = data.graph;
						std::string msg = graphMsg + ";" + std::to_string(data.value);
						m_server.BroadcastMessage(msg);
					}
				}

				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}
		});
		m_publishPlotMessageThread.detach();

		if (m_cfg.UseWebserver) {
			m_webServerThread = std::thread([]() {
				InitWebServer();
			});
			m_webServerThread.detach();
		}
	}

	//static inline void Shutdown() {
	//	if (IsRunning()) {
	//		m_server.Stop();
	//	}
	//}

private:
	static inline bool IsRunning() {
		return m_server.IsRunning();
	}

	static inline void InitWebServer() {
		httplib::Server svr;

		svr.Get("/", [](const httplib::Request&, httplib::Response& res) {
			res.set_content(Content::index_html, "text/html");
		});

		svr.listen("0.0.0.0", 80);
	}

	static inline std::queue<QuickDebugData> m_messageQueue;
	static inline std::thread m_publishPlotMessageThread;

	static inline std::thread m_webServerThread;
	static inline QuickDebugConfig m_cfg;
	static inline std::mutex counter_mutex;

	static inline Ext::WebSocketServer m_server;
};

#endif
