# chroma-engine
Graphics engine for chroma viz built using [GTK][gtk] (v3) and Open GL w/ [GLEW][glew].

## Features

[Chroma Hub][chroma-viz] sends a list of templates to Chroma Engine on startup in order to synchronize templates between Chroma Engine and [Chroma Viz][chroma-viz] instances.
Communicates with [Chroma Viz][chroma-viz] over tcp to render graphics.
Build executable to run Chroma Engine as a standalone window, or embed as a GtkGLRender in other GTK applications using the static library.

![Chroma_Engine](data/chroma-engine.png)
![Chroma_Engine](data/chroma-viz.png)

## Build from Source

- Requires a C compiler, `cmake`, `gtk3`, `glew`, `freetype2` and `libpng`. 
- Clone the git repo
```
git clone https://github.com/jchilds0/chroma-engine
cd chroma-engine
```
- Build chroma-engine using cmake 
```
cmake -S . -B build/
cmake --build build/
```
- Run chroma-engine
```
./build/chroma-engine -c config/config.toml
```

## Disclaimer

This is a personal project, not an application intended for production.

[gtk]: https://github.com/GNOME/gtk
[glew]: https://github.com/nigels-com/glew
[chroma-viz]: https://github.com/jchilds0/chroma-viz
