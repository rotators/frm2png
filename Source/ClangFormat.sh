#!/bin/bash

set -e

for file in *.cpp *.h; do
    clang-format-7 -i $file
done

for dir in Frm Pal; do
    for file in libfalltergeist-mini/Format/$dir/*.cpp libfalltergeist-mini/Format/$dir/*.h; do
        clang-format-7 -i $file
    done
done
