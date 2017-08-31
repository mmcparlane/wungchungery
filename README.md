Wungchungery Game Engine
========================

TODO: Add objectives.


Build Steps
-----------

### Windows

#### Native

1. Install [Visual Studio Build Tools][1].
2. Open a Developer Command Pprompt.
    >Start | All Programs | Visual Studio 2017 | Visual Studio Tools | *Developer Command Prompt for VS 2017*

3. `cd wungchungery & nmake /f Makefile.Win32`
4. Output executables should be in builds directory.

#### Emscripten

1. Install the [Emscripten SDK][2] for Windows.
2. Follow the [instructions][3] to get the SDK running.
3. Launch `emcmdprompt.bat` by double-clicking.
4. Set up Visual Studio developer environment variables (for nmake).

    `"C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\Common7\Tools\VsDevCmd.bat"`

5. TODO

[1]: https://www.visualstudio.com/downloads/#build-tools-for-visual-studio-2017
[2]: https://kripken.github.io/emscripten-site/docs/getting_started/downloads.html
[3]: https://kripken.github.io/emscripten-site/docs/getting_started/downloads.html#sdk-installation-instructions