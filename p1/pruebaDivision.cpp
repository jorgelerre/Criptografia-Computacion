#include <iostream>
#include <cstdint>


using namespace std;

// Estructura para representar un entero sin signo de 128 bits dividido en dos partes de 64 bits
struct UInt128 {
	uint64_t high;
	uint64_t low;
};

/*
// Función para dividir un entero sin signo de 128 bits entre un entero sin signo de 64 bits
UInt128 divide128By64(UInt128 dividend, uint64_t divisor) {
	__uint128_t div, q;
	UInt128 quotient = {0, 0};
	uint64_t remainder = 0;
	
	//Asignamos el dividendo al tipo de dato uint64_t
	div = ((__uint128_t) dividend.high << 64) | dividend.low;
	
	//Realizamos la division
	q = div / divisor;
	
	//Almacenamos el cociente en un UInt128
	quotient.low = q & 0xFFFFFFFFFFFFFFFF;
	quotient.high = (q >> 64) & 0xFFFFFFFFFFFFFFFF;
	return quotient;
}
*/

// Función para dividir un entero sin signo de 128 bits entre un entero sin signo de 64 bits
void divide128By64(UInt128 dividend, uint64_t divisor, UInt128 & quotient, uint64_t & remainder) {
	quotient = {0, 0};
	uint64_t actual_div = 0, resta;
	bool carry = false;
	//Realizamos la division en binario, bit a bit, para la primera parte del cociente
	for(int i = 64; i > 0; i--){
		//Incluimos un nuevo bit en el dividendo actual
		actual_div = (actual_div << 1) | (dividend.high << (64 - i) >> 63);
		cout << "Digito " << i << " = " << actual_div << endl;
		//Restamos el divisor al dividendo actual
		resta = actual_div - divisor;
		//Si no hay acarreo, entonces actual_div > divisor
		//Por tanto, en el cociente colocamos un 1
		if(actual_div >= resta){
			quotient.high = (quotient.high << 1) | 1;
			actual_div = resta;
		}
		//Si hay acarreo, entonces tomamos cociente 0 
		else{
			quotient.high = quotient.high << 1;
		}
	}
	//Realizamos la division en binario, bit a bit, para la segunda parte del cociente
	//A partir de aqui, es posible que el divisor sea mas grande que el valor guardado 
	//en actual_div debido a la perdida del bit mas significativo. Para conservarlo, usamos
	//el booleano carry
	for(int i = 64; i > 0; i--){
		
		//Incluimos un nuevo bit en el dividendo actual
		carry = actual_div >> 63;
		actual_div = (actual_div << 1) | (dividend.low << (64 - i) >> 63);
		cout << "Digito " << i << " = " << actual_div << endl;
		
		//Restamos el divisor al dividendo actual
		resta = actual_div - divisor;
		if(carry)
			printf("C");
		printf("%016lx - %016lx = %016lx \n", actual_div, divisor, resta);
		
		//Si no hay acarreo, o hay acarreo virtual (por la presencia de un 1 a la izquierda 
		//no se hubiera producido acarreo), entonces actual_div > divisor.
		//Por tanto, en el cociente colocamos un 1
		if(actual_div >= resta || carry){
			quotient.low = (quotient.low << 1) | 1;
			actual_div = resta;
			printf("Aniade 1\n");
		}
		//Si hay acarreo, entonces tomamos cociente 0 
		else{
			quotient.low = quotient.low << 1;
			printf("Aniade 0\n");
		}
		printf("Cociente actual: %016lx %016lx\n\n", quotient.high, quotient.low);
	}
	remainder = actual_div;
	printf("Resto: %016lx\n", actual_div);
}


int main() {
		// Dividendo de 128 bits (representado por dos partes de 64 bits)
	UInt128 dividend = {0xF234567876543210, 0xFFFFFFFFFFFFFFFF};
	// Divisor de 64 bits 
	uint64_t divisor = 0xFFFFFFFFFFFFFFF;
	//uint64_t divisor = 0x121651848416;
	// Realizamos la división
	UInt128 quotient;
	uint64_t remainder;
	
	divide128By64(dividend, divisor, quotient, remainder);

	// Imprimimos el resultado
	std::cout << "Cociente: 0x";
	printf("%016lx %016lx\n", quotient.high, quotient.low);
	printf("Resto: %016lx\n", remainder);
	return 0;
}

