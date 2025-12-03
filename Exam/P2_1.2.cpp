/*
Ejercicio 2.
A partir de su solución del problema de los fumadores realizada mediante monitores, se pide:

Crear un cuarto fumador (fumador 3), el cual necesitará el ingrediente 1 para fumar. Cuando el estanquero produzca dicho ingrediente, decidirá aleatoriamente a cual de los dos fumadores liberar.
Crear una hebra verificadora, que será también un ciclo infinito y cuyo código solamente será un nuevo procedimiento del monitor llamado verificacion.
El monitor tendrá una variable permanente ini_verif (booleada, inicializada a false).
En el procedimiento verificacion se comprueba inicialmente el valor de ini_verif. Si es false, se bloquea la hebra verificadora. Si es true (o cuando es desbloqueada), se muestra por pantalla el número total de veces que han fumado los fumadores que comparten el ingrediente 1 (número de veces del fumador 1 y número de veces del fumador 3) y se pone ini_verif a false.
Cada vez que el estanquero genera 12 veces el ingrediente 1, pone ini_verif a true y desbloquea a la hebra verificadora.
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

const int num_fumadores = 4;

mutex mtx;

// Monitor
class Estanco : public HoareMonitor
{
 private:
 
 int ingrediente = -1; // -1: vacío. 0, 1, 2: contiene ese ingrediente.
 CondVar                    // colas condicion:
    espera_fumador[num_fumadores],
    espera_recogida,
    espera_verificador; // para la hebra verificadora

 bool ini_verif = false; // indicador para la hebra verificadora
 int contador_fumador[num_fumadores]; // contadores de veces que ha fumado cada fumador
 int producciones_ingrediente1 = 0; // contador de producciones del ingrediente 1
 int necesita[num_fumadores]; // mapping fumador -> ingrediente que necesita

 public:                    // constructor y métodos públicos
   Estanco() ;             // constructor
   void obtenerIngrediente(int i);
   void ponerIngrediente(int producido, int destino);
   void esperarRecogidaIngrediente();
   void verificacion();
} ;

// Constructor: crear las colas condicion en el monitor
Estanco::Estanco()
{
   for(int i = 0; i < num_fumadores; ++i)
      espera_fumador[i] = newCondVar();
   espera_recogida = newCondVar();
   espera_verificador = newCondVar();
  
   // inicializar contadores
   for(int i=0;i<num_fumadores;++i) contador_fumador[i]=0;
   // mapear qué ingrediente necesita cada fumador
   // fumador 0 necesita 0, fumador 1 necesita 1, fumador 2 necesita 2, fumador 3 necesita 1
   necesita[0] = 0; necesita[1] = 1; necesita[2] = 2; necesita[3] = 1;
}

void Estanco::obtenerIngrediente(int i){
   //espera bloqueado a que su
   //ingrediente esté disponible, 
   //y luego lo retira del mostrador
   while(ingrediente!=necesita[i])
   {
      espera_fumador[i].wait();
   }
   // incrementar contador del fumador que ha obtenido su ingrediente
   contador_fumador[i]++;
   // Primero retirar el ingrediente del mostrador y luego avisar al estanquero.
   // En monitores Hoare la señal debe hacerse después de actualizar el estado
   // compartido para evitar que el productor despierte y vuelva a esperar
   // antes de que el consumidor haya terminado de actualizar.
   ingrediente = -1;
   espera_recogida.signal();
}

void Estanco::ponerIngrediente(int producido, int destino){
   // pone el ingrediente producido en el mostrador y despierta al fumador destino
   ingrediente = producido;
   espera_fumador[destino].signal();

   // si se produjo el ingrediente 1, contar y activar verificador cada 12 producciones
   if(producido == 1){
      producciones_ingrediente1++;
      if(producciones_ingrediente1 >= 12){
         producciones_ingrediente1 = 0;
         ini_verif = true;
         espera_verificador.signal();
      }
   }
}

void Estanco::esperarRecogidaIngrediente(){
   //espera bloqueado hasta
   //que el mostrador está libre
   while(ingrediente!=-1){
      espera_recogida.wait();
   }
}

void Estanco::verificacion(){
   // si ini_verif es false, la hebra verificadora se bloquea
   if(!ini_verif)
      espera_verificador.wait();

   // al despertarse (ini_verif == true), mostrar los contadores de los fumadores que
   // comparten el ingrediente 1 (fumador 1 y fumador 3) y poner ini_verif a false.
   int c1 = contador_fumador[1];
   int c3 = contador_fumador[3];
   cout<<endl;
   cout << "            [VERIFICADOR] Fumador 1 = " << c1 << ", Fumador 3 = " << c3
        << ", total = " << (c1 + c3) << endl;
    cout<<endl;
   ini_verif = false;
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

   const int num_ingrediente = aleatorio<0,2>() ;

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
      // si el ingrediente producido es 1, decidir aleatoriamente liberar al fumador 1 o 3
      int destino = ingrediente;
      if(ingrediente == 1){
         int r = aleatorio<0,1>();
         destino = (r==0 ? 1 : 3);
      }
      monitor->ponerIngrediente(ingrediente, destino);
      monitor->esperarRecogidaIngrediente();
   }
}

// función que ejecuta la hebra verificadora
void funcion_hebra_verificador(MRef<Estanco> monitor)
{
   while(true){
      monitor->verificacion();
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

   // Crear la hebra verificadora
   thread hebra_verificador(funcion_hebra_verificador, monitor);

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
   hebra_verificador.join();
}
