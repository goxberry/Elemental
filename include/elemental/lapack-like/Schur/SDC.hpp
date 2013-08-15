/*
   Copyright (c) 2009-2013, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#pragma once
#ifndef ELEM_LAPACK_SCHUR_SDC_HPP
#define ELEM_LAPACK_SCHUR_SDC_HPP

#include "elemental/blas-like/level1/Axpy.hpp"
#include "elemental/blas-like/level1/Scale.hpp"
#include "elemental/lapack-like/Norm/One.hpp"
#include "elemental/lapack-like/QR.hpp"
#include "elemental/lapack-like/Sign.hpp"
#include "elemental/lapack-like/Trace.hpp"
#include "elemental/matrices/Identity.hpp"

// See Z. Bai, J. Demmel, J. Dongarra, A. Petitet, H. Robinson, and K. Stanley's
// "The spectral decomposition of nonsymmetric matrices on distributed memory
// parallel computers". Currently available at:
// www.netlib.org/lapack/lawnspdf/lawn91.pdf

namespace elem {
namespace schur {

template<typename F>
inline BASE(F)
SpectralDivide( Matrix<F>& A )
{
#ifndef RELEASE
    CallStackEntry cse("schur::SpectralDivide");
#endif
    typedef BASE(F) Real;
    const Int n = A.Height();

    // S := sgn(A)
    Matrix<F> S( A );
    Sign( S );

    // Compute the spectral projector, B := 1/2 ( S + I ), and its trace
    Matrix<F> B; 
    Identity( B, n, n );
    Axpy( F(1), S, B );
    Scale( F(1)/F(2), B );
    // TODO: Compute rank more carefully
    const F trace = Trace(B);
    const Int roundedTrace = Int(round(RealPart(trace)));
    const Int rank = Max( Min( roundedTrace, n ), 0 );

    // Compute the pivoted QR decomposition of the spectral projection
    Matrix<F> t;
    Matrix<Int> p;
    QR( B, t, p );

    // A := Q^H A Q
    const Real oneA = OneNorm( A );
    qr::ApplyQ( LEFT, ADJOINT, B, t, A );
    qr::ApplyQ( RIGHT, NORMAL, B, t, A );

    // Return || E21 ||1 / || A ||1
    Matrix<F> E21;
    LockedView( E21, A, rank, 0, n-rank, rank );
    return OneNorm(E21)/oneA;
}

template<typename F>
inline BASE(F)
SpectralDivide( DistMatrix<F>& A )
{
#ifndef RELEASE
    CallStackEntry cse("schur::SpectralDivide");
#endif
    typedef BASE(F) Real;
    const Int n = A.Height();
    const Grid& g = A.Grid();

    // S := sgn(A)
    DistMatrix<F> S( A );
    Sign( S );

    // Compute the spectral projector, B := 1/2 ( S + I ), and its trace
    DistMatrix<F> B(g);
    Identity( B, n, n );
    Axpy( F(1), S, B );
    Scale( F(1)/F(2), B );
    // TODO: Compute rank more carefully
    const F trace = Trace(B);
    const Int roundedTrace = Int(round(RealPart(trace)));
    const Int rank = Max( Min( roundedTrace, n ), 0 );

    // Compute the pivoted QR decomposition of the spectral projection
    DistMatrix<F,MD,STAR> t(g);
    DistMatrix<Int,VR,STAR> p(g);
    QR( B, t, p );

    // A := Q^H A Q
    const Real oneA = OneNorm( A );
    qr::ApplyQ( LEFT, ADJOINT, B, t, A );
    qr::ApplyQ( RIGHT, NORMAL, B, t, A );

    // Return || E21 ||1 / || A ||1
    DistMatrix<F> E21(g);
    LockedView( E21, A, rank, 0, n-rank, rank );
    return OneNorm(E21)/oneA;
}

} // namespace schur
} // namespace elem

#endif // ifndef ELEM_LAPACK_SCHUR_SDC_HPP