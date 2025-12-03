/*Ejercicio 2.
Modifque la solución del problema de los fumadores realizada con monitores teniendo en cuenta que:

Tenemos un mostrador que tiene capacidad para dos ingredientes y el estanquero pone dos ingredientes diferentes cada vez, permitiendo que dos fumadores distintos tengan sus ingredientes para fumar. Se realizarán unos cambios:

El estado del mostrador se representa con un array de celdas, cada celda almacenará un valor que indicará el ingrediente correspondiente que hay en esa zona del mostrador, o ausencia dl ingrediente en esa zona.
Operaciones del monitor Estanco modificadas:
Poner ingrediente: Como estanquero produce dos ingredientes diferentes en el mostrador, la interfaz de operación y su implementación se tienen que modificar para parmitir que se pongan dos ingredientes en el mostrador e indicar que se permite continuar a dos fumadores.
Obtener ingrediente: La operación se tiene que modificar ya que el fumador debe asegurarse de que su ingrediente está en alguna de las zonas del mostrador, dejar vacía esa zona y, si la otra zona también estaba vacía, indicar al estanquero que el mostrador ha quedado totalmente vacío (por si estuviera esperando).
Esperar recogida ingrediente: La condición lógica que describe el mostrador no está vacío debe cambiarse al ser más compleja esta nueva situacion.
*/

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
 
 int ingredientes[2] = {-1,-1}; // -1: vacío. 0, 1, 2: contiene ese ingrediente.
 CondVar                    // colas condicion:
   espera_fumador[num_fumadores],
   espera_recogida;

 public:                    // constructor y métodos públicos
   Estanco() ;             // constructor
   void obtenerIngrediente(int i);
   void ponerIngrediente(int ing1, int ing2);
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
   if(ingredientes[0]!=i && ingredientes[1]!=i)
   {
      espera_fumador[i].wait();
   }
   // Primero retirar el ingrediente del mostrador y luego avisar al estanquero.
   // En monitores Hoare la señal debe hacerse después de actualizar el estado
   // compartido para evitar que el productor despierte y vuelva a esperar
   // antes de que el consumidor haya terminado de actualizar.
   if(ingredientes[0]==i){
        ingredientes[0] = -1;
   }else if(ingredientes[1]==i){
        ingredientes[1] = -1;
   }

   if(ingredientes[0]==-1 && ingredientes[1]==-1){
        espera_recogida.signal();
   }
    
}

void Estanco::ponerIngrediente(int ing1, int ing2){
    ingredientes[0] = ing1;
    ingredientes[1] = ing2;
    espera_fumador[ing1].signal();
    espera_fumador[ing2].signal();
}

void Estanco::esperarRecogidaIngrediente(){
   //espera bloqueado hasta
   //que el mostrador está libre
   if(ingredientes[0]!=-1 || ingredientes[1]!=-1){
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
      int ingrediente1 = ProducirIngrediente();
      int ingrediente2;
      
      do{
        ingrediente2 = ProducirIngrediente();
      }while(ingrediente2 == ingrediente1);

      monitor->ponerIngrediente(ingrediente1, ingrediente2);
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
