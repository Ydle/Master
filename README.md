ENGLISH VERSION BELOW

Librairies nécéssaires :
- wiringPi
- microhttpd 
- jsoncpp
- curl


* Penser a Modifier les Pin de réception / émissions en fonction de votre config hardware dans les toutes premieres ligne du fichier master.cpp

Par defaut:

Transmission : GPIO 0 
Emission : GPIO 6 

Plan des Pin disponibles ici : http://wiringpi.com/pins/

* Faire un "Make" dans le répertoire master ce qui doit vous compiler un ydlemaster a lancer par un : sudo ./ydlemaster


ENGLISH VERSION :

You will need the following libraries :
- wiringPi
- microhttpd 
- jsoncpp
- curl

* Do not forget to modify pins for reception / emission  depending on your hardware configuration on the first lines of the file master.cpp

Default values : 

reception : GPIO 0 
emission : GPIO 6 

Pins' mapping is available here : http://wiringpi.com/pins/

* You'll have to run a "Make" command in the master directory. This command should compile the ydlemaster, an executable that you can run with : sudo ./ydlemaster
