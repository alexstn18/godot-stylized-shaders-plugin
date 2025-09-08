# godot-stylized-shaders-plugin

This is a **Godot 4.4 plugin** made with *GDExtension C++* as part of a *university project*. This plugin aims to provide a variety of **stylized shaders** that the user can customize to their liking, being able to **layer/stack** the shaders and **change the order of appliance** (to change the overall look).

# Important Disclaimer

This plugin is actively being worked on!!! (development started from the 8th of September 2025 and is supposed to end around the 30th of October 2025)
Please ignore this repository at least until I actually make a release, thank you 😅

More to come later...

# Compiling the plugin yourself

Requirements:
- a version of Python and pip
- SCons (`pip install scons`) & SCons being declared in your PATH [("how-to" here)](https://stackoverflow.com/a/63925889)
- Godot 4.4+

Commands for cloning the repository (with the `godot-cpp` submodule included) and building the plugin with **SCons**:
```
git clone --recursive https://github.com/alexstn18/godot-stylized-shaders-plugin/
cd godot-stylized-shaders-plugin/
scons
```

The build is currently being outputted to `demo/addons/`, but you can easily change this in the `SCons` configuration file (`folder = your_path_here`)
