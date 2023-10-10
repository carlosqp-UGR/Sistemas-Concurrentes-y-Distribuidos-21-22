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

const int   id_productor          = 0,
            etiq_productor        = 0,
            num_productores       = 4,

           id_buffer             = num_productores,
           tam_vector            = 10,

           id_consumidor         = 2,
           etiq_consumidor       = 1,
           num_consumidores      = 5,

           num_procesos_esperado = num_productores + num_consumidores +1,
           num_items             = num_productores * num_consumidores,

           num_items_producir  = num_items/num_productores,
           num_items_consumir  = num_items/num_consumidores;


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
int producir( int orden )
{
   static int contador = orden*num_items_producir ;
   sleep_for( milliseconds( aleatorio<10,100>()) );
   contador++ ;
   cout << "Productor ha producido valor " << contador << endl << flush;
   return contador ;
}
// ---------------------------------------------------------------------

void funcion_productor( int orden )
{
   for ( unsigned int i= 0 ; i < num_items_producir ; i++ )
   {
      // producir valor
      int valor_prod = producir(orden);
      // enviar valor
      cout << "Productor " << orden << " va a enviar valor " << valor_prod << endl << flush;
      MPI_Ssend( &valor_prod, 1, MPI_INT, id_buffer, etiq_productor, MPI_COMM_WORLD );
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

void funcion_consumidor( int orden )
{
   int         peticion,
               valor_rec = 1 ;
   MPI_Status  estado ;

   for( unsigned int i=0 ; i < num_items_consumir; i++ )
   {
      MPI_Ssend( &peticion,  1, MPI_INT, id_buffer, etiq_consumidor, MPI_COMM_WORLD);
      MPI_Recv ( &valor_rec, 1, MPI_INT, id_buffer, etiq_consumidor, MPI_COMM_WORLD, &estado );
      cout << "Consumidor " << orden << " ha recibido valor " << valor_rec << endl << flush ;
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
              etiq_aceptable ;         // identificador de mensaje aceptable
   MPI_Status estado ;                 // metadatos del mensaje recibido


   for(unsigned int i=0; i<num_items*2; i++) {

		// 1. determinar si puede enviar solo prod., solo cons, o todos
		if( num_celdas_ocupadas == 0)
		    etiq_aceptable = etiq_productor;
		else if (num_celdas_ocupadas == tam_vector)
		    etiq_aceptable = etiq_consumidor;
		else
		    etiq_aceptable = MPI_ANY_TAG;

		// 2. recibir un mensaje del emisor o emisores aceptables
		MPI_Recv(&valor, 1, MPI_INT, MPI_ANY_SOURCE, etiq_aceptable, MPI_COMM_WORLD, &estado);

		// 3. procesar el mensaje recibido
		switch (estado.MPI_TAG) {
		    case (etiq_productor):
		        buffer[primera_libre] = valor ;
		        primera_libre = (primera_libre+1) % tam_vector ;
		        num_celdas_ocupadas++ ;
		        cout << "Buffer ha recibido valor " << valor << endl ;
		        break;


		    case(etiq_consumidor) :
		        valor = buffer[primera_ocupada] ;
		        primera_ocupada = (primera_ocupada+1) % tam_vector ;
		        num_celdas_ocupadas-- ;
		        cout << "Buffer va a enviar valor " << valor << endl ;
		        MPI_Ssend( &valor, 1, MPI_INT, estado.MPI_SOURCE, etiq_consumidor, MPI_COMM_WORLD);
		    	break;
		}
   }
}

// ---------------------------------------------------------------------

int main( int argc, char *argv[] )
{
   int id_propio, num_procesos_actual, orden;

   // inicializar MPI, leer identif. de proceso y número de procesos
   MPI_Init( &argc, &argv );
   MPI_Comm_rank( MPI_COMM_WORLD, &id_propio );
   MPI_Comm_size( MPI_COMM_WORLD, &num_procesos_actual );

   if ( num_procesos_esperado == num_procesos_actual )
   {
        if(id_propio<num_productores) {
            orden = id_propio;
            funcion_productor(orden);
        } else if (id_propio == num_productores) {
        	funcion_buffer(); 
		} else { 		// id_propio > num_productores
		    orden = id_propio - (num_productores + 1);
		    funcion_consumidor(orden);
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

