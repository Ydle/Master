ENGLISH VERSION BELOW

Bibliothéques nécéssaires :
- wiringPi
- microhttpd 
- jsoncpp
- curl
- libconfig++
- libboost-signal

Pour les installer :
aptitude install libmicrohttpd-dev libjsoncpp-dev libcurl4-openssl-dev libconfig++-dev libboost-signals-dev build-essential

Pensez à créer un fichier de configuration !
Exemple disponible dans ydle.conf-example

Plan des Pin disponibles ici : http://wiringpi.com/pins/

Pour le lancement :
$ ./configure
$ make
$ sudo ./src/ydlemaster -c ydle.conf

documentation complète disponible ici : http://wiki.ydle.fr/doku.php


ENGLISH VERSION :

You will need the following libraries :
- wiringPi
- microhttpd 
- jsoncpp
- curl
- libconfig++

Don't forget to create a config file !
Example are located in ydle.conf-example

Pins mapping is available here : http://wiringpi.com/pins/

For compiling
$ ./configure
$ make
$ sudo ./src/ydlemaster -c ydle.conf

Complete documentation is available : http://wiki.ydle.fr/doku.php
