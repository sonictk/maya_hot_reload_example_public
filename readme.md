# Readme

This is the readme for the **Hot Reloadable Deformer** example Maya plugin.

# About

This is an example of how to create a Maya deformer that allows for hot-reloading of
re-compiled code on-the-fly, without having to unload/reload the library and
re-setup the scene. 

You can view the full tutorial over [here](https://sonictk.github.io/maya_hot_reload_example_public/)


## Building

### Requirements

* Maya 2016 (and above) needs to be installed in order to link to the libraries.
* You will also need the devkit headers, which are available from the official
  Autodesk Github repository.
* CMake 2.8.11 and higher is required for building.
* If you're on Windows, you can also use the ``build.bat`` file to build the plugin. Edit 
  the build script to point the paths to your own Maya directories, and then run the build using 
  ``build.bat debug``. It will place the generated binaries in a ``msbuild`` folder.

### Instructions for building with CMake

* Create a ``build`` directory and navigate to it.
* Run ``cmake ../ -DCMAKE_BUILD_TYPE=Release`` and then ``cmake --build .
  --config Release --target install``. The binaries should be installed in the
  ``bin`` folder. You can switch to a ``Debug`` build if you're trying to look at
  how the plugin works internally and step through the code in a debugger.

## Sample code

In MEL:

```
file -f -new;

unloadPlugin "maya_hot_reload_example";
loadPlugin "/home/sonictk/maya_hot_reload_example/bin/maya_hot_reload_example.so";

polySphere;
deformer -type "hotReloadableDeformer";
```

# Credits

Siew Yi Liang (a.k.a **sonictk**)
