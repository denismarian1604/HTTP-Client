## Tema 4 Protocoale de Comunicatie : Client Web. Comunicatie cu REST API.
Student: Denis-Marian Vladulescu\
Grupa: 324CB\
Limbaj de programare utilizat: `C`

## Functii ajutatoare si API-uri folosite
In vederea implementarii acestei teme, m-am folosit de functiile realizate in cadrul `laboratorului 9` si de celelalte fisiere `header` si fisiere `sursa` preimplementate; functii carora le-am adus modificari pentru a putea satisface nevoile extinse ale temei, cum ar fi adaugarea, in cadrul `request-urilor`, a campului `Token`.
Pe langa asta, am adaugat o noua functie, `compute_delete_request`, care are scopul de a realiza o cerere de tip `DELETE`.

Pentru parsarea si formatarea mesajelor de tip `JSON` m-am folosit de biblioteca [`parson`](https://github.com/kgabis/parson).

## Functionarea clientului(`client.c`)
In cadrul fisierului `client.c`, am definit cateva functii menite sa ma ajute in cadrul procesarii si verficiarii validatii datelor.
- `check_string_for_spaces`: functie care verifica daca un string, primit ca parametru, contine sau nu caracterul ' '(blank_space)
- `update_cookies`: functie care primeste ca parametri referinta la vectorului de cookie-uri, referinta la numarul curent de cookie-uri curente si raspunsul primit de la server, care contine noul cookie de adaugat
- `update_tokens`: functie similara cu cea `update_cookies`, dar care actualizeaza vectorul de token-uri ale utilizatorului cu noul token primit de la server
- `check_book_for_mistakes`: functie care verifica ca datele furnizate pentru o noua carte sa respecte cerintele impuse de cerinta
- `display_given_book_or_list_of_books`: functie, care in functie de al doilea parametru(0 - o singura carte/1 - o lista de carti), afiseaza, in format `JSON`, cartea/lista de carti preluata din raspunsul server-ului

Ma folosesc de structura `struct pollfd` si de functia `poll` pentru a astepta evenimente de la tastatura(`STDIN`, `fd 0`).
Cand se primeste notificare de eveniment, se verifica ce fel de comanda a fost introdusa:
- `register`: se asteapta introducerea unui `username` si a unei `parole`, dupa care se realizeaza o cerere `POST` catre server la ruta furnizata in cerinta. Se interpreteaza raspunsul primit de la server si se afiseaza un mesaj de eroare sau de succes daca s-a putut realiza inregistrarea noului utilizator.
- `login`: se asteapta introducerea unui `username` si a unei `parole`, dupa care se realizeaza o cerere `POST` catre server la ruta furnizata in cerinta. Se interpreteaza raspunsul primit de la server si se afiseaza un mesaj de eroare sau de succes daca s-a putut realiza conectarea cu numele de utilizator furnizat. Se actualizeaza ulterior cookie-urile.
- `enter_library`: se realizeaza o cerere `GET` catre server la ruta furnizata in cerinta. Se interpreteaza raspunsul primit de la server si se afiseaza un mesaj de eroare sau de succes. Se actualizeaza ulterior token-urile.
- `get_books`: se realizeaza o cerere `GET` catre server la ruta furnizata in cerinta. Se interpreteaza raspunsul primit de la server si se afiseaza un mesaj de eroare sau de succes. Se afiseaza ulterior lista de carti primite, in format `JSON`.
- `get_book`: se asteapta introducerea unui `id`, dupa care se realizeaza o cerere `GET` catre server la ruta furnizata in cerinta. Se interpreteaza raspunsul primit de la server si se afiseaza un mesaj de eroare sau de succes. Se afiseaza ulterior cartea primita de la server, in format `JSON`.
- `add_book`: se asteapta introducerea unui `titlu`, `autor`, `gen`, `publicant` si `numar_de_pagini`, dupa care se realizeaza o cerere `POST` catre server la ruta furnizata in cerinta. Se interpreteaza raspunsul primit de la server si se afiseaza un mesaj de eroare sau de succes.
- `delete_book`: se asteapta introducerea unui `id`, dupa care se realizeaza o cerere `DELETE` catre server la ruta furnizata in cerinta. Se interpreteaza raspunsul primit de la server si se afiseaza un mesaj de eroare sau de succes.
- `logout`: se realizeaza o cerere `GET` catre server la ruta furnizata in cerinta. Se interpreteaza raspunsul primit de la server si se afiseaza un mesaj de eroare sau de succes. Dupa ce se confirma deconectarea, se marcheaza starea utilizatorului ca `0`(deconectat), se reseteaza numarul de `cookie-uri` si de `token-uri` la 0, se elibereaza memoria pentru `cookie-uri` si `token-uri`, daca erau alocate si se seteaza adresele pe `NULL`.
- `exit`: se realizeaza o cerere `GET` catre server, pentru deconectare, daca user-ul era conectat. Se elibereaza memoria pentru `cookie-uri` si `token-uri`, daca era cazul si se incheie executia programului.
- pentru orice alta comanda se va afisa un mesaj de eroare si se continua executia.

Se repeta asteptare si procesarea de comenzi pana la intalnirea comenzii `exit`.