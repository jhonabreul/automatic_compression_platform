#!/usr/bin/env python3

import sys
import csv
import math
import datetime
import pprint

class PerformanceData:
  N_CPU_LOAD_LEVELS = 11
  N_BANDWDITH_LEVELS = 29 # 20 (intervals of 5 until 99) + 9 (intervals of 100 until 1000)
  N_BYTECOUNTING_LEVELS = 10

  def __init__(self):
    # A list of N_CPU_LOAD_LEVELS elements, each of which is a list of
    # N_BANDWDITH_LEVELS, each of which is a list of N_BYTECOUNTING_LEVELS,
    # each of which is an empty dictionary
    self.data = list([[[{} for k in range(self.N_BYTECOUNTING_LEVELS)]
                       for j in range(self.N_BANDWDITH_LEVELS)]
                      for i in range(self.N_CPU_LOAD_LEVELS)])

    '''
    self.data = [{}] * self.N_BYTECOUNTING_LEVELS
    self.data = [self.data] * self.N_BANDWDITH_LEVELS
    self.data = [self.data] * self.N_CPU_LOAD_LEVELS
    '''

  ##################################################

  def cpu_load_level(self, cpu_load):
    return int(cpu_load * 100 / 10)

  ##################################################

  def bandwidth_level(self, bandwidth):
    if bandwidth < 100.0:
      return int(bandwidth / 5)

    if bandwidth < 1000.0:
      return int(bandwidth / 100 + 19)
  
    return self.N_BANDWDITH_LEVELS - 1

  ##################################################

  def bytecounting_level(self, bytecounting):
    return int(bytecounting / 10)

  ##################################################

  def indices(self, cpu_load, bandwidth, bytecounting):
    return self.cpu_load_level(cpu_load), self.bandwidth_level(bandwidth), \
           self.bytecounting_level(bytecounting)

  ##################################################

  def __getitem__(self, indicesValues):

    cpu_load, bandwidth, bytecounting = indicesValues
    cpu_load_level, bandwidth_level, bytecounting_level = \
      self.indices(cpu_load, bandwidth, bytecounting)

    return self.data[cpu_load_level][bandwidth_level][bytecounting_level]

  ##################################################

  def __setitem__(self, indicesValues, value):
    cpu_load, bandwidth, bytecounting = indicesValues
    cpu_load_level, bandwidth_level, bytecounting_level = \
      self.indices(cpu_load, bandwidth, bytecounting)

    self.data[cpu_load_level][bandwidth_level][bytecounting_level] = value

  ##################################################

  def get_best_compressor(self, data):
    best_compressor = ""
    best_transmission_rate = float("-inf")

    for compressor, data in data.items():
      transmission_rate = data["transmission_rate"]

      if transmission_rate > best_transmission_rate:
        best_compressor = compressor
        best_transmission_rate = transmission_rate

    return best_compressor

  ##################################################

  def save(self, filename):
    with open(filename, "w", newline = "") as file:
      csv_writer = csv.writer(file, delimiter = ",", quotechar = '"',
                              quoting = csv.QUOTE_MINIMAL)

      for cpu_level, data_level_1 in enumerate(self.data):
        for bandwidth_level, data_level_2 in enumerate(data_level_1):
          for bytecounting_level, compressors_data in enumerate(data_level_2):
            if compressors_data == {}:# or len(compressors_data) < 2:
            #  if bandwidth_level < 2:
            #    best_compressor = "zlib_6"
            #  elif bandwidth_level >= 40:
            #    best_compressor = "snappy"
            #  elif bandwidth_level >= 20:
            #    best_compressor = "zlib_6"
            #  else:
            #    continue
              continue
            #elif bandwidth_level <= 2:
              #best_compressor = "zlib_6"
            else:
              best_compressor = self.get_best_compressor(compressors_data)

            entry = [cpu_level, bandwidth_level, bytecounting_level,
                     best_compressor]
            csv_writer.writerow(entry)

  def show(self):
    pp = pprint.PrettyPrinter(indent = 4)

    i = 1;
    for cpu_level, data_level_1 in enumerate(self.data):
      for bandwidth_level, data_level_2 in enumerate(data_level_1):
        for bytecounting_level, compressors_data in enumerate(data_level_2):
          print("CPU:", cpu_level)
          print("BW:", bandwidth_level)
          print("BC:", bytecounting_level)
          pp.pprint(compressors_data)
          print("\n")
          print("-" * 80)
          print("\n")

          if i == 50000:
            return

          i += 1

################################################################################

class AutoCompPerformanceDataParser:
  COMPRESSOR_INDEX = 0
  COMPRESSION_LEVEL_INDEX = 1

  CPU_LOAD_INDEX = 2
  BANDWIDTH_INDEX = 3
  BYTECOUNTING_INDEX = 4
  COMPRESSION_RATE_INDEX = 5
  COMPRESSION_RATIO_INDEX = 6
  #TRANSMISSION_RATE_INDEX = -1

  def __init__(self, filename):
    self.filename = filename
    self.data = PerformanceData()

  ##################################################

  def lines(self):
    with open(self.filename, newline = "") as file:
      content = csv.reader(file, delimiter = ",")
      for row in content:
        compressor = row[self.COMPRESSOR_INDEX]
        bytecounting = float(row[self.BYTECOUNTING_INDEX])
        bandwidth = float(row[self.BANDWIDTH_INDEX])
        #transmission_rate = float(row[self.TRANSMISSION_RATE_INDEX])

        #if (bandwidth == 0 or transmission_rate == 0):
        if (compressor == "LZO" or bytecounting > 100 or bandwidth == 0):
        #if (bytecounting > 100 or bandwidth == 0):
          continue

        yield row

  ##################################################

  def get_compressor(self, compressor_data):
      compressor = compressor_data[self.COMPRESSOR_INDEX] \
                   if compressor_data[self.COMPRESSION_LEVEL_INDEX] == "-1" \
                   else "_".join(compressor_data)
      
      return compressor.lower()

  ##################################################

  def parse(self):
    for line in self.lines():
      compressor = self.get_compressor(line[:2])
      cpu_load = float(line[self.CPU_LOAD_INDEX])
      bandwidth = float(line[self.BANDWIDTH_INDEX])
      bytecounting = int(line[self.BYTECOUNTING_INDEX])
      #transmission_rate = float(line[self.TRANSMISSION_RATE_INDEX])
      compression_rate = float(line[self.COMPRESSION_RATE_INDEX])
      compression_ratio = float(line[self.COMPRESSION_RATIO_INDEX])
      transmission_rate = min(bandwidth, compression_rate) * compression_ratio

      entry = self.data[cpu_load, bandwidth, bytecounting]
      compressor_data = entry.get(compressor)

      if compressor_data is None:
        compressor_data = {"transmission_rate": transmission_rate, "count": 1}
      else:
        count = compressor_data["count"]
        last_transmission_rate = compressor_data["transmission_rate"]

        if count % 20 == 0:
          compressor_data["transmission_rate"] = \
            (last_transmission_rate * (count - 1) + transmission_rate) / \
              (count + 1)
        else:
          compressor_data["transmission_rate"] = \
            (last_transmission_rate * count + transmission_rate) / (count + 1)

        compressor_data["count"] += 1

      entry[compressor] = compressor_data
      self.data[cpu_load, bandwidth, bytecounting] = entry

    return self.data

  ##################################################

  def save(self, filename):
    self.data.save(filename)
    #self.data.show()

  ##################################################

  def head(self, max_lines = 4000):
    i = 1

    for line in self.lines():
      print(line)
      if i == max_lines:
        break;
      i += 1

# main #########################################################################

def main(argv):
  if len(argv) != 1:
    print("Invalid number of arguments:", len(argv),
          "were given but 1 are required")
    exit(1)

  performanceDataFilename = argv[0]
  timestamp = datetime.datetime.now().strftime('%Y%m%d-%H%M%S')
  outputFilename = "./log/traning_data_" + timestamp + ".csv"


  parser = AutoCompPerformanceDataParser(performanceDataFilename)
  #print(parser.parse().show())
  parser.parse()
  parser.save(outputFilename)
  
  #data = PerformanceData()
  #print(data[43.5, 67.7, 56])
  #data[43.5, 67.7, 56] = {"¡key": "value!"}
  #print(data[43.5, 67.7, 56])

################################################################################

if __name__ == "__main__":
  main(sys.argv[1:])