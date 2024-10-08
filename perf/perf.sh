#! /bin/bash

perf record --output ./perf/perf.data --call-graph dwarf ./build/chroma-engine 
