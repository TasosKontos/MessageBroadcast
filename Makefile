all: chan p1enc1 p2enc2

chan:
	gcc chan.c -o chan -lpthread -lcrypto

p1enc1:
	gcc p1enc1.c -o p1enc1 -lpthread -lcrypto

p2enc2:
	gcc p2enc2.c -o p2enc2 -lpthread -lcrypto
