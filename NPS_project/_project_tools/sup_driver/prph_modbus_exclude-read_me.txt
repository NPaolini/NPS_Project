Nella configurazione del driver tramite NPS_sVisor (CTRL+1) o tramite il programma di
configurazione off-line (svConfig-V7), occorre mettere nel campo di edit posizionato
alla destra dei parametri di comunicazione il numero di word max che � possibile
leggere/scrivere in un blocco.

Se si legge uno/pi� valori a 32bit, per default, viene eseguito lo swab a 32 bit, cio�
AABBCCDD diventa DDCCBBAA. Se si vuole che indipendentemente dalla dimensione del dato
lo swab sia sempre a 16 bit, es. AABBCCDD diventa BBAADDCC, occorre mettere il valore di
word max come negativo, es. -50. Se non ci sono valori a 32bit � ininfluente.

Il driver usa la scrittura multipla, se il dispositivo implementa solo quella singola �
possibile, tramite il programma di gestione periferiche, indicarlo al driver mettendo
nel valore del db l'id del dispositivo + 256,
es. id 2: scrittura multipla -> 2, scrittura singola -> 258
La lettura � sempre multipla.
