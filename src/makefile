sudo bash -c 'echo 0 > /proc/sys/kernel/randomize_va_space'


gcc -fno-stack-protector -z execstack -mpreferred-stack-boundary=3 -g -c irc_Server_Functions.c
gcc -shared -o libIRCServer.so -fno-stack-protector -z execstack -mpreferred-stack-boundary=3 -g irc_Server_Functions.o
sudo cp libIRCServer.so /usr/lib/
sudo chmod 0755 /usr/lib/libIRCServer.so

gcc -pthread irc_Client.c -o ../build/irc_Client -fno-stack-protector -z execstack -mpreferred-stack-boundary=3 -g

gcc -o ../build/irc_Server -fno-stack-protector -z execstack -mpreferred-stack-boundary=3 -g -pthread irc_Server.c -lIRCServer