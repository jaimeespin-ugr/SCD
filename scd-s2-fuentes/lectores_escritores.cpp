#include <iostream>
#include <iomanip>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "scd.h"

using namespace std ;
using namespace scd ;

// Monitor
class Lec_Esc : public HoareMonitor
{
 private: 
 bool escrib; // true si está escrbiendo
 unsigned n_lec; //numero de lectores leyendo

 CondVar
    lectura,    //esperar cuando ya hay un escritor escibiendo
    escritura;  //esperar cuando hay otro escrito escribiendo

 public:                    // constructor y métodos públicos
   Lec_Esc() ;             // constructor
   void ini_lectura();
   void fin_lectura();
   void ini_escritura();
   void fin_escritura();
   
} ;

Lec_Esc::Lec_Esc(){
    n_lec = 0;
    escrib = false;
    lectura = newCondVar();
    escritura = newCondVar();
}

void Lec_Esc::ini_lectura(){
    if(escrib){
        lectura.wait();
    }

    n_lec++;
    lectura.signal();
}

void Lec_Esc::fin_lectura(){
    n_lec--;
    if(n_lec==0){
        escritura.signal();
    }
}

void Lec_Esc::ini_escritura(){
    if(n_lec>0 || escrib){
        escritura.wait();
    }
    escrib = true;
}

void Lec_Esc::fin_escritura(){
    escrib = false;
    if(!lectura.empty()){
        lectura.signal();
    }else{
        escritura.signal();
    }
}

void leer(int num_hebra){
    // calcular milisegundos aleatorios de duración de la acción
    chrono::milliseconds duracion( aleatorio<20,200>() );

    cout << "Leyendo "<<num_hebra<<endl;

    // espera bloqueada un tiempo igual a 'duracion' milisegundos
    this_thread::sleep_for( duracion );
}

void escribir(int num_hebra){
    // calcular milisegundos aleatorios de duración de la acción
    chrono::milliseconds duracion( aleatorio<20,200>() );

    cout << "Escribiendo "<<num_hebra<<endl;

    // espera bloqueada un tiempo igual a 'duracion' milisegundos
    this_thread::sleep_for( duracion );
}

void funcion_hebra_lectora(MRef<Lec_Esc> monitor, int num_hebra){
    while (true){
        monitor->ini_lectura();
        leer(num_hebra);
        monitor->fin_lectura();
        leer(num_hebra);
    }
}

void funcion_hebra_escritora(MRef<Lec_Esc> monitor, int num_hebra){
    while (true){
        monitor->ini_escritura();
        escribir(num_hebra);
        monitor->fin_escritura();
        escribir(num_hebra);
    }
}

int main(){
    cout << "-------------------------" << endl
        << "Problema de los lectores/escritores" << endl
        << "-------------------------" << endl
        << flush ;

   // Crear el monitor (referencia compartida) y las hebras
   MRef<Lec_Esc> monitor = Create<Lec_Esc>();

   // Crear la hebra del estanquero (pasa el monitor)
   thread hebra_escritora[4];

   // Crear las hebras de los fumadores, pasando el monitor y su número
   thread hebra_lectora[4];

   // Sincronización de hilos
   for(int i = 0; i<4; i++){
    hebra_escritora[i] = thread(funcion_hebra_escritora, monitor, i);
    hebra_lectora[i] = thread(funcion_hebra_lectora, monitor, i);
   }

   for(int i = 0; i<4; i++){
    hebra_escritora[i].join();
    hebra_lectora[i].join();
   }
}