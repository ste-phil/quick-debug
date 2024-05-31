#include <iostream>
#include <thread>

#include "Libs/QuickDebug.hpp"

int main()
{
	QuickDebug::Configure({
		.UseWebServer = false;
	})
	
	std::thread([]() {
		const int MSG_DELAY_MS = 16;
		float i = 0;
		while (true) {
			auto val = std::sin(i++ * MSG_DELAY_MS * 0.005);
			std::cout << "Sending message: " << val << std::endl;
			QuickDebug::Plot("test", val);
			Sleep(MSG_DELAY_MS);
		}
	}).detach();

	auto x = std::cin.get();
}
