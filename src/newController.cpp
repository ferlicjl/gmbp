/*
 * =====================================================================================
 *
 *       Filename:  newController.cpp
 *
 *    Description:  Contains simulating and input file parsing functions
 *
 *        Version:  1.0
 *        Created:  10/1/2017 14:57:27
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  Jeremy Ferlic (s), jferlic@g.harvard.edu
 *   Organization:  Harvard University
 *
 * =====================================================================================
 */

// Random Distributions and Helper functions
#include "Node.h"
#include "NodeList.h"
#include "State.h"
#include "Population.h"

// Includes
#include <iostream>
#include <fstream>
#include <deque>
#include <vector>
#include <string>
#include <algorithm>
#include <map>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <sstream>
#include <iterator>
#include <chrono>
#include <cstring>
#include <cmath>
#include <limits>
#include <gsl/gsl_randist.h>

// Rcpp headers
#include <RcppGSL.h>
#include <Rcpp.h>
#include <Rinternals.h>

// GSL random number generators
gsl_rng* rng = gsl_rng_alloc(gsl_rng_mt19937);
double seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();


// Helper Methods to read in data from text files
template
< typename T
  , template<typename ELEM, typename ALLOC=std::allocator<ELEM> > class Container
  >
std::ostream& operator<< (std::ostream& o, const Container<T>& container)
{
    typename Container<T>::const_iterator beg = container.begin();

    o << "["; // 1

    while(beg != container.end())
    {
        o << " " << *beg++; // 2
    }

    o << " ]"; // 3

    return o;
}

// trim from left
static inline std::string &ltrim(std::string &s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
                                    std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
}

// trim from end
static inline std::string &rtrim(std::string &s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(),
                         std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
}

// trim from both ends
static inline std::string &trim(std::string &s)
{
    return ltrim(rtrim(s));
}


// Read in a vector matrix
std::vector<std::vector<double> > fileToVectorMatrix(std::string name)
{
    std::vector<std::vector<double> > result;
    std::ifstream input (name);
    std::string lineData;

    while(getline(input, lineData))
    {
        double d;
        std::vector<double> row;
        std::stringstream lineStream(lineData);

        while (lineStream >> d)
            row.push_back(d);

        result.push_back(row);
    }

    return result;
}

template <typename T>
std::vector<T> flatten(const std::vector<std::vector<T> >& v)
{
    std::size_t total_size = 0;
    for (const auto& sub : v)
        total_size += sub.size(); // I wish there was a transform_accumulate
    std::vector<T> result;
    result.reserve(total_size);
    for (const auto& sub : v)
        result.insert(result.end(), sub.begin(), sub.end());
    return result;
}

// Read in a vector
std::vector<double> fileToVector(std::string name)
{
    std::vector<std::vector<double> > result;
    std::ifstream input (name);
    std::string lineData;

    while(getline(input, lineData))
    {
        double d;
        std::vector<double> row;
        std::stringstream lineStream(lineData);

        while (lineStream >> d)
            row.push_back(d);

        result.push_back(row);
    }

    return flatten(result);
}

// Read in a text file, output a vector of strings, one string per line, trimmed
std::vector<std::string> inputStringVector(std::string in)
{
    std::string s;
    std::vector<std::string> a;
    std::ifstream infile(in);
    if(infile.is_open())
    {
        while(getline(infile, s))
        {
            // allow comments
            std::string::iterator end_pos = std::remove(s.begin(), s.end(), ' ');
            s.erase(end_pos, s.end());
            a.push_back(trim(s));
        }
    }

    return(a);
}

std::vector<std::vector<std::string>> funct(std::string filename){
	std::ifstream csv(filename);
	std::string line;
	std::vector <std::vector<std::string>> items;

	if (csv.is_open()) {
			for (std::string row_line; std::getline(csv, row_line);)
			{
				items.emplace_back();
				std::istringstream row_stream(row_line);
				for(std::string column; std::getline(row_stream, column, ',');)
					items.back().push_back(column);
			}
	}
	else {
		std::cout << "Unable to open file";
	}

	return items;
}

//' rcpptest
//'
//' rcpptest
//'
//' @export
// [[Rcpp::export]]
double rcpptest(Rcpp::NumericVector x){
	int n = x.size();
	double total = 0;

	for(int i = 0; i < n; ++i) {
		total += x[i];
	}

	std::vector<double> result (x.begin(), x.end());

	for(size_t i = 0; i < result.size(); i ++){
		std::cout << result[i] << std::endl;
	}

	return total / n;
}

//' rcpptest2
//'
//' rcpptest2
//'
//' @export
// [[Rcpp::export]]
double rcpptest2(Rcpp::NumericMatrix x){
	int nrow = x.nrow();
	int ncol = x.ncol();


	for(int i = 0; i < nrow; ++i) {
		Rcpp::NumericVector v = x.row(i);
		std::cout << "Probability " << v[0] << ": {" << v[1];
		for(int j = 2; j < v.length(); j++){
			std::cout << ", " << v[j];
		}
		std::cout << "}" << std::endl;
	}

	return 0;
}

//' gmbp
//'
//' gmbp
//'
//' @export
// [[Rcpp::export]]
double gmbp(int time, std::string file, Rcpp::NumericVector initial, Rcpp::NumericVector lifetimes, Rcpp::NumericMatrix transitions){
	int nrow = transitions.nrow();
	int ncol = transitions.ncol();

	std::vector<int> init(initial.begin(), initial.end());

	State sys(init);

	std::cout << "Making populations..." << std::endl;
	for(size_t i = 0; i < init.size(); i++){
		std::cout << "init[" << i << "]: " << init[i] << std::endl;
		Population* p = new Population(lifetimes[i]);
		sys.addPopulation(p);
	}

	std::cout << "Adding transitions..." << std::endl;
    for(int i = 0; i < nrow; i++){
		Rcpp::NumericVector ro = transitions.row(i);
		std::vector<double> r (ro.begin(), ro.end());
		int population = r[0];
		double probability = r[1];

		std::vector<double>::const_iterator first = r.begin() + 2;
		std::vector<double>::const_iterator last = r.end();
		std::vector<int> transition(first, last);

		sys.getPop(population)->addUpdate(probability, transition);
	}

	sys.print();

	std::cout << "Simulating..." << std::endl;
	sys.simulate(time, file);

	/*
	for(int i = 0; i < nrow; ++i) {
		Rcpp::NumericVector v = x.row(i);
		std::cout << "Probability " << v[0] << ": {" << v[1];
		for(int j = 2; j < v.length(); j++){
			std::cout << ", " << v[j];
		}
		std::cout << "}" << std::endl;
	}
	*/

	return 0;
}


//' test
//'
//' test
//'
//' @export
// [[Rcpp::export]]
double test(int n) {
	std::vector<int> s = {1, 1};
	//std::vector<int> u = {1, 4, 2, -1};
	State sys(s);
	//sys.print();
	//sys.updateState(u);
	//sys.print();

	Population p(0.05);
	Population q(0.05);
	//Population t(0.05);
	//Population v(0.05);

	sys.addPopulation(&p);
	sys.addPopulation(&q);
	//sys.addPopulation(t);
	//sys.addPopulation(v);

	p.addUpdate(0.5, {1, 0});
	p.addUpdate(0.5, {0, 1});
	p.addUpdate(1, {1, 1});
	p.addUpdate(1.0, {-1, 0});
	//p.printUpdates();

	//std::cout << "Q: " << std::endl;

	q.addUpdate(0.5, {1, 0});
	q.addUpdate(0.5, {0, 1});
	q.addUpdate(1, {1, 1});
	q.addUpdate(1.9, {0, -1});
	//q.printUpdates();

	sys.simulate(n, "system.csv");

	return sys.choosePop();
}


