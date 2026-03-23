#!/usr/bin/env bash
set -euo pipefail

if [[ $# -lt 2 || $# -gt 3 ]]; then
    echo "Usage: $0 <ssid> <password> [serial_port]"
    echo "Example: $0 MyWifi MyPass123 /dev/ttyUSB0"
    exit 1
fi

SSID="$1"
PASSWORD="$2"
PORT="${3:-}"
BAUD=115200
COMMAND_TIMEOUT=5
IP_TIMEOUT=30

pick_port() {
    if [[ -n "$PORT" ]]; then
        if [[ ! -e "$PORT" ]]; then
            echo "Serial port not found: $PORT" >&2
            exit 1
        fi
        echo "$PORT"
        return
    fi

    if compgen -G "/dev/serial/by-id/*" > /dev/null; then
        local first_by_id
        first_by_id=$(ls -1 /dev/serial/by-id/* | head -n 1)
        if [[ -n "$first_by_id" ]]; then
            readlink -f "$first_by_id"
            return
        fi
    fi

    local candidates=(/dev/ttyUSB* /dev/ttyACM*)
    for c in "${candidates[@]}"; do
        if [[ -e "$c" ]]; then
            echo "$c"
            return
        fi
    done

    echo "No serial port found. Pass one explicitly as the third argument." >&2
    exit 1
}

PORT="$(pick_port)"

echo "Using serial port: $PORT"
echo "Baud: $BAUD"

stty -F "$PORT" "$BAUD" cs8 -cstopb -parenb -ixon -ixoff -echo -hupcl
exec 3<> "$PORT"

send_cmd() {
    local cmd="$1"
    local display="${2:-$1}"
    echo "Sending: $display"
    printf '%s\r\n' "$cmd" >&3
}

read_until() {
    local timeout="$1"
    shift
    local pattern_count="$1"
    shift

    local patterns=()
    local i
    for ((i=0; i<pattern_count; i++)); do
        patterns+=("$1")
        shift
    done

    local deadline=$((SECONDS + timeout))
    local line
    while (( SECONDS < deadline )); do
        if IFS= read -r -t 0.2 line <&3; then
            line="${line%$'\r'}"
            [[ -z "$line" ]] && continue
            echo "< $line"

            if [[ "$line" =~ Unknown\ command|Unknown\ save\ command|Invalid\ command|Invalid\ value|Error ]]; then
                echo "Device reported an error: $line" >&2
                return 2
            fi

            if (( ${#patterns[@]} == 0 )); then
                continue
            fi

            for pat in "${patterns[@]}"; do
                if [[ "$line" =~ $pat ]]; then
                    return 0
                fi
            done
        fi
    done

    return 1
}

extract_ip_from_line() {
    local line="$1"
    if [[ "$line" =~ IP[[:space:]]Address:[[:space:]]([0-9]{1,3}(\.[0-9]{1,3}){3}) ]]; then
        echo "${BASH_REMATCH[1]}"
        return 0
    fi
    if [[ "$line" =~ WiFi[[:space:]]connected:[[:space:]]([0-9]{1,3}(\.[0-9]{1,3}){3}) ]]; then
        echo "${BASH_REMATCH[1]}"
        return 0
    fi
    if [[ "$line" =~ Captive[[:space:]]portal[[:space:]]IP:[[:space:]]([0-9]{1,3}(\.[0-9]{1,3}){3}) ]]; then
        echo "${BASH_REMATCH[1]}"
        return 0
    fi
    return 1
}

read_until 1 0 || true

send_cmd "#wifi-ssid:$SSID" "#wifi-ssid:<ssid>"
if ! read_until "$COMMAND_TIMEOUT" 2 "Wifi SSID changed to:" "Restart is required after save"; then
    echo "Missing confirmation for #wifi-ssid." >&2
    exit 1
fi

send_cmd "#wifi-pass:$PASSWORD" "#wifi-pass:<password>"
if ! read_until "$COMMAND_TIMEOUT" 2 "Wifi password changed to a value of" "Restart is required after save"; then
    echo "Missing confirmation for #wifi-pass." >&2
    exit 1
fi

send_cmd '$save'
if ! read_until "$COMMAND_TIMEOUT" 1 "Settings saved!"; then
    echo "Missing confirmation for \$save." >&2
    exit 1
fi

send_cmd "#restart"
echo "Waiting for device to reboot and report Wi-Fi IP..."

deadline=$((SECONDS + IP_TIMEOUT))
DEVICE_IP=""
next_query=0
while (( SECONDS < deadline )); do
    if (( SECONDS >= next_query )); then
        send_cmd "#ip"
        next_query=$((SECONDS + 2))
    fi

    if IFS= read -r -t 0.2 line <&3; then
        line="${line%$'\r'}"
        [[ -z "$line" ]] && continue
        echo "< $line"

        candidate_ip=""
        if candidate_ip="$(extract_ip_from_line "$line")"; then
            if [[ "$candidate_ip" != "0.0.0.0" ]]; then
                DEVICE_IP="$candidate_ip"
                break
            fi
        fi
    fi
done

if [[ -z "$DEVICE_IP" ]]; then
    echo "Did not detect device IP in serial logs within timeout." >&2
    exit 1
fi

echo "Device IP: $DEVICE_IP"
echo "Done. Validation passed and device restart completed."
