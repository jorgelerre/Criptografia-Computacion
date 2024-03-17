#include <iostream>
#include <cstdint>
#include "bigInt.h"


using namespace std;

// Estructura para representar un entero sin signo de 128 bits dividido en dos partes de 64 bits


void test1(){
	BigInt bint1,
		   bint2("-0x   9638527410  0123456789abcdef fedcba9876543210"),
		   bint3("-0x 9638527410 0123456789abcdef 0123456776543210"),
		   bint4 = string("-0x 0"),
		   bint5(bint3),
		   bint6, bint7, bint8;
	
	cout << "--------------Test Ejercicio 1--------------" << endl;
	cout << "bint1 (constructor por defecto) = " << bint1 << endl;
	cout << "bint2 (constructor por parametros) = " << bint2 << endl;
	cout << "bint3 (constructor por parametros) = " << bint3 << endl;
	cout << "bint4 (op. asignacion con string) = " << bint4 << endl;
	cout << "bint5 (constructor de copia -por defecto- de bint3) = " << bint5 << endl;
	
	cout << "bint1 == bint2 -> " << (bint1 == bint2) << endl;
	cout << "bint1 < bint2 -> "  << (bint1 < bint2)  << endl;
	cout << "bint1 > bint2 -> "  << (bint1 > bint2)  << endl;
	cout << "bint1 <= bint2 -> " << (bint1 <= bint2) << endl;
	cout << "bint1 >= bint2 -> " << (bint1 >= bint2) << endl;
	
	cout << "bint1 == bint3 -> " << (bint1 == bint3) << endl;
	cout << "bint2 == bint4 -> " << (bint2 == bint4) << endl;
	cout << "bint2 < bint4 -> " << (bint2 < bint4) << endl;
	
	bint6 = -bint2;
	cout << "bint6 = -bint2 = " << bint6 << endl;
	
}

void test2(){
	BigInt a = "0x 0000000000000001 0000000000000000 0000000000000000", result;
	BigInt b = "0x ffffffffffffffff ffffffffffffffff ffffffffffffffff";
	BigInt c = "0x 0000000000000001 0000000000000000 0000000000000000 0000000000000000";
	cout << "--------------Test Ejercicio 2--------------" << endl;
	
	result = a;
	cout << "a = " << result << endl;
	assert(result == creaBigInt("0x 0000000000000001 0000000000000000 0000000000000000"));
	
	result = -a;
	cout << "-a = " << result << endl;
	assert(result == creaBigInt("-0x 0000000000000001 0000000000000000 0000000000000000"));
	
	result = a + a;
	cout << "a + a = " << result << endl;
	assert(result == creaBigInt("0x 0000000000000002 0000000000000000 0000000000000000"));
	
	result = a + (-a);
	cout << "a + (-a) = " << result << endl; 
	assert(result == creaBigInt("0x 0000000000000000"));
	
	result = (-a) + a;
	cout << "(-a) + a = " << result << endl; 
	assert(result == creaBigInt("0x 0000000000000000"));
	
	result = (-a) +(-a);
	cout << "(-a) + (-a) = " << result << endl; 
	assert(result == creaBigInt("-0x 0000000000000002 0000000000000000 0000000000000000"));
	
	result = a - a;
	cout << "a - a = " << result << endl;
	assert(result == creaBigInt("0x 0000000000000000"));
	
	result = a - (-a);
	cout << "a - (-a) = " << result << endl; 
	assert(result == creaBigInt("0x 0000000000000002 0000000000000000 0000000000000000"));
	
	result = (-a) - a;
	cout << "(-a) - a = " << result << endl; 
	assert(result == creaBigInt("-0x 0000000000000002 0000000000000000 0000000000000000"));
	
	result = (-a) - (-a);
	cout << "(-a) - (-a) = " << result << endl; 
	assert(result == creaBigInt("-0x 0000000000000000"));
	
	result = a + creaBigInt("0x1");
	cout << "a + 1 = " << result << endl;
	assert(result == creaBigInt("0x 0000000000000001 0000000000000000 0000000000000001"));
	
	result = a - creaBigInt("0x1");
	cout << "a - 1 = " << result << endl;
	assert(result == creaBigInt("0x ffffffffffffffff ffffffffffffffff"));
	
	result = a + creaBigInt("0x1")- creaBigInt("0x1");
	cout << "a + 1 - 1 = " << result << endl;
	assert(result == creaBigInt("0x 0000000000000001 0000000000000000 0000000000000000"));
	
	cout << "b = " << b << endl;
	result = a + b;
	cout << "a + b = " << result << endl;
	assert(result == creaBigInt("0x 0000000000000001 0000000000000000 ffffffffffffffff ffffffffffffffff"));
	
	cout << "c = " << c << endl;
	result = result - c;
	cout << "a + b - c = " << result << endl;
	assert(result == creaBigInt("0x ffffffffffffffff ffffffffffffffff"));
	
	cout << "----------------Test 2 OK :)-----------------" << endl;
}


void test3(){
	BigInt a= "0x1234567876543210", b="0x 1000000000000001 FFFFFFFF00000001", c="0x1";
	BigInt bi1 = "0x 0000000000000001 0000000000000000 0000000000000000", result;
	BigInt bi2 = "0x ffffffffffffffff ffffffffffffffff ffffffffffffffff";
	cout << "--------------Test Ejercicio 3--------------" << endl;
	
	BigInt bi3 = b*bi2;
	cout << "bi1 = " << b << endl;
	cout << "bi2 = " << bi2 << endl;
	cout << "bi3 = b * bi2 = " << bi3 << endl; 
	
	BigInt bi4 = c*bi2;
	cout << "c = " << c << endl;
	cout << "bi2 = " << bi2 << endl;
	cout << "bi4 = c * bi2 = " << bi4 << endl; 
	
	cout << "----------------Test 3 OK :)-----------------" << endl;
}

void test4(){
	BigInt bi1 = "0x F234567876543210 0000000000000001 0000000000000000 F234567876543210", result;
	BigInt bi2 = "0x 0000000000000001 FFFFFFFF00000001 ffffffffffffffff";
	//BigInt bi2 = "0x ffffffffffffffff ffffffffffffffff ffffffffffffffff";
	BigInt bi1_n = -bi1, bi2_n = -bi2;
	BigInt q, r, prueba;
	
	cout << "--------------Test Ejercicio 4--------------" << endl;
	
	bi1.scholarDivision(bi2, q, r);
	cout << endl << "RESULTADOS FINALES:" << endl;
	cout << "Dividendo: " << bi1 << endl;
	cout << "Divisor: " << bi2 << endl;
	cout << "Cociente: " << q << endl;
	cout << "Resto: " << r << endl;
	prueba = bi2*q + r;
	cout << "Dividendo con prueba de division: " << prueba << endl;
	assert(prueba == bi1);
	
	cout << "-----Division por un numero mayor que el dividendo-----" << endl;
	bi2.scholarDivision(bi1, q, r);
	cout << "Dividendo: " << bi2 << endl;
	cout << "Divisor: " << bi1 << endl;
	cout << "Cociente: " << q << endl;
	cout << "Resto: " << r << endl;
	prueba = bi1*q + r;
	cout << "Dividendo con prueba de division: " << prueba << endl;
	assert(prueba == bi2);
	
	cout << "-----Division con dividendo negativo-----" << endl;
	bi1_n.scholarDivision(bi2, q, r);
	cout << "Dividendo: " << bi1_n << endl;
	cout << "Divisor: " << bi2 << endl;
	cout << "Cociente: " << q << endl;
	cout << "Resto: " << r << endl;
	prueba = bi2*q + r;
	cout << "Dividendo con prueba de division: " << prueba << endl;
	assert(prueba == bi1_n);
	
	cout << "-----Division con divisor negativo-----" << endl;
	bi1.scholarDivision(bi2_n, q, r);
	cout << "Dividendo: " << bi1 << endl;
	cout << "Divisor: " << bi2_n << endl;
	cout << "Cociente: " << q << endl;
	cout << "Resto: " << r << endl;
	prueba = bi2_n*q + r;
	cout << "Dividendo con prueba de division: " << prueba << endl;
	assert(prueba == bi1);
	
	cout << "-----Division con dividendo y divisor negativos-----" << endl;
	bi1_n.scholarDivision(bi2_n, q, r);
	cout << "Dividendo: " << bi1_n << endl;
	cout << "Divisor: " << bi2_n << endl;
	cout << "Cociente: " << q << endl;
	cout << "Resto: " << r << endl;
	prueba = bi2_n*q + r;
	cout << "Dividendo con prueba de division: " << prueba << endl;
	assert(prueba == bi1_n);
	
	cout << "----------------Test 4 OK :)-----------------" << endl;
}

void test5(){
	BigInt a= "0x1234567876543210", b="0x 1000000000000001 FFFFFFFF00000001", c="0x1";
	BigInt bi1 = "0x 0000000000000001 0000000000000000 0000000000000000", result;
	BigInt bi2 = "0x ffffffffffffffff ffffffffffffffff ffffffffffffffff";
	cout << "--------------Test Ejercicio 5--------------" << endl;
	
	BigInt bi3 = b.karatsubaMult(bi2);
	cout << "bi1 = " << b << endl;
	cout << "bi2 = " << bi2 << endl;
	cout << "bi3 = b * bi2 = " << bi3 << endl; 
	
	BigInt bi4 = c.karatsubaMult(bi2);
	cout << "c = " << c << endl;
	cout << "bi2 = " << bi2 << endl;
	cout << "bi4 = c * bi2 = " << bi4 << endl; 
	
	cout << "----------------Test 5 OK :)-----------------" << endl;
}

void test6(){
	BigInt a= "0x1234567876543210", b="0x 1000000000000001 FFFFFFFF00000001", c="0x1";
	BigInt bi1 = "0x 0000000000000001 0000000000000000 0000000000000000", result;
	BigInt bi2 = "0x ffffffffffffffff ffffffffffffffff fffffffffffffff0";
	BigInt mcd, u0, v0, mcm;
	cout << "--------------Test Ejercicio 6--------------" << endl;
	bi1.EEA(bi2, mcd, mcm, u0, v0);
	cout << "bi1 = " << bi1 << endl;
	cout << "bi2 = " << bi2 << endl;
	cout << "mcd = " << mcd << endl;
	cout << "mcm = " << mcm << endl;
	cout << "u0 = " << u0 << endl;
	cout << "v0 = " << v0 << endl;
	
	cout << "Igualdad de Bezout: (bi1, bi2) = u0*bi1 + v0*bi2 " << endl;
	cout << mcd << " = " << u0*bi1 + v0*bi2 << endl; 
	cout << "----------------Test 6 OK :)-----------------" << endl;
}

void test_modInv(){
	BigInt a= "0x123456787654321", b="0x 1000000000000001 FFFFFFFF00000001", c="0x1";
	BigInt bi1 = "0x 0000000000000001 0000000000000000 0000000000000000", result;
	BigInt bi2 = "0x ffffffffffffffff ffffffffffffffff ffffffffffffffff";
	BigInt inv, r;
	BigInt mcd, u0, v0, mcm;
	bool exists;
	cout << "--------Test Ejercicio 7.1 - Prueba de inversos modulares---------" << endl;
	
	cout << "-----Existe inverso 1-----" << endl;
	cout << "n = " << bi2 << endl;
	cout << "m = " << bi1 << endl;
	exists = bi2.modularInverse(bi1, inv, true);
	if(exists){
		cout << "inv = " << inv << endl;
		r = (bi2*inv)%bi1;
		cout << "n * inv mod m = " << r << endl;
	}
	else{
		cout << "No hay inversa :( " << endl;
	}
	
	cout << "-----Existe inverso 2-----" << endl;
	cout << "n = " << a << endl;
	cout << "m = " << bi1 << endl;
	exists = a.modularInverse(bi1, inv, true);
	if(exists){
		cout << "inv = " << inv << endl;
		r = (a*inv)%bi1;
		cout << "n * inv mod m = " << r << endl;
	}
	else{
		cout << "No hay inversa :( " << endl;
	}
	
	cout << "-----No existe inverso-----" << endl;
	cout << "n = " << bi2 << endl;
	cout << "m = " << a << endl;
	exists = bi2.modularInverse(a, inv, true);
	if(exists){
		cout << "inv = " << inv << endl;
		r = (bi2*inv)%a;
		cout << "n * inv mod m = " << r << endl;
	}
	else{
		cout << "No hay inversa :( " << endl;
	}
	
	cout << "----------------Test Inversos Modulares OK :)-----------------" << endl;
}

void test7(){
	
	BigInt a="0x 1000000000000001 FFFFFFFF00000001", b="0x1234567876543210", c="0x1";
	BigInt bi1 = "0x 0000000000000001 0000000000000000 0000000000000000", result;
	BigInt bi2 = "0x ffffffffffffffff ffffffffffffffff fffffffffffffff0";
	BigInt res;
	cout << "--------------Test Ejercicio 7--------------" << endl;
	
	/*
	BigInt a="0x 1000000000000001 FFFFFFFF00000001", b="0x100";
	BigInt bi1 = "0x 0000000000000001 0000000000000000 0000000000000000";
	BigInt res;
	*/
	res = a.quickModExp(b, bi1);
	cout << "a = " << a << endl;
	cout << "b = " << b << endl;
	cout << "m = " << bi1 << endl;
	cout << "a^b mod n = " << res << endl;

	cout << "----------------Test 7 OK :)-----------------" << endl;
}

void test8(){
	
	BigInt a="0x 1000000000000001 FFFFFFFF00000001", b="0x1234567876543210", c="0x1";
	BigInt bi1 = "0x 0000000000000001 0000000000000000 0000000000000000", result;
	BigInt bi2 = "0x ffffffffffffffff ffffffffffffffff fffffffffffffff0";
	a = "0x5af3107a401f";	//Numero primo
	bool res;
	cout << "--------------Test Ejercicio 8--------------" << endl;
	
	res = a.millerRabinTest(10);
	cout << "a = " << a << endl;
	if(res){
		cout << "Es primo :)" << endl;
	}
	else{
		cout << "No es primo :0" << endl;
	}
	
	res = bi2.millerRabinTest(10);
	cout << "a = " << bi2 << endl;
	if(res){
		cout << "Es primo :)" << endl;
	}
	else{
		cout << "No es primo :0" << endl;
	}
	
	cout << "----------------Test 8 OK :)-----------------" << endl;
} 

int main(){
	// Dividendo de 128 bits (representado por dos partes de 64 bits)
	BigInt::initializeGenerator(); // Inicializar el generador de números aleatorios
	test1();
	test2();
	test3();
	test4();
	test5();
	test6();
	test_modInv();
	test7();
	test8();
	return 0;
}
