#!/bin/bash

set -e

export LD_LIBRARY_PATH="../04-CollectSerivce/lib_faceall"
export FACE_MODEL_PATH="../04-CollectSerivce/models"

./build-cmake-Desktop-Debug/CompareService $@
