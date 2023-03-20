## Dokumentácia pre klienta pre remote kalkulačku

#### Autor: Adrián Ponechal

#### Login: xponec01

## Obsah

- Teória nutná k pochopeniu projektu
- Funkčné časti kódu
- Testovanie programu
- Zdroje

<!-- doplnit list -->

## Teória

### TCP

### UDP

## Program

Client `ipkcpc` načíta zo štandartného vstupu `stdin` správy pre daný server, ku ktorému sa pripojí podľa zadaných parametrov. V prípade akejkoľvek chyby vráti nenulovú návratovú hodnotu a chybu vypíše na štandartný chybový výstup `stderr`.

Má 3 havné časti:

1. funkcia `main`
2. funkcia `udp_connection`
3. funnkcia `tcp_connection`

Vo funkcii `main` prebiehajú hlavne kontroly vstupných parametrov. Pri zadaní zlých parametrov vráti chybový kód `10` a na `stderr` vypíše použitie. Pre vypísanie samotného použitia je možné zavolať program v tvare `ipkcpc -h`. Po úspešnom načítaní voláme funkciu pre spojenie podľa hodnoty parametru `-m <mode>`, ktorá sa uloží do globálne premennej `connection_t mode`. Typ `connection_t` je výčtový typ pre hodnotu spojenia (bol vytvorený z dôvodu rýchlejšieho porovnávnia v kóde).

Ak má premenná `mode` hodnotu `udp`, tak sa spustí funkcia `udp_connection`, ktorá zabezpečí spojenie podľa protokolu udp. Spomenie v tejto funkcii môže byť prerušené len poslaním signálu `Ctrl + c`.

V prípade, kde je `mode` nastavené na hodnotu `tcp` sa spustí funkcia `tcp_connection`. Táto funkcia vytvorí spojenie podľa TCP, v inom prípade vrádi návratovú hodnotu `1`. K ukončeniu programu dochádza, ak dostane na štandartný vstup `BYE` alebo príde odpoveď od serveru `BYE`. Prípadne sa korektne ukončí aj pri poslaní signálu `Ctrl + c`.

Funkcie sa líšia hlavne v oblastiach vyplývajúcich z teórie (funkcie `send()` a `sendto()`.`recv()` a `recvfrom()`). Program je kompatibilný ako so systémami Linux a MacOS, tak aj na Windows (aplikácie typu \_WIN32)

![Ukážka kompatibility](./imgs/compatibily_screenshot.png)

## Testovanie

Testovanie programu prebiehalo dvomi spôsobmi:

### 1. automatické testy

Tento typ testovania bol využitý najmä pre testy kontroly správnosti príjmaných parametrov. Na teno účel bol vytvorený script `test_inputs.py`. Pri testovaní scriptu musí byť server vypnutý (najmä localhost). Pre spustenie scriptu je treba mať nainštalovaný python3.
![Testovanie vstupnych parametrov](./imgs/input_tests.png)

### 2. Testy funkčnosti v referenčnom prostredí

Testy boli vykonané v referenčnom protredí. Testované boli hlavne prípady ukončenia programu, ale aj funkčosť výstupu.

# SEM OBRAZKY

## Zdroje

NOTE: upravit podla urciteho formatu zdrojov

- https://www.geeksforgeeks.org/tcp-server-client-implementation-in-c/

* https://www.tutorialspoint.com/c_standard_library/c_function_signal.htm
