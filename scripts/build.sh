#!/bin/bash

SCRIPT_DIR=$(dirname "$0")

WORK_DIR=${SCRIPT_DIR}/../

mkdir ${WORK_DIR}/build

cd ${WORK_DIR}/build

cmake ..

make -j4