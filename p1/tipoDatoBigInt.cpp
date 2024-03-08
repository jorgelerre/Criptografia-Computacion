#include <vector>
#include "stdint.h"
#include <string>
#include <iostream>
#include <cassert>


using namespace std;

struct UInt128 {
	uint64_t high;
	uint64_t low;
};

//Multiplica dos uint64_t y suma un acarreo, devolviendo el producto en otros dos uint64_t:
//result0 es la parte menos significativa (los ultimos 64b), mientras que
//result1 es la parte con los 64b mas significativos
UInt128 multiply64(uint64_t a, uint64_t b, uint64_t c){
	UInt128 result;
	unsigned int carry = 0;	//Acarreo de las sumas en result0, que se agregara a result1
	uint64_t partial_result0, partial_result1;
	
	//Descomponemos a y b en sus 32b mas y menos significativos
	uint64_t aLow = a & 0x00000000FFFFFFFF;
	uint64_t aHigh = a >> 32;
	uint64_t bLow = b & 0x00000000FFFFFFFF;
	uint64_t bHigh = b >> 32;
	
	cout << hex << "aHigh = " << aHigh << "\taLow = " << aLow << endl;
	cout << hex << "bHigh = " << bHigh << "\tbLow = " << bLow << endl;
	cout << hex << "c = " << c << endl; 
	
	//Efectuamos los productos entre las partes de 32b, de forma que el resultado sea de 64b
	uint64_t resultLow = aLow * bLow;
	uint64_t resultMid1 = aLow * bHigh;
	uint64_t resultMid2 = aHigh * bLow;
	uint64_t resultHigh = aHigh * bHigh;
	
	cout << hex << "resultLow: " << resultLow << endl;
	cout << hex << "resultMid1: " << resultMid1 << endl;
	cout << hex << "resultMid2: " << resultMid2 << endl;
	cout << hex << "resultHigh: " << resultHigh << endl;
	
	//Realizamos la suma de las partes para calcular la multiplicacion, que se almacenara
	//entre dos uint64_t
	//Vamos controlando tambien el carry que puede tener esta operacion
	partial_result0 = resultLow + (resultMid1 << 32);
	cout << hex << "partial_result0: " << partial_result0 << endl;
	carry = (partial_result0 < resultLow) ? carry+1 : carry;
	partial_result1 = partial_result0 + (resultMid2 << 32); 
	cout << hex << "partial_result1: " << partial_result1 << endl;
	carry = (partial_result1 < partial_result0) ? carry+1 : carry;
	result.low = partial_result1 + c;
	
	carry = (result.low < partial_result1 || result.low < c) ? carry+1 : carry;
	result.high = resultHigh + (resultMid1 >> 32) + (resultMid2 >> 32) + carry;
	
	cout << hex << "Carry: " << result.high << endl;
	cout << hex << "Result: " << result.low << endl;
	
	return result;
}

// Función para dividir un entero sin signo de 128 bits entre un entero sin signo de 64 bits
void divide128By64(UInt128 dividend, uint64_t divisor, UInt128 & quotient, uint64_t & remainder) {
	quotient = {0, 0};
	uint64_t actual_div = 0, resta;
	bool carry = false;
	//Realizamos la division en binario, bit a bit, para la primera parte del cociente
	for(int i = 64; i > 0; i--){
		//Incluimos un nuevo bit en el dividendo actual
		actual_div = (actual_div << 1) | (dividend.high << (64 - i) >> 63);
		//cout << "Digito " << i << " = " << actual_div << endl;
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
		//cout << "Digito " << i << " = " << actual_div << endl;
		
		//Restamos el divisor al dividendo actual
		resta = actual_div - divisor;
		//if(carry)
		//	printf("C");
		//printf("%016lx - %016lx = %016lx \n", actual_div, divisor, resta);
		
		//Si no hay acarreo, o hay acarreo virtual (por la presencia de un 1 a la izquierda 
		//no se hubiera producido acarreo), entonces actual_div > divisor.
		//Por tanto, en el cociente colocamos un 1
		if(actual_div >= resta || carry){
			quotient.low = (quotient.low << 1) | 1;
			actual_div = resta;
			//printf("Aniade 1\n");
		}
		//Si hay acarreo, entonces tomamos cociente 0 
		else{
			quotient.low = quotient.low << 1;
			//printf("Aniade 0\n");
		}
		//printf("Cociente actual: %016lx %016lx\n\n", quotient.high, quotient.low);
	}
	remainder = actual_div;
	//printf("Resto: %016lx\n", actual_div);
}


class BigInt{
	private:
		vector<uint64_t> digits;
		bool sign;
		//Pone el valor del numero a 0
		void setToZero(){
			digits.clear();
			digits.resize(1);
			digits[0]=0;
			sign = true;	//Positivo->True, Negativo->False
		}
		//Quita los ceros a la izquierda del numero
		void eraseZerosAtLeft(){
			while(digits.back() == 0 && digits.size() > 1){
				digits.pop_back();
			}
		}
		//Borra espacios en blanco de un string
		void eraseBlankSpaces(string &s){
			//Borramos espacios en blanco
			for(int i = 0; i < s.size(); i++){
				if(s[i] == ' '){
					s = s.erase(i, 1);
					i--;
				}
			}
		}
		//Dado un string con un numero hexadecimal, quita la cabecera del numero en el string y
		//devuelve el signo del mismo mediante referencias. Devuelve true si la operacion ha tenido
		//exito, y false en otro caso (p.e. que el numero no tenga el formato esperado).
		bool eraseHeaderFromBigIntString(string& s, bool& sign){
			//Leemos signo y quitamos el 0x
			if(s.find("-0x") == 0){
				sign = false;
				s.erase(0,3);
			}
			else if(s.substr(0,2) == string("0x")){
				sign = true;
				s.erase(0,2);
			}
			else if(s.substr(0,3) == string("+0x")){
				sign = true;
				s.erase(0,3);
			}
			else{
				return false;
			}
			return true;
		}
		
		//Crea un array de Digits a partir de una cadena con un numero en hexadecimal
		//sin signo, cabecera (0x) ni espacios
		vector<uint64_t> createDigitsFromString(const string& s){
			//Borramos el contenido anterior de digits
			vector<uint64_t> digits;
			
			//Pasar de string a uint64 con strtoull
			int digitos = s.size()/16 + (s.size()%16 == 0 ? 0 : 1);
			//cout << "Tamaño string: " << s.size() << endl;
			//cout << "Digitos detectados: " << digitos << endl;
			int p_lectura;
			for(p_lectura= s.size()-16; p_lectura > 0; p_lectura-=16){
				string s_i = s.substr(p_lectura, 16);
				uint64_t n_i = strtoull(s_i.c_str(), NULL, 16);
				digits.push_back(n_i);
			}
			//Leemos el ultimo digito (el mas significativo, y que puede ser mas corto de 16 caracteres)
			string s_i = s.substr(0, p_lectura+16);
			digits.push_back(strtoull(s_i.c_str(), NULL, 16));
			return digits;
		}
		
	public:
		//Constructor por defecto (inicializa el valor a +0)
		BigInt(){
			setToZero();
		}
		//Constructor por parametros
		BigInt(const string& s0){
			//Hacemos una copia del string
			string s = s0;
			//cout << "string a convertir: " << s << endl;
			
			//Borramos espacios en blanco de s
			eraseBlankSpaces(s);
			//cout << "Sin espacios en blanco: " << s << endl;
			
			//Verificamos que el formato es correcto, y extraemos el signo
			bool format_ok = eraseHeaderFromBigIntString(s, sign);
			
			//Si el formato es correcto
			if(format_ok){
				//cout << "Sin signo ni 0x: " << s << endl;
				//Crea digits a partir del string
				digits = createDigitsFromString(s);
				//Quitamos ceros a la izquierda de digits
				eraseZerosAtLeft();
			}
			//Si el formato no es el correcto, ponemos el valor del numero a 0
			else{
				cout << "Error: el número no tiene el formato esperado."  << endl 
					 << "Llamando a constructor por defecto..." << endl;
				setToZero();
			}
		}
		BigInt(const char* s0){
			//Hacemos una copia del string
			string s(s0);
			//cout << "string a convertir: " << s << endl;
			
			//Borramos espacios en blanco de s
			eraseBlankSpaces(s);
			//cout << "Sin espacios en blanco: " << s << endl;
			
			//Verificamos que el formato es correcto, y extraemos el signo
			bool format_ok = eraseHeaderFromBigIntString(s, sign);
			
			//Si el formato es correcto
			if(format_ok){
				//cout << "Sin signo ni 0x: " << s << endl;
				//Crea digits a partir del string
				digits = createDigitsFromString(s);
				//Quitamos ceros a la izquierda de digits
				eraseZerosAtLeft();
			}
			//Si el formato no es el correcto, ponemos el valor del numero a 0
			else{
				cout << "Error: el número no tiene el formato esperado."  << endl 
					 << "Llamando a constructor por defecto..." << endl;
				setToZero();
			}
		}
		//////Getters
		//Digits
		vector<uint64_t> getDigits() const{
			return digits;
		}
		vector<uint64_t>& getDigits() {
			return digits;
		}
		//Sign
		bool getSign() const{
			return sign;
		}
		
		//////Setters
		//Digits
		void setDigits(const vector<uint64_t>& digs){
			digits = digs;
		}
		//Sign
		void setSign(bool s){
			sign = s;
		}
		
		//Operador de asignacion - BigInt
		BigInt& operator=(const BigInt& other){
			// Si los dos son el mismo objeto, devolvemos el objeto
			if (this == &other){
				return *this;
			}
			//Asignamos el estado de other a this
			this->digits = other.getDigits();
			this->sign = other.getSign();
			
			return *this;
		}
		//Operador de asignacion - String
		BigInt& operator=(const string& s0){
			//Hacemos una copia del string
			string s = s0;
			//cout << "string a convertir: " << s << endl;
			
			//Borramos espacios en blanco de s
			eraseBlankSpaces(s);
			//cout << "Sin espacios en blanco: " << s << endl;
			
			//Verificamos que el formato es correcto, y extraemos el signo
			bool format_ok = eraseHeaderFromBigIntString(s, sign);
			
			//Si el formato es correcto
			if(format_ok){
				//cout << "Sin signo ni 0x: " << s << endl;
				//Crea digits a partir del string
				digits = createDigitsFromString(s);
				//Quitamos ceros a la izquierda de digits
				eraseZerosAtLeft();
			}
			
			return *this;
		}
		//Operador de asignacion - cString
		BigInt& operator=(const char* s0){
			//Hacemos una copia del string
			string s = s0;
			//cout << "string a convertir: " << s << endl;
			
			//Borramos espacios en blanco de s
			eraseBlankSpaces(s);
			//cout << "Sin espacios en blanco: " << s << endl;
			
			//Verificamos que el formato es correcto, y extraemos el signo
			bool format_ok = eraseHeaderFromBigIntString(s, sign);
			
			//Si el formato es correcto
			if(format_ok){
				//cout << "Sin signo ni 0x: " << s << endl;
				//Crea digits a partir del string
				digits = createDigitsFromString(s);
				//Quitamos ceros a la izquierda de digits
				eraseZerosAtLeft();
			}
			
			return *this;
		}
		
		//Conversion a string
		//Crea un string con el numero representado en base 16
		string toString() const{
			char digito[18];
			string numero = "";
			if (!sign){
				numero += "-";
			}
			numero += "0x";
			for(int i=digits.size()-1; i >= 0; i--){
				sprintf(digito, " %016lx", digits[i]);
				string d(digito);
				numero += d;
			}
			return numero;
		}
		
		// Sobrecarga del operador << como función amiga para imprimir BigInt en cout
		friend ostream& operator<<(ostream& os, const BigInt& bigint) {
			os << bigint.toString();
			return os;
		}
		
		/////Operadores de comparacion e igualdad
		//Igualdad
		bool operator==(const BigInt& other) const{
			bool equals = true;
			//Comprobamos que los signos y el numero de digitos sean iguales
			if(sign != other.getSign() || digits.size() != other.getDigits().size()){
				equals = false;
			}
			else{
				int i = 0;
				//Comprobamos digito a digito que los valores sean iguales
				while(equals && i < digits.size()){
					if(digits.at(i) != other.getDigits().at(i)){
						equals = false;
					}
					i++;
				}
			}
			return equals;
		}
		//Desigualdad
		bool operator!=(const BigInt& other) const{
			return ! (*this == other);
		}
		//Menor que
		bool operator<(const BigInt& other) const{
			bool lower = false;	//Suponemos que this es igual o mayor a other inicialmente
			//Si los dos numeros son positivos
			if(sign == true && other.getSign() == true){
				//Comparamos el numero de digitos (la representacion no tendra nunca 0s a la izquierda)
				if(digits.size() < other.getDigits().size()){
					
					lower = true;
				}
				else if(digits.size() == other.getDigits().size()){
					bool equals = true;
					for(int i = digits.size()-1; i >= 0 && equals; i--){
						if(digits.at(i) < other.getDigits().at(i)){
							lower = true;
							equals = false;
						}
						else if(digits.at(i) > other.getDigits().at(i)){
							equals = false;
						}
					}
				}
			}
			//Si los dos numeros son negativos
			else if(sign == false && other.getSign() == false){
				//Comparamos el numero de digitos (la representacion no tendra nunca 0s a la izquierda)
				if(digits.size() < other.getDigits().size()){
					lower = true;
				}
				else if(digits.size() == other.getDigits().size()){
					bool equals = true;
					for(int i = digits.size()-1; i >= 0 && equals; i--){
						if(digits.at(i) > other.getDigits().at(i)){
							lower = true;
							equals = false;
						}
						else if(digits.at(i) < other.getDigits().at(i)){
							equals = false;
						}
					}
				}
			}
			//Si this es negativo y other es positivo
			else if(sign == false && other.getSign() == true){
				lower = true;	//Un numero negativo siempre es menor que uno positivo
			}
			//En el ultimo caso (this positivo, other negativo), como un numero negativo 
			//siempre es menor que uno positivo, lower = false (que ya esta puesto inicialmente)
			return lower;
		}
		//Mayor que
		bool operator>(const BigInt& other) const{
			return other < *this;
		}
		//Menor o igual que
		bool operator<=(const BigInt& other) const{
			return ! (other < *this);
		}
		//Mayor o igual que
		bool operator>=(const BigInt& other) const{
			return ! (*this < other);
		}
		
		/////////-------Operaciones aritmeticas elementales-------/////////
		//2.1. Cambio de signo
		BigInt operator-() const{
			BigInt bint = *this;
			bint.setSign(!sign);
			return bint;
		}
		
		BigInt operator+(const BigInt& other) const{
			BigInt sum;
			uint64_t a, b, carry = 0, res, res_c;
			
			//Si los signos son diferentes, realmente se esta haciendo una resta
			if(getSign() != other.getSign()){
				//Si el primer operando es el positivo, haremos op1 - (-op2)
				if(getSign()){
					sum = (*this) - (-other);
				}
				//Si el segundo operando es el positivo, haremos op2 - (-op1)
				else{
					sum = other - (- *this);
				}
			}
			//Si los signos son iguales, sumamos |a| y |b| y se mantiene el mismo signo
			else{
				//Asignamos el mismo signo
				sum.setSign(sign);
				//Eliminamos el 0 almacenado por defecto en sum al construirse por defecto
				sum.getDigits().clear();
				//Realizamos la suma "digito a digito" siguiendo el algoritmo escolar
				for(int i = 0; i < digits.size() || i < other.getDigits().size(); i++){
					//Obtenemos los digitos a sumar de cada BigInt (si no quedan, usamos 0)
					if(i < digits.size()){
						a = digits.at(i);
					}
					else{
						a = 0;
					}
					if(i < other.getDigits().size()){
						b = other.getDigits().at(i);
					}
					else{
						b = 0;
					}
					//Realizamos la suma a+b+carry
					res = a + b;
					res_c = res + carry;
					//Incluimos el digito en el resultado
					sum.getDigits().push_back(res_c);
					//Determinamos si hay overflow en alguna de las sumas anteriores. Un desbordamiento 
					//ocurre si el resultado es menor que cualquiera de los operandos originales.
					//En dicho caso, almacenamos el acarreo en carry.
					carry = (res < a || res < b || res_c < res || res_c < carry) ? 1 : 0;
				}
				//Si queda acarreo, lo colocamos en el ultimo digito del resultado
				if(carry == 1){
					sum.getDigits().push_back(1);
				}
			}
			
			return sum;
		}
		
		BigInt operator-(const BigInt& other) const{
			BigInt dif;
			const BigInt *min, *sus;
			uint64_t a, b, carry = 0, res, res_c;
			
			//Si los signos son diferentes, realmente se esta haciendo una suma
			if(getSign() != other.getSign()){
				dif= (*this) + (-other);
			}
			//Si los signos son iguales, restamos el numero mas grande al mas pequeño
			//El resultado tendra el signo de los operandos si el minuendo es mayor que el sustraendo
			else{
				//Si el this es mayor (en valor absoluto) que other
				if((*this >= other && sign) || (*this <= other && !sign)){
					//Colocamos *this como minuendo y other como sustraendo
					//El signo resultante sera el mismo que tienen los dos operandos
					min = this;
					sus = &other;
					dif.setSign(getSign());
				}
				else{
					//Colocamos other como minuendo y *this como sustraendo
					//El signo resultante sera el contrario al que tienen los dos operandos
					min = &other;
					sus = this;
					dif.setSign(!getSign());
				}
				dif.getDigits().clear();
				//Realizamos la resta "digito a digito" siguiendo el algoritmo escolar
				for(int i = 0; i < min->getDigits().size(); i++){
					//Obtenemos los digitos a restar de cada BigInt (si no quedan, usamos 0)
					a = min->getDigits().at(i);
					if(i < sus->getDigits().size()){
						b = sus->getDigits().at(i);
					}
					else{
						b = 0;
					}
					
					//Realizamos la resta a-b-carry
					res = a - b;
					res_c = res - carry;
					//Incluimos el digito en el resultado
					dif.getDigits().push_back(res_c);
					//Determinamos si hay overflow en alguna de las restas anteriores. Un desbordamiento 
					//ocurre si el resultado es mayor que cualquiera de los minuendos.
					//En dicho caso, almacenamos el acarreo en carry.
					carry = (res_c > res || res > a) ? 1 : 0;
					//cout << "a = " << a << "\tb = " << b << "\tc = " << carry << endl;
					//cout << "res = " << res << "\tres_c = " << res_c << endl;
					//cout << "Carry " << i << " = " << carry << endl;
				}
			}
			dif.eraseZerosAtLeft();
			return dif;
		}
		
		//3. Multiplicacion escolar
		BigInt scholarMult(const BigInt& other) const{
			uint64_t zero = 0;
			BigInt res = "0x0";
			UInt128 mult;
			//Calculamos la multiplicacion
			for(int j = 0; j < other.getDigits().size(); j++){
				BigInt bi_part;
				mult = {0,0};
				bi_part.getDigits().clear();	//Quitamos todos los digitos
				for(int k = 0; k < j; k++){
					bi_part.getDigits().push_back(zero);
				}
				for(int i = 0; i < getDigits().size(); i++){
					cout << "Multiplicando (" << i << "," << j << ")" << endl;
					mult = multiply64(getDigits().at(i),other.getDigits().at(j),mult.high);
					bi_part.getDigits().push_back(mult.low);
				}
				bi_part.getDigits().push_back(mult.high);
				cout << bi_part << endl;
				res = res + bi_part;
				//cout << "getDigits().size() = " << getDigits().size() << (*this) << endl;
				//cout << "other.getDigits().size() = " << other.getDigits().size() << endl;
			}
			res.eraseZerosAtLeft();
			
			//Calculamos el signo (sera positivo sii coinciden los signos)
			bool signo = (getSign() && other.getSign()) || (!getSign() && !other.getSign());
			res.setSign(signo);
			
			cout << "Producto: " << res << endl;
			
			return res;
		}
		
		void scholarDivision(BigInt d, BigInt& q, BigInt& r){
			//Borramos los valores previos de q y r
			q.getDigits().clear();
			r.getDigits().clear();	
			
			//Si el divisor tiene un digito
			if(d.getDigits().size() == 1){
				//Guardamos el divisor en un uint64_t
				uint64_t divisor = d.getDigits().at(0);
				//Comprobamos division entre 0
				if(divisor == 0){
					//cout << "Error: division entre 0" << endl;
					q = "0x0";
					r = "-0x1";
				}
				//Si el divisor es valido
				else{
					UInt128 dividend, q_act;
					uint64_t dividend_rem;
					unsigned int m = getDigits().size() - 1;
					
					//Introducimos en el cociente m + 1 digitos en blanco
					for(int i = m + 1; i > 0; i--){
						q.getDigits().push_back(0);
					}
					//Dividimos por partes
					//Primera iteracion: usamos un 0 a la izquierda del dividendo
					dividend = {0, getDigits()[m]};
					divide128By64(dividend, divisor, q_act, dividend_rem);
					q.getDigits()[m] = q_act.low;
					
					for(int i = m; i > 0; i--){
						dividend = {dividend_rem, getDigits().at(i-1)};
						divide128By64(dividend, divisor, q_act, dividend_rem);
						q.getDigits()[i-1] = q_act.low;
					}
					r.getDigits().push_back(dividend_rem);
				}
			}
			//Si el divisor tiene mas de dos digitos
			else{
				BigInt dividend_norm, divisor_norm, norm_bi, q_hat_bi, zero;
				BigInt dividend2_bi, divisor2_bi, remainder_bi;
				BigInt dividendj_norm, aux_bi;
				UInt128 base = {1,0}, aux128, div_act;
				uint64_t norm, aux, q_hat;
				//Digito mas significativo de divisor
				uint64_t d_n = d.getDigits()[d.getDigits().size()-1];	
				
				
				//Normalizamos los operandos
				if(d_n + 1 == 0){	//Si d_n + 1, entonces es igual a la base, y norm=1
					norm = 1;
				}
				else{
					divide128By64(base, d_n + 1, aux128, aux);
					norm = aux128.low;
				}
				
				norm_bi.getDigits()[0] = norm;
				dividend_norm = (*this) * norm_bi;
				dividend_norm.getDigits().push_back(0);	//Anadimos 0 al final
				
				divisor_norm = d * norm_bi;
				
				cout << "Dividendo: " << hex <<  (*this) << endl;
				cout << "Divisor: " << hex <<  d << endl;
				cout << hex << "d_n = " << d_n << endl;
				cout << "Factor de normalizacion: " << hex <<  norm_bi << endl;
				cout << "Dividendo normalizado: " << hex <<  dividend_norm << endl;
				cout << "Divisor normalizado: " << hex << d * norm_bi << endl;
				
				//Introducimos en el cociente los digitos en blanco necesarios
				for(int i = dividend_norm.getDigits().size() - 1; 
						i >= divisor_norm.getDigits().size() - 1; i--){
					q.getDigits().push_back(0);
				}
				
				for(int i = dividend_norm.getDigits().size() - 1; 
						i >= divisor_norm.getDigits().size() - 1; i--){
					//Obtenemos el segundo digito mas significativo de dividendo y divisor
					//y los guardamos en un BigInt
					dividend2_bi.getDigits()[0] = dividend_norm.getDigits()[i-2];
					divisor2_bi.getDigits()[0] = divisor_norm.getDigits()[divisor_norm.getDigits().size()-2];
					
					if(divisor_norm.getDigits().at(divisor_norm.getDigits().size() - 1)
					   == dividend_norm.getDigits()[i]){
						q_hat = (uint64_t) - 1;
						divide128By64(div_act, q_hat, aux128, aux);
						remainder_bi.getDigits()[0] = aux;
					}
					else{
						div_act.high = dividend_norm.getDigits()[i];
						div_act.low = dividend_norm.getDigits()[i-1];
						divide128By64(div_act, d_n, aux128, aux);
						q_hat = aux128.low;
					}
					q_hat_bi.getDigits()[0] = q_hat;
					
					BigInt a1 = q_hat_bi * divisor2_bi;
					BigInt a2 = remainder_bi + dividend2_bi;
					if(a1 > a2){
						q_hat_bi.getDigits()[0] = q_hat - 1;
						a1 = q_hat_bi * divisor2_bi;
						if(a1 > a2){
							q_hat_bi.getDigits()[0] = q_hat - 2;
						}
					}
					//Obtenemos la parte actual del dividendo
					dividendj_norm.getDigits().clear();
					for(int j = divisor_norm.getDigits().size(); j >= 0 ; j--){
						dividendj_norm.getDigits().push_back(dividend_norm.getDigits()[i-j]);
					}
					dividendj_norm = dividendj_norm - q_hat_bi*divisor_norm;
					
					while(dividendj_norm < zero){
						dividendj_norm = dividendj_norm + divisor_norm;
						q_hat_bi.getDigits()[0]--;
					}
					
					for(int j = divisor_norm.getDigits().size(); j >= 0 ; j--){
						dividend_norm.getDigits()[i-j] = dividendj_norm.getDigits()[divisor_norm.getDigits().size()-j];
					}
					
					q.getDigits()[i-divisor_norm.getDigits().size()] = q_hat_bi.getDigits()[0];
				}
				dividendj_norm.scholarDivision(norm_bi, aux_bi, r);
			}
		}
		/*
		//4. Division
		BigInt operator/(const BigInt& other) const{
			BigInt
		}
		*/
		BigInt operator*(const BigInt& other) const{
			return this->scholarMult(other);
		}
		
};


BigInt creaBigInt(const string& s){
	BigInt b(s);
	return b;
}

void test1(){
	BigInt bint1,
		   bint2("-0x  9638527410 0123456789abcdef fedcba9876543210"),
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
	//
	
	BigInt bi3 = b*bi2;
	cout << "bi1 = " << b << endl;
	cout << "bi2 = " << bi2 << endl;
	cout << "bi3 = b * bi2 = " << bi3 << endl; 
	/*
	BigInt bi4 = c*bi2;
	cout << "c = " << c << endl;
	cout << "bi2 = " << bi2 << endl;
	cout << "bi4 = c * bi2 = " << bi4 << endl; 
	*/
	cout << "----------------Test 3 OK :)-----------------" << endl;
}

void test4(){
	BigInt bi1 = "0x F234567876543210 0000000000000001 0000000000000000 F234567876543210", result;
	BigInt bi2 = "0x ffffffffffffffff ffffffffffffffff";
	BigInt q, r;
	
	cout << "--------------Test Ejercicio 4--------------" << endl;
	bi1.scholarDivision(bi2, q, r);
	cout << "Dividendo: " << bi1 << endl;
	cout << "Divisor: " << bi2 << endl;
	cout << "Cociente: " << q << endl;
	cout << "Resto: " << r << endl;
	
	bi2.scholarDivision(bi1, q, r);
	cout << "Dividendo: " << bi2 << endl;
	cout << "Divisor: " << bi1 << endl;
	cout << "Cociente: " << q << endl;
	cout << "Resto: " << r << endl;
	
	cout << "----------------Test 4 OK :)-----------------" << endl;
}

int main(){
	test3();
	//test4();
}








