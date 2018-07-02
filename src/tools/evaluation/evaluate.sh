#!/bin/bash

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
  ssh ${user}@${host} "cd ${autocompHostPath}; ./src/tools/resource_modulators/cpu_stressor.py --compress ${nCPUStressors} >/dev/null 2>&1" &
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
  eval ${requestCommand} >/dev/null 2>&1
  end="$(date -u +%s.%N)"
  elapsed=$(echo "$end - $start" | bc)

  printf "%f:" "${elapsed}" >> ${logFilename}
}

cleanup()
{
  echo
  echo -n "Killing autocomp server ... "
  
  # Clear network interface
  ./src/tools/resource_modulators/bandwidth_shaper.sh ${networkInterface} 0

  killCPUStressor
  killAutocompServer

  echo " done"

  exit 0
}

nArguments=8

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
nRounds=$8

compressors=("-m compress -c copy" "-m compress -c snappy"
             "-m compress -c zlib -l 5" "-m compress -c bzip2 -l 5"
             "-m autocomp")

#compressors=("-m compress -c snappy"
#             "-m compress -c zlib -l 5" "-m compress -c bzip2 -l 5"
#             "-m autocomp")

#compressors=("-m compress -c copy")

cpuStressors=(0 2 3 5)
#bandwidthLimits=(1000 2000 10000 20000 400000 60000 80000 10000 1000000)

#cpuStressors=(2)
bandwidthLimits=(10000 20000 60000 80000 100000 200000 300000 500000 800000 1000000)
#bandwidthLimits=(10000)

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

trap "cleanup" SIGINT SIGTERM

i=1

timestamp=$(date +"%Y%m%d-%H%M%S")
logFilename=./log/autocompCompressorEvaluationData_${timestamp}.log

printf "%d:" "${cpuStressors[@]}" >> ${logFilename}
printf "\n" >> ${logFilename}
printf "%d:" "${bandwidthLimits[@]}" >> ${logFilename}
printf "\n" >> ${logFilename}
printf "%s:" "${compressors[@]}" >> ${logFilename}
printf "\n" >> ${logFilename}

# Each CPU load ################################################################
for nCPUStressors in "${cpuStressors[@]}"; do
  # Launch cpu stressors
  launchCPUStreessor

  # Each bandwidth limit #######################################################
  for bandwidthLimit in "${bandwidthLimits[@]}"; do
    # Limit bandwidth
    ./src/tools/resource_modulators/bandwidth_shaper.sh ${networkInterface} ${bandwidthLimit} || { echo "Could not shape bandwidth"; cleanup; exit 1; }
    
      # Each compressor ########################################################
      for compressor in "${compressors[@]}"; do
        [[ "$compressor" = "-m compress -c copy" ]] && nReps=5 || nReps=${nRounds}

        echo
        echo -n "[ Run  ] Request $i/$nRequests with "
        echo -n "compressor = \"$compressor\", "
        echo -n "CPU stressors = $nCPUStressors, "
        echo "bandwidth limit = $bandwidthLimit"

        compressorStart="$(date -u +%s.%N)"

        for (( round = 1; round <= nReps; round++ )); do
          echo -n "   Running repetition ${round}/${nReps} for \"${compressor}\" ... "

          roundStart="$(date -u +%s.%N)"

          ########################################################
           
          # Launch AutoComp client
          launchAutoCompClient

          ########################################################

          roundEnd="$(date -u +%s.%N)"
          roundElapsed=$(echo "$roundEnd - $roundStart" | bc)
          printf "done in %.3f sec\n" ${roundElapsed}

          sleep 1
        done

        echo >> ${logFilename}
        
        compressorEnd="$(date -u +%s.%N)"
        compressorElapsed=$(echo "$compressorEnd - $compressorStart - 1" | bc)
        printf "[  OK  ] Done in %.3f sec\n" ${compressorElapsed}
        i=$(( i + 1 ))
      done
  done
  # Kill cpu stressors
  killCPUStressor
done

cleanup