/**
 *  AutoComp Resource Monitoring Functions
 *  monitors.hpp
 *
 *  @author Jhonathan Abreu
 *  @version 1.0
 *  @date 08/24/2018
 */

#ifndef AC_RESOURCE_MONITORS_HPP
#define AC_RESOURCE_MONITORS_HPP

#include <cstdio>
#include <unistd.h>
#include <vector>
#include <mutex>
#include <chrono>
#include <atomic>

#include "network/socket/tcp_socket.hpp"
#include "utils/data_structures.hpp"

namespace autocomp
{

void monitorCPU(ResourceState & resourceState,
				std::atomic<bool> & doneMonitoring);

} // namespace autocomp

#endif // AC_RESOURCE_MONITORS_HPP