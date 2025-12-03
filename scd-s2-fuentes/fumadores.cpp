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

// numero de fumadores 

const int num_fumadores = 3;

mutex mtx;

// Monitor
class Estanco : public HoareMonitor
{
 private:
 
 int ingrediente = -1; // -1: vacío. 0, 1, 2: contiene ese ingrediente.
 CondVar                    // colas condicion:
   espera_fumador[num_fumadores],
   espera_recogida;

 public:                    // constructor y métodos públicos
   Estanco() ;             // constructor
   void obtenerIngrediente(int i);
   void ponerIngrediente(int i);
   void esperarRecogidaIngrediente();
} ;

// Constructor: crear las colas condicion en el monitor
Estanco::Estanco()
{
   for(int i = 0; i < num_fumadores; ++i)
      espera_fumador[i] = newCondVar();
   espera_recogida = newCondVar();
}

void Estanco::obtenerIngrediente(int i){
   //espera bloqueado a que su
   //ingrediente esté disponible, 
   //y luego lo retira del mostrador
   if(ingrediente!=i)
   {
      espera_fumador[i].wait();
   }
   // Primero retirar el ingrediente del mostrador y luego avisar al estanquero.
   // En monitores Hoare la señal debe hacerse después de actualizar el estado
   // compartido para evitar que el productor despierte y vuelva a esperar
   // antes de que el consumidor haya terminado de actualizar.
   ingrediente = -1;
   espera_recogida.signal();
}

void Estanco::ponerIngrediente(int i){
   //pone el ingrediente i en el mostrador
   ingrediente=i;
   espera_fumador[i].signal();
}

void Estanco::esperarRecogidaIngrediente(){
   //espera bloqueado hasta
   //que el mostrador está libre
   if(ingrediente!=-1){
      espera_recogida.wait();
   }
}

//-------------------------------------------------------------------------
// Función que simula la acción de producir un ingrediente, como un retardo
// aleatorio de la hebra (devuelve número de ingrediente producido)

int ProducirIngrediente() //producir_dato
{
   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_produ( aleatorio<10,100>() );

   // informa de que comienza a producir
   cout << "Estanquero : empieza a producir ingrediente (" << duracion_produ.count() << " milisegundos)" << endl;

   // espera bloqueada un tiempo igual a ''duracion_produ' milisegundos
   this_thread::sleep_for( duracion_produ );

   const int num_ingrediente = aleatorio<0,num_fumadores-1>() ;

   // informa de que ha terminado de producir
   cout << "Estanquero : termina de producir ingrediente " << num_ingrediente << endl;

   return num_ingrediente ;
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del estanquero
// El estanquero produce ingredientes de forma indefinida y notifica
// al fumador correspondiente mediante el semáforo asociado.
void funcion_hebra_estanquero( MRef<Estanco> monitor ) //funcion_hebra_consumidora
{
   while(true){
      int ingrediente = ProducirIngrediente();
      monitor->ponerIngrediente(ingrediente);
      monitor->esperarRecogidaIngrediente();
   }
}

//-------------------------------------------------------------------------
// Función que simula la acción de fumar, como un retardo aleatoria de la hebra

void fumar( int num_fumador ) //consumir_dato
{

   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_fumar( aleatorio<20,200>() );

   // informa de que comienza a fumar

    cout << "Fumador " << num_fumador << "  :"
          << " empieza a fumar (" << duracion_fumar.count() << " milisegundos)" << endl;

   // espera bloqueada un tiempo igual a ''duracion_fumar' milisegundos
   this_thread::sleep_for( duracion_fumar );

   // informa de que ha terminado de fumar

    cout << "Fumador " << num_fumador << "  : termina de fumar, comienza espera de ingrediente." << endl;

}


//----------------------------------------------------------------------
// función que ejecuta la hebra del fumador
// Cada fumador espera su ingrediente específico, lo consume (fuma)
// y luego vuelve a esperar en un bucle infinito.
void funcion_hebra_fumador( MRef<Estanco> monitor, int num_fumador )//funcion_hebra_consumidora
{
   while( true )
   {
      monitor->obtenerIngrediente(num_fumador);
      fumar(num_fumador);
   }
}

//----------------------------------------------------------------------
// Función principal
// Crea las hebras del estanquero y los fumadores, y las pone en marcha.
int main()
{
   cout << "-------------------------" << endl
        << "Problema de los fumadores" << endl
        << "-------------------------" << endl
        << flush ;

   // Crear el monitor (referencia compartida) y las hebras
   MRef<Estanco> monitor = Create<Estanco>();

   // Crear la hebra del estanquero (pasa el monitor)
   thread hebra_estanquero(funcion_hebra_estanquero, monitor);

   // Crear las hebras de los fumadores, pasando el monitor y su número
   thread hebra_fumador[num_fumadores];

   for(int i = 0; i<num_fumadores; i++){
    hebra_fumador[i]=thread(funcion_hebra_fumador, monitor, i);
   }

   // Sincronización de hilos
   for(int i = 0; i<num_fumadores; i++){
    hebra_fumador[i].join();
   }

   hebra_estanquero.join();
}
