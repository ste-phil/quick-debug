#ifndef QUICKDEBUG_HPP
#define QUICKDEBUG_HPP

#include <string>
#include <thread>
#include <chrono>
#include <future>

#include "Common.hpp"
#include "WebSocketServer.hpp"
#include "Content/index.html.h"
#include "httplib.h"
#include "Entities.hpp"


struct QuickDebug {
	static const int WEBSERVER_PORT = 80;

	/// @brief Directly sends a value to all connected clients (Use Plot() for minimal overhead and maximal performance)
	/// @param graph In which line of the chart the value should be plotted
	/// @param value The value that should be plotted
	static inline void PlotAndSend(const std::string& graph, float value) {
		Startup();

		// Send message to clients
		m_server.BroadcastMessage(TransmissionMsg::CreatePlotMessage(graph.c_str(), value).message);
	}

	/// @brief Enqueues the transmission of a value to all connected clients. The message will then be sent by a worker thread when it is available.
	/// @param graph In which line of the chart the value should be plotted
	/// @param value The value that should be plotted
	static inline void Plot(const std::string& graph, float value) {
		Plot(graph.c_str(), value);
	}

	/// @brief Enqueues the transmission of a value to all connected clients. The message will then be sent by a worker thread when it is available.
	/// @param graph In which line of the chart the value should be plotted
	/// @param value The value that should be plotted
	static inline void Plot(const char* graph, float value) {
		Startup();

		m_messageQueue.Push(TransmissionMsg::CreatePlotMessage(graph, value));
	}

	/// @brief Registers a key to be updated when a message with the key is received
	/// @param key Make sure to pass a persistent pointer into key, since it will not be copied.
	/// @param value
	static inline void RegisterRecvKey(const char* key, const f32& value)
	{
		RecvMessageConfig config(1, &value);
		m_recvMessageConfigs[key] = config;
	}

	/// @brief Registers a key to be updated when a message with the key is received
	/// @param key Make sure to pass a persistent pointer into key, since it will not be copied.
	/// @param value
	static inline void RegisterRecvKey(const char* key, const i32& value)
	{
		RecvMessageConfig config(2, &value);
		m_recvMessageConfigs[key] = config;
	}

	/// @brief Registers a key to be updated when a message with the key is received
	/// @param key Make sure to pass a persistent pointer into key, since it will not be copied.
	/// @param value
	static inline void RegisterRecvKey(const char* key, const FixedString<32>& value)
	{
		RecvMessageConfig config(3, &value);
		m_recvMessageConfigs[key] = config;
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

		m_server.SetMessageHandler(OnMessageReceived);
		m_server.SetClientConnectedHandler(OnClientConnected);
		m_server.Start(m_cfg.WebsocketPort);
		m_publishPlotMessageThread = std::thread([&]() {
			while (m_server.IsRunning())
			{
				for (int i = 0; i < m_messageQueue.Size(); i++) {
					auto data = m_messageQueue.Pop();
					m_server.BroadcastMessage(data.message);
				}
			}
		});
		m_publishPlotMessageThread.detach();

		if (m_cfg.UseWebserver)
		{
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
	static inline void OnClientConnected(SOCKET s)
	{
		m_messageQueue.Push(TransmissionMsg::CreateConfigurationVariableMessage(m_recvMessageConfigs));
		std::cout << "[QuickDebug] OnClientConnected: " << s << "\n";
		
	}
	
	static inline void OnMessageReceived(SOCKET s, const std::string& msg) {
		auto key = msg.substr(0, msg.find(';'));
		auto value = msg.substr(msg.find(';') + 1);

		auto it = m_recvMessageConfigs.find(key);
		if (it == m_recvMessageConfigs.end())
			return;

		auto config = it->second;
		switch (config.targetValueType)
		{
		case 1:
		{
			auto* targetValue = (float*)config.targetValue;
			*targetValue = std::stof(value); 
			break;
		}
		case 2:
		{
			auto* targetValue = (i32*)config.targetValue;
			*targetValue = std::stoi(value); 
			break;
		}
		case 3:
		{
			auto* targetValue = (FixedString<32>*)(config.targetValue);
			*targetValue = FixedString<32>(value.c_str());
			break;
		}
		default:
			return;
		}
		
	}
	
	static inline bool IsRunning() {
		return m_server.IsRunning();
	}

	static inline void InitWebServer() {
		m_webserver.Get("/", [](const httplib::Request&, httplib::Response& res) {
			res.set_content(Content::index_html, "text/html");
		});

		m_webserver.listen("0.0.0.0", 80);
	}

	static inline ConcurrentQueue<TransmissionMsg> m_messageQueue;
	static inline std::thread m_publishPlotMessageThread;

	static inline std::thread m_webServerThread;
	static inline httplib::Server m_webserver;
	
	static inline QuickDebugConfig m_cfg;
	static inline Ext::WebSocketServer m_server;
	static inline std::map<std::string, RecvMessageConfig> m_recvMessageConfigs; 
};

#endif
