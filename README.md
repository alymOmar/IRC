# IRC

To compile :
```sh
cd ./src
chmod .x ./makefile
./makefile
```

To start the server :
```sh
cd ./buid
./irc_Server
```

To join as a client :
```sh
cd ./buid
./irc_Client <IP> <PORT>
```
ip = 10.0.0.30 and port = 6665

To perform the BOF attack from another machine :
```sh
python ./BOF_IRC.py
```
and
```sh
sudo nc -nlvp 666
```
