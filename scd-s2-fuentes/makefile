.SUFFIXES:
.PHONY: x, pc1, clean, monitor, fifo, lifo, pcmu, lectores_escritores

compilador:=g++
opcionesc:= -std=c++11 -pthread -Wfatal-errors

# Targets to run common executables
x: monitor_em_exe
	./$<

pc1: prodcons1_su_exe
	./$<

pc1b: prodcons1_su_fifo_exe
	./$<

# Generic pattern target (keeps backward compatibility)
%_exe: %.cpp scd.cpp scd.h
	$(compilador) $(opcionesc) -o $@ $< scd.cpp

# Explicit rules for sources in this folder
monitor_em_exe: monitor_em.cpp scd.cpp scd.h
	$(compilador) $(opcionesc) -o $@ $^

prodconsFIFO_exe: prodconsFIFO.cpp scd.cpp scd.h
	$(compilador) $(opcionesc) -o $@ $^

prodconsLIFO_exe: prodconsLIFO.cpp scd.cpp scd.h
	$(compilador) $(opcionesc) -o $@ $^

prodcons_mu_exe: prodcons_mu.cpp scd.cpp scd.h
	$(compilador) $(opcionesc) -o $@ $^

# Rule to build the fumadores example
fumadores_exe: fumadores.cpp scd.cpp scd.h
	$(compilador) $(opcionesc) -o $@ $^

# Explicit rule for lectores_escritores source
lectores_escritores_exe: lectores_escritores.cpp scd.cpp scd.h
	$(compilador) $(opcionesc) -o $@ $^

# Convenience run targets
monitor: monitor_em_exe
	./monitor_em_exe

fifo: prodconsFIFO_exe
	./prodconsFIFO_exe

lifo: prodconsLIFO_exe
	./prodconsLIFO_exe

pcmu: prodcons_mu_exe
	./prodcons_mu_exe

fumadores: fumadores_exe
	./fumadores_exe

clean:
	rm -f *_exe
