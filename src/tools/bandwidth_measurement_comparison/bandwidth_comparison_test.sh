#/bin/bash

## Usage #######################################################################
#
# This script must be run in the server
#
# Example: ./bandwidth_comparison_test.sh 192.168.0.101 mserver 192.168.0.112
#             /home/mserver/Documents/jhonathan/AutomaticCompressionPlatform/bin/autocomp
#             /home/jhonathan/Documents/jhonathan/train /tmp 60 3
#
################################################################################

command -v iperf >/dev/null 2>&1 || ( echo >&2 "iperf is required but it's not installed. Aborting."; exit 1 )
command -v python3 >/dev/null 2>&1 || ( echo >&2 "Python is required but it's not installed. Aborting."; exit 1 )
test "$(python3 -c 'import sys; print(sys.version_info[0])')" != "3" && (echo "Python 3 is required to run this script. Aborting"; exit 1)

N_ARGUMENTS=8

if [ $# -ne ${N_ARGUMENTS} ]; then
  echo >&2 "Invalid number of arguments: $# were given but ${N_ARGUMENTS} are required"
  exit 1
fi

LOCAL_IP=$1
USER=$2
HOST=$3
AUTOCOMP_CLIENT_PATH=$4
TEST_FILE_PATH=$5
CLIENT_OUTPUT_DIR=$6
MEASURE_TIME=$7
REPETITIONS=$8

AUTOCOMP_CLIENT_SLEEP_TIME=$((MEASURE_TIME + 2))

timestamp=$(date +"%Y%m%d-%H%M%S")
autocompServerBandwidthLogFile=./log/autocompServerBandwidthLogFile_${timestamp}.log
iperfClientLogFile=./log/iperfClientLogFile_${timestamp}.log

uintRegex='^[0-9]+$'
if ! [[ $REPETITIONS =~ $uintRegex ]]; then
  echo "Error: Repetitions must be an integer number" >&2
  exit 1
fi

# Launch iperf server in remote ################################################

echo -n "Launching iperf server in "${USER}@${HOST}" ... "

iperfServerLogFile=/tmp/iperfServerLogFile.log
unlink ${iperfServerLogFile} >/dev/null 2>&1
nohup ssh ${USER}@${HOST} iperf -s > ${iperfServerLogFile} 2>&1 &
iperfServerPID=$!

#(tail -f -n0 ${iperfServerLogFile} &) | grep -q 'Server listening\|bind failed'

tail -f ${iperfServerLogFile} | while read line
do
  killall tail
  break
done
rm ${iperfServerLogFile}

kill ${iperfServerPID} >/dev/null 2>&1
wait $! 2>/dev/null # This supresses the Terminated (killed) message when killing the process

echo "Done"

################################################################################

port=60001

for (( i = 1; i <= $REPETITIONS; i++ ))
do
  echo
  echo "Starting repetition "${i}"/"${REPETITIONS}" ..."

  # Launch autocomp server in local ##############################################

  echo -n "Launching autocomp server in localhost ... "

  #./bin/autocomp_server >/dev/null 2>&1 &
  ./bin/autocomp_server -p ${port} >> ${autocompServerBandwidthLogFile} 2>/dev/null &
  autocompServerPID=$!

  echo "Done"

  ################################################################################

  # Launch autocomp client in remote #############################################

  echo -n "  Launching autocomp client in "${USER}@${HOST}" ... "

  #ssh ${USER}@${HOST} "${AUTOCOMP_CLIENT_PATH} -H ${LOCAL_IP} -f ${TEST_FILE_PATH} -d ${CLIENT_OUTPUT_DIR} -m no_compression >/dev/null 2>&1 & sleep ${MEASURE_TIME}; killall -SIGTERM autocomp"
  autocompClientCommand="${AUTOCOMP_CLIENT_PATH} -H ${LOCAL_IP}"
  autocompClientCommand="${autocompClientCommand} -P ${port}"
  autocompClientCommand="${autocompClientCommand} -f ${TEST_FILE_PATH}"
  autocompClientCommand="${autocompClientCommand} -d ${CLIENT_OUTPUT_DIR}"
  autocompClientCommand="${autocompClientCommand} -m no_compression"
  autocompClientCommand="${autocompClientCommand} >/dev/null 2>&1 &"
  autocompClientCommand="${autocompClientCommand} sleep ${AUTOCOMP_CLIENT_SLEEP_TIME};"
  autocompClientCommand="${autocompClientCommand} killall -SIGTERM autocomp"
  #nohup ssh ${USER}@${HOST} "${autocompClientCommand}" >/dev/null 2>&1 &
  ssh ${USER}@${HOST} "${autocompClientCommand}" >/dev/null 2>&1
  autocompClientPID=$!
  # Wait for autocomp output to come
  #tail -f ${autocompServerBandwidthLogFile} | while read line
  #do
  #  killall tail
  #  break
  #done

  #kill ${autocompClientPID} >/dev/null 2>&1
  #wait $! 2>/dev/null # This supresses the Terminated (killed) message when killing the process

  echo "Done"

  # Shut autocomp server in local
  kill -SIGINT ${autocompServerPID} >/dev/null 2>&1

  ################################################################################

  # Launch iperf client in local #################################################
  # NOTE: Don't send iperf client to background. Instead, wait for it to finish and then kill autocomp client

  echo -n "  Launching iperf client in localhost ... "

  iperf -y C -c ${HOST} -t ${MEASURE_TIME} >> ${iperfClientLogFile} 2>/dev/null

  echo "Done"

  ################################################################################

  # Add delimiters to log files
  echo "*" >> ${autocompServerBandwidthLogFile}
  #echo "*" >> ${iperfClientLogFile}
  port=$(($port + 1))

done

# Shut iperf server down in remote
ssh ${USER}@${HOST} killall -SIGINT iperf >/dev/null 2>&1

# Plots
./src/tools/bandwidth_measurement_comparison/bandwidth_stats.py ${autocompServerBandwidthLogFile} ${iperfClientLogFile}