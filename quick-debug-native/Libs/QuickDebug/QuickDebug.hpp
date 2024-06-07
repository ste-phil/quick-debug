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
	const char* graph;
	const float value;

	QuickDebugData(const char* g, const float v) :
		graph(g),
		value(v)
	{

	}
};

struct QuickDebug {
	static const int WEBSERVER_PORT = 80;
	static const int WEBSOCKETSERVER_PORT = 8126;

	static inline void PlotAndSend(const std::string& graph, float value) {
		Startup();

		// Send message to clients
		m_server.BroadcastMessage(graph + ";" + std::to_string(value));
	}

	static inline void Plot(const std::string& graph, float value) {
		Plot(graph.c_str(), value);
	}

	static inline void Plot(const char* graph, float value) {
		Startup();

		QuickDebugData data(graph, value);
		m_messageQueue.Push(data);
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
		if (IsRunning())
			return;

		m_cfg = cfg;

		m_server.Start(WEBSOCKETSERVER_PORT);
		m_publishPlotMessageThread = std::thread([&]() {
			while (m_server.IsRunning())
			{
				for (int i = 0; i < m_messageQueue.Size(); i++) {
					auto data = m_messageQueue.Pop();

					std::string msg = std::string(data.graph) + ";" + std::to_string(data.value);
					m_server.BroadcastMessage(msg);
				}
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

	static inline void Shutdown() {
		if (IsRunning()) {
			m_server.Stop();

			m_publishPlotMessageThread.join();
			m_webserver.stop();
			m_webServerThread.join();
		}
	}

private:
	static inline bool IsRunning() {
		return m_server.IsRunning();
	}

	static inline void InitWebServer() {
		m_webserver.Get("/", [](const httplib::Request&, httplib::Response& res) {
			res.set_content(Content::index_html, "text/html");
			});

		m_webserver.listen("0.0.0.0", 80);
	}

	static inline ConcurrentQueue<QuickDebugData> m_messageQueue;
	static inline std::thread m_publishPlotMessageThread;

	static inline std::thread m_webServerThread;

	static inline httplib::Server m_webserver;
	static inline QuickDebugConfig m_cfg;

	static inline Ext::WebSocketServer m_server;
};

#endif
