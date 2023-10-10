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

const int num_lectores = 5 ;
const int num_escritores = 10;




//Clase Monitor Hoare que controla los procesos de produccion y de consumicion de las hebras estanquero y fumadores

class Lec_Esc : public HoareMonitor
{
	private:
	int n_lectores;
	bool escrib;
	CondVar lectura;
	CondVar escritura;
       
        
	
	
	public:
    Lec_Esc(); 
        
	void ini_lectura(int num_hebra);
	
	void fin_lectura(int num_hebra);
	
	void ini_escritura(int num_hebra);
	
	void fin_escritura(int num_hebra);




};

Lec_Esc::Lec_Esc(){
	n_lectores=0;
	escrib=false;
	
	lectura=newCondVar();
	escritura=newCondVar();



}

void Lec_Esc::ini_lectura(int num_hebra){
	if(escrib){
		lectura.wait();
	}
	n_lectores++;
	cout << "Empieza a leer la hebra " << num_hebra << "(hebras leyendo en el mismo momento " << 		n_lectores << ")" << endl;
	lectura.signal();


}


void Lec_Esc::fin_lectura(int num_hebra){
	n_lectores--;
	cout << "Termina de leer la hebra " << num_hebra << "(hebras leyendo en el mismo momento " << 		n_lectores << ")" << endl;
	if(n_lectores==0)
		escritura.signal();
}

void Lec_Esc::ini_escritura(int num_hebra){
	if(n_lectores>0 || escrib){
		escritura.wait();
	}
	cout << "Empieza a escribir la hebra " << num_hebra << endl;
	escrib=true;
}

void Lec_Esc::fin_escritura(int num_hebra){
	escrib=false;
	cout << "Termina de escribir la hebra " << num_hebra << endl;
	if(!lectura.empty()){
		lectura.signal();
	}else{
		escritura.signal();
	}
}




//----------------------------------------------------------------------
// función que ejecuta las hebras de los escritores

void escritor( MRef<Lec_Esc> monitor, int hebra )
{
   while ( true ){
	   monitor->ini_escritura(hebra);
	   chrono::milliseconds duracion_escritura( aleatorio<10,100>() );
	   this_thread::sleep_for( duracion_escritura );
	   monitor->fin_escritura(hebra);
	   chrono::milliseconds duracion_escritura2( aleatorio<10,100>() );
	   this_thread::sleep_for( duracion_escritura2 );
   }
}
//----------------------------------------------------------------------
// función que ejecuta la hebra del fumador
void  lector( MRef<Lec_Esc> monitor,int hebra )
{
   	
   while( true )
   {
	   	monitor->ini_lectura(hebra);
	   	chrono::milliseconds duracion_lectura( aleatorio<10,100>() );
		this_thread::sleep_for( duracion_lectura );
		monitor->fin_lectura(hebra);
		chrono::milliseconds duracion_lectura2( aleatorio<10,100>() );
		this_thread::sleep_for( duracion_lectura2 );
   }
   
}

//----------------------------------------------------------------------

int main()
{
   cout << "-----------------------------------------------------------------" << endl
        << "Problema de los escritores/lectores" << endl
        << "------------------------------------------------------------------" << endl
        << flush ;
   MRef<Lec_Esc> monitor = Create<Lec_Esc>();

   thread lectores[num_lectores];
   thread escritores[num_escritores];
   for(int i=0;i<num_lectores;i++){
   	lectores[i]=thread (lector,monitor,i);
   }
   
   for(int i=0; i<num_escritores;i++){
   	escritores[i]=thread (escritor,monitor,i);
   }
   
   for(int i=0;i<num_lectores;i++){
   	lectores[i].join();
   }
   
   for(int i=0; i<num_escritores;i++){
   	escritores[i].join();
   }
   
}

