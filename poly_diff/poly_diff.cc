#include <iostream>
#include <map>
#include <sstream>
#include <cmath>

using namespace std;

string derivative(const string& input);
void preprocess(const string& input, string* poly);
void constructPolyMap(const string& poly, map<int, int>* poly_map);

string derivative(const string& input) 
{
	string poly;
	preprocess(input, &poly);
	map<int, int> poly_map;
	constructPolyMap(poly, &poly_map);
	stringstream deriv;
	for(auto i = poly_map.rbegin(); i != poly_map.rend(); ++i) 
	{
		int k = i->second * i->first;
		int p = (i->first > 1) ? (i->first - 1) : 0;
		if(k == 0 && poly_map.size() == 1) 
		{
			deriv << k;
		} 
		else if(k != 0) 
		{
			deriv << (k > 0 ? "+" : "");
			if(p == 0) 
			{
				deriv << k;
			} 
			else 
			{
				deriv << (abs(k) != 1 ? to_string(k) + "*" : (k == -1 ? "-" : "")) + "x" + (p == 1 ? "" : "^" + to_string(p));
			}
		}
	}
	string res = deriv.str();
	return res[0] == '+' ? res.substr(1, res.size()) : res;
}

void constructPolyMap(const string& poly, map<int, int>* poly_map) 
{
	stringstream ss(poly);
	int k, p;
	char ch;
	string line;
	while(getline(ss, line)) 
	{
		stringstream sss(line);
	 	if(sss >> k) 
	 	{
	 		if(sss.peek() == '*' && sss >> ch >> ch) 
	 		{
	 			if(!(sss.peek() == '^' && sss >> ch >> p)) 
	 			{
	 				p = 1;
	 			}
	 		} 
	 		else 
	 		{
	 			p = 0;
	 		}
	 	} 
	 	else 
	 	{
	 		sss.clear();
 			k = 1;
	 		sss >> ch;
	 		if(!(sss.peek() == '^' && sss >> ch >> p)) 
	 		{
	 			p = 1;
	 		}
	 	}
		(*poly_map)[p] += k;
	}
}

void preprocess(const string& input, string* poly) 
{
	for(int i = 0; i < input.size(); ++i) 
	{
		if(input[i] == '+') 
		{
			poly->push_back('\n');
		} 
		else if(input[i] == '-') 
		{
			if(i > 0) 
			{
				poly->push_back('\n');
			}
			poly->push_back('-');
			if(input[i + 1] == 'x') 
			{
				poly->push_back('1');
				poly->push_back('*');
			}
		} 
		else 
		{
			poly->push_back(input[i]);
		}
	}
}
