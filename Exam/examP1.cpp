/*Modificar el problema de los fumadores de la siguiente manera (el archivo se debe llamar
“fuma_examen.cpp”):
• Se debe crear una nueva hebra “sanitaria” (que será un ciclo infinito), que empieza bloqueada y
que es desbloqueada por los fumadores.
• Esta nueva hebra sólo es desbloqueada cada vez que un fumador es desbloqueado porque se ha
puesto su ingrediente en el mostrador después de haber fumado 4 cigarrillos. Es decir, cuando un
fumador X es liberado para fumar el siguiente cigarrillo (el quinto), desbloquea a la hebra
sanitaria y se bloquea (sin retirar el ingrediente) hasta que dicha hebra sanitaria lo libere.
• La hebra sanitaria, cuando es desbloqueada, imprime por pantalla el siguiente mensaje:
“FUMAR MATA: ya lo sabes, fumador X”, luego desbloquea a dicho fumador y vuelve al inicio
de su ciclo quedando bloqueada hasta que algún fumador la despierte.
• El fumador desbloqueado por la hebra sanitaria informa del aviso recibido por dicha hebra con el
siguiente mensaje: “Soy el fumador X y me han llamado vicioso”, y continúa con su ciclo normal
retirando el ingrediente, desbloqueando al estanquero y fumando, empezando de nuevo la cuenta
de 4 cigarrillos por fumar. */

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

const int num_fumadores = 3 ;

Semaphore mostr_vacio = 1;
Semaphore ingr_disp[num_fumadores] = {0, 0, 0};


Semaphore sanidad = 0;  //semaforo para bloquear a la hebra sanitaria si no tiene que actuar
Semaphore poder_fumar = 0;  //semaforo para que el fumador espere a la hebra sanitaria

//Número de veces que fuman sin despertar a la hebra sanitaria
int cigarrillos[num_fumadores] = {0, 0, 0}; 

//Fumador detenido por la hebra sanitaria actualmente
int fumador_vicioso;

//-------------------------------------------------------------------------

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

void funcion_hebra_estanquero(  )
{
	int i;	
	
	while (true) {
		i = producir_ingrediente();
		
		sem_wait(mostr_vacio);  //El mostrador está ocupado, no se pueden producir más ingredientes
		cout << "Puesto ingrediente " << i << " en el mostrador" << endl;
		sem_signal(ingr_disp[i]);
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

    cout << "Fumador " << num_fumador << "  : termina de fumar, comienza espera de ingrediente " << num_fumador << endl;

}

//----------------------------------------------------------------------
// función que ejecuta la hebra del fumador
void  funcion_hebra_fumador( int num_fumador )
{	
   while( true )
   {
	sem_wait(ingr_disp[num_fumador]); //Espera ingrediente
		
	if(cigarrillos[num_fumador] == 4 ) { // el fumador ya ha fumado 4 cigarrillos "seguidos"
				
		fumador_vicioso = num_fumador;
		sem_signal(sanidad);  //libera a la hebra sanitaria
		sem_wait(poder_fumar);  // espera a que la hebra sanitaria lo libere
		
		cigarrillos[num_fumador] = 0;  //vuelve a iniciarse la cuenta de cigarrillos fumados
		cout << "                            Soy el fumador " << num_fumador << " y me han llamado vicioso." << endl;
	}
	
	cout << "Retirando ingrediente " << num_fumador << endl;
	sem_signal(mostr_vacio);
	fumar(num_fumador);
	
	cigarrillos[num_fumador]++; //Se incrementa la cuenta de cigarrillos fumados
   }
}

//----------------------------------------------------------------------
// función que ejecuta la hebra sanitaria
void funcion_hebra_sanitaria()
{

	while(true) {
	
		sem_wait(sanidad); // Esperar a que algún fumador al despierte después de haber fumado 4 veces
		 
		cout << "                            FUMAR MATA: ya lo sabes fumador " << fumador_vicioso << endl;
		
		sem_signal(poder_fumar);  //Liberar al fumador que ha despertado a la hebra sanitaria
	
	}

}

//----------------------------------------------------------------------

int main()
{
   cout << "-----------------------------------------------------------------" << endl
        << "Problema de los fumadores." << endl
        << "------------------------------------------------------------------" << endl
        << flush ;
        
   
   //Declaración de hebras
   thread hebra_estanquero(funcion_hebra_estanquero),
   		  hebras_fumadoras[num_fumadores], 
   		  hebra_sanitaria(funcion_hebra_sanitaria);
   	
   //Poner en marcha las hebras fumadoras	  
   for(int i=0; i<num_fumadores; i++)
   		hebras_fumadoras[i] = thread(funcion_hebra_fumador, i);
   
   
   //Esperar a que terminan las hebras
   hebra_estanquero.join();
   hebra_sanitaria.join();
   
   for(int i=0; i<num_fumadores; i++)
   		hebras_fumadoras[i].join();
              
        
}