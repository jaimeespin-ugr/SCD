#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random>
#include "scd.h"

using namespace std ;
using namespace scd ;

//**********************************************************************
// Variables globales

const unsigned 
   num_productores = 4,
   num_consumidores = 5,
   num_items = 40 ,   // número de items
	tam_vec   = 10 ,   // tamaño del buffer
   p = num_items/num_productores,
   c = num_items/num_consumidores;
   
unsigned  
   cont_prod[num_items] = {0}, // contadores de verificación: para cada dato, número de veces que se ha producido.
   cont_cons[num_items] = {0}, // contadores de verificación: para cada dato, número de veces que se ha consumido.
   items[num_productores]= {0} ; 

//Semáforos
Semaphore libres = tam_vec;   //número entradas libre
Semaphore ocupadas = 0;       //número entradas ocupadas

Semaphore sem_primera_libre=1;
Semaphore sem_primera_ocupada=1;
int primera_libre = 0;
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
      int dato = producir_dato(i) ;
      sem_wait(libres);
      sem_wait(sem_primera_libre);
      buffer[primera_libre]=dato;
      primera_libre++;
      sem_signal(sem_primera_libre);
      sem_signal(ocupadas);
   }
}

//----------------------------------------------------------------------

void funcion_hebra_consumidora(const unsigned int j)
{
   for( unsigned i = 0 ; i < c ; i++ )
   {
      int dato ;
      sem_wait(ocupadas);
      sem_wait(sem_primera_libre);
      primera_libre--;
      dato = buffer[primera_libre];
      sem_signal(sem_primera_libre);
      sem_signal(libres);
      consumir_dato( dato ) ;
    }
}
//----------------------------------------------------------------------

int main()
{
   cout << "-----------------------------------------------------------------" << endl
        << "Problema de los productores-consumidores solución LIFO." << endl
        << "------------------------------------------------------------------" << endl
        << flush ;


   thread productores[num_productores];
   thread consumidores[num_consumidores];

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
   
   test_contadores();
}
