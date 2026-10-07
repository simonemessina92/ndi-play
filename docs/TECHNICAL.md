# Architettura e diagnosi

Il launcher C/Win32 gestisce una sola icona tray e inoltra nuovi file all’istanza esistente. Per ciascun video avvia una copia dello stesso EXE con argomenti interni `--worker`. Solo i worker caricano `libvlc.dll` e i plugin: un crash nativo del player è isolato dal processo tray.

La ricerca di VLC usa registro Windows e percorso standard Program Files/VideoLAN/VLC. Il codice accetta solo la versione principale 3. Le impostazioni salvate di VLC vengono ignorate. Output video/audio NDI, prima traccia audio, cache dei plugin abilitata e scansione plugin abilitata. Non c’è una configurazione esplicita dell’accelerazione hardware nel launcher.

Ripetizione mediante `:input-repeat=2147483647` e tentativo di riavvio al termine. Un evento Windows richiede l’arresto individuale; dopo tre secondi un worker che non risponde viene terminato. Un Job Object raggruppa i processi per la chiusura.

## Task Manager

Cercare `NDI PLAY.exe` nella scheda **Dettagli**: un processo tray più un processo dello stesso nome per ogni file. Non cercare necessariamente `vlc.exe`, perché VLC viene caricato come libreria. La colonna Riga di comando permette di distinguere i worker `--worker`.

## Log

Cartelle `%TEMP%\NDI-PLAY-*`, con `player-N.log` e, in caso di errore, `player-N.error`. Gli errori mostrano il percorso diagnostico. Il logger limita la scrittura dei messaggi VLC intorno a 512 KiB per worker.

- VLC non trovato: controllare installazione 3.x x64 e percorso/registro.
- Output audio NDI mancante: verificare o reinstallare il plugin nell’installazione VLC usata.
- Sorgente non visibile: verificare prima log e funzionamento plugin, poi ricevitore, rete e firewall Windows.
- Carico elevato: ogni file comporta decoding e output NDI propri; misurare CPU, GPU, memoria e rete con il carico effettivo.
- Avvio lento: cache e scansione sono abilitate; non esiste una promessa di eliminazione del ritardo.

Estrazione sorgenti su Windows: `"NDI PLAY.exe" --extract-source source.zip`. Il comando non avvia playback.
