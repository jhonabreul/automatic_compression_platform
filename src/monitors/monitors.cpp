/**
 *  AutoComp Resource Monitoring Functions
 *  monitors.cpp
 *
 *  @author Jhonathan Abreu
 *  @version 1.0
 *  @date 08/24/2018
 */

#include "monitors/monitors.hpp"

namespace autocomp
{

void
monitorCPU(ResourceState & resourceState, std::atomic<bool> & doneMonitoring)
{
  std::vector<float> currentTimes(8);
  float currentTotalTime, currentIdleTime, previousTotalTime, previousIdleTime,
        cpuTotalTime, cpuIdleTime;
  FILE * statsFile;

  auto readCPUTimes = 
    [&statsFile, &currentTimes] ()
    {
      statsFile = std::fopen("/proc/stat", "r");
      std::fscanf(statsFile, "%*s %f %f %f %f %f %f %f %f",
                  &currentTimes[0], &currentTimes[1], &currentTimes[2],
                  &currentTimes[3], &currentTimes[4], &currentTimes[5],
                  &currentTimes[6], &currentTimes[7]);
      std::fclose(statsFile);
    };

  auto calculateTimes = 
    [&currentTimes] (float & totalTime, float & idleTime)
    {
      totalTime = 0;
      for (float time : currentTimes) {
        totalTime += time;
      }

      idleTime = currentTimes[3] + currentTimes[4];
    };


  readCPUTimes();
  calculateTimes(previousTotalTime, previousIdleTime);

  while (not doneMonitoring) {
    ::usleep(500000);

    readCPUTimes();
    calculateTimes(currentTotalTime, currentIdleTime);

    cpuTotalTime = currentTotalTime - previousTotalTime;
    cpuIdleTime = currentIdleTime - previousIdleTime;

    resourceState.cpuLoad.store((cpuTotalTime - cpuIdleTime) / cpuTotalTime);

    previousTotalTime = currentTotalTime;
    previousIdleTime = currentIdleTime;
  }
}

} // namespace autocomp