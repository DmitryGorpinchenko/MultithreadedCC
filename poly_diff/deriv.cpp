#include <iostream>
#include <map>
#include <ctype.h>

struct rcomp {
	bool operator() (int a, int b) { 
		return a > b; 
	}
};

using Poly = std::map<int, int, rcomp>;

void ParsePoly(Poly* poly, const std::string& in);
void PolyDeriv(Poly* deriv, const Poly& poly);
void DisplayPoly(const Poly& poly);

std::string ToString(const Poly& poly);

/*int main(int argc, char **argv) 
{
    Poly poly, deriv;
    ParsePoly(&poly, argv[1]);
    PolyDeriv(&deriv, poly);
    std::cout << ToString(deriv) << std::endl;
    return 0;
}*/

void PolyDeriv(Poly* deriv, const Poly& poly) 
{
	for (auto& term : poly) {
		if (term.first <= 0) {
	    		continue;
		}
		(*deriv)[term.first - 1] = term.first * term.second;
	}
}

void ParsePoly(Poly* poly, const std::string& in) 
{
	int i = 0, k, p, sign;
	auto poly_str = in[0] == '-' ? in : "+" + in;
	while (i < poly_str.size()) {
		k = 0, p = 0;
		sign = poly_str[i++] == '-' ? -1 : 1;
		while (isdigit(poly_str[i])) {
			k = 10 * k + (poly_str[i++] - '0');
		}
		k = sign * (k == 0 ? 1 : k);
		if (poly_str[i] == '*') {
			++i;
		}
		if (poly_str[i] == 'x') {
			++i;
			if (poly_str[i] == '^') {
				++i;
				while (isdigit(poly_str[i])) {
					p = 10 * p + (poly_str[i++] - '0');
				}
			}
			p = p == 0 ? 1 : p;
		}
		(*poly)[p] += k;
	}
}

std::string ToString(const Poly& poly) 
{
	std::string res;
	for (auto& term : poly) {
		res += term.second > 0 ? "+" : "-";
		if (term.first == 0 || std::abs(term.second) > 1) {
			res += std::to_string(std::abs(term.second));
			if (term.first > 0) {
				res += "*";
			}
		}
		if (term.first > 0) {
			res += "x";
			if (term.first > 1) {
				res += "^" + std::to_string(term.first);
			}
		}        
	}
	return res[0] == '+' ? res.substr(1) : res;
}

std::string derivative(std::string polynomial) 
{
    Poly poly, deriv;
    ParsePoly(&poly, polynomial);
    PolyDeriv(&deriv, poly);
    return ToString(deriv);
}
