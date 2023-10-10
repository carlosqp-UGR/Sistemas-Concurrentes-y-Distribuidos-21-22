//-----------------------------------------------------------------------------------------------

#include <iostream>
#include <random>
#include <thread>
#include "scd.h"

using namespace std ;
using namespace scd ;

// Variable global
const int num_fumadores = 3;

//-----------------------------------------------------------------------------------------------
// Monitor SU

class Estanco: public HoareMonitor {
private:
	int ingrediente;	// Valor a consumir por los clientes
	
	CondVar cola_fumadores [num_fumadores];
	CondVar cola_estanquero;

public:
	Estanco();		// Constructor, inicializa las variables
	int obtenerIngrediente(int num_fumador);	// Retira el ingrediente del mostrador
	void ponerIngrediente(int nuevo_ingrediente);		// Pone el ingrediente en el mostrador
	void esperarRecogidaIngrediente();		// Espera bloqueado hasta que el mostrador esta libre
};

//-----------------------------------------------------------------------------------------------

Estanco::Estanco() {
	ingrediente = -1;	// el valor -1 del ingrediente representa que el mostrador esta vacio
	for (int i=0; i<num_fumadores; i++)
		cola_fumadores[i] = newCondVar();
	cola_estanquero = newCondVar();
}

void Estanco::ponerIngrediente(int nuevo_ingrediente) {
	//	Despierta al estanquero
	cola_estanquero.signal();		

	// Se informa del nuevo ingrediente colocado en el mostrador
	cout << "\tEstanquero coloca el nuevo ingrediente (" << nuevo_ingrediente << ") en el mostrador." << endl;

	ingrediente=nuevo_ingrediente;
	
	cola_fumadores[ingrediente].signal();
}

int Estanco::obtenerIngrediente(int num_fumador) {
	
	// Si el mostrador esta vacío, espera bloqueada hasta que el estanquero lo produzca y lo ponga en el mostrador  (o el ingrediente no es el del fumador)
	if (ingrediente!=num_fumador)
		cola_fumadores[num_fumador].wait();
	
	// Se informa de que ha recogido el ingrediente
	cout << "\tFumador " << num_fumador << " recoge el ingrediente " << ingrediente << " del mostrador." << endl;
	
	// Se consume el ingrediente
	const int consumir = ingrediente;
	
	// El mostrador vuelve a vaciarse
	ingrediente = -1;
	
	cola_estanquero.signal();
	
	return consumir;
}

void Estanco::esperarRecogidaIngrediente() {
	// Si el mostrador esta lleno, espera a que alguien lo recoja
	if (ingrediente>-1) {
		cola_estanquero.wait();
	}
}
//-----------------------------------------------------------------------------------------------


// Funciones fumar y producir ingrediente
int ProducirIngrediente() {
	// Calcula la duración aleatoria de producir un ingrediente
	chrono::milliseconds duracion_producir( aleatorio<10,100>() );

	// Informa que comienza a fumar
	cout << "Estanquero comienza a producir ingrediente." << endl;

	// Espera bloqueada de duracion_fumar ms
	this_thread::sleep_for(duracion_producir);

	// Produce el nuevo ingrediente
	const int nuevo_ingrediente = aleatorio<0,num_fumadores-1>();
	
	// Informa de que ha terminado de fumar
	cout << "Estanquero termina de producir ingrediente " << nuevo_ingrediente << " (" << duracion_producir.count() << " ms)." << endl;
		
	return nuevo_ingrediente;
}


void Fumar(int num_fumador) {
	// Calcula la duración aleatoria de fumar
	chrono::milliseconds duracion_fumar( aleatorio<10,100>() );

	// Informa que comienza a fumar
	cout << "Fumador " << num_fumador << " comienza a fumar." << endl;

	// Espera bloqueada de duracion_fumar ms
	this_thread::sleep_for(duracion_fumar);

	// Informa de que ha terminado de fumar
	cout << "Fumador " << num_fumador << " termina de fumar (" << duracion_fumar.count() << " ms)." << endl;	
}

//-----------------------------------------------------------------------------------------------
// Funcion hebra fumadora

void funcion_hebra_fumador ( MRef<Estanco> monitor, int indiceFumador) {
	int contador = 0;
	while(true) {
		monitor->obtenerIngrediente(indiceFumador);
		Fumar(indiceFumador);
		contador++;
		if(contador%2==0)
			cout << "OK!" << endl;
	}
}

//-----------------------------------------------------------------------------------------------
// Funcion hebra estanquero

void funcion_hebra_estanquero ( MRef<Estanco> monitor ) {
	int ingrediente;
	while (true) {
		ingrediente = ProducirIngrediente();
		monitor->ponerIngrediente(ingrediente);
		monitor->esperarRecogidaIngrediente();
	}
}

//-----------------------------------------------------------------------------------------------
// Para ejecutar

int main() {
	cout << "-----------------------------------------------------------------" << endl << "Problema de los fumadores. Version Monitor SU" << endl << "------------------------------------------------------------------" << endl;
   
   MRef<Estanco> monitor = Create<Estanco>();
   thread hebra_estanquero, hebras_fumadores[num_fumadores];
   
   hebra_estanquero = thread(funcion_hebra_estanquero, monitor);
   
   for (int i=0; i<num_fumadores; i++)
   		hebras_fumadores[i] = thread(funcion_hebra_fumador, monitor, i);
   		
	hebra_estanquero.join();
	for (int i=0; i<num_fumadores; i++)
		hebras_fumadores[i].join();

	return 0;
}

//-----------------------------------------------------------------------------------------------

