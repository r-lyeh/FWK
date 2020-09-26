#!/bin/bash 2>nul

python art/tools/split.py --input ./fwk.h --output-path ./
python art/tools/split.py --input ./fwk.c --output-path ./
