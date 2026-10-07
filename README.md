# NDI PLAY

Player portable per Windows x64 che riproduce file video locali in loop e genera feed NDI utilizzando VLC e il plugin NDI ufficiale.

## Golden

La versione consegnata e approvata da Simone è conservata **con metadati aggiornati a 1.0 Gold** in [`dist/NDI PLAY.exe`](dist/NDI%20PLAY.exe). Scaricare con il pulsante **Download raw file** nella pagina del file.

Il binario riporta `1.0 Gold` nelle proprietà Windows. Deriva dalla golden approvata 0.3.3-dev: codice eseguibile invariato, risorse di versione e archivio sorgenti aggiornati. Data di archiviazione: 7 ottobre 2026. Hash SHA-256 in [`dist/SHA256SUMS.txt`](dist/SHA256SUMS.txt).

## Requisiti e utilizzo

- Windows x64; sorgente impostato per API Windows 10.
- VLC **3.x a 64 bit**, installato sul PC.
- Plugin NDI ufficiale per VLC, installato nella stessa installazione VLC x64.
- Un ricevitore NDI, per esempio NDI Studio Monitor o vMix, e rete configurata per NDI.

Il singolo EXE è portable; VLC e il plugin restano dipendenze esterne. Non richiede .NET.

1. Trascinare uno o più video locali su `NDI PLAY.exe`.
2. Selezionare il feed nel ricevitore NDI.
3. Trascinare altri file sull’EXE per aggiungere sorgenti alla stessa istanza.
4. Clic destro sull’icona nell’area di notifica → **Remove source** per fermare un solo file, oppure **Close** per chiudere tutto.

La rimozione della sorgente non cancella il video. Non viene salvata una playlist. Avvio da terminale: `"NDI PLAY.exe" "C:\Video\clip.mp4"`.

## Audio e loop

Viene selezionata la **prima traccia audio** del file. I canali di quella traccia sono affidati al plugin senza un downmix stereo imposto dal launcher: una prima traccia con quattro canali deve arrivare con quattro canali, compatibilmente con decoder e plugin. Non vengono unite tracce audio separate.

Ogni file ha un processo worker indipendente; il limite nel codice è 256, non una capacità garantita del PC. I file ripetono autonomamente: non c’è sincronizzazione tra sorgenti né una garanzia di loop senza interruzioni.

## Repository

- `main`: golden e documentazione stabile.
- `dev`: sviluppo e prove; parte dalla stessa golden.
- `dist/`: eseguibile golden e checksum.
- `src/`: sorgenti originali recuperati dall’archivio incorporato nell’EXE, risorse e header VLC.
- `docs/`: note tecniche, compilazione e verifiche.

Le prossime modifiche si fanno su `dev`; dopo il test Windows e NDI si promuovono su `main`. Non sovrascrivere la golden con build non verificate.

Documentazione: [compilazione](docs/BUILD.md), [architettura e diagnosi](docs/TECHNICAL.md), [stato golden](docs/GOLDEN.md). Licenza originale: [src/LICENSE.txt](src/LICENSE.txt). Gli header VideoLAN conservano i propri avvisi e licenze; VLC e il runtime/plugin NDI non sono distribuiti qui. Progetto indipendente; nessuna affiliazione ufficiale implicita.
