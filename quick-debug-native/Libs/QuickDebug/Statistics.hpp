#include <cstdint>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <string>
#include <mutex>
#include <filesystem>

namespace QD {
  /// Class that calculates the Exponential Moving Average (EMA) of a given value.
  /// The EMA is initialized to 0, use to Set() method to set the initial value.
  /// Template parameter ID is a unique identifier, use this to easily access the same value across different files.
  /// Template parameter WEIGHT is the weight of the EMA.
  template <const int ID, std::uint32_t WEIGHT>
  class EMA {
  public:
    static inline uint32_t Update(uint32_t value) {
      m_ema -= m_ema / WEIGHT;
      m_ema += value / WEIGHT;
      return m_ema;
    }

    static inline uint32_t Get() {
      return m_ema;
    }

    static inline void Set(uint32_t value) {
      m_ema = value;
    }

  private:
    static inline uint32_t m_ema = 0;
  };

  /// Class that records key-value pairs and writes them to a CSV file.
  /// Format of the CSV file:
  /// key1,key2,key3,...
  /// value1,value2,value3,...
  /// value1,,value3,...
  class Logger {
  public:
    /// description will be added as a comment in the CSV file.
    static void StartRecording(const char* description) {
      if (m_enabled)
      {
        std::cout << "[QD.Logger] Tried to start: " << description << ", but recording is already in progress." << std::endl;
        return;
      }

      m_enabled = true;
      std::cout << "[QD.Logger] Starting recording: " << description << std::endl;
      m_recordingDescription = description;
      {
        std::lock_guard lock(m_lock);
        m_data.clear();
      }
    }

    static void StopRecording(const char* filePath) {
      if (!m_enabled) return;

      m_enabled = false;
      WriteToCSV(filePath);

      {
        std::lock_guard lock(m_lock);
        m_data.clear();
      }
    }

    static void Record(const char* key, const char* value) {
      if (!m_enabled) return;

      std::lock_guard lock(m_lock);
      m_data[key].push_back(value);
    }

    static void Record(const char* key, const uint32_t value) {
      if (!m_enabled) return;

      std::lock_guard lock(m_lock);
      m_data[key].push_back(std::to_string(value));
    }

    static void WriteToCSV(const std::string& filename) {
      std::string name = GetCurrentTimestamp() + filename + ".csv";
      std::filesystem::path relative_path(name);
      std::filesystem::path absolute_path = std::filesystem::absolute(relative_path);

      std::ofstream file(name);

      if (!file.is_open()) {
        std::cerr << "[QD.Logger] Failed to open file: " << absolute_path << std::endl;
        return;
      }

      file << "# " << m_recordingDescription << "\n";

      // Write header
      if (!m_data.empty()) {
        for (const auto& key : m_data) {
            file << key.first << ",";
        }
        file << "\n";
      }

      // Find the maximum number of values for any key
      size_t maxValues = 0;
      for (const auto& pair : m_data) {
        if (pair.second.size() > maxValues) {
          maxValues = pair.second.size();
        }
      }

      // Write values for each key
      for (size_t i = 0; i < maxValues; ++i) {
        for (const auto& pair : m_data) {
          if (i < pair.second.size()) {
            file << pair.second[i] << ",";
          } else {
            file << ",";
          }
        }
        file << "\n";
      }

      std::cout << "[QD.Logger] Saved recording: " << m_recordingDescription << " to " << absolute_path << std::endl;
      file.close();
    }

  private:
    static std::string GetCurrentTimestamp() {
        auto now = std::time(nullptr);
        std::tm* nowTm = std::localtime(&now);

        std::ostringstream oss;
        oss << std::put_time(nowTm, "%d%m%Y_%H%M%S_");
        return oss.str();
    }

    static inline std::unordered_map<std::string, std::vector<std::string>> m_data; // Static m_data member to store key-value pairs
    static inline bool m_enabled;
    static inline std::string m_recordingDescription;
    static inline std::mutex m_lock;
};
}
