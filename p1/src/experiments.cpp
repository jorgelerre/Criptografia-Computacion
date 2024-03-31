#include "bigInt.h"
#include <cstdlib> //  función stoi
#include <stdexcept> // excepción invalid_argument y out_of_range
#include <time.h>

using namespace std;

int main(int argc, char** argv){
	int num_digits, repeticiones = 10;
	double t, t_ini, t_fin;
	double t_sum, t_mul, t_div, t_kar, t_eea, t_qme;
	double t_sum2, t_mul2, t_div2, t_kar2, t_eea2, t_qme2;
	t_sum = t_mul = t_div = t_kar = t_eea = t_qme = 0;
	t_sum2 = t_mul2 = t_div2 = t_kar2 = t_eea2 = t_qme2 = 0;
	
	//Procesamiento de entradas
	if (argc != 2) {
        cerr << "Uso: " << argv[0] << " <entero>" << endl;
        return 1;
    }
    try {
        // Convertir el argumento de cadena a entero
        num_digits = stoi(argv[1]);

        // Procesar el entero
        cout << "El entero ingresado es: " << num_digits << endl;

        // Realizar cualquier operación adicional con el entero aquí
    } catch (const invalid_argument& e) {
        cerr << "Error: El argumento no es un entero válido." 
        	 << endl;
        return 1;
    } catch (const out_of_range& e) {
        cerr << "Error: El argumento está fuera del rango de valores permitidos para un entero."
        	 << endl;
        return 1;
    }
    
    for(int i = 0; i < repeticiones; i++){
		BigInt a = BigInt::randomBigInt(num_digits);
		BigInt b = BigInt::randomBigInt(num_digits);
		BigInt m = BigInt::randomBigInt(num_digits);
		BigInt D = BigInt::randomBigInt(2*num_digits);
		BigInt res, mcd, mcm, u0, v0;
		
		//Suma
		t_ini = clock();
		res = a + b;
		t_fin = clock();
		t = (double (t_fin - t_ini)/CLOCKS_PER_SEC);
		t_sum += t; 
		t_sum2 += t*t; 
		//cout << "t Suma = " << t_sum << endl;
		
		//Multiplicacion
		t_ini = clock();
		res = a * b;
		t_fin = clock();
		t = (double (t_fin - t_ini)/CLOCKS_PER_SEC);
		t_mul += t; 
		t_mul2 += t*t; 
		//cout << "t Multiplicacion = " << t << endl;
		
		//Division
		t_ini = clock();
		res = D/a;
		t_fin = clock();
		t = (double (t_fin - t_ini)/CLOCKS_PER_SEC);
		t_div += t; 
		t_div2 += t*t; 
		//cout << "t Division = " << t << endl;
		
		//Karatsuba
		t_ini = clock();
		res = a.karatsubaMult(b);
		t_fin = clock();
		t = (double (t_fin - t_ini)/CLOCKS_PER_SEC);
		t_kar += t; 
		t_kar2 += t*t; 
		//cout << "t Karatsuba = " << t << endl;
		
		//Algoritmo Extendido de Euclides
		t_ini = clock();
		a.EEA(b, mcd, mcm, u0, v0);
		t_fin = clock();
		t = (double (t_fin - t_ini)/CLOCKS_PER_SEC);
		t_eea += t; 
		t_eea2 += t*t; 
		//cout << "t Algoritmo Extendido de Euclides = " << t << endl;
		
		//Exponenciacion modular rapida
		t_ini = clock();
		res = a.quickModExp(b, m);
		t_fin = clock();
		t = (double (t_fin - t_ini)/CLOCKS_PER_SEC);
		t_qme += t; 
		t_qme2 += t*t; 
		//cout << "t Exponenciacion modular rapida = " << t << endl;
		
	}
	
	//Calculamos medias y desviaciones tipicas
	//Suma
	t_sum /= repeticiones; 
	t_sum2 = sqrt(t_sum2/(repeticiones-1.0) - t_sum*t_sum);
	cout << t_sum << endl;
	cout  << t_sum2 << endl;
	//Multiplicacion
	t_mul /= repeticiones; 
	t_mul2 = sqrt(t_mul2/(repeticiones-1.0) - t_mul*t_mul);
	cout << t_mul << endl;
	cout << t_mul2 << endl;
	//Division
	
	t_div /= repeticiones; 
	t_div2 = sqrt(t_div2/(repeticiones-1.0) - t_div*t_div);
	cout << t_div << endl;
	cout << t_div2 << endl;
	
	//Karatsuba
	t_kar /= repeticiones; 
	t_kar2 = sqrt(t_kar2/(repeticiones-1.0) - t_kar*t_kar);
	cout << t_kar << endl;
	cout << t_kar2 << endl;
	
	//Algoritmo Extendido de Euclides
	t_eea /= repeticiones; 
	t_eea2 = sqrt(t_eea2/(repeticiones-1.0) - t_eea*t_eea);
	cout << t_eea << endl;
	cout << t_eea2 << endl;
	
	//Exponenciacion modular rapida
	t_qme /= repeticiones; 
	t_qme2 = sqrt(t_qme2/(repeticiones-1.0) - t_qme*t_qme);
	cout << t_qme << endl;
	cout << t_qme2 << endl;
	
	return 0;
}
