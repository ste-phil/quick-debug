#ifndef _QD_LOGGER_INTEROP
#define _QD_LOGGER_INTEROP

#include <iostream>
#include "QuickDebug.hpp"

#ifdef __ANDROID__
    #define EXPORT_API extern "C" __attribute__((visibility("default")))
#elif defined(_WIN32)
    #define EXPORT_API extern "C" __declspec(dllexport)
#else
    #define EXPORT_API extern "C"
#endif

EXPORT_API inline void StartRecording(const char* experimentDescription) {
    QD::QuickDebug::StartRecording(experimentDescription);
}


EXPORT_API inline void StopRecording(const char* filePath) {
    QD::QuickDebug::StopRecording();
}

#endif
