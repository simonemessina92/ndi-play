# NDI PLAY

A portable Windows x64 player that loops local video files and generates NDI feeds using VLC and the official NDI VLC Plugin.

## 1.0 Gold

Download [NDI PLAY.exe](dist/NDI%20PLAY.exe) using **Download raw file** on its GitHub file page. Published on October 7, 2026.

This release is based on the owner-approved 0.3.3-dev executable. Its version resources and embedded source archive were updated to **1.0 Gold**; executable code remains unchanged. See [SHA-256 checksums](dist/SHA256SUMS.txt).

## Requirements

- Windows x64; the source targets Windows 10 APIs.
- Installed **VLC 3.x, 64-bit**.
- The official NDI VLC Plugin installed in that VLC installation.
- An NDI receiver such as NDI Studio Monitor or vMix, and a network configured for NDI.

The application is a single portable EXE. VLC and the NDI plugin are external dependencies. .NET is not required.

## Usage

1. Drag one or more local videos onto `NDI PLAY.exe`.
2. Select the feed in your NDI receiver.
3. Drop additional files onto the EXE to add sources to the existing tray instance.
4. Right-click the notification-area icon and choose **Remove source** to stop one file, or **Close** to stop all players.

Removing a source does not delete its media file. Playlists are not saved.

Command-line example:

```bat
"NDI PLAY.exe" "C:\Video\clip.mp4"
```

## Audio and looping

The player selects the file's **first audio track**. Its native channel layout is passed to the installed plugin without a stereo downmix imposed by the launcher. A four-channel first track is intended to retain all four channels, subject to decoder and plugin support. Separate audio tracks are not combined.

Each file runs in an independent worker process. The code limit is 256 workers; this is not a guaranteed hardware capacity. Sources loop independently, without synchronization or a guarantee of seamless transitions.

## Repository workflow

- `main`: approved golden version and stable documentation.
- `dev`: development and testing, initially based on the same golden commit.
- `dist/`: golden executable and checksum.
- `src/`: recovered original source, resources, and VLC headers.
- `docs/`: build instructions, technical notes, and validation status.

Develop on `dev`. Promote changes to `main` after Windows and NDI testing. Do not replace the golden executable with an unverified build.

All repository documentation, comments, commit messages, issues, and pull requests must be written in English.

Read the [build instructions](docs/BUILD.md), [technical notes](docs/TECHNICAL.md), and [golden status](docs/GOLDEN.md).

## Licensing

See the original [source license](src/LICENSE.txt). VideoLAN headers retain their own notices and licenses. VLC and the NDI runtime/plugin are not distributed here. This is an independent project; no official affiliation is implied.
