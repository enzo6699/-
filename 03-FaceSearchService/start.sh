#!/bin/bash

set -e

export OMP_NUM_THREADS=1
export OPENBLAS_NUM_THREADS=1
export LD_LIBRARY_PATH="../04-CollectSerivce/lib_faceall"
export FACE_MODEL_PATH="../04-CollectSerivce/models"

./build-cmake-Desktop-Debug/FaceSearchService
#./build/FaceSearchService
