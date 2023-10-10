#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "scd.h"

using namespace std ;
using namespace scd ;

// Numero de fumadores 
const int num_fumadores = 3;

// Semáforos para controlar el acceso a procesos
Semaphore ingrediente_disponible[num_fumadores] = {0,0,0}, 	// Inicialmente a 0 (#Pi-#Ri>=0)
		  mostrador_vacio = 1;							// Inicialmente a 1 (0<= p <=1)
		  
// Único ingrediente en el mostrador
int mostrador=0;
		  
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

   return num_ingrediente;
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del estanquero

void funcion_hebra_estanquero(  )
{
	while (true) {
		int ingrediente_producido = producir_ingrediente(); 		// Genera el ingrediente, asegura que sólo genera un ingrediente cada vez
		
		// Programación concurrente: la hebra espera a que el semaforo este en verde
		mostrador_vacio.sem_wait();
		mostrador = ingrediente_producido;
		cout << "Estanquero : pone en mostrador ingrediente para fumador " << mostrador << endl;
		ingrediente_disponible[mostrador].sem_signal();
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
void  funcion_hebra_fumador( int num_fumador )
{
	while (true) {
		// Se debe esperar a que haya un elemento en el mostrador para recogerlo
		ingrediente_disponible[num_fumador].sem_wait();
		cout << "\tFumador " << num_fumador << "	: obtiene su ingrediente del mostrador." << endl; 
		mostrador_vacio.sem_signal();
		fumar(num_fumador);
	}
}

//----------------------------------------------------------------------

int main()
{
   cout << "-----------------------------------------------------------------" << endl
        << "Problema de los fumadores." << endl
        << "------------------------------------------------------------------" << endl
        << flush ;

   thread hebra_estanquero, hebras_fumadores[num_fumadores];
   
   hebra_estanquero = thread(funcion_hebra_estanquero);
   
   for (int i=0; i<num_fumadores; i++)
   		hebras_fumadores[i] = thread(funcion_hebra_fumador, i);
   		
	hebra_estanquero.join();
	for (int i=0; i<num_fumadores; i++)
		hebras_fumadores[i].join();
		  
  
	return 0;
}
