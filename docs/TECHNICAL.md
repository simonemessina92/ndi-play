# Architecture and troubleshooting

The C/Win32 launcher manages a single tray instance and forwards subsequent file drops to it. Each video starts another copy of the executable with internal `--worker` arguments. Only workers load libVLC and its plugins, isolating native player crashes from the tray process.

VLC is located through the Windows registry or the standard Program Files/VideoLAN/VLC directory. Only VLC major version 3 is accepted. Saved VLC preferences are ignored.

The launcher requests NDI video/audio output and the first audio track. Both plugin caching and plugin scanning are enabled. The launcher does not explicitly configure hardware acceleration.

Looping uses `:input-repeat=2147483647` and a restart attempt when playback ends. A Windows event requests individual shutdown; a worker that does not exit within three seconds is terminated. A Job Object groups owned processes for shutdown.

## Task Manager

Look for `NDI PLAY.exe` in **Details**: one tray process and one additional process for each file. VLC runs as a library, so a separate `vlc.exe` process is not expected. Enable the Command line column to identify `--worker` processes.

## Logs

Diagnostic folders are created under `%TEMP%\NDI-PLAY-*`, containing `player-N.log` and, on failure, `player-N.error`. Error dialogs show the diagnostic path. VLC log messages are capped at approximately 512 KiB per worker.

- **VLC not found:** check the VLC 3.x x64 installation and its path/registry entries.
- **NDI audio output missing:** check or reinstall the plugin in the VLC installation being used.
- **Source not visible:** check logs and plugin operation, then the receiver, network, and Windows firewall.
- **High resource usage:** each file has its own decoding and NDI output workload. Measure CPU, GPU, memory, and network load with the intended media.
- **Slow startup:** plugin caching and scanning are enabled; elimination of startup delay is not guaranteed.

Extract the embedded source archive on Windows:

```bat
"NDI PLAY.exe" --extract-source source.zip
```

This command does not start playback.
