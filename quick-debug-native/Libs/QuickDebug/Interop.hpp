#ifndef _QD_LOGGER_INTEROP
#define _QD_LOGGER_INTEROP

#include <iostream>
#include "QuickDebug.hpp"

extern "C"
{
  __declspec(dllexport) inline void StartRecording(const char* experimentDescription) {
    QD::QuickDebug::StartRecording(experimentDescription);
  }


  __declspec(dllexport) inline void StopRecording(const char* filePath) {
    QD::QuickDebug::StopRecording();
  }
}

#endif
