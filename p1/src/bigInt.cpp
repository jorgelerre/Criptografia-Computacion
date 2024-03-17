#include <vector>
#include "stdint.h"
#include <string>
#include <iostream>
#include <cassert>
#include <cmath>
#include <bitset>
#include <random>
#include "bigInt.h"

using namespace std;


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
	/*
	cout << hex << "aHigh = " << aHigh << "\taLow = " << aLow << endl;
	cout << hex << "bHigh = " << bHigh << "\tbLow = " << bLow << endl;
	cout << hex << "c = " << c << endl; 
	*/
	//Efectuamos los productos entre las partes de 32b, de forma que el resultado sea de 64b
	uint64_t resultLow = aLow * bLow;
	uint64_t resultMid1 = aLow * bHigh;
	uint64_t resultMid2 = aHigh * bLow;
	uint64_t resultHigh = aHigh * bHigh;
	/*
	cout << hex << "resultLow: " << resultLow << endl;
	cout << hex << "resultMid1: " << resultMid1 << endl;
	cout << hex << "resultMid2: " << resultMid2 << endl;
	cout << hex << "resultHigh: " << resultHigh << endl;
	*/
	//Realizamos la suma de las partes para calcular la multiplicacion, que se almacenara
	//entre dos uint64_t
	//Vamos controlando tambien el carry que puede tener esta operacion
	partial_result0 = resultLow + (resultMid1 << 32);
	//cout << hex << "partial_result0: " << partial_result0 << endl;
	carry = (partial_result0 < resultLow) ? carry+1 : carry;
	partial_result1 = partial_result0 + (resultMid2 << 32); 
	//cout << hex << "partial_result1: " << partial_result1 << endl;
	carry = (partial_result1 < partial_result0) ? carry+1 : carry;
	result.low = partial_result1 + c;
	
	carry = (result.low < partial_result1 || result.low < c) ? carry+1 : carry;
	result.high = resultHigh + (resultMid1 >> 32) + (resultMid2 >> 32) + carry;
	/*
	cout << hex << "Carry: " << result.high << endl;
	cout << hex << "Result: " << result.low << endl;
	*/
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


//Pone el valor del numero a 0
void BigInt::setToZero(){
	digits.clear();
	digits.resize(1);
	digits[0]=0;
	sign = true;	//Positivo->True, Negativo->False
}

//Quita los ceros a la izquierda del numero
void BigInt::eraseZerosAtLeft(){
	while(digits.back() == 0 && digits.size() > 1){
		digits.pop_back();
	}
}
//Borra espacios en blanco de un string
void BigInt::eraseBlankSpaces(string &s){
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
bool BigInt::eraseHeaderFromBigIntString(string& s, bool& sign){
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
vector<uint64_t> BigInt::createDigitsFromString(const string& s){
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

//Generacion de numeros aleatorios
void BigInt::initializeGenerator() {
    random_device rd;
    generator.seed(rd()); // Utiliza una semilla aleatoria para inicializar el generador
    random_initialized = true;
}

//Generador de BigInt positivos aleatorios
//limit representa el valor maximo que puede tomar el numero aleatorio
BigInt BigInt::randomBigInt(const BigInt& limit){
	uint64_t random_value, random_value_limit;
	//Inicializamos los numeros aleatorios si es la primera vez
	if(!random_initialized){
		initializeGenerator();
	}
	//Creamos una distribucion uniforme
	uniform_int_distribution<uint64_t> dis;
	
	//Copiamos el limite en r_bi
	BigInt r_bi = limit;
	
	//Cambiamos los digitos menos significativos de r_bi por numeros aleatorios
	for(int i = 0; i < r_bi.getDigits().size() - 1; i++){
		random_value = dis(generator);
		r_bi.getDigits()[i] = random_value;
	}
	
	//Cambiamos el digito mas significativo de r_bi, de forma que no sea mas grande que limit
	//Valor final = Random * valor  / Max_uint64t
	random_value = dis(generator);
	BigInt aux1, aux2, base("0x10000000000000000"), q, r;
	aux1.getDigits()[0] = random_value;
	aux2.getDigits()[0] = limit.getDigits()[r_bi.getDigits().size()-1];
	aux1 = aux1 * aux2;
	q = aux1 / base;
	r_bi.getDigits()[r_bi.getDigits().size()-1] = q.getDigits()[0];
	//En caso de que el numero generado sea mayor, restamos 1 al digito mas significativo
	if(r_bi > limit && r_bi.getDigits()[r_bi.getDigits().size()-1] > 0){
		r_bi.getDigits()[r_bi.getDigits().size()-1]--;
	}
	r_bi.eraseZerosAtLeft();
	return r_bi;
}

//Constructor por defecto (inicializa el valor a +0)
BigInt::BigInt(){
	setToZero();
}
//Constructor por parametros
BigInt::BigInt(const string& s0){
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
BigInt::BigInt(const char* s0){
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
vector<uint64_t> BigInt::getDigits() const{
	return digits;
}
vector<uint64_t>& BigInt::getDigits() {
	return digits;
}
//Sign
bool BigInt::getSign() const{
	return sign;
}

//////Setters
//Digits
void BigInt::setDigits(const vector<uint64_t>& digs){
	digits = digs;
}
//Sign
void BigInt::setSign(bool s){
	sign = s;
}

//Operador de asignacion - BigInt
BigInt& BigInt::operator=(const BigInt& other){
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
BigInt& BigInt::operator=(const string& s0){
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
BigInt& BigInt::operator=(const char* s0){
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
string BigInt::toString() const{
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
ostream& operator<<(ostream& os, const BigInt& bigint) {
	os << bigint.toString();
	return os;
}

/////Operadores de comparacion e igualdad
//Igualdad
bool BigInt::operator==(const BigInt& other) const{
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
bool BigInt::operator!=(const BigInt& other) const{
	return ! (*this == other);
}
//Menor que
bool BigInt::operator<(const BigInt& other) const{
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
		if(digits.size() > other.getDigits().size()){
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
bool BigInt::operator>(const BigInt& other) const{
	return other < *this;
}
//Menor o igual que
bool BigInt::operator<=(const BigInt& other) const{
	return ! (other < *this);
}
//Mayor o igual que
bool BigInt::operator>=(const BigInt& other) const{
	return ! (*this < other);
}

/////////-------Operaciones aritmeticas elementales-------/////////
//2.1. Cambio de signo
BigInt BigInt::operator-() const{
	BigInt bint = *this;
	bint.setSign(!sign);
	return bint;
}

BigInt BigInt::operator+(const BigInt& other) const{
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

BigInt BigInt::operator-(const BigInt& other) const{
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
		/*
		cout << "this = " << *this << endl;
		cout << "other = " << other << endl;
		cout << (*this > other) << endl;
		*/
		//Si this es mayor (en valor absoluto) que other
		if((*this >= other && getSign()) || (*this <= other && !getSign())){
			//Colocamos *this como minuendo y other como sustraendo
			//El signo resultante sera el mismo que tienen los dos operandos
			min = this;
			sus = &other;
			dif.setSign(getSign());
			//cout << "|this| > |other|" << endl;
		}
		else{
			//Colocamos other como minuendo y *this como sustraendo
			//El signo resultante sera el contrario al que tienen los dos operandos
			min = &other;
			sus = this;
			dif.setSign(!getSign());
			//cout << "|this| < |other|" << endl;
		}
		dif.getDigits().clear();
		//Realizamos la resta "digito a digito" siguiendo el algoritmo escolar
		//cout << "Min = " << *min << endl;
		//cout << "Sus = " << *sus << endl;
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
			/*
			cout << hex << "a = " << a << "\tb = " << b << "\tc = " << carry << endl;
			cout << hex <<  "res = " << res << "\tres_c = " << res_c << endl;
			cout << hex << "Carry " << i << " = " << carry << endl;
			*/
		}
	}
	dif.eraseZerosAtLeft();
	return dif;
}

//3. Multiplicacion escolar
BigInt BigInt::scholarMult(const BigInt& other) const{
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
			//cout << "Multiplicando (" << i << "," << j << ")" << endl;
			mult = multiply64(getDigits().at(i),other.getDigits().at(j),mult.high);
			bi_part.getDigits().push_back(mult.low);
		}
		bi_part.getDigits().push_back(mult.high);
		//cout << "bi_part: " << bi_part << endl;
		res = res + bi_part;
	}
	res.eraseZerosAtLeft();
	
	//Calculamos el signo (sera positivo sii coinciden los signos)
	bool signo = (getSign() && other.getSign()) || (!getSign() && !other.getSign());
	res.setSign(signo);
	
	//cout << "Producto: " << res << endl;
	
	return res;
}

BigInt BigInt::operator*(const BigInt& other) const{
	return this->scholarMult(other);
}

//4. Division
void BigInt::scholarDivision(BigInt d, BigInt& q, BigInt& r, bool debug) const{
	//Borramos los valores previos de q y r
	q.getDigits().clear();
	r.getDigits().clear();
	
	//Si el divisor es mayor en valor absoluto que el dividendo, devolvemos 
	//cociente 0 y resto el dividendo
	BigInt D_abs(*this), d_abs(d);
	D_abs.setSign(true);
	d_abs.setSign(true);
	if(D_abs < d_abs){
		if(debug)
			cout << "Dividendo menor que divisor: q = 0, r = D" << endl;
		q = "0x0";
		r = (*this);
	}
	else{
		//Si el divisor tiene un digito
		if(d.getDigits().size() == 1){
			//Guardamos el divisor en un uint64_t
			uint64_t divisor = d.getDigits().at(0);
			//Comprobamos division entre 0
			if(divisor == 0){
				if(debug)
					cout << "Error: division entre 0" << endl;
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
			BigInt dividend2_bi, divisor1_bi, divisor2_bi, remainder_bi;
			BigInt dividendj_norm, aux_bi;
			BigInt base_bi = "0x10000000000000000";
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
			divisor_norm.setSign(true);
			divisor_norm.eraseZerosAtLeft();
			dividend_norm.getDigits().push_back(0);	//Anadimos 0 al final
			
			
			divisor_norm = d * norm_bi;
			divisor_norm.setSign(true);
			divisor_norm.eraseZerosAtLeft();
			if(debug){
				cout << "Dividendo: " << hex <<  (*this) << endl;
				cout << "Divisor: " << hex <<  d << endl;
				cout << hex << "d_n = " << d_n << endl;
				cout << "Factor de normalizacion: " << hex <<  norm_bi << endl;
				cout << "Dividendo normalizado: " << hex <<  dividend_norm << endl;
				cout << "Divisor normalizado: " << hex << d * norm_bi << endl;
			}
			//Introducimos en el cociente los digitos en blanco necesarios
			for(int i = dividend_norm.getDigits().size() - 1; 
					i >= divisor_norm.getDigits().size() - 1; i--){
				q.getDigits().push_back(0);
			}
			//Obtenemos el segundo digito mas significativo del divisor
			//y los guardamos en un BigInt
			divisor1_bi.getDigits()[0] = divisor_norm.getDigits()[divisor_norm.getDigits().size()-1];
			divisor2_bi.getDigits()[0] = divisor_norm.getDigits()[divisor_norm.getDigits().size()-2];
			
			for(int i = dividend_norm.getDigits().size() - 1; 
					i > divisor_norm.getDigits().size() - 1; i--){
				if(debug)
					cout << "----------------- i = " << i << " -----------------" << endl;
				//Obtenemos el segundo digito mas significativo del dividendo actual
				//y los guardamos en un BigInt
				dividend2_bi.getDigits()[0] = dividend_norm.getDigits()[i-1];
				
				div_act.high = dividend_norm.getDigits()[i];
				div_act.low = dividend_norm.getDigits()[i-1];
				if(debug)
					cout << "Dividendo activo: " << div_act.high << " " << div_act.low << endl;
				
				//Obtenemos la aproximacion del cociente en q-hat
				//Si el primer digito de dividendo y divisor coinciden, se prueba con el 
				//valor mas grande
				if(divisor_norm.getDigits().at(divisor_norm.getDigits().size() - 1)
				   == dividend_norm.getDigits()[i]){
					q_hat = (uint64_t) -1;	//Valor sin signo mas alto posible en uint64_t
					divide128By64(div_act, q_hat, aux128, aux);		  //Obtenemos el resto
				}
				//Si no, se aproxima con la division de los primeros digitos
				else{
					divide128By64(div_act, divisor_norm.getDigits()[divisor_norm.getDigits().size()-1], aux128, aux);
					q_hat = aux128.low;
				}
				q_hat_bi.getDigits()[0] = q_hat;
				remainder_bi.getDigits()[0] = aux;
				if(debug){
					cout << "q-hat = " << q_hat_bi << endl;
					cout << "resto inicial = " << remainder_bi << endl;
				}
				
				if(debug){
					cout << "Segundo digito del dividendo: " << dividend2_bi << endl;
					cout << "Segundo digito del divisor: " << divisor2_bi << endl;
				}
				
				BigInt a1 = q_hat_bi * divisor2_bi;
				BigInt a2 = remainder_bi * base_bi + dividend2_bi;
				if(debug){
					cout << "a1 = " << a1 << endl;
					cout << "a2 = " << a2 << endl;
				}
				if(a1 > a2){
					q_hat_bi.getDigits()[0] = q_hat - 1;
					a1 = q_hat_bi * divisor2_bi;
					remainder_bi = remainder_bi + divisor1_bi;
					a2 = remainder_bi * base_bi + dividend2_bi;
					if(debug){
						cout << "Resta 1 a q-hat: " << q_hat_bi << endl;
						cout << "a1 = " << a1 << endl;
						cout << "a2 = " << a2 << endl;
					}
					if(a1 > a2){
						q_hat_bi.getDigits()[0] = q_hat - 2;
						if(debug){
							cout << "Resta 1 a q-hat: " << q_hat_bi << endl;
						}
					}
				}
				//Obtenemos la parte actual del dividendo
				dividendj_norm.getDigits().clear();
				for(int j = divisor_norm.getDigits().size(); j >= 0 ; j--){
					dividendj_norm.getDigits().push_back(dividend_norm.getDigits()[i-j]);
				}
				//Restamos a la parte afectada del dividendo el divisor por el cociente
				if(debug){
					cout << "Dividendo actual: " << dividendj_norm << endl;
					cout << "Restamos: " << q_hat_bi << "*" << divisor_norm << " = " 
						 << q_hat_bi*divisor_norm << endl;
				}
				if(debug){
					cout << "dividend_norm sin modificar: " << dividend_norm << endl;
					cout << "dividendj_norm: " << dividendj_norm << endl;
				}
				dividendj_norm = dividendj_norm - q_hat_bi*divisor_norm;
				
				while(dividendj_norm < zero){
					dividendj_norm = dividendj_norm + divisor_norm;
					q_hat_bi.getDigits()[0]--;
					if(debug){
						cout << "Resta 1 a q-hat en bucle: " << q_hat_bi << endl;
						cout << "Dividendo nuevo: " << dividendj_norm << endl;
					}
				}
				if(debug){
					cout << "Restando a dividendj_norm... " << endl;
					cout << "dividendj_norm: " << dividendj_norm << endl;
				}
				for(int j = 0; j < dividendj_norm.getDigits().size(); j++){
					int index = i-j-1;
					if(debug){
						cout << "dividend_norm.getDigits()[" << index << "] = " 
						<< "dividendj_norm.getDigits()[" 
						<< dividendj_norm.getDigits().size()-j-1 
						<< "] = " 
						<<  dividendj_norm.getDigits()[dividendj_norm.getDigits().size()-j-1]
						<< endl;
					}
					dividend_norm.getDigits()[index] = dividendj_norm.getDigits()[dividendj_norm.getDigits().size()-j-1];
					//cout << "acceso a = " << divisor_norm.getDigits().size()-j << endl;
					//cout << dividend_norm.getDigits()[i-j] << endl;
				}
				
				if(debug)
					cout << "Resto del dividendo: " << dividend_norm << endl;
				q.getDigits()[i-divisor_norm.getDigits().size()] = q_hat_bi.getDigits()[0];
			}
			dividendj_norm.scholarDivision(norm_bi, r, aux_bi);
			if(debug){
				cout << "---------Obtencion del resto---------" << endl;
				cout << "Dividendo: " << dividendj_norm << endl;
				cout << "Divisor: " << norm_bi << endl;
				cout << "Cociente (resto final): " << r << endl;
				cout << "Resto (debe ser 0): " << aux_bi << endl;
			}
		}
	}
	
	////Obtenemos el signo y resto finales
	//Si el dividendo es negativo, el resto que tenemos ahora es negativo
	
	
	if(!getSign()){
		BigInt one = "0x1";
		r.setSign(false);
		if(debug){
			cout << "q = " << q << endl;
			cout << "r = " << r << endl;
			cout << "d = " << d << endl;
		}
		//Hacemos el resto positivo sumando o restando el divisor
		if(d.getSign()){
			r = r + d;
			q = q + one;
		}
		else{
			r = r - d;
			q = q - one;
		}
		
	}
	//Si dividendo y divisor tienen el mismo signo
	if(getSign() == d.getSign()){
		q.setSign(true);	//El cociente sera positivo
	}
	//Si dividendo y divisor tienen distinto signo
	else{
		q.setSign(false);	//El cociente sera negativo
	}
	
	//Quitamos ceros a la izquierda
	q.eraseZerosAtLeft();
	r.eraseZerosAtLeft();
}

BigInt BigInt::operator/(const BigInt& other) const{
	BigInt q,r;
	scholarDivision(other, q, r);
	return q;
}

BigInt BigInt::operator%(const BigInt& other) const{
	BigInt q,r;
	scholarDivision(other, q, r);
	return r;
}

//5. Algoritmo de Karatsuba

BigInt BigInt::karatsubaMult(const BigInt& other, bool debug) const{
	BigInt m1(*this), m2(other);
	//m1.setSign(true);
	//m2.setSign(true);
	BigInt c;
	int tam_max = max(log2(m1.getDigits().size()), log2(m2.getDigits().size()));
	float tam = pow(2, tam_max);
	if(m1.getDigits().size() > tam || m2.getDigits().size() > tam){
		tam = tam * 2;
	}
	//Hacemos que ambas partes tengan el mismo numero de digitos
	if(debug)
		cout << "Tamanio de operandos = " << tam << endl;
	while(m1.getDigits().size() < tam){
		m1.getDigits().push_back(0);
		if(debug)
			cout << "Aniade 0 a la izquierda. Tamanio actual = " << m1.getDigits().size() << endl;
	}
	
	while(m2.getDigits().size() < tam){
		m2.getDigits().push_back(0);
		if(debug)
			cout << "Aniade 0 a la izquierda. Tamanio actual = " << m2.getDigits().size() << endl;
	}
	if(debug){
		cout << "Karatsuba tamanio " << getDigits().size() << endl;
		cout << "m1 = " << m1 << endl;
		cout << "m2 = " << m2 << endl;
	}
	//Si la longitud es de un digito, multiplicamos directamente
	if(m1.getDigits().size() == 1){
		UInt128 mult;
		mult = multiply64(m1.getDigits()[0], m2.getDigits()[0], 0);
		c.getDigits().clear();
		c.getDigits().push_back(mult.low);
		c.getDigits().push_back(mult.high);
		if(debug)
			cout << "Multiplicacion directa = " << c << endl;
	}
	else{
		BigInt a0, a1, b0, b1, c0, c1, c2, c3, aux0, aux1, aux2;
		//Dividimos a y b en dos partes
		a0.getDigits().clear();
		a1.getDigits().clear();
		b0.getDigits().clear();
		b1.getDigits().clear();
		for(int i = 0; i < tam/2; i++){
			a0.getDigits().push_back(m1.getDigits()[i]);
			b0.getDigits().push_back(m2.getDigits()[i]);
		}
		for(int i = tam/2; i < tam; i++){
			a1.getDigits().push_back(m1.getDigits()[i]);
			b1.getDigits().push_back(m2.getDigits()[i]);
		}
		
		//Realizamos las llamadas recursivas
		c0 = a0.karatsubaMult(b0);
		c1 = (a1 - a0).karatsubaMult(b0 - b1);
		c2 = a1.karatsubaMult(b1);
		
		//Realizamos la suma final
		//aux1 = (c0 + c1 + c2) con 2^m-1 ceros al final
		aux0 = c0 + c1 + c2;
		
		aux1.getDigits().clear();
		for(int i = 0; i < tam/2; i++)
			aux1.getDigits().push_back(0);
		for(int i = 0; i < aux0.getDigits().size(); i++)
			aux1.getDigits().push_back(aux0.getDigits()[i]);
		
		aux2.getDigits().clear();
		for(int i = 0; i < tam; i++)
			aux2.getDigits().push_back(0);
		for(int i = 0; i < c2.getDigits().size(); i++)
			aux2.getDigits().push_back(c2.getDigits()[i]);
		if(debug){
			cout << "Suma final (nivel " << tam << ")" << endl;
			cout << "c0 = " << c0 << endl;
			cout << "aux1 = " << aux1 << endl;
			cout << "aux2 = " << aux2 << endl;
		}
		c = c0 + aux1 + aux2;
		bool signo = (m1.getSign() && m2.getSign()) || (!m1.getSign() && !m2.getSign());
		c.setSign(signo);
		if(debug)
			cout << "Suma final = " << c << endl;
	}
	c.eraseZerosAtLeft();
	return c;
}

//6. Algoritmo Extendido de Euclides
void BigInt::EEA(const BigInt& b_ini, BigInt& mcd, BigInt& mcm, BigInt& u0, BigInt& v0) const{
	BigInt u1("0x0"), v1("0x1");
	BigInt b = b_ini;
	BigInt q, r;
	BigInt u, v, zero;
	
	mcd = *this;
	u0 = "0x1";
	v0 = "0x0";
	
	while(b != zero){
		mcd.scholarDivision(b, q, r);
		mcd = b;
		b = r;
		u = u0 - q*u1;
		v = v0 - q*v1;
		u0 = u1;
		v0 = v1;
		u1 = u;
		v1 = v;
	}
	
	mcm = (*this) * u1;
	mcm.setSign(true);
}

//7. Exponenciacion rapida

BigInt BigInt::quickModExp(const BigInt& exp, const BigInt& mod) const{
	BigInt i = "0x0", exp_act = exp, zero = "0x0", one = "0x1", two = "0x2", p;
	bool first_digit_read = false;
	int bin_digit;
	//Comprobaciones iniciales
	assert(*this > zero && exp >= zero);
	
	//Inicializamos p = 1
	p = "0x1";
	
	//Por cada digito que tengamos en el exponente
	for(int i = exp.getDigits().size() - 1; i >= 0; i--){
		bitset<64> digit(exp.getDigits()[i]);
		//cout << "Num actual: " << bitset<64>(digit) << endl;
		//Por cada digito binario en el digito
		for(int j = 63; j >= 0; j--){
			bin_digit = digit[j];
			//cout << "Bit actual: " << bin_digit << endl;
			//Si el digito es 1
			if(bin_digit == 1){
				p = (p * p * (*this)) % mod;
			}
			//Si el digito es 0
			else{
				p = (p * p) % mod;
			}
			//cout << "p = " << p << endl;
		}
		//cout << endl;
	}
	return p;
}

//8. Test de Miller-Rabin

bool BigInt::strongPseudoprime(const BigInt& base, bool debug) const{
	bool is_pseudoprime = false;
	BigInt mcd, mcm, u0, v0;
	BigInt q, r;
	BigInt b;
	BigInt zero = "0x0", one = "0x1", two = "0x2";
	//Comprobamos que sean coprimos y que el primo que comprobamos sea mayor que 1
	this->EEA(base, mcd, mcm, u0, v0);
	if(debug)
		cout << "mcd de a y la base = " << mcd << endl;
	if(mcd == one && one < (*this)){
		//Calculamos s,t tales que n-1 = 2^s*t con t impar
		BigInt t = (*this) - one, s = zero;
		t.scholarDivision(two, q, r);
		while (r == zero){
			t = q;
			s = s + one;
			t.scholarDivision(BigInt(two), q, r);
		}
		if(debug){
			cout << "s = " << s << endl;
			cout << "t = " << t << endl;
			cout << "2^s*t = " << two.quickModExp(s,*this)*t << endl;	//el modulo es mayor que 2^s
		}
		
		b = base.quickModExp(t, *this);
		
		if(b == one || b == (*this) - one){
			is_pseudoprime = true;
		}
		for(BigInt r = one; r < s - one && !is_pseudoprime; r = r + one){
			b = (b*b) % (*this);
			if(b == (*this) - one){
				is_pseudoprime = true;
			}
		}
	}
	return is_pseudoprime;
}

bool BigInt::millerRabinTest(int k, bool debug){
	bool no_fail = true;
	BigInt rn;
	for(int i = 0; i < k && no_fail; i++){
		rn = randomBigInt(*this);
		if(debug){
			cout << "-------------i = " << i << "-------------" << endl;
			cout << "Numero aleatorio generado = " << rn << endl;
		}
		if(strongPseudoprime(rn, debug) == false){
			no_fail = false;
		}
	}
	return no_fail;
}

//Extra

bool BigInt::modularInverse(const BigInt& mod, BigInt& inverse, bool debug) const{
	bool exists_inverse = false;
	BigInt aux, rem;
	BigInt mcd, mcm, u, v;
	BigInt one("0x1");
	
	//Comprobamos que el modulo sea valido
	assert(mod > one);
	
	//Guardamos en rem el valor de *this en modulo mod
	this->scholarDivision(mod, aux, rem);
	//Aplicamos el algoritmo extendido de Euclides
	mod.EEA(rem, mcd, mcm, u, v);
	if(debug)
		cout << "mcd = " << mcd << endl;
	//Si el mcd es 1, entonces calculamos el inverso multiplicativo
	//Si no es 1, entonces no existe el inverso multiplicativo y devolvemos -1
	if(mcd == one){
		v.scholarDivision(mod, aux, inverse);
		exists_inverse = true;
	}
	
	return exists_inverse;
}



bool BigInt::random_initialized = false; // Inicialización del atributo estático fuera de la clase
std::mt19937_64 BigInt::generator; // Inicialización del atributo estático


BigInt creaBigInt(const string& s){
	BigInt b(s);
	return b;
}










