#!/bin/bash

echo "========================================================================="
echo " Data: zero"
echo " ----------"
echo

./src/tools/evaluation/evaluate.sh 192.168.0.117 mserver 192.168.0.112 /home/mserver/Documents/jhonathan/AutomaticCompressionPlatform /home/mserver/Documents/jhonathan/test_data/zero ../../tmp enp3s0 10
rm -rf ../../tmp
cp -p "`ls -dtr1 ./log/autocompCompressorEvaluationData* | tail -1`" ./src/tools/evaluation/log
sleep 5

echo "========================================================================="
echo
echo

echo "========================================================================="
echo " Data: code"
echo " ----------"
echo

./src/tools/evaluation/evaluate.sh 192.168.0.117 mserver 192.168.0.112 /home/mserver/Documents/jhonathan/AutomaticCompressionPlatform /home/mserver/Documents/jhonathan/test_data/code ../../tmp enp3s0 10
rm -rf ../../tmp
cp -p "`ls -dtr1 ./log/autocompCompressorEvaluationData* | tail -1`" ./src/tools/evaluation/log
sleep 5

echo "========================================================================="
echo
echo

echo "========================================================================="
echo " Data: latex"
echo " ----------"
echo

./src/tools/evaluation/evaluate.sh 192.168.0.117 mserver 192.168.0.112 /home/mserver/Documents/jhonathan/AutomaticCompressionPlatform /home/mserver/Documents/jhonathan/test_data/latex ../../tmp enp3s0 10
rm -rf ../../tmp
cp -p "`ls -dtr1 ./log/autocompCompressorEvaluationData* | tail -1`" ./src/tools/evaluation/log
sleep 5

echo "========================================================================="
echo
echo

echo "========================================================================="
echo " Data: media"
echo " ----------"
echo

./src/tools/evaluation/evaluate.sh 192.168.0.117 mserver 192.168.0.112 /home/mserver/Documents/jhonathan/AutomaticCompressionPlatform /home/mserver/Documents/jhonathan/test_data/media ../../tmp enp3s0 5
rm -rf ../../tmp
cp -p "`ls -dtr1 ./log/autocompCompressorEvaluationData* | tail -1`" ./src/tools/evaluation/log
sleep 5

echo "========================================================================="
echo
echo

echo "========================================================================="
echo " Data: random"
echo " ----------"
echo

./src/tools/evaluation/evaluate.sh 192.168.0.117 mserver 192.168.0.112 /home/mserver/Documents/jhonathan/AutomaticCompressionPlatform /home/mserver/Documents/jhonathan/test_data/random ../../tmp enp3s0 5
rm -rf ../../tmp
cp -p "`ls -dtr1 ./log/autocompCompressorEvaluationData* | tail -1`" ./src/tools/evaluation/log
sleep 5

echo "========================================================================="
echo
echo