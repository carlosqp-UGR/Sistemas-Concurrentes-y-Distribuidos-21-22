.SUFFIXES:
.PHONY: x1, x2, clean

compilador:=g++
opcionesc:= -std=c++11 -pthread -Wfatal-errors

x1: fumadores_mon_exe
	./$<

x2: lec_esc_mon_exe
	./$<

fumadores_mon_exe: fumadores_mon.cpp scd.cpp scd.h
	$(compilador) $(opcionesc)  -o $@ $<  scd.cpp

lec_esc_mon_exe: lec_esc_mon.cpp scd.cpp scd.h
	$(compilador) $(opcionesc)  -o $@ $<  scd.cpp

clean:
	rm -f *_exe
