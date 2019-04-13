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

// For plugin system
#ifndef USE_PRECOMPILED_HEADERS
#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#else
#include <dlfcn.h>
#endif
#endif

// Classes
#include "System.h"
#include "Update.h"
#include "StopCriterion.h"
#include "Rate.h"
#include "ConstantRate.h"

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

bool silent = false;

gsl_integration_workspace *workspace;


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

//' listtest
//'
//' listtest
//'
//' @export
// [[Rcpp::export]]
Rcpp::NumericVector listtest(Rcpp::List l, int i){
	Rcpp::NumericVector fixed = Rcpp::as<Rcpp::NumericVector>(l["fixed"]);
	Rcpp::NumericVector random = Rcpp::as<Rcpp::NumericVector>(l["random"]);

	Rcpp::NumericVector s = Rcpp::as<Rcpp::NumericVector>(l[i]);

	std::vector<double> result (s.begin(), s.end());

	for(size_t i = 0; i < result.size(); i ++){
		std::cout << result[i] << std::endl;
	}

	if(l.containsElementNamed("test"))
	std::cout << "List contained an element named test" << std::endl;

	return fixed;
}

//' list2test
//'
//' list2test
//'
//' @export
// [[Rcpp::export]]
double list2test(Rcpp::List l){

	int n = l.size();

	for(int i = 0; i < n; i++){
		Rcpp::List list = Rcpp::as<Rcpp::List>(l[i]);
		Rcpp::NumericVector s = Rcpp::as<Rcpp::NumericVector>(list[0]);

		bool a = list[1];
		std::cout << "is_random: " << a << std::endl;

		std::vector<double> result (s.begin(), s.end());
		for(size_t i = 0; i < result.size(); i ++){
			std::cout << result[i] << std::endl;
		}
		std::cout << std::endl;
	}

	return 0;
}

/*
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

System sys(init);

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

sys.getPop(population)->addUpdate(probability, Update(transition));
}

sys.print();

std::cout << "Simulating..." << std::endl;
sys.simulate(time, file);

return 0;
}
*/



//' gmbp2
//'
//' gmbp2
//'
//' @export
// [[Rcpp::export]]
double gmbp2(int time, std::string file, Rcpp::NumericVector initial, Rcpp::NumericVector lifetimes, Rcpp::List transitions){
	std::cout << "Starting process... " << std::endl;
	int n = transitions.length();

	std::cout << "Initialization system..." << std::endl;
	// Initial population sizes
	std::vector<int> init(initial.begin(), initial.end());

	// Initialize system
	System sys(init);

	/*
	// Create populations with lifetimes
	std::cout << "Making populations..." << std::endl;
	for(size_t i = 0; i < init.size(); i++){
	std::cout << "init[" << i << "]: " << init[i] << std::endl;
	Population* p = new Population(lifetimes[i]);
	sys.addPopulation(p);
	}
	*/

	// Add transitions
	std::cout << "Adding transitions..." << std::endl;

	// Iterate over transitions list
	for(int i = 0; i < n; i++){

		// Get the ith transition
		Rcpp::List list_i = Rcpp::as<Rcpp::List>(transitions[i]);

		// Get popuation, is_random, probability
		int population = list_i[0];
		bool is_random = list_i[1];
		double rate = list_i[2];

		// If there is a random component, get which indices of the vector will be generated randomly
		if(is_random){
			// Get the offspring distribution of the Transition
			Rcpp::NumericVector oVec = Rcpp::as<Rcpp::NumericVector>(list_i[3]);
			std::vector<double> offspringVec (oVec.begin(), oVec.end());

			// Get the distribution
			std::string dist = list_i[4];

			// Get the offspring distribution of the Transition
			Rcpp::NumericVector p = Rcpp::as<Rcpp::NumericVector>(list_i[5]);
			std::vector<double> offspringParams (p.begin(), p.end());

			// Add this transition
			sys.addUpdate(rate, population, Update(is_random, offspringVec, dist, offspringParams));
			std::cout << "Added a random transtion" << std::endl;
		} else{
			// Get the fixed portion of the Transition
			Rcpp::NumericVector fix = Rcpp::as<Rcpp::NumericVector>(list_i[3]);
			std::vector<int> fixed (fix.begin(), fix.end());

			// add update
			sys.addUpdate(rate, population, Update(fixed));
		}
	}

	//sys.print();

	// Simulate
	std::cout << "Simulating..." << std::endl;
	sys.simulate(time, file);
	std::cout << "Ending process..." << std::endl;

	return 0.0;
}


//' gmbp3
//'
//' gmbp3
//'
//' @export
// [[Rcpp::export]]
double gmbp3(int time, std::string file, Rcpp::NumericVector initial, Rcpp::List transitions, Rcpp::List stops, bool silence, SEXP seed = R_NilValue){
	double seedcpp;
	if(Rf_isNull(seed)){
		seedcpp = std::chrono::high_resolution_clock::now().time_since_epoch().count();
	} else {
		seedcpp = Rf_asReal(seed);
	}
    gsl_rng_set(rng, seedcpp);
	silent = silence;

	if(!silent)
	std::cout << "Starting process... " << std::endl;
	int nTrans = transitions.length();
	int nStops = stops.length();

	if(!silent)
	std::cout << "Initialization system..." << std::endl;
	// Initial population sizes
	std::vector<int> init(initial.begin(), initial.end());

	// Initialize system
	System sys(init);

	/*
	// Create populations with lifetimes
	std::cout << "Making populations..." << std::endl;
	for(size_t i = 0; i < init.size(); i++){
	std::cout << "init[" << i << "]: " << init[i] << std::endl;
	Population* p = new Population(lifetimes[i]);
	sys.addPopulation(p);
	}
	*/

	// Add transitions
	if(!silent)
	std::cout << "Adding transitions..." << std::endl;

	// Iterate over transitions list
	for(int i = 0; i < nTrans; i++){

		// Get the ith transition
		Rcpp::List list_i = Rcpp::as<Rcpp::List>(transitions[i]);

		// Get popuation, is_random, probability
		int population = list_i[0];
		bool is_random = list_i[1];
		double rate = list_i[2];

		// If there is a random component, get which indices of the vector will be generated randomly
		if(is_random){
			// Get the offspring distribution of the Transition
			Rcpp::NumericVector oVec = Rcpp::as<Rcpp::NumericVector>(list_i[3]);
			std::vector<double> offspringVec (oVec.begin(), oVec.end());

			// Get the distribution
			std::string dist = list_i[4];

			// Get the offspring distribution of the Transition
			Rcpp::NumericVector p = Rcpp::as<Rcpp::NumericVector>(list_i[5]);
			std::vector<double> offspringParams (p.begin(), p.end());

			// Add this transition
			sys.addUpdate(rate, population, Update(is_random, offspringVec, dist, offspringParams));
		} else{
			// Get the fixed portion of the Transition
			Rcpp::NumericVector fix = Rcpp::as<Rcpp::NumericVector>(list_i[3]);
			std::vector<int> fixed (fix.begin(), fix.end());

			// add update
			sys.addUpdate(rate, population, Update(fixed));
		}
	}

	// Add transitions
	if(!silent)
	std::cout << "Adding stopping criteria..." << std::endl;

	// Iterate over transitions list
	for(int i = 0; i < nStops; i++){

		// Get the ith transition
		Rcpp::List list_i = Rcpp::as<Rcpp::List>(stops[i]);

		// Get popuation, is_random, probability

		// Get the offspring distribution of the Transition
		Rcpp::NumericVector ind = Rcpp::as<Rcpp::NumericVector>(list_i[0]);
		std::vector<int> indices (ind.begin(), ind.end());

		std::string ineq = list_i[1];
		double value = list_i[2];

		sys.addStop(StopCriterion(indices, ineq, value));
	}

	//sys.print();

	// Simulate
	if(!silent)
	std::cout << "Simulating..." << std::endl;
	sys.simulate(time, file);
	if(!silent)
	std::cout << "Ending process..." << std::endl;

	return 0.0;
}

//' test
//'
//' test
//'
//' @export
// [[Rcpp::export]]
double test(double a) {
	ConstantRate h(1.234234);
	std::cout << h(1.344) << std::endl;

	LinearRate l(10, 2);
	std::cout << l(4) << std::endl;
	std::cout << l(0, 4) << std::endl;
	//return sys.choosePop();
	return 0;
}

// Bojan Nikolic <bojan@bnikolic.co.uk
// Simple Gauss-Konrod integration with GSL

#include <cmath>
#include <iostream>

//#include <boost/shared_ptr.hpp>
//#include <boost/format.hpp>

#include <gsl/gsl_integration.h>

struct f_params {
	// Amplitude
	double a;
	// Phase
	double phi;
};

double f(double x, void *p)
{
	f_params &params= *reinterpret_cast<f_params *>(p);
	return (pow(x, params.a)+params.phi);
}

double g(double x, void *p)
{
	f_params &params= *reinterpret_cast<f_params *>(p);
	if(x >= 5)
	return 10;
	else
	return 5;
	//return (pow(x, params.a)+params.phi);
}


void dointeg(void)
{
	f_params params;
	params.a=34.0;
	params.phi=0.0;

	gsl_function F;
	F.function = &f;
	F.params = reinterpret_cast<void *>(&params);

	double result, error;
	size_t neval;

	const double xlow=0;
	const double xhigh=10;
	const double epsabs=1e-4;
	const double epsrel=1e-4;

	int code=gsl_integration_qng (&F,
	xlow,
	xhigh,
	epsabs,
	epsrel,
	&result,
	&error,
	&neval);
	if(code)
	{
		std::cerr<<"There was a problem with integration: code " << code
		<<std::endl;
	}
	else
	{
		std::cout<<"Result " << result << " +/- " << error << " from " << neval << " evaluations" <<
		std::endl;
	}
}

void doqags(void)
{
	gsl_integration_workspace *workspace = gsl_integration_workspace_alloc(1000);

	f_params params;
	params.a=34.0;
	params.phi=0.0;

	gsl_function F;
	F.function = &g;
	F.params = reinterpret_cast<void *>(&params);

	double result, error;
	size_t neval;

	const double xlow=0;
	const double xhigh=10;
	const double epsabs=1e-4;
	const double epsrel=1e-4;

	int code=gsl_integration_qags (&F,
	xlow,
	xhigh,
	epsabs,
	epsrel,
	1000,
	workspace,
	&result,
	&error);
	gsl_integration_workspace_free(workspace);

	if(code)
	{
		std::cerr<<"There was a problem with integration: code " << code
		<<std::endl;
	}
	else
	{
		std::cout<<"Result " << result << " +/- " << error << " from " << neval << " evaluations" <<
		std::endl;
	}
}

//' t2
//'
//' t2
//'
//' @export
// [[Rcpp::export]]
std::vector<double> t2(SEXP custom_distribution_file = R_NilValue){
	/*for(int i = 0; i < 1e6; i++){
	doqags();
	dointeg();
	}
	*/

	workspace = gsl_integration_workspace_alloc(1000);
	double (*rate)(double, void*);

#ifdef _WIN32
	HINSTANCE lib_handle;
	HINSTANCE lib_handle_newclone;
#else
	void *lib_handle;
	void *lib_handle_newclone;
#endif

	// Name for plugin library location
	const char* plugin_location = CHAR(Rf_asChar(custom_distribution_file));

	Rate r;

	// Load plugin library
#ifdef _WIN32
	lib_handle = LoadLibrary(plugin_location);
	if(!lib_handle)
	{
		Rcpp::stop("invalid file name for distribution file");
	}
	rate = (double (*)(double, void*))GetProcAddress(lib_handle, "birth0");

	void* ptr;
	std::cout << (*rate)(1.1, ptr) << std::endl;

	r = Rate(rate);

#else
	lib_handle = dlopen(plugin_location, RTLD_NOW);
	if(!lib_handle)
	{
		Rcpp::stop("invalid file name for distribution file");
	}
	rate = (double (*)(double, void*))dlsym(lib_handle, "CA");
	r = Rate(rate);
#endif

	//Rate r = Rate(&g);

	std::vector<double> ret;
	for(double i = 0.0; i <= 20; i+=0.1){

		ret.push_back(r.eval(i));
	}
	std::cout << r.integrateFunct(0, 10) << std::endl;
	std::cout << r.rate_homog << std::endl;


#ifdef _WIN32
	FreeLibrary(lib_handle);
#else
	dlclose(lib_handle);
#endif

	return ret;
}

//' timeDepBranch
//'
//' timeDepBranch
//'
//' @export
// [[Rcpp::export]]
double timeDepBranch(int time, std::string file, Rcpp::NumericVector initial, Rcpp::List transitions, Rcpp::List stops, bool silence){
	double seedcpp = std::chrono::high_resolution_clock::now().time_since_epoch().count();
	gsl_rng_set(rng, seedcpp);
	silent = silence;
	workspace = gsl_integration_workspace_alloc(1000);

	#ifdef _WIN32
	HINSTANCE lib_handle;
	HINSTANCE lib_handle_newclone;
	#else
	void *lib_handle;
	void *lib_handle_newclone;
	#endif


	if(!silent)
	std::cout << "Starting process... " << std::endl;
	int nTrans = transitions.length();
	int nStops = stops.length();

	if(!silent)
	std::cout << "Initialization system..." << std::endl;
	// Initial population sizes
	std::vector<int> init(initial.begin(), initial.end());

	// Initialize system
	System sys(init);

	/*
	// Create populations with lifetimes
	std::cout << "Making populations..." << std::endl;
	for(size_t i = 0; i < init.size(); i++){
	std::cout << "init[" << i << "]: " << init[i] << std::endl;
	Population* p = new Population(lifetimes[i]);
	sys.addPopulation(p);
	}
	*/

	// Add transitions
	if(!silent)
	std::cout << "Adding transitions..." << std::endl;

	// Iterate over transitions list
	for(int i = 0; i < nTrans; i++){

		// Get the ith transition
		Rcpp::List list_i = Rcpp::as<Rcpp::List>(transitions[i]);

		// Get popuation, is_random, probability
		int population = list_i[0];
		bool is_random = list_i[1];
		Rcpp::List rateList = Rcpp::as<Rcpp::List>(list_i[2]);
		Rate* r;
		if(rateList.containsElementNamed("type")){
			if(Rcpp::as<int>(rateList["type"]) == 0){
				std::cout << "Constant rate found!" << std::endl;
				r = new ConstantRate(Rcpp::as<double>(rateList[1]));
			} else if (Rcpp::as<int>(rateList["type"]) == 1){
				Rcpp::NumericVector params = Rcpp::as<Rcpp::NumericVector>(rateList["params"]);
				std::cout << "Linear rate found!" << std::endl;
				r = new LinearRate(params[0], params[1]);
			} else if (Rcpp::as<int>(rateList["type"]) == 2){
				Rcpp::NumericVector params = Rcpp::as<Rcpp::NumericVector>(rateList["params"]);
				std::cout << "Switch rate found!" << std::endl;
				r = new SwitchRate(params[0], params[1], params[2]);
			} else if (Rcpp::as<int>(rateList["type"]) == 3){
				Rcpp::StringVector params = Rcpp::as<Rcpp::StringVector>(rateList["params"]);
				std::cout << "Custom rate found!" << std::endl;
				double (*rate)(double, void*);

				// Name for plugin library location
				const char* plugin_location = CHAR(Rf_asChar(params[0]));
				
				// Load plugin library
				#ifdef _WIN32
					lib_handle = LoadLibrary(plugin_location);
					if(!lib_handle)
					{
						Rcpp::stop("invalid file name for custom dll");
					}
				rate = (double (*)(double, void*))GetProcAddress(lib_handle, params[1]);
				#else
					lib_handle = dlopen(plugin_location, RTLD_NOW);
					if(!lib_handle)
					{
						Rcpp::stop("invalid file name custom dll");
					}
					rate = (double (*)(double, void*))dlsym(lib_handle, "CA");
				#endif
				
				r = new Rate(rate);
			}
		} else {
			r = new ConstantRate(Rcpp::as<double>(rateList[0]));
		}


		Update u;
		// If there is a random component, get which indices of the vector will be generated randomly
		if(is_random){
			// Get the offspring distribution of the Transition
			Rcpp::NumericVector oVec = Rcpp::as<Rcpp::NumericVector>(list_i[3]);
			std::vector<double> offspringVec (oVec.begin(), oVec.end());

			// Get the distribution
			std::string dist = list_i[4];

			// Get the offspring distribution of the Transition
			Rcpp::NumericVector p = Rcpp::as<Rcpp::NumericVector>(list_i[5]);
			std::vector<double> offspringParams (p.begin(), p.end());

			// Add this transition
			u = Update(is_random, offspringVec, dist, offspringParams);
			//sys.addUpdate(rate, population, Update(is_random, offspringVec, dist, offspringParams));
		} else{
			// Get the fixed portion of the Transition
			Rcpp::NumericVector fix = Rcpp::as<Rcpp::NumericVector>(list_i[3]);
			std::vector<int> fixed (fix.begin(), fix.end());

			// add update
			u = Update(fixed);
			//sys.addUpdate(rate, population, Update(fixed));
		}

		sys.addUpdate(r, population, u);
	}

	// Add transitions
	if(!silent)
	std::cout << "Adding stopping criteria..." << std::endl;

	// Iterate over transitions list
	for(int i = 0; i < nStops; i++){

		// Get the ith transition
		Rcpp::List list_i = Rcpp::as<Rcpp::List>(stops[i]);

		// Get popuation, is_random, probability

		// Get the offspring distribution of the Transition
		Rcpp::NumericVector ind = Rcpp::as<Rcpp::NumericVector>(list_i[0]);
		std::vector<int> indices (ind.begin(), ind.end());

		std::string ineq = list_i[1];
		double value = list_i[2];

		sys.addStop(StopCriterion(indices, ineq, value));
	}

	//sys.print();

	// Simulate
	if(!silent)
	std::cout << "Simulating..." << std::endl;
	sys.simulate2(time, file);
	if(!silent)
	std::cout << "Ending process..." << std::endl;

	gsl_integration_workspace_free(workspace);
	#ifdef _WIN32
		FreeLibrary(lib_handle);
	#else
		dlclose(lib_handle);
	#endif

	return 0.0;
}



