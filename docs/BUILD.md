# Building NDI PLAY

The source was recovered from the source archive embedded in the approved executable, rather than reconstructed through decompilation. Version resources and embedded documentation were subsequently updated for 1.0 Gold.

## Windows build

Use MSYS2 with MinGW x64 `gcc` and `windres` available on PATH. From a Windows command prompt, run `build.cmd` inside `src`. The output is `src/NDI PLAY.exe`; it does not automatically replace the golden executable in `dist`.

Before building a changed version, regenerate the embedded source archive using Python 3:

```bat
python scripts\package_source.py
cd src
build.cmd
```

Run the first command from the repository root. The packaging script includes current source files and resources, excluding the archive itself and build outputs. Do not include media or credentials.

The application loads libVLC dynamically from the local installation. It does not distribute VLC or NDI DLLs. `app.rc` embeds the icon, manifest, version information, and source archive. Update version information explicitly for future releases.

The embedded archive represents the source snapshot packaged with the executable. Documentation or tooling-only commits may be newer than that snapshot; regenerate it when building the next executable.

## Validation

A successful compilation does not establish NDI functionality. Test on Windows with VLC 3 x64 and the official plugin. Check audio channels, looping, source addition/removal, shutdown, and HD/4K workloads before updating `dist`, checksums, and golden notes.

Byte-identical builds are not guaranteed across different toolchains.
