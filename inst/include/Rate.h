/*
 * =====================================================================================
 *
 *       Filename:  Rate.h
 *
 *    Description:  Time-dependent rate functions
 *
 *        Version:  1.0
 *        Created:  09/10/2018 14:57:27
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  Jeremy Ferlic (), jferlic@g.harvard.edu
 *   Organization:  Harvard University
 *
 * =====================================================================================
 */

#pragma once
#include <string>
#include <iostream>
#include <ostream>
#include <sstream>
#include <vector>
#include <gsl/gsl_min.h>
#include <Rcpp.h>

//#include <map>



class Rate {
public:
	// Members
	int type; // 0 (constant), 1 (linear), 2 (quadratic), 3 (custom)
	std::vector<double> params;
	double rate_homog;
	double tot_error;
	gsl_function funct;

	// Constructors
	Rate();
	Rate(double (*f)(double, void*));
	virtual ~Rate();

	double eval(double time);

	virtual double operator()(double time);
};
