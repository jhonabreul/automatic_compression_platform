#!/usr/bin/env python3

import sys, getopt, re, csv, random, numpy, datetime
from scipy import stats
import matplotlib.pyplot as plot

# parseAutocompBandwidthFile ###################################################

def parseAutocompBandwidthFile(filename):
  with open(filename, "r") as input:
    bandwidthMeasurements = input.readlines()

  bandwidthIndices = [-1]

  bandwidthIndices.extend([index
                           for index, item in enumerate(bandwidthMeasurements)
                           if item == "*\n"])

  bandwidthMeans = {
    "means": [],
    "harmonic_means": []
  }

  # Until (len(bandwidthIndices) - 1) because we don't want to count the last *
  for i in range(0, len(bandwidthIndices) - 1):
    start = bandwidthIndices[i] + 1
    end = bandwidthIndices[i + 1]
    bandwidths = [float(bandwidth.split()[0])
                  for bandwidth in bandwidthMeasurements[start:end]
                  if bandwidth.find("Mbits/s") != -1]
    bandwidthMeans["means"].append(numpy.mean(bandwidths)),
    bandwidthMeans["harmonic_means"].append(stats.hmean(bandwidths))

  return bandwidthMeans

################################################################################

# parseIperfBandwidthFile ######################################################

def parseIperfBandwidthFile(filename):
  with open(filename, "r") as input:
    #bandwidthMeasurements = input.readlines()
    csvReader = csv.reader(input)
    return [1E-6 * float(row[-1]) for row in csvReader]

  #return [float(re.search("\S+\s[MK]bits/sec", bandwidth).group(0).split()[0])
  #        for bandwidth in bandwidthMeasurements
  #        if bandwidth.find("bits/sec") != -1]

################################################################################

# plotBandwidthBars ############################################################

def plotBandwidthBars(autocompMeasurements, iperfMeasurements, figureFilename):
  nMeasurements = len(iperfMeasurements)
  nBars = 10

  if nMeasurements > nBars:
    autocompArithmeticMeans = []
    autocompHarmonicMeans = []
    iperfBandwidths = []
    measurementNumberLabels = []
    indices = list(range(nMeasurements))
    nMeasurements = nBars

    for i in range(nBars):
      index = random.randint(0, len(indices))
      index = indices.pop(index)
      autocompArithmeticMeans.append(autocompMeasurements["means"][index])
      autocompHarmonicMeans.append(autocompMeasurements["harmonic_means"][index])
      iperfBandwidths.append(iperfMeasurements[index])
      measurementNumberLabels.append(str(index))
  else:
    autocompArithmeticMeans = autocompMeasurements["means"]
    autocompHarmonicMeans = autocompMeasurements["harmonic_means"]
    iperfBandwidths = iperfMeasurements
    measurementNumberLabels = [str(label)
                               for label in range(1, nMeasurements + 1)]

  #measurementNumberLabels = [str(label) for label in range(1, nMeasurements + 1)]

  plotIndices = numpy.arange(nMeasurements)
  width = 0.25
  bottom = 0.5 * min(min(min(autocompArithmeticMeans),
                         min(autocompHarmonicMeans)),
                     min(iperfBandwidths))
  autocompArithmeticMeans = [value - bottom for value in autocompArithmeticMeans]
  autocompHarmonicMeans = [value - bottom for value in autocompHarmonicMeans]
  iperfBandwidths = [value - bottom for value in iperfBandwidths]

  _, ax = plot.subplots()
  ax.spines["right"].set_visible(False)
  ax.spines["top"].set_visible(False)

  autocompMeansPlot = plot.bar(plotIndices, autocompArithmeticMeans, width,
                               bottom = bottom, zorder = 3)
  autocompHarmonicMeansPlot = plot.bar(plotIndices + width,
                                       autocompHarmonicMeans, width,
                                       bottom = bottom, zorder = 3)
  iperfPlot = plot.bar(plotIndices + 2 * width, iperfBandwidths, width,
                       bottom = bottom, zorder = 3)


  plot.ylabel("Ancho de banda (Mbits/s)")
  plot.xlabel("Mediciones")
  #plot.title('Mediciones de ancho de banda AutoComp vs. iPerf', y = 1.08)
  plot.xticks(plotIndices + width, measurementNumberLabels)
  #plot.yticks(numpy.arange(bottom, (max(max(max(autocompArithmeticMeans),
  #                                           max(autocompHarmonicMeans)),
  #                                       max(iperfBandwidths)) + bottom) * 1.20))
  plot.legend((autocompMeansPlot[0], autocompHarmonicMeansPlot[0], iperfPlot[0]),
              ("AutoComp (Media aritmética)", "AutoComp (Media armónica)", "iPerf"),
              bbox_to_anchor=(0.0, 1.05, 1.0, 0.102), loc = 3, ncol = 3,
              mode = "expand", borderaxespad = 0.0)
  plot.grid(zorder = 0)

  plot.show()

  #figure = plot.gcf() # get current figure
  #figure.set_size_inches(8, 5)
  # when saving, specify the DPI
  #plot.savefig(figureFilename, bbox_inches = "tight", dpi = 300)
  #plot.close()

################################################################################

# plotRelativeErrorHistogram ###################################################

def plotRelativeErrorHistogram(autocompMeasurements, iperfMeasurements,
                               figureFilename):
  autocompBandwidths = numpy.array(autocompMeasurements)
  iperfBandwidths = numpy.array(iperfMeasurements)

  relativeErrors = abs(iperfBandwidths - autocompBandwidths) / iperfBandwidths

  plotIndices = numpy.arange(len(relativeErrors))

  _, ax = plot.subplots()
  ax.spines["right"].set_visible(False)
  ax.spines["top"].set_visible(False)

  #plot.bar(plotIndices, relativeErrors, zorder = 3)
  n, bins, patches = ax.hist(relativeErrors, rwidth = 0.95, zorder = 3)

  stats = r"Estadísticos: " \
          "\n" \
          r"$\mu = {:.4f}$" \
          "\n" \
          r"$s = {:.4f}$" \
            .format(numpy.mean(relativeErrors), numpy.std(relativeErrors))
  #ax.text(0.9, 0.9, stats, bbox = {"facecolor": "white", "pad": 5},
  #        horizontalalignment = "center", verticalalignment = "center",
  #        transform = ax.transAxes)
  plot.xlabel("Errores relativos (Valor real: iPerf)", fontsize = 18)
  plot.ylabel("Mediciones", fontsize = 18)
  #plot.xticks(plotIndices, range(1, len(relativeErrors) + 1))
  plot.grid(zorder = 0)

  plot.tick_params(labelsize = 16)

  #plot.show()

  figure = plot.gcf() # get current figure
  figure.set_size_inches(8, 5)
  # when saving, specify the DPI
  plot.savefig(figureFilename, bbox_inches = "tight", dpi = 300)
  plot.close()

################################################################################

# plotRelativeErrorBoxPlot ###################################################

def plotRelativeErrorBoxPlot(autocompMeasurements, iperfMeasurements,
                             figureFilename):
  autocompBandwidths = numpy.array(autocompMeasurements)
  iperfBandwidths = numpy.array(iperfMeasurements)

  relativeErrors = abs(iperfBandwidths - autocompBandwidths) / iperfBandwidths

  _, ax = plot.subplots()
  ax.spines["right"].set_visible(False)
  ax.spines["top"].set_visible(False)

  #plot.bar(plotIndices, relativeErrors, zorder = 3)
  flierprops = dict(marker = "o", markerfacecolor = "C0", linestyle = "none",
                    markeredgecolor = "C0", alpha = 0.8)
  medianprops = dict(linestyle = "-", linewidth = 1.5, color = "crimson")
  ax.boxplot(relativeErrors, flierprops = flierprops, vert = False,
             medianprops = medianprops)
  #ax.set_aspect(5)

  stats = r"Estadísticos: " \
          "\n" \
          r"$\mu = {:.4f}$" \
          "\n" \
          r"$s = {:.4f}$" \
            .format(numpy.mean(relativeErrors), numpy.std(relativeErrors))
  ax.text(0.9, 0.9, stats, bbox = {"facecolor": "white", "pad": 5},
          horizontalalignment = "center", verticalalignment = "center",
          transform = ax.transAxes, fontsize = 18)
  
  plot.ylabel("Mediciones", fontsize = 18)
  plot.xlabel("Errores relativos (Valor real: iPerf)", fontsize = 18)
  #plot.xticks(plotIndices, range(1, len(relativeErrors) + 1))
  #plot.grid(zorder = 0)
  plot.tick_params(labelsize = 16)

  #plot.show()

  figure = plot.gcf() # get current figure
  figure.set_size_inches(8, 5)
  # when saving, specify the DPI
  plot.savefig(figureFilename, bbox_inches = "tight", dpi = 300)
  plot.close()

################################################################################

# plotBandwidthMeasurements ####################################################

def plotBandwidthMeasurements(autocompMeasurements, iperfMeasurements):
  timestamp = datetime.datetime.now().strftime('%Y%m%d-%H%M%S')
  barChartFilename = "./log/bandwidthBarChart_" + timestamp + ".png"
  meanHistogramFilename = "./log/bandwidthDiffMeanHistogram_" + timestamp + ".png"
  harmonicMeanHistogramFilename = "./log/bandwidthDiffHarmonicMeanHistogram_" +\
                                  timestamp + ".png"
  meanBoxPlotFilename = "./log/bandwidthDiffMeanBoxPlot_" + timestamp + ".png"
  harmonicMeanBoxPlotFilename = "./log/bandwidthDiffHarmonicMeanBoxPlot_" +\
                                  timestamp + ".png"

  plotBandwidthBars(autocompMeasurements, iperfMeasurements, barChartFilename)
  plotRelativeErrorHistogram(autocompMeasurements["means"], iperfMeasurements,
                             meanHistogramFilename)
  plotRelativeErrorHistogram(autocompMeasurements["harmonic_means"],
                             iperfMeasurements, harmonicMeanHistogramFilename)
  plotRelativeErrorBoxPlot(autocompMeasurements["means"], iperfMeasurements,
                             meanBoxPlotFilename)
  plotRelativeErrorBoxPlot(autocompMeasurements["harmonic_means"],
                             iperfMeasurements, harmonicMeanBoxPlotFilename)

################################################################################

# main #########################################################################

def main(argv):
  if len(argv) != 2:
    print("Invalid number of arguments:", len(argv),
          "were given but 2 are required")
    exit(1)

  autocompBandwidthFilename = argv[0]
  iperfBandwidthFilename = argv[1]

  autocompBandwidthMeasurements = \
    parseAutocompBandwidthFile(autocompBandwidthFilename)
  iperfBandwidthMeasurements = parseIperfBandwidthFile(iperfBandwidthFilename)

  #print("Autocomp:\n  ", end = '')
  #print(autocompBandwidthMeasurements)

  #print("\niperf:\n  ", end = '')
  #print(*iperfBandwidthMeasurements, sep = "\n  ")

  plotBandwidthMeasurements(autocompBandwidthMeasurements,
                            iperfBandwidthMeasurements)

################################################################################

if __name__ == "__main__":
  random.seed(251194)
  main(sys.argv[1:])