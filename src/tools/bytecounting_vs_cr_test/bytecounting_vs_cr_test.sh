#/bin/bash

command -v gzip >/dev/null 2>&1 || { echo >&2 "gzip is required but it's not installed. Aborting."; exit 1; }
#command -v lzop >/dev/null 2>&1 || { echo >&2 "lzop is required but it's not installed. Aborting."; exit 1; }
command -v bzip2 >/dev/null 2>&1 || { echo >&2 "bzip2 is required but it's not installed. Aborting."; exit 1; }
#command -v lzma >/dev/null 2>&1 || { echo >&2 "lzma is required but it's not installed. Aborting."; exit 1; }
command -v python3 >/dev/null 2>&1 || { echo >&2 "Python is required but it's not installed. Aborting."; exit 1; }
test "$(python3 -c 'import sys; print(sys.version_info[0])')" != "3" && { echo "Python 3 is required to run this script. Aborting"; exit 1; }

N_ARGUMENTS=1

if [ $# -ne ${N_ARGUMENTS} ]; then
  echo >&2 "Invalid number of arguments: $# were given but ${N_ARGUMENTS} are required"
  exit 1
fi

# ${1%/} removes last '/' to $1, if it has it
TEST_DIRECTORY=${1%/}

timestamp=$(date +"%Y%m%d-%H%M%S")
testDirectoryName=$(basename ${TEST_DIRECTORY})
compressionRatioLogFile=./log/compressionRatio_${testDirectoryName}_${timestamp}.log
touch ${compressionRatioLogFile}

if [ ! -e ${TEST_DIRECTORY} ] || [ ! -d ${TEST_DIRECTORY} ]; then
  echo "${TEST_DIRECTORY} doesn't exist or is not a directory"
  exit 1
fi

for file in ${TEST_DIRECTORY}/*; do
  if [ ! -f "${file}" ]; then
    continue
  fi

  echo -n "${file},$(stat -c%s "${file}")," >> ${compressionRatioLogFile}

  # Compress with gzip #########################################################

  compressedFilename="${file}".gz
  gzip < "${file}" > "${compressedFilename}"
  performanceData="zlib,$(stat -c%s "${compressedFilename}"),"
  echo -n ${performanceData} >> ${compressionRatioLogFile}
  rm "${compressedFilename}"
  
  ##############################################################################

  # Compress with lzo ##########################################################

  #compressedFilename="${file}".lzo
  #lzop < "${file}" > "${compressedFilename}"
  #performanceData="lzo,$(stat -c%s "${compressedFilename}"),"
  #echo -n ${performanceData} >> ${compressionRatioLogFile}
  #rm "${compressedFilename}"
  
  ##############################################################################

  # Compress with bzip2 ########################################################

  compressedFilename="${file}".bzip2
  bzip2 -k < "${file}" > "${compressedFilename}"
  performanceData="bzip2,$(stat -c%s "${compressedFilename}")"
  echo ${performanceData} >> ${compressionRatioLogFile}
  rm "${compressedFilename}"
  
  ##############################################################################

  # Compress with lzma #########################################################

  #compressedFilename="${file}".lzma
  #lzma < "${file}" > "${compressedFilename}"
  #performanceData="lzma,$(stat -c%s "${compressedFilename}")"
  #echo ${performanceData} >> ${compressionRatioLogFile}
  #rm "${compressedFilename}"
  
  ##############################################################################
done

./src/tools/bytecounting_vs_cr_test/bytecounting_stats.py ${compressionRatioLogFile}