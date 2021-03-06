// ===============================================================================
// Authors: AFRL/RQQA
// Organization: Air Force Research Laboratory, Aerospace Systems Directorate, Power and Control Division
// 
// Copyright (c) 2017 Government of the United State of America, as represented by
// the Secretary of the Air Force.  No copyright is claimed in the United States under
// Title 17, U.S. Code.  All Other Rights Reserved.
// ===============================================================================

#pragma once
#include <cstdlib>

// return a uniformly distributed random value in (0,1)
double randu();

// return a zero-mean normally distributed random value with variance 1
double randn();

// returns mean 1 exponentially distributed random variable
double rande();
