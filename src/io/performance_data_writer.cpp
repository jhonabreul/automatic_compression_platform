/**
 *  AutoComp Performance Data Writer
 *  performance_data_writer.cpp
 *
 *  @author Jhonathan Abreu
 *  @version 1.0
 *  @date 07/18/2018
 */

#include "io/performance_data_writer.hpp"

namespace autocomp
{
  namespace io
  {

  //SynchronizedFile PerformanceDataWriter::file;

  void PerformanceDataWriter::write(const CompressionPerformanceData & data)
  {
    this->file.write(this->formatData(data));
  }

  /*
  template<class ... DataTypes>
  void PerformanceDataWriter::write(const DataTypes & ... data)
  {
    this->file.write(this->formatData(data ...));
  }
  */

  std::string
  PerformanceDataWriter::formatData(const CompressionPerformanceData & data)
  {
    std::string formattedData(data.compressor);

    formattedData.append(",")
                 .append(std::to_string(data.compressionLevel))
                 .append(",")
                 .append(std::to_string(data.elapsedTime))
                 .append(",")
                 .append(std::to_string(data.originalSize))
                 .append(",")
                 .append(std::to_string(data.finalSize));

    return formattedData;
  }

  /*
  template<class ... DataTypes>
  std::string PerformanceDataWriter::formatData(const DataTypes & ... data)
  {
    std::string formattedData;
    
    auto lastItemIterator = std::prev(data.end());

    for (auto iterator = data.begin();
         iterator != lastItemIterator;
         iterator++) {
      formattedData.append(*iterator).append(",");
    }

    formattedData.append(*lastItemIterator);

    return formattedData;
  }
  */

  } // namespace io
} // namespace autocomp