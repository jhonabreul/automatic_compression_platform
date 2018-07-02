#!/usr/bin/env python3

import zlib
import os
import argparse
import sys
import signal
import time
import math
import random

children = []
cpuModulatorPID = 0

def ignoreSignal(signalNumber, frame):
  return

def quitSilently(signalNumber, frame):
  for pid in children:
    os.kill(pid, signal.SIGTERM)

  exit(0)

def resetSignalHandlers():
  signal.signal(signal.SIGINT, signal.SIG_DFL)
  signal.signal(signal.SIGTERM, signal.SIG_DFL)
  signal.signal(signal.SIGQUIT, signal.SIG_DFL)
  signal.signal(signal.SIGHUP, signal.SIG_DFL)

# randomZlibCompressor #########################################################

def randomZlibCompressor():
  nBytes = 32*1024

  with open("/dev/urandom", "rb") as inputStream:
    with open("/dev/null", "wb") as outputStream:
      while True:
        data = inputStream.read(nBytes)
        outputStream.write(zlib.compress(data, 1))
        #time.sleep(1E-3)


################################################################################

# forkCompressors ##############################################################

def forkCompressors(nCompressionStressors):

  for i in range(nCompressionStressors):
    try:
      pid = os.fork()
    except:
      print("Could not fork compression stressor #%d" % i + 1,
            file = sys.stderr)
      return
    
    if pid == 0:
      resetSignalHandlers()
      randomZlibCompressor()
      exit(0)

    children.append(pid)

################################################################################

# sqrtStressor #################################################################

def sqrtStressor():
  #number = 2645.23

  while True:
    math.sqrt(random.random())
    #for i in range(100000):
    #  math.sqrt(number)
    #time.sleep(1E-3)

################################################################################

# forkSqrtStressors ############################################################

def forkSqrtStressors(nSqrtStressors):

  for i in range(nSqrtStressors):
    try:
      pid = os.fork()
    except:
      print("Could not fork sqrt stressor #%d" % i + 1,
            file = sys.stderr)
      return
    
    if pid == 0:
      resetSignalHandlers()
      sqrtStressor()
      exit(0)

    children.append(pid)

################################################################################

# modulateCPULoad ##############################################################

def modulateCPULoad(interval):
  signal.signal(signal.SIGINT, quitSilently)
  signal.signal(signal.SIGTERM, quitSilently)
  signal.signal(signal.SIGQUIT, quitSilently)
  signal.signal(signal.SIGHUP, quitSilently)

  sqrtStressorsNumbers = [1, 2, 3, 4, 5, 6, 7, 8]
  while True:
    for nSqrtStressors in sqrtStressorsNumbers:
      forkSqrtStressors(nSqrtStressors)
      time.sleep(interval)
      for pid in children:
        os.kill(pid, signal.SIGTERM)

################################################################################

# main #########################################################################

def main():
  parser = argparse.ArgumentParser()
  parser.add_argument("--compress", type = int,
                      help = "Launch compression stressors")
  parser.add_argument("--sqrt", type = int, 
                      help = "Launch sqrt stressors")
  group = parser.add_mutually_exclusive_group()
  group.add_argument("--timeout", type = int, default = 0, help = "Timeout")
  group.add_argument("--modulate", action='store_true',
                     help = "Modulate CPU load")
  parser.add_argument("--interval", type = int, default = 5,
                      help = "Modulation intervals")
  args = parser.parse_args()
 
  if not args.modulate and args.compress is None and args.sqrt is None:
    parser.error("--compress and/or --sqrt are required")

  nCompressionStressors = 0 if args.compress is None else args.compress
  nSqrtStressors = 0 if args.sqrt is None else args.sqrt

  if args.modulate:
    modulateCPULoad(abs(args.interval))
  else:
    forkCompressors(nCompressionStressors)
    forkSqrtStressors(nSqrtStressors)

    if args.timeout > 0:
      time.sleep(args.timeout)
    else:
      signal.pause()

  for pid in children:
    os.kill(pid, signal.SIGTERM)

################################################################################

if __name__ == "__main__":
  signal.signal(signal.SIGINT, ignoreSignal)
  signal.signal(signal.SIGTERM, ignoreSignal)
  signal.signal(signal.SIGQUIT, ignoreSignal)
  signal.signal(signal.SIGHUP, ignoreSignal)

  main()