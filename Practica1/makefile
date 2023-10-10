.SUFFIXES:
.PHONY: pc1,pc2,fu,clean 
.PRECIOUS: %_exe

compilador:=g++

pc1: prodcons-plantillaLIFO_exe
	./$<

pc2: prodcons-plantillaFIFO_exe
	./$<
	
fu: fumadores-plantilla_exe
	./$< 

## solo para tests
t: test-semaforos_exe
	./$< | more

## regla para compilar (.cpp --> _exe)
%_exe: %.cpp scd.cpp scd.h
		$(compilador) -std=c++11 -pthread -o $@ $< scd.cpp

clean:
	rm -f *_exe
