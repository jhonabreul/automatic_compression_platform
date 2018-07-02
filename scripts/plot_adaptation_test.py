#!/usr/bin/env python3

import sys, getopt, re, csv, random, numpy, datetime
from scipy import stats
import matplotlib.pyplot as plot
import numpy as np
import pandas as pd
import pprint

# parseFile ####################################################################

def parseFile(filename):
  dictionary = {
    "SNAPPY": {
      "label": "snappy",
      "value": 1
    },
    "ZLIB": {
      "label": "zlib",
      "value": 2
    }#,
    #"COPY": {
    #  "label": "copia",
    #  "value": 3
    #},
    #"BZIP2": {
    #  "label": "bzip2",
    #  "value": 4
    #}
  }
  data = []

  with open(filename, "r") as input:
    for line in input.readlines()[:14400]:
      data.append(dictionary[line[:-1]]["value"]);

  return data, dictionary

################################################################################


# plotAdaptationData ###########################################################

def plotAdaptationData(data, dictionary, figureFilename):
  figure, ax = plot.subplots(1, 1)
  lineStyles = ["--", "-.", ":", "-"]
  markers = ["o", "v", "^", "s"]

  x = list(range(len(data)))
  adaptationPlot = ax.plot(x, data, color = "C0", zorder = 3)

  ax.axvspan(3800, 11100, facecolor = "C1", alpha = 0.15)
  ax.text(0.15, 0.9, "15 Mbits/s", horizontalalignment='center',
          verticalalignment='center', transform=ax.transAxes,
          fontsize = 16)
  ax.text(0.52, 0.9, "1 Gbit/s", horizontalalignment='center',
          verticalalignment='center', transform=ax.transAxes,
          fontsize = 16)
  ax.text(0.86, 0.9, "15 Mbits/s", horizontalalignment='center',
          verticalalignment='center', transform=ax.transAxes,
          fontsize = 16)
  #############
  #ax.axvspan(9050, 10500, facecolor = "C1", alpha = 0.15)
  #ax.text(0.3, 0.9, "1 Gbit/s", horizontalalignment='center',
  #        verticalalignment='center', transform=ax.transAxes,
  #        fontsize = 16)
  #ax.text(0.66, 0.9, "15 Mbits/s", horizontalalignment='center',
  #        verticalalignment='center', transform=ax.transAxes,
  #        fontsize = 16)
  #ax.text(0.88, 0.9, "1 Gbit/s", horizontalalignment='center',
  #        verticalalignment='center', transform=ax.transAxes,
  #        fontsize = 16)
  ax.spines["right"].set_visible(False)
  ax.spines["top"].set_visible(False)
  #ax.set_title("")
  ax.set_ylim(0.5, 2.5)
  ax.set_xticks(list(range(0, len(x), 3000)))
  #ax.set_xticklabels(times.index.values, rotation = 45, ha = "right")
  ax.set_yticks([value["value"] for key, value in dictionary.items()])
  ax.set_yticklabels([value["label"] for key, value in dictionary.items()],
                     rotation = "vertical",
                     fontdict = {"verticalalignment": "center",
                                 "horizontalalignment": "center"})
  ax.set_xlabel("Trozos o mensajes", fontsize = 16)
  plot.tick_params(axis = "y", labelsize = 16, pad = 15)
  #ax.set_ylabel("")
  #ax.legend()
  plot.grid(axis = "y", zorder = 0)

  figure.set_size_inches(8, 5)
  # when saving, specify the DPI
  plot.savefig(figureFilename, bbox_inches = "tight", dpi = 300)
  plot.close()

################################################################################

# main #########################################################################

def main(argv):
  if len(argv) != 1:
    print("Invalid number of arguments:", len(argv),
          "were given but 2 is required")
    exit(1)

  filename = argv[0]
  data, dictionary = parseFile(filename)

  timestamp = datetime.datetime.now().strftime('%Y%m%d-%H%M%S')
  figureFilename = "./log/adaptationPlot_" + timestamp + ".png"
  plotAdaptationData(data, dictionary, figureFilename)

################################################################################

if __name__ == "__main__":
  main(sys.argv[1:])