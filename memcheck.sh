#! /bin/bash

cmake -S . -B build
cmake --build build/
G_DEBUG=resident-modules valgrind --tool=memcheck --leak-check=full --log-file=./log/valgrind.log --suppressions=/usr/share/gtk-3.0/valgrind/gtk.supp --suppressions=/usr/share/glib-2.0/valgrind/glib.supp ./build/chroma-engine
