#!/bin/bash

## Usage #######################################################################
#
# This script must be run in the client
#
# Example: ./retrieve_training_data.sh 192.168.0.101 mserver 192.168.0.112
#             /home/mserver/Documents/jhonathan/AutomaticCompressionPlatform
#             /home/mserver/Documents/jhonathan/train /tmp enp2s0
#
################################################################################

export LC_NUMERIC="en_US.UTF-8"

command -v python3 >/dev/null 2>&1 || { echo >&2 "Python is required but it's not installed. Aborting."; exit 1; }
test "$(python3 -c 'import sys; print(sys.version_info[0])')" != "3" && { echo "Python 3 is required to run this script. Aborting"; exit 1; }

launchAutoCompServer()
{
  ssh ${user}@${host} "cd ${autocompHostPath}; ./bin/autocomp_server >/dev/null 2>&1" &
  #./bin/autocomp_server >/dev/null 2>&1 &
  autocompServerPID=$!
  sleep 1

  kill -0 $autocompServerPID >/dev/null 2>&1 || { echo "Could not start AutoComp server. Aborting"; exit 1; }
  
  until kill -0 $autocompServerPID >/dev/null 2>&1 ; do
    kill -SIGTERM $autocompServerPID >/dev/null 2>&1
    usleep 100000
  done
}

killAutocompServer()
{
  ssh ${user}@${host} "pkill -9 -f autocomp" >/dev/null 2>&1
}

launchCPUStreessor()
{
  ssh ${user}@${host} "cd ${autocompHostPath}; ./src/tools/resource_modulators/cpu_stressor.py --sqrt ${nCPUStressors} >/dev/null 2>&1" &
  cpuStressorPID=$!
  usleep 300000

  kill -0 $cpuStressorPID >/dev/null 2>&1 || { echo "Could not launch CPU stressors"; killAutocompServer; exit 1; }
  
  until kill -0 $cpuStressorPID >/dev/null 2>&1 ; do
    kill -SIGTERM $cpuStressorPID >/dev/null 2>&1
    usleep 100000
  done
}

killCPUStressor()
{
  ssh ${user}@${host} "pkill -9 -f cpu_stressor.py" >/dev/null 2>&1
}

launchAutoCompClient()
{
  requestCommand="${baseRequestCommand} ${compressor}"
  start="$(date -u +%s.%N)"
  if eval ${requestCommand} >/dev/null 2>&1 ; then
    
    echo -n "   [  OK  ] "
  else
    echo -n "   [ FAIL ] "
  fi

  end="$(date -u +%s.%N)"
  elapsed=$(echo "$end - $start" | bc)
  printf "Done in %.3f sec\n" ${elapsed}
}

stabilizeServer()
{
  stabilizeRequestCommand="./bin/autocomp"
  stabilizeRequestCommand="${stabilizeRequestCommand} -H ${host}"
  stabilizeRequestCommand="${stabilizeRequestCommand} -f ${autocompHostPath}/build"
  stabilizeRequestCommand="${stabilizeRequestCommand} -d /tmp"
  stabilizeRequestCommand="${stabilizeRequestCommand} -m compress -c lzma -l 9"

  for (( i = 0 ; i < 2 ; i++ )); do
    eval ${stabilizeRequestCommand} >/dev/null 2>&1
  done
}

cleanup()
{
  echo
  echo -n "Killing autocomp server ... "
  
  killCPUStressor
  killAutocompServer
  # Clear network interface
  ./src/tools/resource_modulators/bandwidth_shaper.sh ${networkInterface} 0

  echo " done"

  exit 0
}

nArguments=7

if [ $# -ne ${nArguments} ]; then
  echo >&2 "Invalid number of arguments: $# were given but ${nArguments} are required"
  exit 1
fi

localIP=$1
user=$2
host=$3
autocompHostPath=${4%/}
testDirPath=$5
outputDir=$6
networkInterface=$7

compressors=("-c copy" "-c snappy" "-c zlib -l 6" "-c bzip2 -l 9")

#compressors=("-c copy" "-c snappy"
#             "-c zlib -l 1" "-c zlib -l 6" "-c zlib -l 9"
#             "-c bzip2 -l 1" "-c bzip2 -l 5" "-c bzip2 -l 9"
#             "-c lzo -l 6" "-c lzo -l 7" "-c lzo -l 8"
#             "-c lzma -l 1" "-c lzma -l 3" "-c lzma -l 6")

cpuStressors=(0 2 4 5)
bandwidthLimits=(1000000 900000 800000 70000 600000 500000 400000 300000 200000
                 100000 80000 60000 40000 20000 10000 6000)

#cpuStressors=(0)
#bandwidthLimits=(50000)

echo -n "Launching autocomp server in ${user}@${host} ... "
launchAutoCompServer
echo " done"

compressorsSize=${#compressors[@]}
cpuStressorsSize=${#cpuStressors[@]}
bandwidthLimitsSize=${#bandwidthLimits[@]}

nRequests=$(( compressorsSize * cpuStressorsSize * bandwidthLimitsSize ))

baseRequestCommand="./bin/autocomp"
baseRequestCommand="${baseRequestCommand} -H ${host}"
baseRequestCommand="${baseRequestCommand} -f ${testDirPath}"
baseRequestCommand="${baseRequestCommand} -d ${outputDir}"
baseRequestCommand="${baseRequestCommand} -m train"

#echo -n "Launching autocomp base test request ... "
#./src/tools/resource_modulators/bandwidth_shaper.sh ${networkInterface} 20000 || { echo "Could not shape bandwidth"; killCPUStressor; killAutocompServer; exit 1; }
#stabilizeServer
#echo " done"
#echo

trap "cleanup" SIGINT SIGTERM

nRounds=10
for (( round = 1; round <= nRounds; round++ )); do
  echo "Running round ${round}/${nRounds} ..."

  roundStart="$(date -u +%s.%N)"

  i=1

  # Each bandwidth limit
  for bandwidthLimit in "${bandwidthLimits[@]}"; do
    # Limit bandwidth
    ./src/tools/resource_modulators/bandwidth_shaper.sh ${networkInterface} ${bandwidthLimit} || { echo "Could not shape bandwidth"; cleanup; exit 1; }

    #stabilizeServer

    # Each CPU load
    for nCPUStressors in "${cpuStressors[@]}"; do
      # Launch cpu stressors
      launchCPUStreessor
      sleep 1

      # Each compressor
      for compressor in "${compressors[@]}"; do
        echo -n "   [ Run  ] Request $i/$nRequests with "
        echo -n "compressor = \"$compressor\", "
        echo -n "CPU stressors = $nCPUStressors, "
        echo "bandwidth limit = $bandwidthLimit" 

        # Launch AutoComp client
        launchAutoCompClient

        i=$(( i + 1 ))
      done

      # Kill cpu stressors
      killCPUStressor
    done
  done

  roundEnd="$(date -u +%s.%N)"
  roundElapsed=$(echo "$roundEnd - $roundStart" | bc)
  printf "Round %d/%d done in %.3f sec\n" ${round} ${nRounds} ${roundElapsed}
done

cleanup