#pragma once
#include <map>
#include <string>
#include <ranges>


namespace QD {
    struct QuickDebugConfig {
        bool UseWebserver = false;
        ui16 WebsocketPort = 8126;
    };

    struct RecvMessageConfig
    {
        i8 targetValueType;
        const void* targetValue; //Contains the address of the value that should be updated on receival of a mesage with the key

        RecvMessageConfig() = default;

        RecvMessageConfig(const i8 tvt, const void* tv) :
            targetValueType(tvt),
            targetValue(tv)
        {

        }
    };

    const struct TransmissionMsg
    {
        std::string message;

        static TransmissionMsg CreatePlotMessage(const char* graph, const float value)
        {
            TransmissionMsg x;

            const char* messageType = "1";
            constexpr size_t graphSize = sizeof(graph) - 1;  //-1 removes the null terminator
            x.message.reserve(1 + 1 + graphSize + 1 + std::to_string(value).size());

            x.message.append(messageType);
            x.message.append(";");
            x.message.append(graph);
            x.message.append(";");
            x.message.append(std::to_string(value));

            return x;
        }

        static TransmissionMsg CreateConfigurationVariableMessage(const std::map<std::string, RecvMessageConfig>& variables)
        {
            TransmissionMsg x;

            const char* messageType = "2";

            std::ostringstream oss;
            oss << messageType << ";";
            for (const auto& key : variables | std::views::keys)
            {
                oss << key << ";";
            }

            x.message = oss.str();

            std::cout << "[ConfigurationMessage] Configuration message: " << x.message << std::endl;
            return x;
        }

        static TransmissionMsg CreateStartRecordingMessage(const char* name)
        {
            TransmissionMsg x;

            const char* messageType = "3";
            x.message.reserve(4 * 1 + strlen(name));
            x.message.append(messageType);
            x.message.append(";");
            x.message.append("1"); //enabled
            x.message.append(";");
            x.message.append(name);

            return x;
        }

        static TransmissionMsg CreateStopRecordingMessage()
        {
            TransmissionMsg x;

            const char* messageType = "3";
            x.message.reserve(3);
            x.message.append(messageType);
            x.message.append(";");
            x.message.append("0");

            return x;
        }
    };
}
