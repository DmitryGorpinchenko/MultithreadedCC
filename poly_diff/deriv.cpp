#include <map>

struct rcomp { bool operator()(int a, int b) { return a > b; } };

using Poly = std::map<int, int, rcomp>;

Poly ParsePoly(const std::string& in);
Poly PolyDeriv(const Poly& poly);
std::string ToString(const Poly& poly);

Poly PolyDeriv(const Poly& poly)
{
    Poly deriv;
    for (auto& term : poly) {
        if (term.first > 0) {
            deriv[term.first - 1] = term.first * term.second;
        }
    }
    return deriv;
}

Poly ParsePoly(const std::string& in)
{
    Poly poly;
    auto poly_str = in[0] == '-' ? in : "+" + in;
    for (int i = 0; i < poly_str.size(); ) {
        int k = 0, p, sign = poly_str[i] == '-' ? -1 : 1;
        while (isdigit(poly_str[++i])) {
            k = 10 * k + (poly_str[i] - '0');
        }
        if ((p = (poly_str[i += (poly_str[i] == '*')] == 'x')) && poly_str[++i] == '^') {
            p = 0;
            while (isdigit(poly_str[++i])) {
                p = 10 * p + (poly_str[i] - '0');
            }
        }
        poly[p] += sign * (k ? k : 1);
    }
    return poly;
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

std::string derivative(std::string polynomial) {
    return ToString(PolyDeriv(ParsePoly(polynomial)));
}

