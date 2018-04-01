CC=g++
CFLAGS=-g -c -Og -Wall -std=c++11
LIBS=-lpthread

ic7300jig: ic7300jig.o serial.o ic7300.o scope_waveform_data.o scope_waveform_data.o
	$(CC) -o ic7300jig ic7300jig.o serial.o ic7300.o scope_waveform_data.o $(LIBS)

ic7300jig.o: ic7300jig.cxx serial.h
	$(CC) $(CFLAGS) ic7300jig.cxx

serial.o: serial.cxx serial.h ic7300.h
	$(CC) $(CFLAGS) serial.cxx

ic7300.o: ic7300.cxx ic7300.h 
	$(CC) $(CFLAGS) ic7300.cxx

scope_waveform_data.o: scope_waveform_data.cxx scope_waveform_data.h ic7300.h
	$(CC) $(CFLAGS) scope_waveform_data.cxx

.PHONY: all
all: ic7300jig

.PHONY: clean
clean:
	rm -f *~ *.o ic7300jig a.out
