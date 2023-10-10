// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Práctica 3. Implementación de algoritmos distribuidos con MPI
//
// Archivo: filosofos-plantilla.cpp
// Implementación del problema de los filósofos (sin camarero).
// Plantilla para completar.
//
// Historial:
// Actualizado a C++11 en Septiembre de 2017
// -----------------------------------------------------------------------------


#include <mpi.h>
#include <thread> // this_thread::sleep_for
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include <iostream>

using namespace std;
using namespace std::this_thread ;
using namespace std::chrono ;

const int
		num_filosofos = 5 ,              // número de filósofos 
		num_filo_ten  = 2*num_filosofos, // número de filósofos y tenedores 
		num_procesos  = num_filo_ten +1, // número de procesos total (filo, ten y cam)
		id_camarero 	= num_procesos -1, // id del camarero (último proceso)
		etiq_tenedor 	= 0,
		etiq_sentarse	= 1,
		etiq_levantarse = 2;

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

// CAMBIAR
void funcion_filosofos( int id )
{
  int id_ten_izq = (id+1)              % num_filo_ten, //id. tenedor izq.
      id_ten_der = (id+num_filo_ten-1) % num_filo_ten; //id. tenedor der.
      
  int msj = 0;

  while ( true )
  {
  	// ... solicitar sentarse en la mesa (al camarero)
    cout <<"Filósofo " <<id << " solicita sentarse en la mesa." <<endl;
    MPI_Ssend(&msj, 1, MPI_INT, id_camarero, etiq_sentarse, MPI_COMM_WORLD);
  	
    cout <<"Filósofo " <<id << " solicita ten. izq." <<id_ten_izq <<endl;
    // ... solicitar tenedor izquierdo (completar)
    MPI_Ssend (&msj, 1, MPI_INT, id_ten_izq, etiq_tenedor, MPI_COMM_WORLD);

    cout <<"Filósofo " <<id <<" solicita ten. der." <<id_ten_der <<endl;
    // ... solicitar tenedor derecho (completar)
    MPI_Ssend (&msj, 1, MPI_INT, id_ten_der, etiq_tenedor, MPI_COMM_WORLD);

    cout <<"Filósofo " <<id <<" comienza a comer" <<endl ;
    sleep_for( milliseconds( aleatorio<10,100>() ) );

    cout <<"Filósofo " <<id <<" suelta ten. izq. " <<id_ten_izq <<endl;
    // ... soltar el tenedor izquierdo (completar)
    MPI_Ssend (&msj, 1, MPI_INT, id_ten_izq, etiq_tenedor, MPI_COMM_WORLD);
    
    cout<< "Filósofo " <<id <<" suelta ten. der. " <<id_ten_der <<endl;
    // ... soltar el tenedor derecho (completar)
    MPI_Ssend (&msj, 1, MPI_INT, id_ten_der, etiq_tenedor, MPI_COMM_WORLD);

		// ... levantarse de la mesa
    cout <<"Filósofo " <<id << " solicita levantarse de la mesa." <<endl;
    MPI_Ssend(&msj, 1, MPI_INT, id_camarero, etiq_levantarse, MPI_COMM_WORLD);
    
    cout << "Filosofo " << id << " comienza a pensar" << endl;
    sleep_for( milliseconds( aleatorio<10,100>() ) );
 }
}
// ---------------------------------------------------------------------

void funcion_tenedores( int id )
{
  int valor, id_filosofo ;  // valor recibido, identificador del filósofo
  MPI_Status estado ;       // metadatos de las dos recepciones

  while ( true )
  {
     // ...... recibir petición de cualquier filósofo (completar)
     MPI_Recv(&valor, 1, MPI_INT, MPI_ANY_SOURCE, etiq_tenedor, MPI_COMM_WORLD, &estado);
     
     // ...... guardar en 'id_filosofo' el id. del emisor (completar)
     id_filosofo = estado.MPI_SOURCE;
     cout <<"Ten. " <<id <<" ha sido cogido por filo. " <<id_filosofo <<endl;

     // ...... recibir liberación de filósofo 'id_filosofo' (completar)
     MPI_Recv(&valor, 1, MPI_INT, id_filosofo, 0, MPI_COMM_WORLD, &estado);
     cout <<"Ten. "<< id<< " ha sido liberado por filo. " <<id_filosofo <<endl ;
  }
}

// ---------------------------------------------------------------------

void funcion_camarero() 
{
	const int MAX_EN_MESA = num_filosofos-1;
	int valor=0, en_mesa = 0; // Valor recibido, Contador de filósofos en la mesa
	MPI_Status estado;

	while(true) {
		// Se establece qué acción puede ser realizada en cada momento
		int etiq_valida = MPI_ANY_TAG;
		if ( en_mesa == 0 ) // Mesa vacía -> sólo puede recibir solicitudes de sentarse
			etiq_valida = etiq_sentarse;
		else if ( en_mesa == MAX_EN_MESA )	// Mesa llena -> sólo puede recibir solicitudes de levantarse
			etiq_valida = etiq_levantarse;
			
		// Recibe el mensaje
		MPI_Recv(&valor, 1, MPI_INT, MPI_ANY_SOURCE, etiq_valida, MPI_COMM_WORLD, &estado);
		
		// Procesa el mensaje
		switch ( estado.MPI_TAG ) {
			case ( etiq_sentarse ) :
				en_mesa ++;
				cout << "\tCamarero sienta a Filósofo " << estado.MPI_SOURCE << " (en mesa: " << en_mesa << ")." << endl;
				break;
			case ( etiq_levantarse ) :
				en_mesa --;
				cout << "\tCamarero levanta a Filósofo " << estado.MPI_SOURCE << " (en mesa: " << en_mesa << ")." << endl;
				break;
		}
		
	}
}
// ---------------------------------------------------------------------

int main( int argc, char** argv )
{
   int id_propio, num_procesos_actual ;

   MPI_Init( &argc, &argv );
   MPI_Comm_rank( MPI_COMM_WORLD, &id_propio );
   MPI_Comm_size( MPI_COMM_WORLD, &num_procesos_actual );


   if ( num_procesos == num_procesos_actual )
   {
      // ejecutar la función correspondiente a 'id_propio'
   		if ( id_propio == id_camarero )		// si es el ultimo proceso, camarero
   			funcion_camarero();
 			else if ( id_propio % 2 == 0 )          // si es par
         funcion_filosofos( id_propio ); //   es un filósofo
      else if ( id_propio % 2 != 0 )                           // si es impar
         funcion_tenedores( id_propio ); //   es un tenedor
   }
   else
   {
      if ( id_propio == 0 ) // solo el primero escribe error, indep. del rol
      { cout << "el número de procesos esperados es:    " << num_procesos << endl
             << "el número de procesos en ejecución es: " << num_procesos_actual << endl
             << "(programa abortado)" << endl ;
      }
   }

   MPI_Finalize( );
   return 0;
}

// ---------------------------------------------------------------------
