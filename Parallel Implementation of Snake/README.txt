Bina Marius Andrei - bina.marius@gmail.com

Prezentare generala :
Am adaugat in structura snake un camp de timp coord (tail) unde tin ultimul punct din fiecare sarpe, pe care atunci cand se deplaseaza il "sterg" (pun 0 in matrice acolo)
si adaug de la head in directia in care merge sarpele. Pentru a obtine cozile tuturor seprilor parcurg matricea pentru a gasi puncte diferite de 0 (adica atribuite unui sarpe)
si care respecta conditia de a fi punct de inceput sau sfarsit as sarpelui (adica in jurul lui vom gasi un singur punct cu acelasi encoding). Punctul care respecta conditia asta
si nu e head inseamna ca e tail. Apoi in functie de directie calculez coordonatele unui nou camp adaugat in structura snake, newHead, care reprezinta pozitia unde va fi varful
dupa mutare, el este salvat dar nu este actualizat la momentul respectiv in matrice. Totodata pentru fiecare sapre trebuie gasit noul tail, astfel de la actualul tail se merge
in cele 4 directii pentru a gasi urmatorul punct care devine tail. Dupa ce am salvat noile pozitii pentru tail si head, se verifica coliziunile, se parcurge vectorul de serpi
si daca in matrice la coordonatele punctului newHead este 0 atunci este actualizat cu encodingul sarpelui, daca nu este 0 atunci are loc o coliziune, se da break si se salveaza
indexul cu care se parcurge bucla for, acest index salvat ne ajuta sa obtinem o performanta mai buna, astfel se face o noua bucla for pana la acest index, in care se reseteaza
practic pozitiile pe care le-am actualizat pentru a ajunge la starea de inainte de coliziune. Daca totul se desfasoara fara coliziuni atunci head-ul este actualizat la newHead
si vechile tail-uri se sterg (se pun 0 in matrice).

Timpi obtinuti pe ibm-nehalem :
Pentru 3 rulari succesive :
1.
1 Thread - 5.8517
2 Threads - 4.8773
4 Threads - 4.1916
6 Threads - 4.0162
8 Threads - 3.9381

2.
1 Thread - 5.8561
2 Threads - 4.6662
4 Threads - 4.1516
6 Threads - 3.9717
8 Threads - 3.9185

3.
1 Thread - 5.8554
2 Threads - 4.6652
4 Threads - 4.0803
6 Threads - 3.9050
8 Threads - 3.8874