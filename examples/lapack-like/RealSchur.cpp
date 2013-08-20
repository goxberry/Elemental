/*
   Copyright (c) 2009-2013, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
// NOTE: It is possible to simply include "elemental.hpp" instead
#include "elemental-lite.hpp"
#include "elemental/lapack-like/Schur.hpp"
#include "elemental/lapack-like/Norm/Frobenius.hpp"
#include "elemental/matrices/Identity.hpp"
#include "elemental/matrices/Uniform.hpp"
#include "elemental/io/Display.hpp"
using namespace std;
using namespace elem;

typedef double Real;

int
main( int argc, char* argv[] )
{
    Initialize( argc, argv );

    try 
    {
        const Int n = Input("--size","height of matrix",100);
        const Int cutoff = Input("--cutoff","cutoff for QR alg.",256);
        const Int maxInnerIts = Input("--maxInnerIts","maximum RURV its",1);
        const Int maxOuterIts = Input("--maxOuterIts","maximum it's/split",10);
        const Real relTol = Input("--relTol","rel. tol.",Real(0));
        const bool display = Input("--display","display matrices?",false);
        ProcessInput();
        PrintInputReport();

        DistMatrix<Real> A;
        Uniform( A, n, n );
        const Real frobA = FrobeniusNorm( A );

        // Compute the Schur decomposition of A, but do not overwrite A
        DistMatrix<Real> T( A ), Q;
        schur::SDC( T, Q, true, cutoff, maxInnerIts, maxOuterIts, relTol );

        if( display )
        {
            Display( A, "A" );
            Display( T, "T" );
            Display( Q, "Q" );
        }

        DistMatrix<Real> G;
        Gemm( NORMAL, NORMAL, Real(1), Q, T, G );
        Gemm( NORMAL, ADJOINT, Real(-1), G, Q, Real(1), A );
        MakeTrapezoidal( LOWER, T, -2 );
        const Real frobOffT = FrobeniusNorm( T );
        if( display )
        {
            Display( A, "E" );
        }
        const Real frobE = FrobeniusNorm( A ); 
        if( mpi::WorldRank() == 0 )
        {
            std::cout << " || A - Q T Q^H ||_F / || A ||_F = " << frobE/frobA 
                      << "\n"
                      << " || stril(T) ||_F    / || A ||_F = " << frobOffT/frobA
                      << "\n"
                      << std::endl;
        }
    }
    catch( exception& e ) { ReportException(e); }

    Finalize();
    return 0;
}
