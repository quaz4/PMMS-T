OBJECTS = pmms.o fileIO.o

pmms : $(OBJECTS)
	gcc $(OBJECTS) -o pmms -lpthread

pmms.o : pmms.c
	gcc -c pmms.c fileIO.c -Wall -pedantic

fileIO.o : fileIO.c
	gcc -c pmms.c fileIO.c -Wall -pedantic

clean : 
	rm -f pmms $(OBJECTS)
