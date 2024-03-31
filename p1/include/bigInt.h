#include <vector>
#include "stdint.h"
#include <string>
#include <iostream>
#include <cassert>
#include <cmath>
#include <bitset>
#include <random>

using namespace std;

struct UInt128 {
	uint64_t high;
	uint64_t low;
};


class BigInt{
	private:
		vector<uint64_t> digits;	// Vector con los digitos del BigInt en formato Little Endian
		bool sign;	//Signo del entero (true = positivo, false = negativo)
		static bool random_initialized;
		static mt19937_64 generator; // Generador de números aleatorios

		//Pone el valor del numero a 0
		void setToZero();
		//Quita los ceros a la izquierda del numero
		void eraseZerosAtLeft();
		//Borra espacios en blanco de un string
		void eraseBlankSpaces(string &s);
		//Dado un string con un numero hexadecimal, quita la cabecera del numero en el string y
		//devuelve el signo del mismo mediante referencias. Devuelve true si la operacion ha tenido
		//exito, y false en otro caso (p.e. que el numero no tenga el formato esperado).
		bool eraseHeaderFromBigIntString(string& s, bool& sign);
		
		//Crea un array de Digits a partir de una cadena con un numero en hexadecimal
		//sin signo, cabecera (0x) ni espacios
		vector<uint64_t> createDigitsFromString(const string& s);
		
	public:
		//Generacion de numeros aleatorios
		static void initializeGenerator();
		
		//Generador de BigInt positivos aleatorios
		//limit representa el valor maximo que puede tomar el numero aleatorio
		static BigInt randomBigInt(const BigInt& limit);
		//por numero de digitos
		static BigInt randomBigInt(int digs);
		//Constructor por defecto (inicializa el valor a +0)
		BigInt();
		//Constructor por parametros
		BigInt(const string& s0);
		BigInt(const char* s0);
		
		//////Getters
		//Digits
		vector<uint64_t> getDigits() const;
		vector<uint64_t>& getDigits();
		//Sign
		bool getSign() const;
		
		//////Setters
		//Digits
		void setDigits(const vector<uint64_t>& digs);
		//Sign
		void setSign(bool s);
		
		//Operador de asignacion - BigInt
		BigInt& operator=(const BigInt& other);
		//Operador de asignacion - String
		BigInt& operator=(const string& s0);
		//Operador de asignacion - cString
		BigInt& operator=(const char* s0);
		
		//Conversion a string
		//Crea un string con el numero representado en base 16
		string toString() const;
		
		// Sobrecarga del operador << como función amiga para imprimir BigInt en cout
		friend ostream& operator<<(ostream& os, const BigInt& bigint);
		
		/////Operadores de comparacion e igualdad
		//Igualdad
		bool operator==(const BigInt& other) const;
		//Desigualdad
		bool operator!=(const BigInt& other) const;
		//Menor que
		bool operator<(const BigInt& other) const;
		//Mayor que
		bool operator>(const BigInt& other) const;
		//Menor o igual que
		bool operator<=(const BigInt& other) const;
		//Mayor o igual que
		bool operator>=(const BigInt& other) const;
		
		/////////-------Operaciones aritmeticas elementales-------/////////
		//2.1. Cambio de signo / Suma / Resta
		BigInt operator-() const;
		BigInt operator+(const BigInt& other) const;
		BigInt operator-(const BigInt& other) const;
		
		//3. Multiplicacion escolar
		BigInt scholarMult(const BigInt& other) const;
		BigInt operator*(const BigInt& other) const;
		
		//4. Division
		void scholarDivision(BigInt d, BigInt& q, BigInt& r, bool debug = false) const;
		BigInt operator/(const BigInt& other) const;
		BigInt operator%(const BigInt& other) const;
		
		//5. Algoritmo de Karatsuba
		BigInt karatsubaMult(const BigInt& other, bool debug = false) const;
		
		//6. Algoritmo Extendido de Euclides
		void EEA(const BigInt& b_ini, BigInt& mcd, BigInt& mcm, BigInt& u0, BigInt& v0) const;
		
		//7. Exponenciacion rapida
		BigInt quickModExp(const BigInt& exp, const BigInt& mod) const;
		
		//8. Test de Miller-Rabin
		bool strongPseudoprime(const BigInt& base, bool debug = false) const;
		bool millerRabinTest(int k, bool debug = false);
		
		//Extra
		bool modularInverse(const BigInt& mod, BigInt& inverse, bool debug = false) const;
		
};

BigInt creaBigInt(const string& s);


