# Snmp2otel - Exportér SNMP Gauge metrik do OpenTelemetry (OTEL)

## Zadanie
Napište program snmp2otel, který bude v pravidelných intervalech dotazovat zadaný SNMP agent (zařízení) na vybrané OID a naměřené hodnoty exportovat jako OTEL Metrics pomocí rozhraní OTLP/HTTP (JSON) na zadaný OTEL endpoint (typicky OpenTelemetry Collector). Analýza a sestavení SNMP v2c zpráv i OTLP/HTTP JSON těla může být implementována libovolným způsobem dle vašeho návrhu. Uvažujte komunikaci přes UDP/161 pro SNMP (klient) a TCP/HTTP pro export (HTTPS volitelné v rozšíření).

Program musí podporovat pouze metriky typu Gauge.

### Obmedzenia
- Povoleny jsou libovolné knihovny.
- Instalace knihoven do systému není povolena; projekt se musí přeložit standardně pomocí Makefile.
- Program poběží bez root oprávnění; nesmí vyžadovat privilegované porty pro lokální naslouchání.

### Podporované protokoly a datové typy
- SNMP v2c: podporujte pouze operaci Get pro skalární OID zakončená .0.
- ASN.1 BER: implementujte minimum potřebné pro SNMPv2c Get/Response (INTEGER, OCTET STRING, OID, SEQUENCE).
- OTEL export: data se posílají pomocí OTLP/HTTP (JSON).
Každý naměřený údaj je reprezentován jako Gauge.

### Výstup aplikace
- V základním režimu program nic nevypisuje.
- Ve verbose režimu (-v) vypisujte ladicí informace (dotazy, odpovědi, export).
- Chybové hlášky vypisujte na standardní chybový výstup.

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
Pre posielanie SNMPv2c správ a získavanie informácií z agentov využívame knižnicu BasicSNMP (silderan/BasicSNMP). Táto knižnica implementuje nevyhnutné funkcie pre zostavenie a spracovanie SNMP správ v správnom formáte.

#### Generovanie a Odosielanie Požiadaviek
1. Využitie Knižnice BasicSNMP: Knižnica je primárne zodpovedná za vytváranie a kódovanie SNMP správ, ktoré slúžia na získavanie informácií o sieťových metrikách pomocou ich unikátnych OID (Object Identifier).
2. Kódovanie ASN.1 BER: Pre zabezpečenie, že správa je formálne a štrukturálne správna podľa štandardu SNMP, knižnica využíva kódovanie ASN.1 BER (Basic Encoding Rules). Tento proces transformuje dátovú štruktúru požiadavky (ktorá zahŕňa Community String, verziu protokolu a požadované OIDs) do binárneho formátu vhodného pre prenos.
3. Prenos K Agentovi: Takto zostavená binárna SNMP správa je odoslaná na cieľového SNMP Agenta, ktorý beží na spravovanom sieťovom zariadení.

![Zdroj: https://www.ranecommercial.com/legacy/note161.html](Images/snmpV2cPacket.png)

## Použitie
```bash
snmp2otel -t target [-C community] -o oids_file -e endpoint [-i interval] [-r retries] [-T timeout] [-p port] [-v] [-O oids_formatingMappingFile]
```

### Parametre

-t target — IP adresa nebo DNS jméno SNMP agenta.  
-C community — SNMP v2c community string. Výchozí: public.  
-o oids_file — soubor se seznamem OID, které se mají dotazovat.  
-e endpoint — URL OTEL endpointu pro export (OTLP/HTTP JSON), např. http://localhost:4318/v1/metrics.  
-i interval — perioda dotazování v sekundách (> 0). Výchozí: 10.  
-r retries — počet retransmisí při timeoutu. Výchozí: 2.  
-T timeout — timeout SNMP dotazu v ms. Výchozí: 1000.  
-p port — UDP port SNMP agenta. Výchozí: 161.  
-v — verbose režim.  

### Formát souboru se seznamem OID

Textový ASCII soubor: jedno OID na řádek, numerická forma. Prázdné řádky a řádky začínající # ignorujte.

```
# System uptime
1.3.6.1.2.1.1.3.0
```

### Formát mapping souboru (volitelný)
```
{
  "1.3.6.1.2.1.1.3.0": { "name": "snmp.sysUpTime", "unit": "ms", "type": "gauge" }
}
```
*type* může být pouze gauge. Pokud položka chybí, použije se název odvozený z OID.

## Použité technológie

## Testovanie

### Zdroje

SnmpV2c  
https://www.ranecommercial.com/legacy/note161.html
https://www.dpstele.com/snmp/tutorial/packet-types-structure.php
https://support.huawei.com/enterprise/en/doc/EDOC1100174721/684b4c64/snmpv1

OTEL  
https://stackoverflow.com/questions/18119428/c-how-to-exit-out-of-a-while-loop-recvfrom
