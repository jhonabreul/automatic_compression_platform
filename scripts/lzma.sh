#!/bin/bash

# compress #####################################################################

compress()
{
  inFile=${1}
  outFile=${2}
  compressionLevel=${3}

  if [ -z "$compressionLevel" ]
  then
    lzma < ${inFile} > ${outFile}
  else
    lzma -${compressionLevel} < ${inFile} > ${outFile}
  fi  
}

################################################################################

# decompress ###################################################################

decompress()
{
  inFile=${1}
  outFile=${2}

  lzma -d < ${inFile} > ${outFile}
  rm ${inFile}
}

################################################################################

# main #########################################################################

while getopts ":d" option; do
  case "${option}" in
    d)
      decompress=true
      ;;

    *)
      echo "Invalid option: -$OPTARG" >&2
      exit -1
      ;;
  esac
done

shift $((OPTIND-1))

inFile=${1}
outFile=${2}
compressionLevel=${3}

if [ "$decompress" = true ]
then
  decompress ${inFile} ${outFile}
else
  compress ${inFile} ${outFile} ${compressionLevel}
fi

exit 0

################################################################################