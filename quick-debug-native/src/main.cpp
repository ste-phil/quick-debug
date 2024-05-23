#include <iostream>
#include <thread>

#include "Libs/httplib.h"
#include "Libs/Ext/RemoteDebug.hpp"


void InitWebServer() {
	httplib::Server svr;

	svr.Get("/", [](const httplib::Request&, httplib::Response& res) {
		res.set_content(Content::index_html, "text/html");
	});

	svr.listen("0.0.0.0", 80);
}

int main()
{
	std::thread([]() {
		const int MSG_DELAY_MS = 16;
		float i = 0;
		while (true) {
			auto val = std::sin(i++ * MSG_DELAY_MS * 0.005);
			std::cout << "Sending message: " << val << std::endl;
			RemoteDebug::Plot("test", val);
			Sleep(MSG_DELAY_MS);
		}
	}).detach();

	auto x = std::cin.get();
	// InitWebServer();
}
