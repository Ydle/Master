Librairies nécéssaires :
- wiringPi
- microhttpd 
- jsoncpp
- curl


*Penser a Modifier les Pin de réception / émissions en fonction de votre config hardware dans les toutes premieres ligne du fichier master.cpp

Par defaut:

Transmission : GPIO 0 
Emission : GPIO 6 

Plan des Pin disponibles ici : http://wiringpi.com/pins/

*Faire un "Make" dans le répertoire master ce qui doit vous compiler un ydlemaster a lancer par un : sudo ./ydlemaster


