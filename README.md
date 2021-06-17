### ⚠️ Test Release ⚠️
### This plugin does *NOT* currently work. I'm publishing it here so people can test it. Please describe any issues you're experiencing as detailed as possible by opening an issue [here](https://github.com/CaptainProton42/GodotHoloPlay/issues)! Thanks! ⚠️

### Current state:

* Quilt rendering seems to work (you can enable direct output of the quilt by enabling the `Dummy` and `Debug View` properties of `HoloPlayVolume`.
* Output on the Looking Glass itself does *not* seem to work at the moment.
* I added some debugging output to the console that should display the properties of your Looking Glass Portrait. Please include it in your issue reports.

***Report all issues [here](https://github.com/CaptainProton42/GodotHoloPlay/issues)!***

# HoloPlay for Godot 3.x GDNative Source

The jHoloPlay plugin adds suport for holographics displays made by [Looking Glass Factory](https://lookingglassfactory.com/) to the [Godot Engine](https://godotengine.org/).

**This is the source of the `libgdholoplay.dll` GDNative library, the complete plugin source including the compiled library can be found at https://github.com/CaptainProton42/GodotHoloPlay.**

## Building from Source

The library can be built using [Scons](https://scons.org/) with the command `scons platform=windows`.

Note that currently only Windows is supported. I used MSVC v142 for compilation, you might need to download the correct GLFW static libraries when using another compiler.

After compilation, the library will be placed at `build/libgdholoplay.dll` (you might need to create the directory before compiling). Clone the [plugin repository](https://github.com/CaptainProton42/GodotHoloPlay) and place the `.dll` file in `addons/holoplay/bin/win64` to test your changes.

## License

The plugin source is available under the [MIT license](LICENSE.md).

This plugin was built using the *HoloPlay Core SDK* which is distributed by from Looking Glass Factory under a separate [license](HoloPlayCore/LICENSE.md) (the included header files are available under the MIT license).

Glad is available under the [MIT license](glad/LICENSE.md).

GLFW is available under the [zlib/libpng license](glfw/LICENSE.md).
