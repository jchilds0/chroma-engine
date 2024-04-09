# chroma-engine
Graphics engine for chroma viz built using [GTK][gtk] (v3) and Open GL w/ [GLEW][glew].

## Features

[Chroma Hub][chroma-viz] sends a list of templates to Chroma Engine on startup in order to synchronize templates between Chroma Engine and [Chroma Viz][chroma-viz] instances.
Communicates with [Chroma Viz][chroma-viz] over tcp to render graphics.
Runs as either a standalone window or embedded in [Chroma Viz][chroma-viz] using gtk plug

![Chroma_Engine](data/chroma-engine.png)
![Chroma_Engine](data/chroma-viz.png)

## Build from Source

- Requires a C compiler, `cmake`, `gtk3`, `glew`, `freetype2` and `libpng`. 
    Current versions: `gtk3 3.24.41-1`, `glew 2.2.0-6`, `freetype2 2.13.2-1` and `libpng 1.6.43-1`.
- Clone the git repo
```
git clone https://github.com/jchilds0/chroma-engine
cd chroma-engine
```
- Update the C Macro `INSTALL_DIR` in `src/chroma-engine.h` to the absolute path of `chroma-engine`
- Build chroma-engine using cmake 
```
cmake -S . -B build/
cmake --build build/
```

## Disclaimer

This is a personal project, not an application intended for production.

[gtk]: https://github.com/GNOME/gtk
[glew]: https://github.com/nigels-com/glew
[chroma-viz]: https://github.com/jchilds0/chroma-viz
