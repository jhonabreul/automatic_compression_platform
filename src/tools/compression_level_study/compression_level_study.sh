#/bin/bash

command -v gzip >/dev/null 2>&1 || ( echo >&2 "gzip is required but it's not installed. Aborting."; exit 1 )
command -v lzop >/dev/null 2>&1 || ( echo >&2 "lzop is required but it's not installed. Aborting."; exit 1 )
command -v bzip2 >/dev/null 2>&1 || ( echo >&2 "bzip2 is required but it's not installed. Aborting."; exit 1 )
command -v lzma >/dev/null 2>&1 || ( echo >&2 "lzma is required but it's not installed. Aborting."; exit 1 )
command -v python3 >/dev/null 2>&1 || ( echo >&2 "Python is required but it's not installed. Aborting."; exit 1 )
test "$(python3 -c 'import sys; print(sys.version_info[0])')" != "3" && (echo "Python 3 is required to run this script. Aborting"; exit 1)

N_ARGUMENTS=1

if [ $# -ne ${N_ARGUMENTS} ]; then
  echo >&2 "Invalid number of arguments: $# were given but ${N_ARGUMENTS} are required"
  exit 1
fi

# ${1%/} removes last '/' to $1, if it has it
TEST_DIRECTORY=${1%/}

timestamp=$(date +"%Y%m%d-%H%M%S")
testDirectoryName=$(basename ${TEST_DIRECTORY})
compressionLevelLogFile=./log/compressionLevelStudy_${testDirectoryName}_${timestamp}.log
touch ${compressionLevelLogFile}

if [ ! -e ${TEST_DIRECTORY} ] || [ ! -d ${TEST_DIRECTORY} ]; then
  echo "${TEST_DIRECTORY} doesn't exist or is not a directory"
  exit 1
fi

for file in ${TEST_DIRECTORY}/*; do
  if [ ! -f "${file}" ]; then
    continue
  fi

  echo -n "${file},$(stat -c%s "${file}")," >> ${compressionLevelLogFile}

  # Compress with gzip #########################################################

  for (( level = 1; level <= 9; level++ ))
  do
    compressedFilename="${file}".gz
    start="$(date -u +%s.%N)"
    gzip -${level} < "${file}" > "${compressedFilename}"
    end="$(date -u +%s.%N)"
    elapsed=$(echo "$end - $start" | bc)
    performanceData="zlib,${level},$(stat -c%s "${compressedFilename}"),${elapsed},"
    echo -n ${performanceData} >> ${compressionLevelLogFile}
    rm "${compressedFilename}"
  done
  
  ##############################################################################

  # Compress with lzo ##########################################################
  for (( level = 1; level <= 9; level++ ))
  do
    compressedFilename="${file}".lzo
    start="$(date -u +%s.%N)"
    lzop -${level} < "${file}" > "${compressedFilename}"
    end="$(date -u +%s.%N)"
    elapsed=$(echo "$end - $start" | bc)
    performanceData="lzo,${level},$(stat -c%s "${compressedFilename}"),${elapsed},"
    echo -n ${performanceData} >> ${compressionLevelLogFile}
    rm "${compressedFilename}"
  done
  
  ##############################################################################

  # Compress with bzip2 ########################################################

  for (( level = 1; level <= 9; level++ ))
  do
    compressedFilename="${file}".bzip2
    start="$(date -u +%s.%N)"
    bzip2 -k -${level} < "${file}" > "${compressedFilename}"
    end="$(date -u +%s.%N)"
    elapsed=$(echo "$end - $start" | bc)
    performanceData="bzip2,${level},$(stat -c%s "${compressedFilename}"),${elapsed},"
    echo -n ${performanceData} >> ${compressionLevelLogFile}
    rm "${compressedFilename}"
  done
  
  ##############################################################################

  # Compress with lzma #########################################################

  for (( level = 1; level <= 9; level++ ))
  do
    compressedFilename="${file}".lzma
    start="$(date -u +%s.%N)"
    lzma -${level} < "${file}" > "${compressedFilename}"
    end="$(date -u +%s.%N)"
    elapsed=$(echo "$end - $start" | bc)
    performanceData="lzma,${level},$(stat -c%s "${compressedFilename}"),${elapsed},"
    echo -n ${performanceData} >> ${compressionLevelLogFile}
    rm "${compressedFilename}"
  done

  echo "" >> ${compressionLevelLogFile}
  
  ##############################################################################
done

./src/tools/compression_level_study/compression_level_plots.py ${compressionLevelLogFile}