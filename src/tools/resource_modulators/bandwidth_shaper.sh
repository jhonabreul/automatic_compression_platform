#!/bin/bash

clearInterface()
{
  interface=$1

  ./external/wondershaper/wondershaper -c -a ${interface}
  returnCode=$?

  if [ $returnCode -ne 0 ] && [ $returnCode -ne 2 ]; then
    exit 2
  fi
}

shapeInterfaceBandwidth()
{
  interface=$1
  rate=$2

  ./external/wondershaper/wondershaper -a ${interface} -d ${rate} -u ${rate}

  if [ $? -ne 0 ]; then
    exit 3
  fi
}

if [ $# -ne 2 ]; then
  echo >&2 "Invalid number of arguments: $# were given but 2 are required"
  exit 1
fi

INTERFACE=$1
RATE=$2

clearInterface ${INTERFACE}

if [ $RATE -gt 0 ]; then
  shapeInterfaceBandwidth ${INTERFACE} ${RATE}
fi

exit 0