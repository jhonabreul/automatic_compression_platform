#!/usr/bin/env python3

import os
import argparse
import sys
import signal
import time
import subprocess

keepModulating = True
bandwidthLimits = []

bandwidthShaperPath = "./src/tools/resource_modulators/bandwidth_shaper.sh"

def ignoreSignal(signalNumber, frame):
  return

def quitSilently(signalNumber, frame):
  exit(0)

def shapeBandwidth(interface, rate):
  try:
    output = subprocess.check_output([bandwidthShaperPath, interface,
                                      str(int(rate * 1000))],
                                     stderr = subprocess.PIPE)
  except subprocess.CalledProcessError as error:
    print("Error running wondershaper to limit bandwidth. Return code:",
          error.returncode, file = sys.stderr)
    exit(1)

# main #########################################################################

def main():
  parser = argparse.ArgumentParser()
  parser.add_argument("bandwidths", type = float, nargs = "+",
                      help = "bandwidth limits")
  group = parser.add_mutually_exclusive_group()
  group.add_argument("--timeout", type = int, help = "Timeout")
  group.add_argument("--modulate", action='store_true',
                     help = "Modulate bandwidth limit")
  parser.add_argument("--interval", type = int, default = 15,
                      help = "Modulation intervals")
  parser.add_argument("--interface", required = True,
                      help = "Interface to modulate")
  args = parser.parse_args()

  if args.timeout is not None:
    signal.signal(signal.SIGALRM, quitSilently)
    signal.signal(signal.SIGINT, ignoreSignal)
    signal.signal(signal.SIGTERM, ignoreSignal)
    signal.signal(signal.SIGQUIT, ignoreSignal)
    signal.signal(signal.SIGHUP, ignoreSignal)

    signal.alarm(abs(args.timeout))
  else:
    signal.signal(signal.SIGINT, quitSilently)
    signal.signal(signal.SIGTERM, quitSilently)
    signal.signal(signal.SIGQUIT, quitSilently)
    signal.signal(signal.SIGHUP, quitSilently)

  while keepModulating:
    for bandwidthLimit in args.bandwidths:
      print(bandwidthLimit)
      shapeBandwidth(args.interface, bandwidthLimit)
      time.sleep(args.interval)

  shapeBandwidth(args.interface, 0)

################################################################################

if __name__ == "__main__":
  main()