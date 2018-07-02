/**
 *  AutoComp Performance Data Writer
 *  performance_data_writer.hpp
 *
 *  @author Jhonathan Abreu
 *  @version 1.0
 *  @date 07/18/2018
 */

#ifndef AC_PERFORMANCE_DATA_WRITER_HPP
#define AC_PERFORMANCE_DATA_WRITER_HPP

#include <string>
#include <sstream>

#include "utils/constants.hpp"
#include "utils/exceptions.hpp"
#include "utils/data_structures.hpp"
#include "io/synchronized_file.hpp"

namespace autocomp
{
  namespace io
  {

  /**
   * A class that writes performance data to diks in an asynchronous manner
   */
  class PerformanceDataWriter
  {
    SynchronizedFile file;

  public:

    /**
    * Writes performance data in csv format to the synchronized file
    *
    * @param data Data to be writen to disk
    */
    void write(const CompressionPerformanceData & data);

    /**
    * Writes performance data in csv format to the synchronized file
    *
    * @param data Data to be writen to disk
    */
    template<class ... DataTypes>
    void write(const DataTypes & ... data)
    {
      this->file.write(this->formatData(data...));
    }

  private:

    /**
    * Formats the given data in csv format
    *
    * @param data Data to be formatted
    */
    std::string formatData(const CompressionPerformanceData & data);

    /**
    * Formats the given data in csv format
    *
    * @param data Data to be formatted
    */
    template<class... DataTypes>
    std::string formatData(const DataTypes & ... data)
    {
      std::ostringstream formattedDataStream;

      return this->formatData(formattedDataStream, data...);
    }

    template<class T>
    std::string formatData(std::ostringstream & stream, const T & lastItem)
    {      
      stream << lastItem;

      return stream.str();
    }

    template<class T, class... DataTypes>
    std::string formatData(std::ostringstream & stream, const T & firstItem,
                           const DataTypes & ... remainingData)
    {
      stream << firstItem << ",";

      return this->formatData(stream, remainingData...);
    }

  }; // class PerformanceDataWriter

  } // namespace io
} // namespace autocomp

#endif // AC_PERFORMANCE_DATA_WRITER_HPP