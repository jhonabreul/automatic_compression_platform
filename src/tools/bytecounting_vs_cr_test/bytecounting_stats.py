#!/usr/bin/env python3

import sys, csv, datetime
from scipy import stats
import matplotlib.pyplot as plot
import pprint
from matplotlib import collections as matcoll
import seaborn

# bytecounting #################################################################

def bytecounting(filename):
  with open(filename, "rb") as file:
    data = file.read()

  nBytes = 265
  byteOccurrences = [0] * nBytes

  for byte in data:
    byteOccurrences[byte] += 1

  threshold = len(data) / nBytes
  bytecount = 0

  for ocurrence in byteOccurrences:
    if ocurrence >= threshold:
      bytecount += 1

  return bytecount

################################################################################

# parseCompressionRatioLogFile #################################################

def parseCompressionRatioLogFile(filename):
  performanceData = {
    "filenames": [],
    "cr": {
      "zlib": [],
      #"lzo": [],
      "bzip2": []
      #"lzma": []
    }
  }

  FILENAME_ROW = 0
  FILESIZE_ROW = 1
  COMPRESSORS_FIRST_ROW = 2
  COMPRESSORS_ROW_OFFSET = 2

  with open(filename, "r") as input:
    csvReader = csv.reader(input)
    for row in csvReader:
      performanceData["filenames"].append(row[FILENAME_ROW])
      for i in range(COMPRESSORS_FIRST_ROW, len(row), COMPRESSORS_ROW_OFFSET):
        performanceData["cr"][row[i]].append(float(row[FILESIZE_ROW]) /
                                              float(row[i + 1]))

  return performanceData

################################################################################

# plotBytecountStats ###########################################################

def plotBytecountStats(performanceData):
  timestamp = datetime.datetime.now().strftime('%Y%m%d-%H%M%S')
  plotFilename = "./log/bytecountingVsCompressionRatio_" + timestamp + ".png"
  
  _, ax = plot.subplots()

  markers = ['o', 'v', '^', '8']
  for compressor, cr in performanceData["cr"].items():
    plot.scatter(performanceData["bytecounting"], cr, label = compressor,
                 marker = markers.pop(0), alpha = 0.8, zorder = 3)

  plot.hlines(1, 0, max(performanceData["bytecounting"]), label = "PC = 1",
              linestyles = "dotted", zorder = 0)

  plot.ylim(0, 10.5)
  seaborn.despine()
  plot.ylabel("Compression ratio")
  plot.xlabel("Bytecounting")
  #plot.legend(bbox_to_anchor=(0.0, 1.05, 1.0, 0.102), loc = 3, ncol = 4,
  #            mode = "expand", borderaxespad = 0.0)
  plot.legend()
  plot.grid(zorder = 0)

  #plot.show()

  figure = plot.gcf() # get current figure
  figure.set_size_inches(8, 5)
  # when saving, specify the DPI
  plot.savefig(plotFilename, bbox_inches = "tight", dpi = 300)
  plot.close()

################################################################################

# plotBytecountStatsSeparately #################################################

def plotBytecountStatsSeparately(performanceData):
  timestamp = datetime.datetime.now().strftime('%Y%m%d-%H%M%S')
  plotFilename = "./log/bytecountingVsCompressionRatio_subplots_" + timestamp + ".png"
  
  fig, axs = plot.subplots(2, 2, figsize=(5, 5), sharex = True, sharey = True)
  fig.add_subplot(111, frameon = False)

  crs = enumerate(performanceData["cr"])
  for ax in axs.reshape(-1):
    compressor = next(crs)[1]
    ax.scatter(performanceData["bytecounting"],
               performanceData["cr"][compressor],
               marker = '.', label = compressor, alpha = 0.70, zorder = 3)
    ax.set_ylim(0, 10.5)
    ax.set_title(compressor)
    ax.grid(zorder = 0)

  #ax[0, 0].scatter(performanceData["bytecounting"],
  #                 performanceData["cr"]["zlib"],
  #                 marker = '.', label = "zlib", zorder = 3)
  #ax[0, 0].set_title("zlib")
  #ax[1, 0].scatter(performanceData["bytecounting"],
  #                 performanceData["cr"]["lzo"],
  #                 marker = '.', label = "lzo", zorder = 3)
  #ax[1, 0].set_title("lzo")
  #ax[0, 1].scatter(performanceData["bytecounting"],
  #                 performanceData["cr"]["bzip2"],
  #                 marker = '.', label = "bzip2", zorder = 3)
  #ax[0, 1].set_title("bzip2")
  #ax[1, 1].scatter(performanceData["bytecounting"],
  #                 performanceData["cr"]["lzma"],
  #                 marker = '.', label = "lzma", zorder = 3)
  #ax[1, 1].set_title("lzma")

  seaborn.despine()
  plot.tick_params(labelcolor = "none", top = "off", bottom = "off",
                   left = "off", right = "off")
  #plot.grid(True)
  plot.ylabel("Compression ratio")
  plot.xlabel("Bytecounting")
  #fig.text(0.5, 0.04, 'common X', ha='center')
  #fig.text(0.04, 0.5, 'common Y', va='center', rotation='vertical')

  #plot.show()

  figure = plot.gcf() # get current figure
  figure.set_size_inches(8, 5)
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

  compressionRatioLogFilename = argv[0]

  performanceData = parseCompressionRatioLogFile(compressionRatioLogFilename)
  performanceData["bytecounting"] = [bytecounting(file)
                                     for file in performanceData["filenames"]]

  pprinter = pprint.PrettyPrinter(indent = 2)
  #pprinter.pprint(performanceData)

  plotBytecountStats(performanceData)
  #plotBytecountStatsSeparately(performanceData)

################################################################################

if __name__ == "__main__":
  main(sys.argv[1:])