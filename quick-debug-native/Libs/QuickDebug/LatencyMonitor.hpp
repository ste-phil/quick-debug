#ifndef STREAM_LATENCY_MONITOR
#define STREAM_LATENCY_MONITOR

#include <cstdint>
#include <vector>
#include <chrono>
#include <map>
#include "QuickDebug.hpp"
#include "Common/Types.hpp"


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
    static void SetMaxBufferSize(ui32 count) {
      s_maxElementCount = count;
    }

    static void TakeTimestampMeasurement(i64 xrTimestamp, const char* name) {
      std::lock_guard<std::mutex> lock(s_apiMutex);

      auto* timestampMeasurement = &s_registeredTimestampMeasurements[xrTimestamp];
      timestampMeasurement->timestamps.push_back(std::make_pair(name, std::chrono::steady_clock::now()));

      ClearOldTimestampData();
    }

    /*
      Measures the time delta between this timestamp and the previous one with the given name.
     */
    static std::chrono::microseconds MeasureTimeDelta(i64 xrTimestamp, const char* name) {
      std::lock_guard<std::mutex> lock(s_apiMutex);
      auto it = s_registeredTimestampMeasurements.find(xrTimestamp);

      std::chrono::steady_clock::time_point latestTimestamp;
      if (it != s_registeredTimestampMeasurements.end() && it != s_registeredTimestampMeasurements.begin()) {
        const auto& latestMeasurementTimestamps = it->second.timestamps;
        auto measurementIt = std::find_if(latestMeasurementTimestamps.begin(), latestMeasurementTimestamps.end(), [name](const auto& pair) {
            return std::strcmp(pair.first, name) == 0;
        });
        if (measurementIt == latestMeasurementTimestamps.end()) {
            return std::chrono::microseconds(0); // No measurement found using xrTimestamp with the given name
        }
        latestTimestamp = measurementIt->second;
      }

      while (it != s_registeredTimestampMeasurements.end() && it != s_registeredTimestampMeasurements.begin()) {
        --it;
        const auto& prevMeasurementTimestamps = it->second.timestamps;
        auto measurementPrevIt = std::find_if(prevMeasurementTimestamps.begin(), prevMeasurementTimestamps.end(), [name](const auto& pair) {
          return std::strcmp(pair.first, name) == 0;
        });
        if (measurementPrevIt == prevMeasurementTimestamps.end()) {
          continue; // The preceding measurment was found, but the entry with the given name was not found
                    // Check further preceding elements, since it can happen that an entry in the measurement is skipped in between multiple measurements
        }
        auto prevTimestamp = measurementPrevIt->second;

        auto latency = std::chrono::duration_cast<std::chrono::microseconds>(latestTimestamp - prevTimestamp);
        return latency;
      } 
      
        return std::chrono::microseconds(0); // No preceding measurement was taken (this is the first measurement)
    }

    static std::chrono::microseconds MeasureElapsedTime(i64 xrTimestamp, const char* begin, const char* end) {
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


    /*
     Sends the timestamp measurements to the dashboard
     "deleteSentElements": Toggle for the automatic data cleanup on data transmission. 
                           Useful in combination with the MeasureTimestampDelta method since it requires comparison with old data, 
                           that might not exist anymore if it has already been sent
    */
    static void SendTimestampMeasurement(i64 xrTimestamp, bool deleteSentElements = false) {
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
        QD::QuickDebug::Plot(sendName, static_cast<float>(latency.count()));
      }

      if (deleteSentElements)
        s_registeredTimestampMeasurements.erase(xrTimestamp);
    }

    static void EraseTimestamp(int64_t xrTimestamp) {
        s_registeredTimestampMeasurements.erase(xrTimestamp);
    }


    static void StartMeasurement(const char* name) {
      s_registeredMeasurements[name] = Measurement{-1, std::chrono::steady_clock::now()};
    }

    static std::chrono::microseconds StopMeasurement(const char* name) {
        auto endTime = std::chrono::steady_clock::now();
        if (s_registeredMeasurements.find(name) == s_registeredMeasurements.end()) {
            return std::chrono::microseconds(0);
        }

        auto measurement = s_registeredMeasurements[name];
        return std::chrono::duration_cast<std::chrono::microseconds>(endTime - measurement.startTime);
    }

  private:
      static void ClearOldTimestampData() {
          if (s_registeredTimestampMeasurements.size() <= s_maxElementCount) {
              return;
          }

          //DEBUG_PRINT("Clearing old data " PRINT_INT64 " / " PRINT_INT64 "\r\n", s_registeredTimestampMeasurements.size(), s_maxElementCount);
          while (s_registeredTimestampMeasurements.size() > 50) {
              s_registeredTimestampMeasurements.erase(s_registeredTimestampMeasurements.begin()->first);
          }
      }

  private:
    static inline std::mutex s_apiMutex;
    static inline std::uint32_t s_maxElementCount = 100;
    static inline std::map<const char*, Measurement> s_registeredMeasurements;
    static inline std::map<int64_t, TimestampMeasurement> s_registeredTimestampMeasurements;
  };
}

#endif
