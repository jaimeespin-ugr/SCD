#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random>
#include "scd.h"

using namespace std ;
using namespace scd ;
/*
Ejercicio 1.
A partir de su solución de múltiples productores y consumidores con planificación FIFO realizada mediante semáforos, se pide:

Cambiar las condiciones iniciales, para que tengamos:
5 productores.
3 consumidores.
El tamaño del buffer intermedio tenga 4 espacios.
Se produzcan un total de 45 datos.
Se deberá contabilizar en todo momento (en una variable compartida n_producidos) el número de elementos total que se han producido hasta ese momento (por todas las hebras productoras).
Se debe crear una nueva hebra impresora, que inicialmente estará bloqueada, y que es liberada una vez por cada hebra productora cuando produzca su quinto dato.
Cuando un productor produce su quinto dato, después de añadir el elemento al vector, debe desbloquear a la hebra impresora y luego debe bloquearse hasta que lo libere la hebra impresora.
La hebra impresora, una vez desbloqueada, imprime por pantalla el número de elementos producidos hasta ese momento (variable n_producidos), desbloquea al productor que la ha liberado y vuelve al principio de su ciclo a esperar ser liberada por otro productor.
La hebra impresora debe finalizar correctamente.
Puede encontrar la solución al problema en el siguiente enlace.
*/
//**********************************************************************
// Variables globales

const unsigned 
   num_productores = 5,
   num_consumidores = 3,
   num_items = 45 ,   // número de items
   tam_vec   = 4 ,   // tamaño del buffer
   p = num_items/num_productores,   //items por productor
   c = num_items/num_consumidores;  //items por consumidor


unsigned  
   cont_prod[num_items] = {0}, // contadores de verificación: para cada dato, número de veces que se ha producido.
   cont_cons[num_items] = {0}, // contadores de verificación: para cada dato, número de veces que se ha consumido.
   items[num_productores]= {0},
   n_producidos = 0; //número de elementos total que se han producido hasta ese momento (por todas las hebras productoras)
   

//Semáforos
Semaphore libres = tam_vec;   //número entradas libre (espera productir)
Semaphore ocupadas = 0;       //número entradas ocupadas (espera consumidor)
Semaphore impresora = 0;    //Semaforo de la hebra impresora

Semaphore sem_primera_libre=1;
Semaphore sem_primera_ocupada=1;
Semaphore sem_n_producidos=1;
Semaphore sem_prod_espera[num_productores] = {0, 0, 0, 0, 0};

int id_productor_a_liberar = -1; // Para que el productor le diga a la impresora a quién debe liberar

int primera_libre = 0;
int primera_ocupada = 0; // índice de la siguiente posición ocupada (lectura FIFO)
int buffer[tam_vec];

//**********************************************************************
// funciones comunes a las dos soluciones (fifo y lifo)
//----------------------------------------------------------------------

unsigned producir_dato(const unsigned int i)
{
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));
   const unsigned base = i*p;
   const unsigned dato_producido = base + items[i];
   items[i]++;
   cont_prod[dato_producido] ++ ;
   cout << "producido: " << dato_producido << endl << flush ;
   return dato_producido ;
}
//----------------------------------------------------------------------

void consumir_dato( unsigned dato )
{
   assert( dato < num_items );
   cont_cons[dato] ++ ;
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));

   cout << "                  consumido: " << dato << endl ;

}


//----------------------------------------------------------------------

void test_contadores()
{
   bool ok = true ;
   cout << "comprobando contadores ...." ;
   for( unsigned i = 0 ; i < num_items ; i++ )
   {  if ( cont_prod[i] != 1 )
      {  cout << "error: valor " << i << " producido " << cont_prod[i] << " veces." << endl ;
         ok = false ;
      }
      if ( cont_cons[i] != 1 )
      {  cout << "error: valor " << i << " consumido " << cont_cons[i] << " veces" << endl ;
         ok = false ;
      }
   }
   if (ok)
      cout << endl << flush << "solución (aparentemente) correcta." << endl << flush ;
}

//----------------------------------------------------------------------

void  funcion_hebra_productora(const unsigned int i)
{
   for( unsigned j = 0 ; j < p ; j++ )
   {
      int dato = producir_dato(i);
      
      sem_wait(libres);
      sem_wait(sem_primera_libre);
      buffer[primera_libre]=dato;
      primera_libre=(primera_libre+1)%tam_vec;
      sem_signal(sem_primera_libre);
      sem_signal(ocupadas);

      sem_wait(sem_n_producidos);
      n_producidos++;
      sem_signal(sem_n_producidos);

      if(j==4){ //El productor va a fumar su quinto dato
         id_productor_a_liberar = i;
         sem_signal(impresora);
         sem_wait(sem_prod_espera[i]);
      }
        
   }
}

//----------------------------------------------------------------------

void funcion_hebra_consumidora(const unsigned int j)
{
   for( unsigned i = 0 ; i < c ; i++ )
   {
      int dato ;
      sem_wait(ocupadas);
      sem_wait(sem_primera_ocupada);
      dato = buffer[primera_ocupada];
      primera_ocupada = (primera_ocupada+1)%tam_vec;
      sem_signal(sem_primera_ocupada);
      sem_signal(libres);
      consumir_dato( dato );
    }
}

void función_hebra_impresora(){
    for(int i = 0; i<num_productores; i++){
      sem_wait(impresora); //Espera a que la productora lo despierte
      
      sem_wait(sem_n_producidos);
      cout<<"           (Impresora) n_producidos: "<<n_producidos <<endl;
      sem_signal(sem_n_producidos);

      sem_signal(sem_prod_espera[id_productor_a_liberar]);
    }
}
//----------------------------------------------------------------------

int main()
{
   cout << "-----------------------------------------------------------------" << endl
        << "Problema de los productores-consumidores solución FIFO." << endl
        << "------------------------------------------------------------------" << endl
        << flush ;


   thread productores[num_productores];
   thread consumidores[num_consumidores];
   thread impresora_hebra(función_hebra_impresora);

   for(int i = 0; i<num_productores; i++){
      productores[i]=thread(funcion_hebra_productora, i);
   }

   for(int i = 0; i<num_consumidores; i++){
      consumidores[i]=thread(funcion_hebra_consumidora, i);
   }

   for(int i = 0; i < num_productores; i++) {
      productores[i].join();
   }

   for(int i = 0; i < num_consumidores; i++) {
      consumidores[i].join();
   }
   impresora_hebra.join();
   
   test_contadores();
}


