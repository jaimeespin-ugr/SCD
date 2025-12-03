// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Seminario 2. Introducción a los monitores en C++11.
//
// Archivo: prodcons_mu_fifo.cpp
//
// Ejemplo de un monitor en C++11 con semántica SU, para el problema
// del productor/consumidor, con productor y consumidor únicos.
// Opcion FIFO (cola circular)
//
// -----------------------------------------------------------------------------------


#include <iostream>
#include <iomanip>
#include <cassert>
#include <random>
#include <thread>
#include "scd.h"

using namespace std ;
using namespace scd ;

const int
   num_productores = 4,
   num_consumidores = 5;
constexpr int
   num_items = 40;   // número de items a producir/consumir
constexpr int
   items_por_productor = num_items/num_productores,
   items_por_consumidor = num_items/num_consumidores;

int producidos_por_hebra[num_productores];

constexpr int
   min_ms    = 5,     // tiempo minimo de espera en sleep_for
   max_ms    = 20 ;   // tiempo máximo de espera en sleep_for


mutex
   mtx ;                 // mutex de escritura en pantalla
unsigned
   cont_prod[num_items] = {0}, // contadores de verificación: producidos
   cont_cons[num_items] = {0}; // contadores de verificación: consumidos

//**********************************************************************
// funciones comunes a las dos soluciones (fifo y lifo)
//----------------------------------------------------------------------

int producir_dato( int num_hebra )
{
   this_thread::sleep_for( chrono::milliseconds( aleatorio<min_ms,max_ms>() ));
   const int valor_producido = num_hebra * items_por_productor + producidos_por_hebra[num_hebra];
   producidos_por_hebra[num_hebra]++;
   mtx.lock();
   cout << "hebra productora " << num_hebra << ", produce " << valor_producido << endl << flush ;
   mtx.unlock();
   cont_prod[valor_producido]++ ;
   return valor_producido ;
}
//----------------------------------------------------------------------

void consumir_dato( unsigned valor_consumir, int num_hebra )
{
   if ( num_items <= valor_consumir )
   {
      cout << " valor a consumir === " << valor_consumir << ", num_items == " << num_items << endl ;
      assert( valor_consumir < num_items );
   }
   cont_cons[valor_consumir] ++ ;
   this_thread::sleep_for( chrono::milliseconds( aleatorio<min_ms,max_ms>() ));
   mtx.lock();
   cout << "                  hebra consumidora "<< num_hebra <<", consume: " << valor_consumir << endl ;
   mtx.unlock();
}
//----------------------------------------------------------------------

void test_contadores()
{
   bool ok = true ;
   cout << "comprobando contadores ...." << endl ;

   for( unsigned i = 0 ; i < num_items ; i++ )
   {
      if ( cont_prod[i] != 1 )
      {
         cout << "error: valor " << i << " producido " << cont_prod[i] << " veces." << endl ;
         ok = false ;
      }
      if ( cont_cons[i] != 1 )
      {
         cout << "error: valor " << i << " consumido " << cont_cons[i] << " veces" << endl ;
         ok = false ;
      }
   }
   if (ok)
      cout << endl << flush << "solución (aparentemente) correcta." << endl << flush ;
}

// *****************************************************************************
// clase para monitor buffer, version FIFO (cola circular), semántica SU, multiples prod/cons

class ProdConsMu : public HoareMonitor
{
 private:
 static const int           // constantes ('static' ya que no dependen de la instancia)
   num_celdas_total = 10;   //   núm. de entradas del buffer
 int                        // variables permanentes
    buffer[num_celdas_total],//   buffer de tamaño fijo, con los datos
    primera_libre,           //   indice de celda de la próxima inserción
    primera_ocupada,         //   indice de la próxima extracción (lectura)
                                        //  (FIFO-only: LIFO no tiene este índice)
    num_ocupadas ;           //   número de celdas ocupadas
                                        //  (FIFO-only: contador de ocupadas; LIFO usa 'primera_libre' como ocupadas)

 CondVar                    // colas condicion:
   ocupadas,                //  cola donde espera el consumidor (n>0)
   libres ;                 //  cola donde espera el productor  (n<num_celdas_total)

 public:                    // constructor y métodos públicos
   ProdConsMu() ;             // constructor
   int  leer();                // extraer un valor (sentencia L) (consumidor)
   void escribir( int valor ); // insertar un valor (sentencia E) (productor)
} ;
// -----------------------------------------------------------------------------

ProdConsMu::ProdConsMu(  )
{
   primera_libre = 0 ;
   primera_ocupada = 0 ;   // FIFO: inicializa índice cabeza de lectura (no presente en LIFO)
   num_ocupadas = 0 ;      // FIFO: inicializa contador de elementos ocupados (LIFO usa primera_libre)
   ocupadas      = newCondVar();
   libres        = newCondVar();
}
// -----------------------------------------------------------------------------
// función llamada por el consumidor para extraer un dato (FIFO)

int ProdConsMu::leer(  )
{
   // FIFO: si no hay ocupadas, espera el consumidor
   // (diferencia con LIFO: LIFO comprueba 'primera_libre == 0')
   while ( num_ocupadas == 0 )
      ocupadas.wait();

   assert( num_ocupadas > 0 );

   // FIFO: leer el elemento en la cabeza de la cola (primera_ocupada)
   // (diferencia con LIFO: LIFO leería desde buffer[primera_libre-1])
   const int valor = buffer[primera_ocupada] ;

   // FIFO: avanzar índice circular de lectura
   primera_ocupada = (primera_ocupada + 1) % num_celdas_total;
   num_ocupadas-- ;

   // señalar al productor que hay un hueco libre, por si está esperando
   libres.signal();

   // devolver valor
   return valor ;
}
// -----------------------------------------------------------------------------

void ProdConsMu::escribir( int valor )
{
   // FIFO: si la cola está llena, espera el productor
   // (diferencia con LIFO: LIFO comprueba 'primera_libre == num_celdas_total')
   while ( num_ocupadas == num_celdas_total )
      libres.wait();

   assert( num_ocupadas < num_celdas_total );

   // FIFO: insertar el valor en la posición de 'primera_libre' (final de la cola)
   // y avanzar índice circular. (En LIFO se hacía buffer[primera_libre] y ++ sin módulo,
   // y la lectura era desde la última posición.)
   buffer[primera_libre] = valor ;
   primera_libre = (primera_libre + 1) % num_celdas_total; // FIFO: índice circular
   num_ocupadas++ ;

   // señalar al consumidor que ya hay una celda ocupada (por si esta esperando)
   ocupadas.signal();
}
// *****************************************************************************
// funciones de hebras

void funcion_hebra_productora( MRef<ProdConsMu> monitor, int num_hebra )
{
   for( unsigned i = 0 ; i < items_por_productor ; i++ )
   {
      int valor = producir_dato( num_hebra ) ;
      monitor->escribir( valor );
   }
}
// -----------------------------------------------------------------------------

void funcion_hebra_consumidora( MRef<ProdConsMu>  monitor, int num_hebra )
{
   for( unsigned i = 0 ; i < items_por_consumidor ; i++ )
   {
      int valor = monitor->leer();
      consumir_dato( valor, num_hebra ) ;
   }
}
// -----------------------------------------------------------------------------

int main()
{
   cout << "--------------------------------------------------------------------" << endl
        << "Problema de los productores-consumidores (Monitor SU, buffer FIFO). " << endl
        << "--------------------------------------------------------------------" << endl
        << flush ;

    // inicializar contadores y siguiente_dato
    for( int i=0; i<num_items; i++ )
    {
        cont_prod[i] = 0;
        cont_cons[i] = 0;
    }
    for( int i=0; i<num_productores; i++ )
        producidos_por_hebra[i] = 0;

   // crear monitor  ('monitor' es una referencia al mismo, de tipo MRef<...>)
   MRef<ProdConsMu> monitor = Create<ProdConsMu>() ;

   // crear y lanzar las hebras
   thread hebras_prod[num_productores];
   for( int i = 0; i < num_productores; i++ )
      hebras_prod[i] = thread( funcion_hebra_productora, monitor, i );

   thread hebras_cons[num_consumidores];
   for( int i = 0; i < num_consumidores; i++ )
      hebras_cons[i] = thread( funcion_hebra_consumidora, monitor, i );

   // esperar a que terminen las hebras
   for( int i = 0; i < num_productores; i++ )
      hebras_prod[i].join();
   for( int i = 0; i < num_consumidores; i++ )
      hebras_cons[i].join();

   test_contadores() ;
}
