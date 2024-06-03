#define NOMINMAX
#include <WinSock2.h>
#include <windows.h>
#include <synchapi.h>

#include <iostream>
#include <thread>


#include "Libs/QuickDebug/QuickDebug.hpp"
#include "Libs/Analysis.h"

int main()
{
	QuickDebug::Startup({
		.UseWebserver = false
	});
	

	std::thread([]() {
		const int MSG_DELAY_MS = 16;
		float i = 0;
		while (true) {
			auto val = std::sin(i++ * MSG_DELAY_MS * 0.005);
			// std::cout << "Sending message: " << val << std::endl;
			{
				Timer("sin");
				QuickDebug::Plot("sin", val);
			}
			Sleep(MSG_DELAY_MS);
		}
	}).detach();

	std::thread([]() {
		const int MSG_DELAY_MS = 16;
		float i = 0;
		while (true) {
			auto val = std::cos(i++ * MSG_DELAY_MS * 0.005);
			// std::cout << "Sending message: " << val << std::endl;
			{
				Timer("cos");
				QuickDebug::Plot("cos", val);
			}
			Sleep(MSG_DELAY_MS);
		}
	}).detach();

	std::thread([]() {
		const int MSG_DELAY_MS = 16;
		float i = 0;
		while (true) {
			auto val = std::acos(i++ * MSG_DELAY_MS * 0.005);
			// std::cout << "Sending message: " << val << std::endl;
			{
				Timer("acos");
				QuickDebug::Plot("acos", val);
			}
			Sleep(MSG_DELAY_MS);
		}
	}).detach();

	auto x = std::cin.get();
}


