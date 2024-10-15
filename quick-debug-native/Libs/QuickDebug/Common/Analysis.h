#pragma once
#include <chrono>
#include <iostream>
#include <map>
#include <iomanip>
#include <string>
#include <string_view>
#include <cassert>
#include "Dbg.hpp"
#include "StrFormat.hpp"

//static char* CreateOutputString(char const* msg) {
//	size_t needed = snprintf(NULL, 0, "%s: %s (%d)", msg, strerror(errno), errno) + 1;
//	char* buffer = new char[needed];
//	sprintf(buffer, "%s: %s (%d)", msg, strerror(errno), errno);
//	return buffer;
//}

namespace QD {

	class GroupTimer {
	public:
		struct Group {
			std::map<std::string, Group> subGroups;
			long long time;

			Group() : time(0) {}

			Group(long long timeS) : time(timeS)
			{

			}
		};

	private:

		inline static Group baseGroup;

		GroupTimer() = delete;

	public:
		static void RegisterTimer(const char* groupHierarchy)
		{
			auto gr = std::string_view(groupHierarchy);
			auto* group = FindOrCreateGroup(&baseGroup, gr);
		}

		static void AppendTime(const char* groupHierarchy, long long time)
		{
			auto gr = std::string_view(groupHierarchy);
			auto* group = FindOrCreateGroup(&baseGroup, gr);

			group->time += time;
		}

		static void PrintTracked(bool clearTimers = true)
		{
			DEBUG_PRINT_NOARGS("Timers\r\n");
			DEBUG_PRINT_NOARGS("------------------------------------------------\r\n");

			auto sum = PrintGroup(baseGroup);

			DEBUG_PRINT_NOARGS("------------------------------------------------\r\n");
			auto x = StrFormat::PadR("Overall: ", 20);
			auto y = StrFormat::PadR(std::to_string(sum), 10);
			auto z = StrFormat::PadR(std::to_string(sum / 1000.f), 20);
			DEBUG_PRINT(PRINT_STRING PRINT_STRING" us " PRINT_STRING " ms \r\n", x.c_str(), y.c_str(), z.c_str());

			if (clearTimers)
				baseGroup = Group();
		}



		static Group GetAndReset()
		{
			auto temp = baseGroup;
			baseGroup = Group();
			return temp;
		}

		static std::map<std::string, long long> Average(Group groups[], int groupCount)
		{
			std::map<std::string, long long> resMap;
			for (size_t i = 0; i < groupCount; i++)
			{
				std::map<std::string, long long> map;
				Flatten(groups[i], map);

				for (auto it = map.begin(); it != map.end(); ++it)
				{
					resMap[it->first] += it->second;
				}
			}

			for (auto it = resMap.begin(); it != resMap.end(); ++it)
			{
				it->second /= groupCount;
			}

			return resMap;
		}

	private:
		constexpr static Group* FindOrCreateGroup(Group* group, std::string_view& name) {
			auto pos = name.find('>');
			if (pos == std::string::npos) {
				auto grName = std::string(name.data(), name.size());
				auto& map = group->subGroups;
				return &map[grName];
			}

			std::string_view groupName = name.substr(0, pos);
			std::string_view nextGroupName = name.substr(pos + 1);

			//DEBUG_PRINT("groupName: %s nextGroupName: %s", groupName.data(), nextGroupName.data());

			auto grName = std::string(groupName.data(), groupName.size());
			auto* nextGroup = &group->subGroups[grName];

			return FindOrCreateGroup(nextGroup, nextGroupName);
		}

		static long long PrintGroup(const Group& group, int indent = 0) {
			long long totalTime = 0;
			for (auto it = group.subGroups.begin(); it != group.subGroups.end(); ++it) {
				auto& name = it->first;
				auto& nextGroup = it->second;

				std::string padding;
				for (size_t j = 0; j < indent; j++)
				{
					padding += " -> ";
				}

				DEBUG_PRINT(PRINT_STRING ":" PRINT_STRING "us " PRINT_STRING " ms \r\n",
					StrFormat::PadL(padding + std::string(name), 20).c_str(),
					StrFormat::PadR(std::to_string(nextGroup.time), 10).c_str(),
					StrFormat::PadR(std::to_string(nextGroup.time / 1000.f), 20).c_str()
				);

				totalTime += nextGroup.time;
				PrintGroup(nextGroup, indent + 1);
			}

			return totalTime;
		}

		static void Flatten(const Group& group, std::map<std::string, long long>& outmap) {

			for (auto it = group.subGroups.begin(); it != group.subGroups.end(); ++it) {
				auto& name = it->first;
				auto& nextGroup = it->second;

				if (nextGroup.subGroups.size() == 0)
					outmap[name] = nextGroup.time;
				else
				{
					outmap[name] = nextGroup.time;
					Flatten(nextGroup, outmap);
				}
			}

		}

		struct separate_thousands : std::numpunct<char> {
			char_type do_thousands_sep() const override { return ' '; }  // separate with commas
			string_type do_grouping() const override { return "\3"; } // groups of 3 digit
		};
	};

	/*
	 * Usage:
	 * IntervalTimer timer("Timer");
	 * timer.Measure();
	 *
	 * Prints:
	 *   Timer: 1000 us (1 ms)
	 *
	 * Call Measure() to print the time since the last Measure() call.
	 */
	class IntervalTimer {
	public:
		IntervalTimer(const char* text) : _text(text) {
			_intervalTime = std::chrono::high_resolution_clock::now();
		}

		void Measure() {
			auto now = std::chrono::high_resolution_clock::now();

			auto start = std::chrono::time_point_cast<std::chrono::microseconds>(_intervalTime).time_since_epoch();
			auto end = std::chrono::time_point_cast<std::chrono::microseconds>(now).time_since_epoch();

			auto duration = end - start;
			auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration);

			DEBUG_PRINT(PRINT_STRING ":" PRINT_INT64 " us (" PRINT_INT64 " ms)  \n",
				_text,
				duration.count(),
				ms.count()
			);

			_intervalTime = now;
		}

		std::chrono::time_point<std::chrono::high_resolution_clock> _intervalTime;
		const char* _text;
	};

	/*
	 * Scoped Timer for measuring time between construction and destruction of the Timer Object
	 *
	 * Usage:
	 * {
	 *     Timer timer("Timer");
	 *     // Code to measure
	 * }
	 *
	 * Prints:
	 *   Timer: 1000 us (1 ms)
	 *
	 * Can also be used for measuring time for a group of timers:
	 *
	 *
	 * {
	 *     Timer timer("Group", true);
	 *	   // Code to measure
	 *     {
	 *			Timer timer("Group>SubGroup", true);
	 *			// Code to measure
	 *	   }
	 * }
	 *
	 * Prints:
	 *   Group: 1000 us (1 ms)
	 *   -> SubGroup: 500 us (0.5 ms)
	 *
	 *
	 * Print the group timers using GroupTimer::PrintTracked();
	 */
	class Timer {
	public:
		Timer(const char* text, bool useForGroup = false) : _text(text), _useForGroup(useForGroup) {
			_startTime = std::chrono::high_resolution_clock::now();
			if (useForGroup)
				GroupTimer::RegisterTimer(text);
		}

		Timer(const std::string& text, bool useForGroup = false) : _text(text.c_str()), _useForGroup(useForGroup) {
			_startTime = std::chrono::high_resolution_clock::now();
			if (useForGroup)
				GroupTimer::RegisterTimer(_text);
		}

		Timer() : _text("Timer"), _useForGroup(false) {
			_startTime = std::chrono::high_resolution_clock::now();
		}

		~Timer() {
			Stop();
		}

		void Stop() {
			auto endTime = std::chrono::high_resolution_clock::now();

			auto start = std::chrono::time_point_cast<std::chrono::microseconds>(_startTime).time_since_epoch();
			auto end = std::chrono::time_point_cast<std::chrono::microseconds>(endTime).time_since_epoch();

			auto duration = end - start;
			auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration);

			if (_useForGroup) {
				GroupTimer::AppendTime(_text, duration.count());
			}
			else {
				DEBUG_PRINT(PRINT_STRING ":" PRINT_INT64 " us (" PRINT_INT64 " ms)  \n",
					_text,
					duration.count(),
					ms.count()
				);
			}
		}

	private:
		std::chrono::time_point<std::chrono::high_resolution_clock> _startTime;
		const char* _text;
		bool _useForGroup;
	};

	class MemoryTracker {
	private:
		MemoryTracker() {}
	public:
		inline static std::map<std::string, uint64_t> store;


		static void Snapshot(const std::string key, uint64_t value) {
			if (value > store[key])
				store[key] = value;
		}

		static void Print() {
			std::cout << "------------------------------------------------" << std::endl;
			for (auto it = store.begin(); it != store.end(); ++it) {
				std::cout << std::setw(20) << it->first << ": " << std::setw(10) << it->second << " B " << std::endl;
			}
		}

		static void Reset()
		{
			store.clear();
		}
	};

}