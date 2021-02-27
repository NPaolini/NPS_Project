Lo sketch 'mega' è fatto per l'arduino mega2560 con lo shield ethernet, l'altro, 'wemos' per lo 
shield 'wemos' composto da un mega2560 con incorporato un esp8266 (i firmware vanno caricati
separatamente).

Il file 'domotica.ods' indica le periferiche utilizzate ed i contatti impostati.

Nella cartella 'site' ci sono i file da caricare nella SD, cartella 'site' per il sito web.
Si possono caricare anche tramite programma 'domotica.exe' ... ma si fa prima a mano da pc.

IL programma 'domotica.exe', collegato via seriale, permette di leggere/caricare i dati necessari
al funzionamento (poi le altre impostazioni/comandi si possono fare via web).

Nella cartella 'rfid_nps' c'è lo sketch (per arduino uno) per programmare schede rfid che possono
poi essere usate nello sketch 'domotica' per attivazioni/disattivazione allarmi.
La programmazione delle schede avviene tramite programma 'rfid-progr.exe'.

Il circuito 'serrandine' viene usato per evitare che il motore per la serrandina
riceva contemporaneamente i due contatti (su e giù) mandandolo in corto.
Il pulsante di avvio è in parallelo col filo proveniente dall'arduino.
Se viene avviato tramite web es. l'alzata e si preme sul pulsante di discesa, il motore
va in corto cicuito. Anteponendo ai due relè questo circuito si fa si che, se entrambi i 
fili siano ON, l'attivazione sia impedita.
N.B. L'attivazione del Relè (ON) avviene ponendo a massa l'ingresso.
P.S. I file che terminano con *W.pdf sono pcb con piste abbastanza larghe, quelli che terminano
con *MF.pdf sono utilizzabili con le basette millefori.
