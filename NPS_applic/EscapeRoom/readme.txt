Applicazione nata per gestire un gioco di sala: Escape Room.

	Lo scopo è di visualizzare, in uno schermo secondario, informazioni utili ai contendenti del gioco
	rinchiusi nella stanza.
	Si può attivare una musica di sottofondo, scrivere informazioni sullo schermo, notificare, tramite
	un suono, che ci sono nuove informazioni, scegliere il timer, ecc.
	Nel setup si scelgono cartella contenente immagini di sfondo da cui scegliere, cartella di musiche
	di sottofondo, cartella di suoni di notifica con la rispettiva durata ed il timer generale per la durata
	del gioco.
	IL pulsante 'Move' sposta la schermata di visualizzazione dallo schermo principale a quello secondario
	(impostato come estendi) e viceversa. 'Full screen' lo manda a tutto schermo. 'Sound' avvia la musica 
	di sottofondo, lo slider ne imposta il volume. 'Alert' emette il suono di notifica. 'Reset timer' reimposta
	il timer all'inizio. 'Pause/Resume' ferma e fa ripartire il timer.
	'Tips' apre la finestra dei suggerimenti. Qui si possono aggiungere/cancellare/modificare i suggerimenti
	presenti. Si può impostare il font ed il colore e scegliere se avere, per il testo, un bordo nero, uno 
	bianco più uno nero o nessuno. Si può anche aggiungere una immagine (centrata) sopra o sotto il testo e
	la sua percentuale in altezza rispetto allo spazio del blocco inferiore.
	Nelle tre finestre a tendina si possono scegliere, in ordine, sottofondo musicale, suono di notifica e 
	suggerimento da visualizzare. Per cambiare, aggiungere un suggerimento non è necessario aprire la sua
	finestra di dialogo, basta scriverlo nel campo di edit della tendina e premendo invio o il pulsante alla
	sua destra andrà a sostituire quello presente.
	Se si scrive a mano occorre mettere un '\n' per forzare un accapo.
	La barra che c'è tra l'orario del timer e la parte dei suggerimenti può essere spostata per ingrandire 
	una parte o l'altra. 
	I numeri che compongono il timer sono presi dalla cartella 'led'. Se non ci sono verranno usati dei 
	bitmap interni (bassa risoluzione, si vedranno sgranati). Se si vogliono sostituire, basta spostare
	quelli già presenti in un'altra cartella e mettercene di nuovi avendo l'accortezza di usare il formato
	.png e di chiamarli con lo stesso nome.
	Nella parte destra ci sono le immagini di sfondo da cui selezionare quella che sarà attiva.
