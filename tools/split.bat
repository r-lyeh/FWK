#!/bin/bash 2>nul

python tools/split.py --input ./engine/fwk.h --output-path ./engine/split/
python tools/split.py --input ./engine/fwk.c --output-path ./engine/split/
python tools/split.py --input ./engine/fwk   --output-path ./engine/split/
