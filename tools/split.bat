#!/bin/bash 2>nul

python tools/split.py --input ./engine/engine.h --output-path ./engine/split/
python tools/split.py --input ./engine/engine.c --output-path ./engine/split/
python tools/split.py --input ./engine/engine   --output-path ./engine/split/
