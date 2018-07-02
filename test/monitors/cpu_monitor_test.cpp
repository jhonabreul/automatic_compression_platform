#include <iostream>
#include <thread>
#include <mutex>
#include <memory>
#include <vector>
#include <atomic>
#include <csignal>
#include <unistd.h>
#include <cstdlib>

#include "monitors/monitors.hpp"
#include "utils/data_structures.hpp"
#include "network/socket/tcp_socket.hpp"

#define PORT 55355

std::atomic<bool> doneMonitoring(false);

void signalHandler(int signalNumber)
{
  doneMonitoring.store(true);
}

int main()
{
  std::signal(SIGINT, signalHandler);

  autocomp::ResourceState resourceState;
  std::thread cpuMonitorThread;
  float cpuLoad;

  cpuMonitorThread = std::thread(autocomp::monitorCPU, std::ref(resourceState),
                                 std::ref(doneMonitoring));

  ::sleep(1);

  while (not doneMonitoring) {
    ::sleep(1);

    system("clear");
    std::cout << "CPU: " << resourceState.cpuLoad << std::endl;
  }

  cpuMonitorThread.join();

  return 0;
}