//    Simularea unui hard disk la nivel fizic si la nivel logic

//
//    - un hard disk este organizat fizic in sectoare si logic in partitii
//    - un sector este un grup cu lungime fixa de octeti
//    - o partitie este o portiune contigua dintr-un hard disk
//    - o partitie este organizata logic in clustere
//    - un cluster este un grup cu lungime fixa de sectoare
//    - un fisier este un grup contiguu cu lungime variabila de date
//      [] in RAM: este organizat ca un grup contiguu cu lungime variabila de octeti
//      [] pe partitie (respectiv pe hard disk): este organizat in clustere (respectiv sectoare)
//      [] abstract: este considerat ca un grup contiguu de clustere, suficiente pentru a contine toate datele
//    - terminologie:
//      [] sectoarele hard disk-ului se numesc ABSOLUTE
//      [] sectoarele si clusterele partitiei se numesc LOGICE
//      [] clusterele si sectoarele din organizarea abstracta a fisierului se numesc VIRTUALE
//    - fisierele pot fi stocate (scrise) pe partitie, incarcate (citite) de pe partitie sau eliminate (sterse) de pe partitie
//    - pentru realizarea actiunilor mentionate, se vor defini functiile WriteFile, ReadFile si DeleteFile
//      [] aceste functii vor fi legate de structura HdImageT, care va gestiona o partitie
//    - reguli simultane de respectat la scrierea pe partitie:
//      [] se scrie sector cu sector in ordinea numerelor lor
//      [] clusterele logice se aleg in ce secventa se doreste, inclusiv aleator
//      [] odata ales un cluster logic, fie se completeaza, fie se termina sectoarele
//      [] un sector virtual se scrie pe un sector logic
//      [] sectoarele logice consecutive stocheaza sectoare virtuale consecutive
//      [] (redundant) un cluster logic se completeaza incepind cu primul sau sector
//    - un grup de clustere, consecutive ca numere si simultan consecutive ca alegere, formeaza un fragment
//
//    - catalogarea (cartografierea) fragmentelor  ->  mapping pairs
//      [] fragmentul:
//        () este determinat de o pozitie(numarul primului sau cluster) si o lungime(numarul sau de clustere)
//        () <pozitia fragmentului> = <deplasamentul pozitiei fragmentului> + <pozitia fragmentului anterior>
//        () <pozitia fragmentului anterior> pentru primul fragment este 0
//        () lungimea fragmentului si <deplasamentul pozitiei fragmentului> sint valori aritmetice cu semn
//      [] mapping pairs:
//        () reprezinta un sir de inregistrari cu lungime variabila
//        () inregistrarea este formata din:
//          o. primul octet:
//            - 4 LSb = numarul de octeti pentru valoarea lungimii fragmentului
//              (daca  0 == 4 LSb  atunci valoarea lungimii fragmentului este 0)
//            - 4 MSb = numarul de octeti pentru valoarea deplasamentului pozitiei fragmentului
//              (daca  0 == 4 MSb  atunci valoarea deplasamentului pozitiei fragmentului este 0)
//          o. octetii pentru valoarea lungimii fragmentului in ordinea  LE
//          o. octetii pentru valoarea deplasamentului pozitiei fragmentului in ordinea  LE
//        () ultima inregistrare are primul octet cu valoarea 0
//      [] OBS.:
//        () un octet pentru una din valori reprezinta o cifra a acelei valori in baza de numeratie 256
//        () penultima inregistrare nu poate reprezenta un fragment cu lungime zero
//        () desi se permite altfel, primul octet al unei inregistrari corect formatate indeplineste conditia <( 0 != 4 LSb ) si ( 0 != 4 MSb )>
