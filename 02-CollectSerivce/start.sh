#!/bin/bash

set -e

#export OMP_NUM_THREADS=1
#export OPENBLAS_NUM_THREADS=1
export LD_LIBRARY_PATH="./lib:./lib/HCNetSDKCom"
export FACE_MODEL_PATH="./models"

#gdb ./build-cmake-Desktop-Debug/CollectService
./build-cmake-Desktop-Debug/CollectService $@
