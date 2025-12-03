# Variables
CXX = g++
CXXFLAGS = -std=c++11 -pthread -Wall

# Archivos fuente y ejecutables
SOURCES = scd.cpp
EXECUTABLES = prodcons-LIFO prodcons-FIFO fumadores prodcons-FIFOmult prodcons-LIFOmult

# Regla general para compilar todos los ejecutables
all: $(EXECUTABLES)

# Reglas específicas para cada ejecutable
prodcons-LIFO: prodcons-LIFO.cpp scd.h scd.cpp
	$(CXX) $(CXXFLAGS) prodcons-LIFO.cpp scd.cpp -o prodcons-LIFO

prodcons-FIFO: prodcons-FIFO.cpp scd.h scd.cpp
	$(CXX) $(CXXFLAGS) prodcons-FIFO.cpp scd.cpp -o prodcons-FIFO

fumadores: fumadores.cpp scd.h scd.cpp
	$(CXX) $(CXXFLAGS) fumadores.cpp scd.cpp -o fumadores

prodcons-FIFOmult: prodcons-FIFOmult.cpp scd.h scd.cpp
	$(CXX) $(CXXFLAGS) prodcons-FIFOmult.cpp scd.cpp -o prodcons-FIFOmult

prodcons-LIFOmult: prodcons-LIFOmult.cpp scd.h scd.cpp
	$(CXX) $(CXXFLAGS) prodcons-LIFOmult.cpp scd.cpp -o prodcons-LIFOmult

# Reglas para ejecutar cada programa
run-prodcons-LIFO: prodcons-LIFO
	./prodcons-LIFO

run-prodcons-FIFO: prodcons-FIFO
	./prodcons-FIFO

run-fumadores: fumadores
	./fumadores

run-prodcons-FIFOmult: prodcons-FIFOmult
	./prodcons-FIFOmult

run-prodcons-LIFOmult: prodcons-LIFOmult
	./prodcons-LIFOmult

# Limpieza
clean:
	rm -f $(EXECUTABLES)