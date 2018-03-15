#!/bin/bash

# Remove previous message queues
ipcrm --all=msg

echo "Starting packet_sender process"
gnome-terminal -x bash -c "./packet_sender 3; echo \"Press any key to close terminal\";read -n1"

echo "Starting packet_receiver process"
gnome-terminal -x bash -c "./packet_receiver 3; echo \"Press any key to close terminal\";read -n1"
