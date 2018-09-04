code: commandline_parser.o helpm.o ui.o dispatcher.o scheduler.o
	gcc commandline_parser.o helpm.o ui.o scheduler.o dispatcher.o -o aubatch -lpthread

commandline_parser.o: commandline_parser.c
	gcc -c commandline_parser.c

helpm.o: helpm.c
	gcc -c helpm.c

ui.o: ui.c
	gcc -c ui.c

scheduler.o: scheduler.c
	gcc -c scheduler.c

dispatcher.o: dispatcher.c
	gcc -c dispatcher.c

clean:
	rm *.o aubatch
process: 
