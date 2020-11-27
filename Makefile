serial: src/main.o src/bruteforce/bruteforce-serial.o src/dictionary/dictionary-serial.o src/hash/hash-sha256.o
	mkdir -p bin
	gcc -o bin/serial-cracker src/main.o src/bruteforce/bruteforce-serial.o src/dictionary/dictionary-serial.o src/hash/hash-sha256.o -lm -lcrypto

src/main.o: src/main.c
	gcc -c src/main.c -o src/main.o

src/bruteforce/bruteforce-serial.o: src/bruteforce/bruteforce-serial.c
	gcc -c src/bruteforce/bruteforce-serial.c -o src/bruteforce/bruteforce-serial.o

src/dictionary/dictionary-serial.o: src/dictionary/dictionary-serial.c
	gcc -c src/dictionary/dictionary-serial.c -o src/dictionary/dictionary-serial.o

src/hash/hash-sha256.o: src/hash/hash-sha256.c
	gcc -c src/hash/hash-sha256.c -o src/hash/hash-sha256.o

clean:
	rm -rf src/*.o src/*/*.o bin 