#include <iostream>
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

//Tres semáforos, uno para cada fumador.
Semaphore s[num_fumadores]={0,0,0};


//-------------------------------------------------------------------------
// Función que simula la acción de producir un ingrediente, como un retardo
// aleatorio de la hebra (devuelve número de ingrediente producido)

int producir_ingrediente()
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
void funcion_hebra_estanquero(  )
{
   while(true){
      int ingrediente = producir_ingrediente();
      sem_signal(s[ingrediente]);
   }
}

//-------------------------------------------------------------------------
// Función que simula la acción de fumar, como un retardo aleatoria de la hebra

void fumar( int num_fumador )
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
void funcion_hebra_fumador( int num_fumador )
{
   while( true )
   {
      // Esperar el ingrediente correspondiente
      sem_wait(s[num_fumador]);

      // Fumar
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

   // Crear la hebra del estanquero
   thread hebra_estanquero(funcion_hebra_estanquero);

   // Crear las hebras de los fumadores, pasando su número como argumento
   thread hebra_fumador1(funcion_hebra_fumador, 0);
   thread hebra_fumador2(funcion_hebra_fumador, 1);
   thread hebra_fumador3(funcion_hebra_fumador, 2);

   // Sincronización de hilos
   hebra_estanquero.join();
   hebra_fumador1.join();
   hebra_fumador2.join();
   hebra_fumador3.join();
}
