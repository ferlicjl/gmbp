/*
 * =====================================================================================
 *
 *       Filename:  ConstantRate.h
 *
 *    Description:  Time-dependent rate functions
 *
 *        Version:  1.0
 *        Created:  09/13/2018 14:57:27
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
//#include <map>

#include "Rate.h"

struct constant_params{
	double rate;
};


class ConstantRate : public Rate {
public:

constant_params params;

ConstantRate(double r);

~ConstantRate();

virtual double operator()(double time);

};

struct linear_params{
	double slope;
	double intercept;
};

class LinearRate : public Rate {
public:

linear_params params;

LinearRate(double i, double s);

~LinearRate();

virtual double operator()(double time);

};

struct switch_params{
	double pre;
	double post;
	double tswitch;
};

class SwitchRate : public Rate {
public:

switch_params params;

SwitchRate(double pre, double post, double s);

~SwitchRate();

virtual double operator()(double time);

};

struct pulse_params{
	double totPeriod;
	double lowPeriod;
	double low;
	double high;
};

class PulseRate : public Rate {
public:

pulse_params params;

PulseRate(double totPeriod, double lowPeriod, double low, double high);

~PulseRate();

virtual double operator()(double time);

};


