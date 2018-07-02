#!/usr/bin/env python3

import sys, getopt, re, csv, random, numpy, datetime
from scipy import stats
import matplotlib.pyplot as plot
import numpy as np
import pandas as pd
import pprint

# parseAutoCompEvaluationLogFile ###############################################

def removeOutliers(x, outlierConstant = 1.5):
    a = np.array(x)
    upper_quartile = np.percentile(a, 75)
    lower_quartile = np.percentile(a, 25)
    IQR = (upper_quartile - lower_quartile) * outlierConstant
    quartileSet = (lower_quartile - IQR, upper_quartile + IQR)
    resultList = []
    for y in a.tolist():
        if y >= quartileSet[0] and y <= quartileSet[1]:
            resultList.append(y)

    return resultList

################################################################################

# parseAutoCompEvaluationLogFile ###############################################

def parseAutoCompEvaluationLogFile(filename):
  data = {}
  cpuStressors = None
  bandwidths = None
  compressors = None

  with open(filename, "r") as input:
    cpuStressors = input.readline().split(":")[:-1]
    bandwidths = [str(int(int(bandwidth) / 1000)) + "Mbit/s"
                    if int(bandwidth) < 1000000
                    else str(int(int(bandwidth) / 1000000)) + "Gbit/s"
                  for bandwidth in input.readline().split(":")[:-1]]
    compressors = [compressor.split(" ")[:4][-1]
                  for compressor in input.readline().split(":")[:-1]]

    #data["bandwidths"] = bandwidths;
    #data["times"] = {}

    for cpuStressor in cpuStressors:
      #data["times"][cpuStressor] = {}
      matrix = []
      for bandwidth in bandwidths:
        matrix.append([])
        for compressor in compressors:
          times = [float(time) for time in input.readline().split(":")[:-1]]
          meanTransmissionTime = numpy.mean(removeOutliers(times))
          matrix[-1].append(meanTransmissionTime)
          '''
          if compressor in data["times"][cpuStressor]:
            data["times"][cpuStressor][compressor].append(meanTransmissionTime)
          else:
            data["times"][cpuStressor][compressor] = [meanTransmissionTime]
          '''
      data[cpuStressor] = pd.DataFrame(data = matrix, index = bandwidths,
                                       columns = compressors)

  return data

################################################################################


# plotEvaluationData ############################################################

def plotEvaluationData(data, dataType, figureFilenamePrefix,
                       figureFilenamePostfix):
  for nCPUStressors, times in data.items():
    figure, ax = plot.subplots(1, 1)
    lineStyles = ["--", "-.", ":", "-"]
    markers = ["o", "v", "^", "s"]
    i = 0
    x = list(range(times.index.values.size))

    for compressor in times:
      if compressor == "copy":
        continue

      y = np.divide(np.subtract(times["copy"], times[compressor]),
                    times["copy"]) * 100
      timesPlot = ax.plot(x, y, marker = markers[i], label = compressor,
                          zorder = 3)
      i += 1

    ax.spines["right"].set_visible(False)
    ax.spines["top"].set_visible(False)
    ax.set_ylim(-80, 25)
    #ax.set_title("Desempeño para datos tipo " + dataType + " con " + \
    #             nCPUStressors + " procesos de carga")
    ax.set_xticks(x)
    ax.set_xticklabels(times.index.values, rotation = 45, ha = "right")
    ax.set_xlabel("Ancho de banda")
    ax.set_ylabel("Porcentaje de mejora contra copia")
    ax.legend(loc = "lower left")
    plot.grid(axis = "y", zorder = 0)

    figure.set_size_inches(8, 5)
    # when saving, specify the DPI
    figureFilename = figureFilenamePrefix + dataType + "_" + \
                      nCPUStressors + "-cpu-stressors" + figureFilenamePostfix
    plot.savefig(figureFilename, bbox_inches = "tight", dpi = 300)
    plot.close()

################################################################################

# main #########################################################################

def main(argv):
  if len(argv) != 2:
    print("Invalid number of arguments:", len(argv),
          "were given but 2 is required")
    exit(1)

  autocompEvaluationLogFilename = argv[0]
  dataType = argv[1]
  autocompTimes = parseAutoCompEvaluationLogFile(autocompEvaluationLogFilename)

  for key, matrix in autocompTimes.items():
    print("-" * 80)
    print(key, "CPU stressors:\n")
    print(matrix)
    print("-" * 80)

  timestamp = datetime.datetime.now().strftime('%Y%m%d-%H%M%S')
  plotFilenamePrefix = "./log/evaluationPlot_"
  plotFilenamePostfix = "_" + timestamp + ".png"
  plotEvaluationData(autocompTimes, dataType, plotFilenamePrefix,
                     plotFilenamePostfix)

################################################################################

if __name__ == "__main__":
  main(sys.argv[1:])