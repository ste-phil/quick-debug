#ifndef STREAM_LATENCY_MONITOR
#define STREAM_LATENCY_MONITOR

#include <cstdint>
#include <vector>
#include <chrono>
#include <map>
#include <QuickDebug/QuickDebug.hpp>


namespace QD
{
  struct TimestampMeasurement {
    int64_t xrTimestamp;
    std::vector<std::pair<const char*, std::chrono::time_point<std::chrono::steady_clock>>> timestamps;
  };

  struct Measurement {
    int64_t xrTimestamp;
    std::chrono::time_point<std::chrono::steady_clock> startTime;
  };

  class LatencyMonitor {
  public:
    static void TakeTimestampMeasurement(int64_t xrTimestamp, const char* name) {
      std::lock_guard<std::mutex> lock(s_apiMutex);

      auto* timestampMeasurement = &s_registeredTimestampMeasurements[xrTimestamp];
      timestampMeasurement->timestamps.push_back(std::make_pair(name, std::chrono::steady_clock::now()));
    }

    /*
      Measures the time delta between this timestamp and the previous one with the given name.
     */
    static std::chrono::microseconds MeasureTimeDelta(int64_t xrTimestamp, const char* name) {
      auto it = s_registeredTimestampMeasurements.find(xrTimestamp);

      if (it != myMap.end() && it != myMap.begin()) {
        auto latestMeasurement = it->second;
        auto prevMeasurement = --it->second;

        auto measurementIt = std::find_if(latestMeasurement->timestamps.begin(), latestMeasurement->timestamps.end(), [name](const auto& pair) {
          return std::strcmp(pair->first, name) == 0;
        });
        if (measurementIt == latestMeasurement->timestamps.end()) {
          return std::chrono::microseconds(0); // No measurement found using xrTimestamp with the given name
        }
        auto latestTimestamp = measurementIt->second;


        auto measurementPrevIt = std::find_if(prevMeasurement->timestamps.begin(), prevMeasurement->timestamps.end(), [name](const auto& pair) {
          return std::strcmp(pair.first, name) == 0;
        });
        if (measurementIt == latestMeasurement->timestamps.end()) {
          return std::chrono::microseconds(0); // The preceding measurment was found, but the entry with the given name was not found
        }
        auto prevTimestamp = measurementPrevIt->second;

        auto latency = std::chrono::duration_cast<std::chrono::microseconds>(latestTimestamp - prevTimestamp);
        return latency;
      } else {
        return std::chrono::microseconds(0); // No preceding measurement was taken (this is the first measurement)
      }
    }

    static std::chrono::microseconds MeasureElapsedTime(int64_t xrTimestamp, const char* begin, const char* end) {
      std::lock_guard<std::mutex> lock(s_apiMutex);

      auto* timestampMeasurement = &s_registeredTimestampMeasurements[xrTimestamp];

      auto beginIt = std::find_if(timestampMeasurement->timestamps.begin(), timestampMeasurement->timestamps.end(), [begin](const auto& pair) {
        return std::strcmp(pair.first, begin) == 0;
      });

      if (beginIt == timestampMeasurement->timestamps.end()) {
        return std::chrono::microseconds(0);
      }

      auto endIt = std::find_if(timestampMeasurement->timestamps.begin(), timestampMeasurement->timestamps.end(), [end](const auto& pair) {
        return std::strcmp(pair.first, end) == 0;
      });

      if (endIt == timestampMeasurement->timestamps.end()) {
        return std::chrono::microseconds(0);
      }

      auto latency = std::chrono::duration_cast<std::chrono::microseconds>(endIt->second - beginIt->second);
      return latency;
    }

    static void SendTimestampMeasurement(int64_t xrTimestamp) {
      std::lock_guard<std::mutex> lock(s_apiMutex);

      auto* timestampMeasurement = &s_registeredTimestampMeasurements[xrTimestamp];
      for (size_t i = 1; i < timestampMeasurement->timestamps.size(); i++)
      {
        auto& [name, timestamp] = timestampMeasurement->timestamps[i];
        auto& [prevName, prevTimestamp] = timestampMeasurement->timestamps[i-1];
        auto latency = std::chrono::duration_cast<std::chrono::microseconds>(timestamp - prevTimestamp);

        std::string sendName;
        sendName += prevName;
        sendName += "->";
        sendName += name;
        QD::QuickDebug::Plot(sendName, latency.count());
      }

      s_registeredTimestampMeasurements.erase(xrTimestamp);
    }


    static void StartMeasurement(const char* name) {
      s_registeredMeasurements[name] = Measurement{-1, std::chrono::steady_clock::now()};
    }

    static std::chrono::microseconds StopMeasurement(const char* name) {
      auto endTime = std::chrono::steady_clock::now();
      auto measurement = s_registeredMeasurements[name];

      return std::chrono::duration_cast<std::chrono::microseconds>(endTime - measurement.startTime);
    }

  private:
    static inline std::mutex s_apiMutex;
    static inline std::map<const char*, Measurement> s_registeredMeasurements;
    static inline std::map<int64_t, TimestampMeasurement> s_registeredTimestampMeasurements;
  };
}

#endif
