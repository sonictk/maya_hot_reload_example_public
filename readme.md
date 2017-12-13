# Readme

This is the readme for the **Hot Reloadable Deformer** example Maya plugin.

# About

This is an example of how to create a Maya deformer that allows for hot-reloading of
re-compiled code on-the-fly, without having to unload/reload the library and
re-setup the scene.


## Building

### Requirements

* Maya 2016 (and above) needs to be installed in order to link to the libraries.
* You will also need the devkit headers, which are available from the official
  Autodesk Github repository.
* CMake 2.8.11 and higher is required for building.

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

## Developer resources

I use Emacs and clang, so this is mere convenience for me to be able to copy
paste my IDE settings between different machines. If you're using a different
IDE, you can use these as a guide as well.

### Emacs ``.dir-locals.el`` sample setup file

```
(
(nil . ((tab-width . 4)
    (indent-tabs-mode . t)))

(c++-mode . ((c-basic-offset . 4)
       (tab-width . 4)
       (indent-tabs-mode . t)
       (compile-command . "cmake --build . --config Debug --target INSTALL")
       (cd-compile-directory . "C:\\Users\\sonictk\\Git\\experiments\\maya_hot_reload_example\\build")
       (cc-search-directories . ("..\\thirdparty"
                                 "..\\src"
                                 "C:\\Users\\sonictk\\Git\\maya\\thirdparty\\Maya-devkit\\win\\include"
                                 "C:\\Users\\sonictk\\Git\\maya\\thirdparty\\Maya-devkit\\win\\include\\qt-4.8.6-include"
                                 "C:\\Program Files (x86)\\Windows Kits\\10\\include\\10.0.14393.0\\shared"
                                 "C:\\Program Files (x86)\\Windows Kits\\10\\include\\10.0.14393.0\\ucrt"
                                 "C:\\Program Files (x86)\\Windows Kits\\10\\include\\10.0.14393.0\\um"
                                 "C:\\Program Files (x86)\\Windows Kits\\10\\include\\10.0.14393.0\\winrt"))
       ))

(c-mode . ((c-basic-offset . 4)
     (tab-width . 4)
     (indent-tabs-mode . t)
     (compile-command . "cmake --build . --config Debug --target INSTALL")
     (cd-compile-directory . "C:\\Users\\sonictk\\Git\\experiments\\maya_hot_reload_example\\build")
     (cc-search-directories . ("..\\thirdparty"
                               "..\\src"
                                 "C:\\Users\\sonictk\\Git\\maya\\thirdparty\\Maya-devkit\\win\\include"
                                 "C:\\Users\\sonictk\\Git\\maya\\thirdparty\\Maya-devkit\\win\\include\\qt-4.8.6-include"
                                 "C:\\Program Files (x86)\\Windows Kits\\10\\include\\10.0.14393.0\\shared"
                                 "C:\\Program Files (x86)\\Windows Kits\\10\\include\\10.0.14393.0\\ucrt"
                                 "C:\\Program Files (x86)\\Windows Kits\\10\\include\\10.0.14393.0\\um"
                                 "C:\\Program Files (x86)\\Windows Kits\\10\\include\\10.0.14393.0\\winrt"))
     ))
)

```

### ``.clang_complete`` configuration file

#### Windows

```
-std=c++14
-Wall
-Wextra
-pthread
-Wno-pragma-once-outside-header
-Ithirdparty
-Isrc
-Ic:\\Users\\sonictk\\Git\\maya\\thirdparty\\Maya-devkit\\win\\include\\
-Ic:\\Users\\sonictk\\Git\\maya\\thirdparty\\Maya-devkit\\win\\include\\qt-4.8.6-include
-Ic:\\Program Files (x86)\\Windows Kits\\10\\include\\10.0.14393.0\\shared
-Ic:\\Program Files (x86)\\Windows Kits\\10\\include\\10.0.14393.0\\ucrt
-Ic:\\Program Files (x86)\\Windows Kits\\10\\include\\10.0.14393.0\\um
-Ic:\\Program Files (x86)\\Windows Kits\\10\\include\\10.0.14393.0\\winrt
```

#### Linux

```
-std=c++14
-Wall
-Wextra
-pthread
-Wno-pragma-once-outside-header
-Ithirdparty
-Isrc
-I/home/yliangsiew/Git/maya/Maya-devkit/linux/include
-I/opt/rh/devtoolset-7/root/usr/include/c++/7
```

# Credits

Siew Yi Liang (a.k.a **sonictk**)
