Utilizzo programmatore di schede RFID.

	Tramite setup si sceglie la comunicazione seriale, l'intestazione 'NPS' e la versione '001'.
	La master card � una scheda che viene usata per entrare in modalit� programmazione.
	Se non c'� una master memorizzata internamente all'arduino, la prima scheda avvicinata al
	lettore diventer� master. In modalit� programmazione ogni scheda successiva avvicinata avr� 
	il codice numerico (max 10 cifre) scritto nella finestra di sinistra (un codice per ogni riga).
	Se si riavvicina la master si esce dalla programmazione e si rientra in lettura.
	Se si vuole programmare anche la scheda usata come master, basta cliccare sul pulsante
	'Reset Master Card' ed avvicinare un'altra card che diventer� la nuova master. A quel punto la
	vecchia master ridiventa una scheda normale programmabile con la stessa procedura.
