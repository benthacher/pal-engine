# PAL Engine

The PAL Engine is a custom game engine meant to run on Palygon, a hexagonal tamagotchi type device. Development and more info on the physical device can be found [here](https://github.com/benthacher/palygon).

The PAL Engine is platform agnostic and is comprised of game logic, an entity system, graphics and sound library, and a physics engine. To build a project with the engine, a backend file has to be created for the PAL (Palygon abstraction layer) to operate properly and interact with hardware. In the example directory, this file is called `pal_backend.c`, but no naming convention is enforced. The example directory's `CMakeLists.txt` has more information about variables to set for building to work.