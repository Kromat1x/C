CFLAGS = /nologo /W3 /D_CRT_SECURE_NO_DEPRECATE

build: tema1

tema1: tema1.obj main.obj hash.lib
	cl $(CFLAGS) tema1.obj main.obj hash.lib
	
tema1.o: tema1.c
	cl $(CFLAGS)/c tema1.c

main.o: main.c
	cl $(CFLAGS) /c main.c

clean:
	del *.o tema1.exe