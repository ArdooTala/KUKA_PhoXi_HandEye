#!/bin/bash
# Exit immediately if a command exits with a non-zero status
set -e

# --- SYSTEM D-BUS START ---
# System D-Bus PID file location for tracking its persistence
SYSTEM_BUS_PID_FILE="/tmp/dbus_system_pid"

# Check if the System D-Bus is running by checking the PID file
if [ ! -f "$SYSTEM_BUS_PID_FILE" ] || ! kill -0 "$(cat "$SYSTEM_BUS_PID_FILE" 2>/dev/null)" 2>/dev/null; then
    echo "--- Starting persistent D-Bus System Bus ---"
    
    echo "Deleting dbus.pid"

    if [ -f "/var/run/dbus/pid" ]; then
        rm /var/run/dbus/pid 2>/dev/null
    fi

    # Launch System D-Bus daemon and capture the PID immediately.
    SYSTEM_PID=$(/usr/bin/dbus-daemon --system --fork --print-pid)

    if [ -z "$SYSTEM_PID" ]; then
        echo "ERROR: System D-Bus failed to start. Check daemon logs."
        # Don't exit here, allow Session D-Bus to continue if possible
    else
        # 2. Write the captured PID to the tracking file
        echo "$SYSTEM_PID" > "$SYSTEM_BUS_PID_FILE"
        echo "System D-Bus daemon PID: $SYSTEM_PID"
    fi
else
    echo "--- D-Bus System Bus already running (PID: $(cat "$SYSTEM_BUS_PID_FILE")) ---"
fi
# --- SYSTEM D-BUS END ---


# --- SESSION D-BUS START ---
# File to store the D-Bus Session Bus address and PID
DBUS_ADDRESS_FILE="/tmp/dbus_session_address"
DBUS_PID_FILE="/tmp/dbus_session_pid"

# Check if the PID file exists and the process is still running.
if [ ! -f "$DBUS_PID_FILE" ] || ! kill -0 $(cat "$DBUS_PID_FILE" 2>/dev/null) 2>/dev/null; then
    echo "--- Starting persistent D-Bus Session Bus ---"
    
    # Launch D-Bus persistently in the background.
    # The 'eval' sets DBUS_SESSION_BUS_ADDRESS and DBUS_SESSION_BUS_PID in the current environment.
    dbus-launch --sh-syntax > "$DBUS_ADDRESS_FILE"
    
    # Save the address and PID to shared files.
    echo "$DBUS_SESSION_BUS_PID" > "$DBUS_PID_FILE"
    
    echo "Session D-Bus daemon PID: $DBUS_SESSION_BUS_PID"
else
    # D-Bus is already running from a previous container launch
    echo "--- D-Bus Session Bus already running (PID: $(cat "$DBUS_PID_FILE")) ---"
fi

if [ -f "$DBUS_ADDRESS_FILE" ]; then
    # Load the address for the current ENTRYPOINT shell
    source "$DBUS_ADDRESS_FILE"
fi

if [ -z "$DBUS_SESSION_BUS_ADDRESS" ]; then
    echo "ERROR: D-Bus session address is missing after initialization."
    exit 1
fi

echo "D-Bus session ready: $DBUS_SESSION_BUS_ADDRESS"

# Execute the main command (e.g., 'bash')
exec "$@"
