#define NOMINMAX
#include <WinSock2.h>
#include <windows.h>
#include <synchapi.h>

#include <iostream>
#include <thread>


#include "Libs/QuickDebug/QuickDebug.hpp"
#include "Libs/QuickDebug/LatencyMonitor.hpp"
#include "Libs/QuickDebug/Common/Analysis.h"

void TestAnalysis() {
	using namespace QD;
	{
		Timer x1("Group", true);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		Timer x2("Group>Subgroup", true);
		std::this_thread::sleep_for(std::chrono::milliseconds(500));

		{
			Timer x3("Group>Subgroup>A", true);
			std::this_thread::sleep_for(std::chrono::milliseconds(50));

		}

		{
			Timer x3("Group>Subgroup>B", true);
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}

	GroupTimer::PrintTracked();
}

int main()
{
	TestAnalysis();
	return 0;


	QD::QuickDebug::Startup({
		.UseWebserver = false
	});


	f32 amplitude = 1;
	FixedString<32> text = "Default text";
	i32 offset = 0;
	QD::QuickDebug::RegisterRecvKey("Amplitude", amplitude);
	QD::QuickDebug::RegisterRecvKey("Text", text);
	QD::QuickDebug::RegisterRecvKey("Offset", offset);
 
	std::thread([&]() {
		const int MSG_DELAY_MS = 16;
		i32 i = 0;
		while (true) {
			auto val = std::sin((offset + i++) * MSG_DELAY_MS * 0.005);
			// std::cout << "Sending message: " << val << std::endl;
			{
				// Timer("sin");
				QD::QuickDebug::Plot("sin", val);

				if (i % 4 == 0)
					QD::LatencyMonitor::TakeTimestampMeasurement(i, "sin");
				auto elapsed = QD::LatencyMonitor::MeasureTimeDelta(i, "sin");
				QD::LatencyMonitor::SendTimestampMeasurement(i, false);
				QD::QuickDebug::Plot("sin dt (ms)", elapsed.count() / 1000);
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(MSG_DELAY_MS));
		}
	}).detach();
	
	std::thread([&]() {
		const int MSG_DELAY_MS = 16;
		i32 i = 0;
		while (true) {
			auto val = amplitude * std::cos((i++) * MSG_DELAY_MS * 0.005);
			// std::cout << "Sending message: " << val << std::endl;
			{
				// Timer("cos");
				QD::LatencyMonitor::TakeTimestampMeasurement(i, "cos");
				QD::QuickDebug::Plot("cos", val);
			}

			// std::cout << text << std::endl;
			Sleep(MSG_DELAY_MS);
		}

	}).detach();
	
	// std::thread([&]() {
	// 	const int MSG_DELAY_MS = 16;
	// 	i32 i = 0;
	// 	while (true) {
	// 		auto val = 2 * std::sin((offset + i++) * MSG_DELAY_MS * 0.005);
	// 		// std::cout << "Sending message: " << val << std::endl;
	// 		{
	// 			Timer("acos");
	// 			QuickDebug::Plot("2sin", val);
	// 		}
	// 		Sleep(MSG_DELAY_MS);
	// 	}
	// }).detach();

	auto x = std::cin.get();
}


