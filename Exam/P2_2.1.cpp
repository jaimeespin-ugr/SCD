/*Ejercicio 1.
Modifique su solución del problema de los fumadores realizada mediante semáforos con las siguientes modalidades:

El mostrador tiene capacidad para dos ingredientes diferentes en vez de solo uno.
El estanquero debe realizar sus iteraciones de dos en dos. Es decir:
Producir dos ingredientes en cada iteración. Además, estos dos deben ser diferentes. 
Se produce el primero de ellos de forma aleatoria y después se puede entrar en un bucle donde se genere el otro hasta que sea distinto del primero.
Tras producir ambos ingredientes, se imprimirá un mensaje por pantalla indicando los ingredientes generados.
Depositar ambos en el mostrador, avisando a los fumadores que necesitan esos ingredientes.
Cuando el estanquero ha avisado a los fumadores, espera la confirmación de ambos para volver a poner otros ingredientes distintos. 
Por lo tanto, el estanquero debe asegurares que cada uno de los fumadores ha cogido su ingrediente, haciendo una espera, si fuera necesario, para cada fumador.
El comportamiento de los fumadores no cambia con respecto a la versión original, ya que cada fumador espera que le indiquen si su ingrediente está en el mostrador, y avisa al estanquero de ello, antes de fumar. Se debe usar un array de hebras para definir las hebras fumadoras y esas hebras fumadoras se deben generar a partir de una misma función.
*/
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
Semaphore sem_estanquero_espera = 0;



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
    int ingrediente1, ingrediente2;
   while(true){
        ingrediente1 = producir_ingrediente();
        
        do{
            ingrediente2 = producir_ingrediente();
        }while(ingrediente2 == ingrediente1);
        
        cout << "           Se produjeron los ingredientes: "<<ingrediente1<<" y "<<ingrediente2<<endl;

        sem_signal(s[ingrediente1]);
        sem_signal(s[ingrediente2]);

        sem_wait(sem_estanquero_espera);
        sem_wait(sem_estanquero_espera);
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
      sem_signal(sem_estanquero_espera);
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
   thread fumadores[num_fumadores];
   for (int i = 0; i < num_fumadores; ++i)
      fumadores[i] = thread(funcion_hebra_fumador, i);

   // Sincronización de hilos
   hebra_estanquero.join();
   for (int i = 0; i < num_fumadores; ++i)
      fumadores[i].join();
}
