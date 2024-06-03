#pragma once
#include <chrono>
#include <iostream>
#include <map>
#include <iomanip>
#include <string>
#include <string_view>
#include <cassert>

#define DEBUG_PRINT_NOARGS(x) printf(x)
#define DEBUG_PRINT(x, ...) printf(x, __VA_ARGS__)
#define PRINT_STRING "%s"
#define PRINT_INT64 "%d"

static char* CreateOutputString(char const* msg) {
	size_t needed = snprintf(NULL, 0, "%s: %s (%d)", msg, strerror(errno), errno) + 1;
	char* buffer = new char[needed];
	sprintf(buffer, "%s: %s (%d)", msg, strerror(errno), errno);
	return buffer;
}

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
	static void AppendTime(const char* groupHierarchy, long long time)
	{
		auto gr = std::string_view(groupHierarchy);
		auto* group = FindOrCreateGroup(&baseGroup, gr);

		group->time += time;

		return;
	}

	static void PrintTracked(bool includeSummary = true)
	{
		DEBUG_PRINT_NOARGS("Timers");
		DEBUG_PRINT_NOARGS("------------------------------------------------");

		auto sum = PrintGroup(baseGroup);

		DEBUG_PRINT_NOARGS("------------------------------------------------");
		auto x = StrPadR("Overall: ", 20);
		auto y = StrPadR(std::to_string(sum), 10);
		auto z = StrPadR(std::to_string(sum / 1000.f), 20);
		DEBUG_PRINT(PRINT_STRING PRINT_STRING" us "PRINT_STRING" ms \n", x, y, z);
	}

	static std::string StrPadR(const std::string& text, const int padding)
	{
		assert(text.length() <= padding);
		auto insertPadding = padding - text.length();

		std::string res;
		res.resize(padding);

		for (size_t i = 0; i < insertPadding; i++) {
			res[i] = ' ';
		}
		for (size_t i = insertPadding; i < padding; i++) {
			res[i] = text[i - insertPadding];
		}

		return res;
	}

	static std::string StrPadL(const std::string& text, const int padding)
	{
		assert(text.length() <= padding);

		std::string res;
		res.resize(padding);

		for (size_t i = 0; i < text.length(); i++) {
			res[i] = text[i];
		}
		for (size_t i = text.length(); i < padding; i++) {
			res[i] = ' ';
		}

		return res;
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
	static Group* FindOrCreateGroup(Group* group, std::string_view& name) {
		auto pos = name.find('>');
		if (pos == std::string::npos) {
			auto grName = std::string(name.data(), name.size());
			auto& map = group->subGroups;
			return &map[grName];
		}

		auto groupName = name.substr(0, pos);
		auto nextGroupName = name.substr(pos + 1);

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

			/*auto thousands = std::make_unique<separate_thousands>();
			std::cout.imbue(std::locale(std::cout.getloc(), thousands.release()));

			std::cout <<
				std::left << std::setw(20) << (padding + std::string(name)) << ": " <<
				std::right << std::setw(10) << nextGroup.time << " us " <<
				std::setw(10) << std::fixed << std::setprecision(0) << (nextGroup.time / 1000.f) << " ms" <<
			std::endl;*/

			DEBUG_PRINT(PRINT_STRING":"PRINT_STRING "us "PRINT_STRING" ms",
				StrPadL(padding + std::string(name), 20),
				StrPadR(std::to_string(nextGroup.time), 10),
				StrPadR(std::to_string(nextGroup.time / 1000.f), 20)
			);

			totalTime += nextGroup.time;
			totalTime += PrintGroup(nextGroup, indent + 1);
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

class IntervalTimer {
public:
    IntervalTimer(const char* text): _text(text) {
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

class Timer {
public:
	Timer(const char* text, bool useForGroup = false) : _text(text), _useForGroup(useForGroup) {
		_startTime = std::chrono::high_resolution_clock::now();
	}

	Timer(const std::string& text, bool useForGroup = false) : _text(text.c_str()), _useForGroup(useForGroup) {
		_startTime = std::chrono::high_resolution_clock::now();
	}

	Timer() : _text(), _useForGroup(false) {
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
			DEBUG_PRINT(PRINT_STRING":"PRINT_INT64" us ("PRINT_INT64" ms)  \n",
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
