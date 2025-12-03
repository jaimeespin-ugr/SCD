// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Práctica 3. Implementación de algoritmos distribuidos con MPI
//
// Archivo: prodcons2.cpp
// Implementación del problema del productor-consumidor con
// un proceso intermedio que gestiona un buffer finito y recibe peticiones
// en orden arbitrario
// (versión con un único productor y un único consumidor)
//
// Historial:
// Actualizado a C++11 en Septiembre de 2017
// -----------------------------------------------------------------------------

#include <iostream>
#include <thread> // this_thread::sleep_for
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include <mpi.h>

using namespace std;
using namespace std::this_thread ;
using namespace std::chrono ;

const int
   id_productor          = 0 ,
   
   id_consumidor         = 2 ,
   
   num_items             = 20, //debe ser multiplo de num_prod y num_cons
   tam_vector            = 10,

   etiq_dato = 1,
   etiq_peticion = 2,
   num_prod = 4,
   num_cons = 5,
   id_buffer             = num_prod ,
   num_procesos_esperado = num_prod + num_cons + 1;

//**********************************************************************
// plantilla de función para generar un entero aleatorio uniformemente
// distribuido entre dos valores enteros, ambos incluidos
// (ambos tienen que ser dos constantes, conocidas en tiempo de compilación)
//----------------------------------------------------------------------

template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}
// ---------------------------------------------------------------------
// ptoducir produce los numeros en secuencia (1,2,3,....)
// y lleva espera aleatorio
int producir()
{
   static int contador = 0 ;
   sleep_for( milliseconds( aleatorio<10,100>()) );
   contador++ ;
   cout << "Productor ha producido valor " << contador << endl << flush;
   return contador ;
}
// ---------------------------------------------------------------------

void funcion_productor( int id_orden)
{
   const int k = num_items / num_prod; //Items por productor
   int valor_inicial = (id_orden * k) + 1;

   for(int i = 0; i<k; i++){
      int valor_prod = valor_inicial+i;  //Valor que se envia
      sleep_for( milliseconds( aleatorio<10,100>()) );
      cout << "Productor " << id_orden << " va a enviar valor " << valor_prod << endl << flush;
      MPI_Ssend( &valor_prod, 1, MPI_INT, id_buffer, etiq_dato, MPI_COMM_WORLD );
   }
}
// ---------------------------------------------------------------------

void consumir( int valor_cons )
{
   // espera bloqueada
   sleep_for( milliseconds( aleatorio<110,200>()) );
   cout << "Consumidor ha consumido valor " << valor_cons << endl << flush ;
}
// ---------------------------------------------------------------------

void funcion_consumidor( int id_orden)
{
   const int k = num_items/num_cons; //Items por consumidor
   int         peticion,
               valor_rec = 1 ;
   MPI_Status  estado ;

   for( unsigned int i=0 ; i < k; i++ )
   {
      MPI_Ssend( &peticion,  1, MPI_INT, id_buffer, etiq_peticion, MPI_COMM_WORLD);
      MPI_Recv ( &valor_rec, 1, MPI_INT, id_buffer, etiq_peticion, MPI_COMM_WORLD,&estado );
      cout << "Consumidor " << id_orden << " ha recibido valor " << valor_rec << endl << flush ;
      consumir( valor_rec );
   }
}
// ---------------------------------------------------------------------

void funcion_buffer()
{
   int        buffer[tam_vector],      // buffer con celdas ocupadas y vacías
              valor,                   // valor recibido o enviado
              primera_libre       = 0, // índice de primera celda libre
              primera_ocupada     = 0, // índice de primera celda ocupada
              num_celdas_ocupadas = 0, // número de celdas ocupadas
              etiq_aceptable;
   MPI_Status estado ;                 // metadatos del mensaje recibido

   for( unsigned int i=0 ; i < num_items*2 ; i++ )
   {
      // 1. determinar si puede enviar solo prod., solo cons, o todos
      // Lógica de espera selectiva

      if ( num_celdas_ocupadas == 0 ){               // si buffer vacío
         etiq_aceptable = etiq_dato; //Cuando buffer vacio, solo aceptan mansajes con etiq_dato
      }else if ( num_celdas_ocupadas == tam_vector ){ // si buffer lleno
         etiq_aceptable = etiq_peticion; //Cuando buffer lleno, solo aceptan mansajes con etiq_peticion
      }else{                                          // si no vacío ni lleno
         etiq_aceptable = MPI_ANY_TAG;
      }
      // 2. recibir un mensaje del emisor o emisores aceptables

      MPI_Recv( &valor, 1, MPI_INT, MPI_ANY_SOURCE, etiq_aceptable, MPI_COMM_WORLD, &estado );

      // 3. procesar el mensaje recibido

      switch( estado.MPI_TAG ) // leer emisor del mensaje en metadatos
      {
         case etiq_dato: // si ha sido el productor: insertar en buffer
            buffer[primera_libre] = valor ;
            primera_libre = (primera_libre+1) % tam_vector ;
            num_celdas_ocupadas++ ;
            cout << "Buffer ha recibido valor " << valor << " del productor "<< estado.MPI_SOURCE<<endl ;
            break;

         case etiq_peticion: // si ha sido el consumidor: extraer y enviarle
            valor = buffer[primera_ocupada] ;
            primera_ocupada = (primera_ocupada+1) % tam_vector ;
            num_celdas_ocupadas-- ;
            cout << "Buffer va a enviar valor " << valor << " al consumidor "<< estado.MPI_SOURCE<< endl ;
            MPI_Ssend( &valor, 1, MPI_INT, estado.MPI_SOURCE, etiq_peticion, MPI_COMM_WORLD);
            break;
      }
   }
}

// ---------------------------------------------------------------------

int main( int argc, char *argv[] )
{
   int id_propio, num_procesos_actual;
   int id_orden_propio = 0;

   // inicializar MPI, leer identif. de proceso y número de procesos
   MPI_Init( &argc, &argv );
   MPI_Comm_rank( MPI_COMM_WORLD, &id_propio );
   MPI_Comm_size( MPI_COMM_WORLD, &num_procesos_actual );

   const int total_procesos_esperados = num_prod + num_cons + 1;

   if ( total_procesos_esperados == num_procesos_actual )
   {
      // ejecutar la operación apropiada a 'id_propio'
      if ( id_propio < num_prod ){
         id_orden_propio = id_propio;
         funcion_productor(id_orden_propio);
      }
      else if ( id_propio == num_prod ){
         funcion_buffer();
      }
      else{
         id_orden_propio = id_propio - (num_prod + 1);
         funcion_consumidor(id_orden_propio);
      }
   }
   else
   {
      if ( id_propio == 0 ) // solo el primero escribe error, indep. del rol
      { cout << "el número de procesos esperados es:    " << num_procesos_esperado << endl
             << "el número de procesos en ejecución es: " << num_procesos_actual << endl
             << "(programa abortado)" << endl ;
      }
   }

   // al terminar el proceso, finalizar MPI
   MPI_Finalize( );
   return 0;
}
