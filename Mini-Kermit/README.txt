Bina Marius Andrei - bina.marius@gmail.com

Programul incepe prin constructia pachetului de initializare a conexiunii (INIT),
acest pachet se stabilesc timpul de timeout si lungimea maxima a fisierului.
Pachetul se trimite asemenator cu modul in care se trimit si restul pachetelor,
procesul de trimitere al unui pachet este urmatorul : Pachetul se construieste
cu campurile SOH, TYPE, etc. In afara de pachetele ACK si NAK care sunt
construite de la inceput si li se modifica doar numarul de secventa folosind
funcita change_msg_seq (Functia change_msg_seq la receiver doar schimba numarul
de secventa, iar la sender pe langa schimbarea secventei se recaluleaza si CRC-ul,
asta deoarece canalul de conexiune de la receiver la sender nu se corupe). Dupa
trimiterea unui pachet se astepta raspuns de la receiver ACK sau NAK, daca pachetul
ajunge la receiver acesta verifica CRC-ul, numarul de secventa si daca ceva nu e
in regula se retrimite ultimul pachet pentru a anunta senderul de problema, daca nu
ajunge niciun pachet se da timeout si, de asemenea, se retrimite ultimul pachet
pentru a anunta senderul de problema. Cand senderul primeste un pachet de ACK de la
receiver, acesta verifica secventa pentru a stii ce sa trimita, daca secventa e buna
atunci se va trimite pachetul urmator, daca nu e buna secventa se retrimite ultimul
pachet. Acest lucru este util atunci cand senderul trimite un pachet, receiverul nu il
primeste si trimite ultimul pachet care sa il consideram ack, daca senderul nu verifica
secventa va considera ca poate merge mai departe pentru ca a primit confirmare, insa in
realitate este sarit un pachet, daca receiverul astepta pachete cu date, senderul sare
un pachet si il trimite pe urmatorul, pierzandu-se date.
Trimiterea mai multor fisiere se realizeaza cu un for pe argumentele primite
iterand prin numele fisierelor si trimitandu-le pe rand in pachete de MAXL bytes.
