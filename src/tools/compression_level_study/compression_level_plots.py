#!/usr/bin/env python3

import sys, csv, datetime
from scipy import stats
import matplotlib.pyplot as plot
import pprint
from matplotlib import collections as matcoll
import seaborn
import os

N_LEVELS = 9

# parseCompressionLevelLogFile #################################################

def parseCompressionLevelLogFile(filename):
  performanceData = []

  FILENAME_ROW = 0
  FILESIZE_ROW = 1
  COMPRESSORS_FIRST_ROW = 2
  COMPRESSION_LEVEL_ROW_OFFSET = 4
  COMPRESSORS_ROW_OFFSET = N_LEVELS * COMPRESSION_LEVEL_ROW_OFFSET

  with open(filename, "r") as input:
    csvReader = csv.reader(input)
    for row in csvReader:
      performanceInfo = {
        "filename": row[FILENAME_ROW],
        "cr": {
          "zlib": [],
          "lzo": [],
          "bzip2": [],
          "lzma": []
        },
        "ct": {
          "zlib": [],
          "lzo": [],
          "bzip2": [],
          "lzma": []
        }
      }
      for i in range(COMPRESSORS_FIRST_ROW, len(row) - 1, COMPRESSORS_ROW_OFFSET):
        for j in range(0, N_LEVELS):
          compressorIndex = i + j * COMPRESSION_LEVEL_ROW_OFFSET
          performanceInfo["cr"][row[compressorIndex]] \
            .append(float(row[FILESIZE_ROW]) / float(row[compressorIndex + 2]))
          performanceInfo["ct"][row[compressorIndex]] \
            .append(float(row[compressorIndex + 3]))

      performanceData.append(performanceInfo)

  return performanceData

################################################################################

# plotCompressionLevelStats #################################################

def plotCompressionLevelStats(performanceData):
  timestamp = datetime.datetime.now().strftime('%Y%m%d-%H%M%S')
  plotFilename = "./log/compressionLevelStudy_" + timestamp + ".png"
  
  fig, axs = plot.subplots(2, len(performanceData), figsize=(5, 5),
                           sharex = True)
  fig.add_subplot(111, frameon = False)
  
  axs[0, 0].set_ylabel("Compression ratio")
  axs[1, 0].set_ylabel("Tiempo de compresión")

  compressorPlotsHandles = []
  compressors = []

  for i in range(len(performanceData)):
    # Plot crs in axs[0, i]
    axs[0, i].set_title(os.path.basename(performanceData[i]["filename"]))
    axs[0, i].grid(True)
    #axs[0, i].tick_params(labelcolor = "none", top = "off", bottom = "off",
    #                      left = "off", right = "off")
    handles = []
    compressorsNames = []
    for j, compressor in enumerate(performanceData[i]["cr"]):
      crPlot = axs[0, i].plot(list(range(1, N_LEVELS + 1)),
                              performanceData[i]["cr"][compressor],
                              marker = '.', label = compressor, zorder = 3)
      handles.append(crPlot[0])
      compressorsNames.append(compressor)

    compressorPlotsHandles = handles
    compressors = compressorsNames

    # Plot cts in axs[1, i]
    axs[1, i].grid(True)
    #axs[1, i].tick_params(labelcolor = "none", top = "off", bottom = "off",
    #                      left = "off", right = "off")
    for j, compressor in enumerate(performanceData[0]["cr"]):
      axs[1, i].plot(list(range(1, N_LEVELS + 1)),
                     performanceData[i]["ct"][compressor],
                     marker = '.', label = compressor, zorder = 3)

  seaborn.despine()
  plot.tick_params(labelcolor = "none", top = "off", bottom = "off",
                   left = "off", right = "off")
  plot.xlabel("Nivel de compresión")
  plot.legend(compressorPlotsHandles, compressors,
              bbox_to_anchor=(0.0, 1.07, 1.0, 0.102), loc = 3, ncol = 4,
              mode = "expand", borderaxespad = 0.0)

  #plot.show()

  figure = plot.gcf() # get current figure
  figure.set_size_inches(15, 7)
  # when saving, specify the DPI
  plot.savefig(plotFilename, bbox_inches = "tight", dpi = 300)
  plot.close()

################################################################################

# main #########################################################################

def main(argv):
  if len(argv) != 1:
    print("Invalid number of arguments:", len(argv),
          "were given but 1 is required")
    exit(1)

  compressionLevelLogFile = argv[0]

  performanceData = parseCompressionLevelLogFile(compressionLevelLogFile)

  #pprinter = pprint.PrettyPrinter(indent = 2)
  #pprinter.pprint(performanceData)

  plotCompressionLevelStats(performanceData)

################################################################################

if __name__ == "__main__":
  main(sys.argv[1:])