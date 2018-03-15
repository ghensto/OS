#!/bin/bash


cd SourceCode
make clean
make
echo "Starting server"
gnome-terminal -x bash -c "./web_server 9000 ../testing 100 100 100;read -n1"

echo " Single file query"
cd ../testing
gnome-terminal -x bash -c "wget http://127.0.0.1:9000/image/jpg/29.jpg; echo \"Press any key to close terminal\";read -n1"


echo " Multiple files query put in results"
gnome-terminal -x bash -c "wget -i ../testing/urls -O results; echo \"Press any key to close terminal\";read -n1"

echo "Official file query"
gnome-terminal -x bash -c "wget -i ../testing/urls -O myres; echo \"Press any key to close terminal\";read -n1"



