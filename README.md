# InteraccionHumanoComputador: Whac-a-Mole with Leap Motion

Final project for the Human–Computer Interaction course (Interacción Humano-Computador, 2018).
It is a 3D *Whac-a-Mole* (moles are "topos"; the mole is a Diglett model) rendered with
OpenGL/GLUT. You play with your bare hands, tracked by a **Leap Motion** controller.
Several players can share one game: a small TCP server keeps track of which moles are up,
and each client connects to it.

[![Demo video](previus.png)](https://youtu.be/I3TlLthCp1Q)

*Click for the demo video (also in the repository as `IHCFinal.mp4`).*

Tips from the original notes: keep your hand 15–30 cm in front of the screen, in natural light.

## Build

| Target | Needs | Status on Linux (checked 2026-11) |
|---|---|---|
| `make server` | C++11, pthreads | builds and runs |
| `make tortuga`, `make smooth` | freeglut | build and run (the lighting exercise in `iluminandoTortuga/`) |
| `make check-client` | freeglut headers | `client.cc` and `main2.cc` compile |
| `make client LEAP_SDK=...` | **Leap Motion SDK v2.3.1** (`libLeap.so`), freeglut, libpng | **not verified**: the SDK is proprietary, discontinued, and not in this repository |

```sh
sudo apt install freeglut3-dev libpng-dev
make                                   # server, tortuga, smooth, compile-check of the clients
make client LEAP_SDK=$HOME/LeapSDK     # only with the Leap SDK installed
```

On macOS the Makefile links the GLUT/OpenGL frameworks. The original macOS steps,
including the `install_name_tool` fix for `libLeap.dylib`, are in `compile.sh`.

## Run

```sh
./server
# Port 1200: connection success        <- tries 1200, 1201, ... until one binds
```

```sh
./client        # asks for the port, then connects to the server
```

**The client connects to the hard-coded address `172.20.10.2`** (`client.cc`, `main`),
which was the server's address on the LAN used for the demo. To run it elsewhere, change
that address.

`main2.cc` is an earlier single-player version of the game.

## Why the repository is 57 MB

| Object | Size |
|---|---|
| `IHCFinal.mp4`, the demo video | 51 MB |
| `AWARD_ODUL_OBJ.obj`, a model deleted later but still in history | 11 MB |
| `11538_Hand_v2.obj`, hand model | 6 MB |

The rest (code, textures, small models) is under 3 MB. The history was not rewritten.
The video could move to a release asset or to YouTube only.

## Layout

| Path | Contents |
|---|---|
| `client.cc` | game client: GLUT scene, menu, game loop, Leap input, server connection |
| `server.cpp`, `utilidad.h` | multiplayer TCP server |
| `Leap/` | Leap Motion SDK headers and the hand-tracking listener |
| `Model/` | OBJ/material/mesh loading (`ParseObj/obj.h`) |
| `primitivas.h`, `light.h`, `camara.h`, `Vector_tools.h`, `png_texture.h` | drawing helpers, lights, camera, texture loading |
| `*.obj`, `*.mtl`, `*.png`, `*.jpg` | models and textures (Diglett, mole hole, trophy, sun, stars) |
| `iluminandoTortuga/` | separate lighting exercise (turtle lit by directional/point/spot lights) |
