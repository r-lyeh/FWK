#!/bin/bash 2>nul

python tools/join.py --template engine/split/engine.h.inl --path ./engine/split/ --output ./engine/engine.h
python tools/join.py --template engine/split/engine.c.inl --path ./engine/split/ --output ./engine/engine.c
python tools/join.py --template engine/split/engine.x.inl --path ./engine/split/ --output ./engine/engine
