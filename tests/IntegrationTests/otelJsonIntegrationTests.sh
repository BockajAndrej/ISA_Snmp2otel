#!/bin/bash

# Názov vášho skompilovaného C++ programu
PROGRAM_NAME="../../build/snmp2otel" # Upravená cesta podľa vášho výstupu

# Programové argumenty pre váš C++ program
PROGRAM_ARGS="-t 127.0.0.1 -e http://localhost:4318/v1/metrics -o ../../Data/Inputs/oids_file.txt -p 161 -r 10 -T 10000"

# Názov súboru pre výstup strace
STRACE_OUTPUT="strace_output.log"

# Názov súboru pre výstup tcpdump (pcap formát)
TCPDUMP_OUTPUT="tcpdump_output.pcap"

# Port, na ktorom očakávate HTTP komunikáciu (zmenené na 4318 podľa vášho výstupu)
HTTP_PORTS="port 4318"

# Dĺžka čakania na spustenie programu a tcpdump
INIT_WAIT_SECONDS=5
RUN_WAIT_SECONDS=10

# --- Kontrola závislostí ---
check_command() {
    if ! command -v "$1" &> /dev/null; then
        echo "Chyba: Príkaz '$1' nebol nájdený."
        echo "Prosím, nainštalujte ho: (napr. 'sudo apt-get install $1' pre Debian/Ubuntu)"
        exit 1
    fi
}

check_command "strace"
check_command "tcpdump"

# Kontrola, či program existuje
if [ ! -f "$PROGRAM_NAME" ]; then
    echo "Chyba: Program '$PROGRAM_NAME' nebol nájdený."
    echo "Uistite sa, že je skompilovaný a je v správnej ceste: $PROGRAM_NAME"
    exit 1
fi

echo "Spúšťam testovanie programu '$PROGRAM_NAME' s argumentmi: $PROGRAM_ARGS"

# --- Spustenie tcpdump na pozadí (vyžaduje sudo) ---
echo "Spúšťam tcpdump na sledovanie sieťovej prevádzky na portoch ($HTTP_PORTS)..."
echo "Výstup bude uložený do '$TCPDUMP_OUTPUT'. Vyžaduje sudo."

# Odstránime starý tcpdump súbor, aby sme predišli problémom s oprávneniami
if [ -f "$TCPDUMP_OUTPUT" ]; then
    sudo rm -f "$TCPDUMP_OUTPUT"
fi

# Spustite tcpdump s sudo. Uistite sa, že -w vytvára súbor, ktorý je zapisovateľný pre root (čo je predvolené).
# 'tcpdump -i any' vyžaduje sudo
sudo tcpdump -i any -s 0 -w "$TCPDUMP_OUTPUT" "$HTTP_PORTS" &
TCPDUMP_PID=$!
echo "tcpdump beží s PID: $TCPDUMP_PID"
echo "Čakám $INIT_WAIT_SECONDS sekúnd na spustenie tcpdump..."
sleep $INIT_WAIT_SECONDS

# --- Spustenie C++ programu so strace ---
echo "Spúšťam strace na sledovanie systémových volaní (vrátane sieťových operácií)..."
echo "Výstup bude uložený do '$STRACE_OUTPUT'."
# Všimnite si, že $PROGRAM_ARGS sú pridané tu
strace -f -e network,write,sendto,sendmsg -o "$STRACE_OUTPUT" "$PROGRAM_NAME" $PROGRAM_ARGS &
PROGRAM_PID=$!
echo "Program beží s PID: $PROGRAM_PID"

echo "Čakám $RUN_WAIT_SECONDS sekúnd, aby program mohol odoslať HTTP požiadavky..."
sleep $RUN_WAIT_SECONDS

echo "Zastavujem program a tcpdump..."

# Ukončenie programu (ak stále beží)
if ps -p $PROGRAM_PID > /dev/null; then
    kill $PROGRAM_PID
    echo "Program '$PROGRAM_NAME' bol ukončený."
else
    echo "Program '$PROGRAM_NAME' už nebeží."
fi

# Ukončenie tcpdump
# Používame sudo kill, pretože tcpdump bol spustený s sudo
if ps -p $TCPDUMP_PID > /dev/null; then
    sudo kill $TCPDUMP_PID
    echo "tcpdump bol ukončený."
else
    echo "tcpdump už nebeží (možno došlo k chybe a ukončil sa)."
fi


echo "Analýza výsledkov:"
echo "-------------------"

# Analýza výstupu strace
if [ -f "$STRACE_OUTPUT" ]; then
    echo "Kontrola '$STRACE_OUTPUT' pre sieťové aktivity (prvých 20 riadkov):"
    grep -E 'socket|connect|sendto|sendmsg|write' "$STRACE_OUTPUT" | head -n 20
    echo "... (ďalšie výsledky v '$STRACE_OUTPUT')"
else
    echo "Súbor '$STRACE_OUTPUT' nebol vytvorený (strace možno zlyhal)."
fi

# Analýza výstupu tcpdump
echo ""
echo "Sieťová prevádzka bola zachytená do '$TCPDUMP_OUTPUT'."
echo "Pre podrobnú analýzu otvorte tento súbor v Wiresharku alebo použite tshark:"
echo "  tshark -r $TCPDUMP_OUTPUT -Y http"
echo ""
echo "Môžete tiež skúsiť extrahovať HTTP pakety priamo pomocou tshark (ak je nainštalovaný):"
if command -v tshark &> /dev/null; then
    # Zmena: prístup k súboru môže byť len s sudo, ak bol vytvorený rootom
    sudo tshark -r "$TCPDUMP_OUTPUT" -Y "http"
else
    echo "Tshark nie je nainštalovaný. Nainštalujte ho pomocou 'sudo apt-get install tshark' (Debian/Ubuntu) alebo 'brew install wireshark' (macOS)."
fi

echo "Testovanie dokončené."