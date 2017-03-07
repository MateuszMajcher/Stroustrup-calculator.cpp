#include <iostream>
#include <map>
#include <cctype>
#include <type_traits>
int no_of_errors;

template<typename T>
std::ostream& operator<<(typename std::enable_if<std::is_enum<T>::value, std::ostream>::type& stream, const T& e)
{
    return stream << static_cast<typename std::underlying_type<T>::type>(e);
}

double error(const char* s) {
	no_of_errors++;
	std::cerr<<"error " <<s<<"\n";
	return 1;
}

std::map<std::string, double> table;
double expr(bool);
double term(bool);
double prim(bool);

enum class Kind : char {
	name, number, end, 
	plus='+',minus='.', mul='*', div='/', print=';', assign='=', lp='(', rp=')'
};


struct Token {
	Kind kind;
	std::string string_value;
	double number_value;
};

class Token_stream {
	public:
		Token_stream(std::istream& s): ip(&s), owns(false) {}
		Token_stream(std::istream* p): ip(p), owns(true) {}
		
		~Token_stream() {
			close();
		}
		
		void set_input(std::istream& s) {
			close();
			ip = &s;
			owns = false; 
		}
		
		void set_input(std::istream* p) {
			close();
			ip = p;
			owns = true; 
		}
		
		Token get();  //read and return next token
		const Token& current(); 
		
		private:
			void close() {
				if (owns)
					delete ip;
			}
			
			std::istream* ip;
			bool owns;
			Token ct{Kind::end};
};

Token Token_stream::get() {
	char ch = 0;

	do {
		if (!ip->get(ch)) {
	 			return ct = {Kind::end};
			}	
	} while (ch != '\n' && isspace(ch));

std::cout<<"--"<<ch<<std::endl;

	switch(ch) {
		case ';':
		case '\n': 
			return ct = {Kind::print};
		case '(':
		case ')':
		case '+':
		case '-':
		case '*':
		case '/':
		case '%':
		case '=':
			return ct = {static_cast<Kind>(ch)};
		case '.':
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			ip->putback(ch); //wstawienie liczby do strumienia wejsciowego
			*ip >> ct.number_value;//wczytanie liczby do ct;
			std::cout<<"Wsatwiona liczba to "<<ct.number_value<<std::endl;
			ct.kind = Kind::number;
			return ct;
		default:
			if (isalpha(ch)) {  //jesli nazwa dodaanie
				ct.string_value = ch;
				while (ip->get(ch) && isalnum(ch))
					ct.string_value += ch;
				ip->putback(ch);
				std::cout<<"Wsatwiona nazwa to "<<ct.string_value<<std::endl;
				return ct={Kind::name};
			}
			
			error("bad token");
			return ct = {Kind::print};
	}
}

const Token& Token_stream::current() {
	return ct;
} 

Token_stream ts{std::cin};

void calculate() {
		
	for (;;) {
		ts.get();
		std::cout<<ts.current().kind<<std::endl;
	
		if (ts.current().kind == Kind::end) break;
		if (ts.current().kind == Kind::print) continue;
		
	   std::cout<<expr(false)<<'\n';
		
	}
}


double expr(bool get) {
	double left = term(get);
	
	for (;;) {
		switch (ts.current().kind) {
			case Kind::plus:
				left += term(true);
				break;
			case Kind::minus:
				left-=term(true);
				break;
			default:
				return left;
		}
	}
}

double term(bool get) {
	double left = prim(get);
	
	for (;;) {
		switch (ts.current().kind) {
			case Kind::mul:
				left *= prim(true);
				break;
			case Kind::div:
				if (auto d = prim(true)) {
					left /= d;
					break;
				}
				return error("divide by zero");
			default:
				return left;
		}
	}
}

double prim(bool get) {
	if (get) ts.get();
	
	switch (ts.current().kind) {
		case Kind::number:
			{
				double v = ts.current().number_value;
				ts.get();
				return v;
			}
		case Kind::name:
			{
				double& v = table[ts.current().string_value];
				if (ts.get().kind == Kind::assign) v = expr(true);
				return v;
			}
		case Kind::minus:
			return -prim(true);
		case Kind::lp:
			{
				auto e = expr(true);
				if (ts.current().kind != Kind::rp) return error("')' expected");
				ts.get(); // eat ’)’
				return e;
			}
		default:
			return error("primary expected");
	}
}

int main() {
	table["pi"] = 3.1415926535897932385; // inser t predefined names
	table["e"] = 2.7182818284590452354;
	
	calculate();
	std::cout<<"koniec";
	return no_of_errors;
}
