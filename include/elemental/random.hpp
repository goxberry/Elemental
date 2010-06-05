/*
   This file is part of elemental, a library for distributed-memory dense 
   linear algebra.

   Copyright (C) 2009-2010 Jack Poulson <jack.poulson@gmail.com>

   This program is released under the terms of the license contained in the 
   file LICENSE.
*/
#ifndef ELEMENTAL_RANDOM_HPP
#define ELEMENTAL_RANDOM_HPP 1

#ifndef WITHOUT_COMPLEX
#include <complex>
#endif

namespace elemental {

// Generate a sample from a uniform PDF over the unit ball about the origin 
// of the vector space implied by the type T
template<typename T> T Random();

} // elemental

//----------------------------------------------------------------------------//
// Implementation begins here                                                 //
//----------------------------------------------------------------------------//

namespace elemental {

const double Pi = 3.141592653589793;

template<>
inline int
Random<int>()
{
    int sample = rand();
    if( sample <= RAND_MAX/3 )
        return -1;
    else if( sample <= (RAND_MAX/3)*2 )
        return 0;
    else
        return +1;
}

template<>
inline float
Random<float>()
{ return ( 2*static_cast<float>(rand())/RAND_MAX-1 ); }

template<>
inline double
Random<double>()
{ return ( 2*static_cast<double>(rand())/RAND_MAX-1 ); }

#ifndef WITHOUT_COMPLEX
template<>
inline std::complex<float>
Random< std::complex<float> >()
{
    float r = Random<float>();
    float angle = Pi * Random<float>();

    std::complex<float> u = std::polar(r,angle);

    return u;
}

template<>
inline std::complex<double>
Random< std::complex<double> >()
{
    double r = Random<double>();
    double angle = Pi * Random<double>();

    std::complex<double> u = std::polar(r,angle);

    return u;
}
#endif

} // elemental

#endif  /* ELEMENTAL_RANDOM_HPP */

