#!/bin/bash

set -e

for file in *.cpp *.h; do
    clang-format-7 -i $file
done
