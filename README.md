Copyright - Bloțiu Mihnea-Andrei

Load Balancer - 09.05.2021

Pentru această problemă, am gândit "load-balancer-ul" ca o structură de date
ce reține dimensiunea acestuia, adica numarul de servere existente la un anumit
moment de timp și doi vectori. Fiecare dintre acești vectori conțin id-ul serverului
asociat cu valoarea hash-ului acestuia si respectiv adresa la care se află acesta.

Pentru început, în funcția de inițiere, alocăm memorie pentru load-balancer ținând
cont de numărul maxim de servere posibile și setăm dimensiunea acestuia la 0.

Fiecare server este de fapt un hashtable pentru care am creat in server.c urmatoarele
funcționalități: cea de inițiere a unui server care se ocupă cu alocarea de memorie,
cea de adăugare a unei prechi (cheie, valoare) în hashtable-ul corespunzator, operația inversă
(cea de eliminare), operația de afișare a valorii de la o anumită cheie și nu în ultimul rând,
operația de eliberare a memoriei pentru fiecare server.

Pentru a determina pe ce server trebuie să punem o anumită pereche (cheie, valoare), 
calculăm hash-ul cheii primite și deci, serverul ce are hash-ul imediat următor 
deține acestă valoare deoarece noi reținem aceste servere în ordinea crescătoare a hash-ului. 
De asemenea, dacă nu s-a găsit un astfel de server înseamnă ca hash-ul cheii noastre are o 
valoare mai mare decât cel mai mare hash al vreunui server, deci trebuie să o adăugăm pe primul server.

Operația de afișare a unei valori corespunzătoare unei chei este similară cu unica diferență
că după ce am găsit serverul pe care acestă cheie ar trebui să existe, întoarcem valoarea
existentă la acea cheie, dacă există.

În cazul în care dorim să adăugăm un server, ținem cont și de faptul că trebuie să adăugăm 
și două copii ale sale conform cerinței. Acestea vor avea hash-uri diferite față de
serverul principal, dar vor memora adresa aceluiași server.
Astfel, adăugăm cele 3 server (originalul + 2 replici) în ordinea crescătoare a hash-urilor. 
În acest sens, implementarea fiind făcută cu un vector, după fiecare adăugare a unui server, 
apelăm o funcție ce shiftează toate elementele spre dreapta astfel încât poziția la care 
trebuie să punem serverul să rămână liberă.

Dupa fiecare adaugare urmează să se realizeze redistribuirea datelor conform regulii
inițiale, adică facem o nouă parcurgere prin hash-ring pentru fiecare dintre cele 3 replici și
în momentul în care găsim poziția la care am pus serverul anterior verificăm dacă nu cumva
avem elemente pe serverul imediat următor ce ar trebui adăugate pe serverul introdus recent.
Dacă da, facem aceste modificări.

Funcția de remove este similară: căutăm în hash-ring serverul și cele două copii ale sale pe
care dorim să le scoatem. Realizăm redistribuirea elementelor de pe aceste servere conform
regulii precizate anterior, facem din nou shiftare elementelor pentru a păstra ordinea
crescătoare a hash-urilor și eliberăm memoria pentru serverele eliminate.

În final, funcția de free, eliberează pe rând fiecare server de pe hash-ring, realizează
deplasarea elementelor și în final eliberează hash-ring-ul, vectorul de adrese și
load-balancer-ul cu totul.

Pentru aceste funcții am folosit implementările standard de liste și hashtable, exact
ca în laboratoare.



