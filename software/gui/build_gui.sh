#!/bin/bash

DIR=$(dirname "$0")

cmake -DCMAKE_BUILD_TYPE=Release -S $DIR -B $DIR/build
cmake --build $DIR/build

