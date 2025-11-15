# Snmp2otel

## Úvod


## Protokol SNMPv2: Popis Funkčnosti
Protokol SNMPv2 (Simple Network Management Protocol verzia 2) je rozšírená a vylepšená verzia štandardného protokolu pre správu a monitorovanie sieťových zariadení, ako sú smerovače, prepínače, servery a tlačiarne. Umožňuje centrálnemu manažérovi (NMS - Network Management Station) získavať informácie a meniť konfiguráciu spravovaných zariadení.

### 1. Kľúčové Komponenty
SNMP model pozostáva z troch základných komponentov:
- Správca Siete (Network Management Station - NMS): Centrálna stanica, ktorá spúšťa aplikácie pre správu. Odošle požiadavky a prijíma odpovede a nežiaduce notifikácie (trapy).
- Spravované Zariadenie (Managed Device): Sieťový komponent (napr. router, switch), ktorý obsahuje dáta, ktoré sa majú spravovať.
- Agent: Softvérový modul bežiaci na spravovanom zariadení. Zhromažďuje informácie o zariadení, spracováva požiadavky od NMS a posiela odpovede/notifikácie.

### 2. Dátová Štruktúra
- MIB (Management Information Base): Je to hierarchická databáza (virtuálna informačná štruktúra) na spravovanom zariadení, ktorá definuje premenné (objekty), ktoré môže NMS spravovať. Každá premenná má jedinečný OID (Object Identifier).

- OID (Object Identifier): Unikátny identifikátor v stromovej štruktúre, ktorý presne odkazuje na konkrétnu premennú v MIB (napr. počet chybových paketov na danom porte).

### 3. Komunikácia a Správy
SNMPv2 primárne využíva protokol UDP (User Datagram Protocol) na portoch 161 (požiadavky/odpovede) a 162 (notifikácie/trapy) pre nespoľahlivú a rýchlu komunikáciu.

#### A. Základné Typy PDU (Protocol Data Unit - Typy Správ)
| Typ Správy | Odosielateľ | Príjemca | Popis Funkcie |
| :--- | :--- | :--- | :--- |
| `GetRequest` | NMS | Agent | Žiadosť o jednu alebo viac hodnôt premenných. |
| `GetNextRequest` | NMS | Agent | Žiadosť o hodnotu **nasledujúcej** premennej v MIB, umožňuje prechádzanie (walk) celej MIB. |
| `GetBulkRequest` | NMS | Agent | **Novinka v v2:** Efektívna žiadosť o prenos veľkého bloku dát. |
| `SetRequest` | NMS | Agent | Žiadosť o **zmenu** hodnôt premenných na zariadení (konfigurácia). |
| `Response` | Agent | NMS | Odpoveď na akúkoľvek Get alebo Set požiadavku, obsahuje požadované dáta alebo stav operácie. |
| `Trap` | Agent | NMS | **Nežiaduce** asynchrónne upozornenie na **významnú udalosť** (napr. reštart zariadenia). |
| `InformRequest` | NMS | NMS | **Novinka v v2:** Notifikácia zaslaná medzi dvoma NMS; vyžaduje **potvrdenie** o prijatí. |

#### B. Mechanizmus Komunít (Communities)
SNMPv2 používa "reťazce komunity" (Community Strings) ako formu základného hesla/autentifikácie.

- Agent prijme požiadavku len vtedy, ak sa reťazec komunity v správe zhoduje s nakonfigurovaným reťazcom na zariadení.

Existujú dva bežné typy:
- read-only (len na čítanie): Umožňuje len Get operácie.
- read-write (čítanie/zápis): Umožňuje Get aj Set operácie.

## OpenTelemetry (OTEL) a HTTP/JSON Metriky
OpenTelemetry (OTEL) je open source projekt, ktorý poskytuje unifikovaný súbor nástrojov, API a SDK na inštrumentáciu, generovanie, zhromažďovanie a export telemetrických dát (logy, tracy a metriky).

> Protokol pre prenos týchto dát sa nazýva OTLP (OpenTelemetry Protocol). Ten podporuje dva hlavné formáty kódovania/prenosu:

- OTLP/gRPC (preferovaný): Využíva binárny formát Protocol Buffers pre efektívny a rýchly prenos.

- OTLP/HTTP/JSON (vlastná požiadavka): Využíva štandardný protokol HTTP s dátovou záťažou (payload) kódovanou vo formáte JSON.

### Čo je to OTEL/HTTP/JSON Metrics?
Ide o štandardizovaný formát a metódu prenosu metriky z aplikácie alebo služby do kolektora alebo monitorovacieho backendu.

> Používa sa HTTP/HTTPS pre prenos.

Štandardizovaný Formát: Metriky sú štruktúrované podľa OTLP Data Modelu a odosielajú sa ako JSON objekt v tele HTTP požiadavky (zvyčajne POST).

#### Štruktúra Metriky
- **Názov** (Name), **Popis** (Description) a **Jednotku** (Unit).
- **Typ metriky** (napr. Counter, Gauge, Histogram).
- **Dátové body** (Data Points): Samotné merané hodnoty, ktoré zahŕňajú časovú značku (timestamp) a sadu Atribútov (Attributes).
- **Atribúty**: Pár kľúč-hodnota, ktoré poskytujú kontext 
    - Napr. http.request.method: POST, status_code: 200, service.name: nazov-programu.

### Načo sa OTEL/HTTP/JSON Metriky Používajú?
Hlavným cieľom je dosiahnuť observability (pozorovateľnosť) aplikácií a infraštruktúry bez vendor lock-in.

> Jednotný Štandard: Poskytuje konzistentný spôsob, ako zbierať metriky, bez ohľadu na to, v akom jazyku bola aplikácia napísaná (Java, Python, .NET, Go, C++, atď.).

Agregácia a Analýza: Metriky sú základom pre monitorovanie stavu, výkonu a dostupnosti dakeho programu (napr. latencia HTTP požiadaviek, počet chýb, využitie pamäte).

> [!Note] Flexibilita Exportu
Umožňuje odosielať dáta do rôznych systémov (Prometheus, Grafana, Jaeger, komerčné backends) prostredníctvom jedného protokolu – OTLP.


## Implementácia

## Použitie
```bash
snmp2otel -t target [-C community] -o oids_file -e endpoint [-i interval] [-r retries] [-T timeout] [-p port] [-v]
```

#### Parametre
-t target — IP adresa alebo DNS meno SNMP agenta.
-C community — SNMP v2c community string. Predvolené: public.
-o oids_file — Súbor so zoznamom OID, ktoré sa majú dotazovať.
-e endpoint — URL OTEL endpointu pre export (OTLP/HTTP JSON), napr. http://localhost:4318/v1/metrics.
-i interval — Perióda dotazovania v sekundách (> 0). Predvolené: 10.
-r retries — Počet retransmisií pri timeoutu. Predvolené: 2.
-T timeout — Timeout SNMP dotazu v ms. Predvolené: 1000.
-p port — UDP port SNMP agenta. Predvolené: 161.
-v — Verbose režim.
## Použité technológie

## Testovanie

