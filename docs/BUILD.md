# Compilazione

I sorgenti sono quelli estratti dal binario golden, non ricostruiti tramite decompilazione. `src/source.zip` è l’archivio originale incorporato e consente di conservarne la provenienza.

## Windows

Usare MSYS2 con compilatore MinGW x64 `gcc` e `windres` nel PATH. Da un prompt Windows entrare in `src` ed eseguire `build.cmd`. L’output è `src/NDI PLAY.exe`; non sostituisce automaticamente il binario in `dist`.

Prima di una nuova build rigenerare l’archivio delle sorgenti (Python 3):

```bat
python scripts\package_source.py
cd src
build.cmd
```

Eseguire il primo comando dalla radice del repository. Lo script include i sorgenti e le risorse correnti, escludendo l’archivio stesso e i prodotti della compilazione. Non includere media o credenziali.

La compilazione usa libVLC caricato dinamicamente dall’installazione locale: non collega o distribuisce DLL VLC/NDI. Il file `app.rc` incorpora icona, manifest, versione e archivio sorgenti. Aggiornare esplicitamente le informazioni di versione nelle future release.

## Validazione

Una compilazione riuscita non dimostra il funzionamento NDI. Provare su Windows con VLC 3 x64 e plugin ufficiale, controllare canali audio, loop, aggiunta/rimozione individuale, chiusura e carico HD/4K. Solo dopo aggiornare `dist`, checksum e note golden. Non è garantita una compilazione byte-identica con toolchain diversa.
