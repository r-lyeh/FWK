#!/bin/bash 2>nul

python tools/join.py --template engine/split/fwk.h.inl --path ./engine/split/ --output ./engine/fwk.h
python tools/join.py --template engine/split/fwk.c.inl --path ./engine/split/ --output ./engine/fwk.c
python tools/join.py --template engine/split/fwk.x.inl --path ./engine/split/ --output ./engine/fwk
