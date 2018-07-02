/**
 *  AutoComp Common I/O Helper functions and classes.
 *  io_common.hpp
 *
 *  @author Jhonathan Abreu
 *  @version 1.0
 *	@date 07/17/2018
*/

#ifndef AC_IO_COMMON_HPP
#define AC_IO_COMMON_HPP

#include <string>

#include "g3log/g3log.hpp"
#include "g3log/logworker.hpp"

#include "utils/constants.hpp"

namespace autocomp
{
  namespace io
  {

  /**
   * Formats every event log message. 
   *
   * @param message The log message to be formatted.
   *
   * @returns The formatted log message.
   */
  inline std::string formatLogMessage(const g3::LogMessage & message)
  {
    std::string formattedMessage("[");

    formattedMessage.append(message.timestamp())
                    .append("] [")
                    .append(message.level())
                    .append("]\t[")
                    .append(message.threadID())
                    .append(" ")
                    .append(message.file())
                    .append("->")
                    .append(message.function())
                    .append(":")
                    .append(message.line())
                    .append("] :\t");

      return formattedMessage;
  }

  /**
   * Initializes the logger.
   *
   * @param worker Logging worker
   */
  void initLogger(std::unique_ptr<g3::LogWorker> & worker);

  } // namespace io

} // namespace autocomp

#endif // AC_IO_COMMON_HPP