/*
 * =====================================================================================
 *
 *       Filename:  Update.h
 *
 *    Description:  Class representing a system update
 *
 *        Version:  1.0
 *        Created:  06/26/20178 14:57:27
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



class Update {
public:
	// Members
	bool is_random;
	std::vector<int> fixed;
	std::vector<double> offspring_vec;
	std::string offspring_dist;
	std::vector<double> params;
	
	

	// Constructors
	Update();
	Update(std::vector<int> f);
	Update(bool rand, std::vector<double> o, std::string dist, std::vector<double> p);
	~Update();

	// Methods
	std::vector<int> get();
};

