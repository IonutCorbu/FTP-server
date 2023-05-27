Pentru implementarea acestei teme am folosit 2 surse: server.c si client.c si un header care contine toate structurile folosite ca parametrii la intrarea in executie a rutinelor unor threaduri sau structura de date folosita pentru retinerea primelor 10 cuvinte din fiecare fisier.

Folderul de fisiere de pe server a fost denumit ./files.

Comanda LIST a fost construita pe baza structurii DIR din headerul dirent.h. Nu afiseaza directoarele . si ..  .
Comanda GET a fost construita cu ajutorul apelului sendfile astfel incat la executarea unei comenzi precum GET fis1 in client, continutul fisierului va fi disponibil clientului.
Pentru comanda DELETE am folosit apelul rmdir.
Pentru comenzile PUT si UPDATE este necesara introducerea unei comenzi precum PUT 4 fis7 4 mere, respectiv UPDATE 4 fis7 1 3 ana.
In cazul celor 2, am constatat prea tarziu faptul ca apelarea succesiva a functiei send din client presupune ca bufferul din server sa nu contina toate datele trimise la fiecare apel, astfel, pentru UPDATE am comentat un test hard-coded.
Pentru comanda SEARCH, la executarea aplicatiei server, se creaza un vector cu mai multe liste care corespund fisierelor existe in folder. Pentru fiecare lista vom putea gasi numarul de cuvinte (10 sau mai putine daca nu exista mai multe cuvinte in fisier), lista de cuvinte propriu-zisa si numele fisierului.

Nu am implementat actualizarea la operatiile DELETE, PUT sau UPDATE.

Logul are rolul de a scrie in fisierul logfile.txt toate operatiile sub forma descrisa in cerinta.