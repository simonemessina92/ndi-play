# 1.0 Gold — October 7, 2026

[Download NDI PLAY.exe — Windows x64](https://github.com/simonemessina92/ndi-play/raw/b844eebf9483daecaf46f02e1a9e61478afe39c9/dist/NDI%20PLAY.exe)

- File: `dist/NDI PLAY.exe`
- Size: 816795 bytes
- Embedded version: `1.0 Gold`
- SHA-256: `e36f15fe880d1252522000efe2653d18bc92f466f65df7fd324ce0eb9fdf677d`

Publication updated version resources, the embedded source archive, and the PE checksum without recompiling or changing executable code. The original executable's hash is recorded in [ORIGINAL_SHA256.txt](ORIGINAL_SHA256.txt).

Publication checks covered Windows x64 PE format, embedded ZIP integrity, source/resource extraction, unchanged executable code, and checksum generation. This packaging update did not include another Windows/NDI playback test or a recompilation.

The original verification notes record four-channel audio reception in vMix and NDI Analysis. Version 0.3.3 enabled plugin caching compared with 0.3.2 while retaining scanning. This publication does not certify startup timing or HD/4K capacity.

Before promoting a new golden, test one source, a multichannel first audio track, multiple sources, looping, individual removal, subsequent additions, clean shutdown with no remaining workers, and HD/4K workloads on the target PC. Record VLC/plugin versions, hardware, and results before merging dev into main.

Subsequent English documentation updates do not change this executable or its packaged source snapshot.

