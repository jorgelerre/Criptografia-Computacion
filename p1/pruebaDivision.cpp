#include <iostream>
#include <cstdint>


using namespace std;

// Estructura para representar un entero sin signo de 128 bits dividido en dos partes de 64 bits
struct UInt128 {
	uint64_t high;
	uint64_t low;
};

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

int main() {
	// Dividendo de 128 bits (representado por dos partes de 64 bits)
	UInt128 dividend = {0x1234567876543210, 0xFFFFFFFFFFFFFFFF};
	// Divisor de 64 bits 
	//uint64_t divisor = 0xFFFFFFFFFFFFFFFF;
	uint64_t divisor = 0x121651848416;
	// Realizamos la división
	UInt128 quotient = divide128By64(dividend, divisor);

	// Imprimimos el resultado
	std::cout << "Cociente: 0x";
	printf("%016lx %016lx\n", quotient.high, quotient.low);

	return 0;
}

