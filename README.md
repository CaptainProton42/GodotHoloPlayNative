# GDNative source for HoloPlay for Godot 3.x

This is the GDNative source for the HoloPlay for Godot 3.x plugin.

If you're just looking for the plugin, visit this repository: https://github.com/CaptainProton42/GodotHoloPlay

## Building from Source

Clone the repository and initialise all submodules:
```
git clone https://github.com/CaptainProton42/GodotHoloPlayNative
cd GodotHoloPlayNative
git submodule update --init --recursive
```

Compile the C++ bindings:

```
cd godot-cpp
scons platform=windows generate_bindings=yes -j4
cd ..
```

Use [Scons](https://scons.org/) scons to build the DLL:

```
scons platform=windows
```

Note that currently only Windows is supported. I used MSVC v142 for compilation, you might need to download the correct GLFW static libraries when using another compiler.

After compilation, the library will be placed at `build/libgdholoplay.dll` (you might need to create the directory before compiling). Clone the [plugin repository](https://github.com/CaptainProton42/GodotHoloPlay) and place the `.dll` file in `addons/holoplay/bin/win64` to test your changes.

## License

The plugin source is available under the [MIT license](LICENSE.md).

This plugin was built using the *HoloPlay Core SDK* which is distributed by from Looking Glass Factory under a separate [license](HoloPlayCore/LICENSE.md) (the included header files are available under the MIT license).

Glad is available under the [MIT license](glad/LICENSE.md).

GLFW is available under the [zlib/libpng license](glfw/LICENSE.md).
