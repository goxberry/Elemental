/*
   Copyright (c) 2009-2013, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#pragma once
#ifndef ELEM_BLAS_QUASIDIAGONALSOLVE_HPP
#define ELEM_BLAS_QUASIDIAGONALSOLVE_HPP

#include "elemental/blas-like/level1/Symmetric2x2Scale.hpp"

namespace elem {

template<typename F,typename FMain>
inline void
QuasiDiagonalScale
( LeftOrRight side, UpperOrLower uplo, Orientation orientation,
  const Matrix<FMain>& d, const Matrix<F>& dSub, 
  Matrix<F>& X, bool conjugated=false )
{
    DEBUG_ONLY(CallStackEntry cse("QuasiDiagonalScale"))
    const Int m = X.Height();
    const Int n = X.Width();
    Matrix<F> D( 2, 2 );
    if( side == LEFT && uplo == LOWER )
    {
        Int i=0;
        while( i < m )
        {
            Int nb;
            if( i < m-1 && Abs(dSub.Get(i,0)) > 0 )
                nb = 2;
            else
                nb = 1;

            if( nb == 1 )
            {
                auto xRow = View( X, i, 0, nb, n );
                Scale( d.Get(i,0), xRow );
            }
            else
            {
                D.Set(0,0,d.Get(i,0));    
                D.Set(1,1,d.Get(i+1,0));
                D.Set(1,0,dSub.Get(i,0));
                auto XRow = View( X, i, 0, nb, n );
                Symmetric2x2Scale( LEFT, LOWER, D, XRow, conjugated );
            }

            i += nb;
        }
    }
    else if( side == RIGHT && uplo == LOWER )
    {
        Int j=0;
        while( j < n )
        {
            Int nb;
            if( j < n-1 && Abs(dSub.Get(j,0)) > 0 )
                nb = 2;
            else
                nb = 1;

            if( nb == 1 )
            {
                auto xCol = View( X, 0, j, m, nb );
                Scale( d.Get(j,0), xCol );
            }
            else
            {
                D.Set(0,0,d.Get(j,0));    
                D.Set(1,1,d.Get(j+1,0));
                D.Set(1,0,dSub.Get(j,0));
                auto XCol = View( X, 0, j, m, nb );
                Symmetric2x2Scale( RIGHT, LOWER, D, XCol, conjugated );
            }

            j += nb;
        }
    }
    else
        LogicError("This option not yet supported");
}

template<typename F,typename FMain,Distribution U,Distribution V>
inline void
LeftQuasiDiagonalScale
( UpperOrLower uplo, Orientation orientation, 
  const DistMatrix<FMain,U,STAR> d,
  const DistMatrix<FMain,U,STAR> dPrev,
  const DistMatrix<FMain,U,STAR> dNext,
  const DistMatrix<FMain,U,STAR> dSub,
  const DistMatrix<FMain,U,STAR> dSubPrev,
  const DistMatrix<FMain,U,STAR> dSubNext,
        DistMatrix<F,U,V>& X,
  const DistMatrix<F,U,V>& XPrev,
  const DistMatrix<F,U,V>& XNext,
  bool conjugated=false )
{
    DEBUG_ONLY(CallStackEntry cse("LeftQuasiDiagonalScale"))
    if( uplo == UPPER || orientation != NORMAL )
        LogicError("This option not yet supported");
    const Int m = X.Height();
    const Int mLocal = X.LocalHeight();
    const Int nLocal = X.LocalWidth();
    const Int colShift = X.ColShift();
    const Int colStride = X.ColStride();
    DEBUG_ONLY(
        const Int colAlignPrev = (X.ColAlign()+colStride-1) % colStride;
        const Int colAlignNext = (X.ColAlign()+1) % colStride;
        if( d.ColAlign() != X.ColAlign() || dSub.ColAlign() != X.ColAlign() )
            LogicError("data is not properly aligned");
        if( XPrev.ColAlign() != colAlignPrev ||
            dPrev.ColAlign() != colAlignPrev || 
            dSubPrev.ColAlign() != colAlignPrev )
            LogicError("'previous' data is not properly aligned");
        if( XNext.ColAlign() != colAlignNext || 
            dNext.ColAlign() != colAlignNext || 
            dSubNext.ColAlign() != colAlignNext )
            LogicError("'next' data is not properly aligned");
    )
    const Int colShiftPrev = XPrev.ColShift();
    const Int colShiftNext = XNext.ColShift();
    const Int prevOff = ( colShiftPrev==colShift-1 ? 0 : -1 );
    const Int nextOff = ( colShiftNext==colShift+1 ? 0 : +1 );
    if( !X.Participating() )
        return;

    // It is best to separate the case where colStride is 1
    if( colStride == 1 )
    {
        QuasiDiagonalScale
        ( LEFT, uplo, orientation, d.LockedMatrix(), dSub.LockedMatrix(),
          X.Matrix(), conjugated );
        return;
    }

    Matrix<F> D11( 2, 2 );
    for( Int iLoc=0; iLoc<mLocal; ++iLoc )
    {
        const Int i = colShift + iLoc*colStride;
        const Int iLocPrev = iLoc + prevOff;
        const Int iLocNext = iLoc + nextOff;

        auto x1Loc = View( X.Matrix(), iLoc, 0, 1, nLocal );

        if( i<m-1 && dSub.GetLocal(iLoc,0) != F(0) )
        {
            // Handle 2x2 starting at i
            D11.Set( 0, 0, d.GetLocal(iLoc,0) ); 
            D11.Set( 1, 1, dNext.GetLocal(iLocNext,0) );
            D11.Set( 1, 0, dSub.GetLocal(iLoc,0) );

            auto x1NextLoc = 
                LockedView( XNext.LockedMatrix(), iLocNext, 0, 1, nLocal );
            FirstHalfOfSymmetric2x2Scale
            ( LEFT, LOWER, D11, x1Loc, x1NextLoc, conjugated );
        }
        else if( i>0 && dSubPrev.GetLocal(iLocPrev,0) != F(0) )
        {
            // Handle 2x2 starting at i-1
            D11.Set( 0, 0, dPrev.GetLocal(iLocPrev,0) );
            D11.Set( 1, 1, d.GetLocal(iLoc,0) );
            D11.Set( 1, 0, dSubPrev.GetLocal(iLocPrev,0) );

            auto x1PrevLoc = 
                LockedView( XPrev.LockedMatrix(), iLocPrev, 0, 1, nLocal );
            SecondHalfOfSymmetric2x2Scale
            ( LEFT, LOWER, D11, x1PrevLoc, x1Loc, conjugated );
        }
        else
        {
            // Handle 1x1
            Scale( d.GetLocal(iLoc,0), x1Loc );
        }
    }
}

template<typename F,typename FMain,Distribution U,Distribution V>
inline void
RightQuasiDiagonalScale
( UpperOrLower uplo, Orientation orientation, 
  const DistMatrix<FMain,V,STAR> d,
  const DistMatrix<FMain,V,STAR> dPrev,
  const DistMatrix<FMain,V,STAR> dNext,
  const DistMatrix<FMain,V,STAR> dSub,
  const DistMatrix<FMain,V,STAR> dSubPrev,
  const DistMatrix<FMain,V,STAR> dSubNext,
        DistMatrix<F,U,V>& X,
  const DistMatrix<F,U,V>& XPrev,
  const DistMatrix<F,U,V>& XNext,
  bool conjugated=false )
{
    DEBUG_ONLY(CallStackEntry cse("LeftQuasiDiagonalScale"))
    if( uplo == UPPER || orientation != NORMAL )
        LogicError("This option not yet supported");
    const Int n = X.Width();
    const Int mLocal = X.LocalHeight();
    const Int nLocal = X.LocalWidth();
    const Int rowShift = X.RowShift();
    const Int rowStride = X.RowStride();
    DEBUG_ONLY(
        const Int rowAlignPrev = (X.RowAlign()+rowStride-1) % rowStride;
        const Int rowAlignNext = (X.RowAlign()+1) % rowStride;
        if( d.RowAlign() != X.RowAlign() || dSub.RowAlign() != X.RowAlign() )
            LogicError("data is not properly aligned");
        if( XPrev.RowAlign() != rowAlignPrev ||
            dPrev.RowAlign() != rowAlignPrev || 
            dSubPrev.RowAlign() != rowAlignPrev )
            LogicError("'previous' data is not properly aligned");
        if( XNext.RowAlign() != rowAlignNext || 
            dNext.RowAlign() != rowAlignNext || 
            dSubNext.RowAlign() != rowAlignNext )
            LogicError("'next' data is not properly aligned");
    )
    const Int rowShiftPrev = XPrev.RowShift();
    const Int rowShiftNext = XNext.RowShift();
    const Int prevOff = ( rowShiftPrev==rowShift-1 ? 0 : -1 );
    const Int nextOff = ( rowShiftNext==rowShift+1 ? 0 : +1 );
    if( !X.Participating() )
        return;

    // It is best to separate the case where rowStride is 1
    if( rowStride == 1 )
    {
        QuasiDiagonalScale
        ( LEFT, uplo, orientation, d.LockedMatrix(), dSub.LockedMatrix(),
          X.Matrix(), conjugated );
        return;
    }

    Matrix<F> D11( 2, 2 );
    for( Int jLoc=0; jLoc<nLocal; ++jLoc )
    {
        const Int j = rowShift + jLoc*rowStride;
        const Int jLocPrev = jLoc + prevOff;
        const Int jLocNext = jLoc + nextOff;

        auto x1Loc = View( X.Matrix(), 0, jLoc, mLocal, 1 );

        if( j<n-1 && dSub.GetLocal(jLoc,0) != F(0) )
        {
            // Handle 2x2 starting at j
            D11.Set( 0, 0, d.GetLocal(jLoc,0) ); 
            D11.Set( 1, 1, dNext.GetLocal(jLocNext,0) );
            D11.Set( 1, 0, dSub.GetLocal(jLoc,0) );

            auto x1NextLoc = 
                LockedView( XNext.LockedMatrix(), 0, jLocNext, mLocal, 1 );
            FirstHalfOfSymmetric2x2Scale
            ( RIGHT, LOWER, D11, x1Loc, x1NextLoc, conjugated );
        }
        else if( j>0 && dSubPrev.GetLocal(jLocPrev,0) != F(0) )
        {
            // Handle 2x2 starting at j-1
            D11.Set( 0, 0, dPrev.GetLocal(jLocPrev,0) );
            D11.Set( 1, 1, d.GetLocal(jLoc,0) );
            D11.Set( 1, 0, dSubPrev.GetLocal(jLocPrev,0) );

            auto x1PrevLoc = 
                LockedView( XPrev.LockedMatrix(), 0, jLocPrev, mLocal, 1 );
            SecondHalfOfSymmetric2x2Scale
            ( RIGHT, LOWER, D11, x1PrevLoc, x1Loc, conjugated );
        }
        else
        {
            // Handle 1x1
            Scale( d.GetLocal(jLoc,0), x1Loc );
        }
    }
}

template<typename F,typename FMain,Distribution U1,Distribution V1,
                                   Distribution U2,Distribution V2>
inline void
QuasiDiagonalScale
( LeftOrRight side, UpperOrLower uplo, Orientation orientation,
  const DistMatrix<FMain,U1,V1>& d, const DistMatrix<F,U1,V1>& dSub, 
  DistMatrix<F,U2,V2>& X, bool conjugated=false )
{
    DEBUG_ONLY(CallStackEntry cse("QuasiDiagonalScale"))
    const Grid& g = X.Grid();
    const Int colAlign = X.ColAlign();
    const Int rowAlign = X.RowAlign();
    if( side == LEFT )
    {
        const Int colStride = X.ColStride();
        DistMatrix<FMain,U2,STAR> d_U2_STAR(g);
        DistMatrix<F,U2,STAR> dSub_U2_STAR(g);
        d_U2_STAR.AlignWith( X );
        dSub_U2_STAR.AlignWith( X );
        d_U2_STAR = d;
        dSub_U2_STAR = dSub;
        if( colStride == 1 )
        {
            QuasiDiagonalScale
            ( side, uplo, orientation, 
              d_U2_STAR.LockedMatrix(), dSub_U2_STAR.LockedMatrix(),
              X.Matrix(), conjugated );
            return;
        }

        DistMatrix<FMain,U2,STAR> dPrev_U2_STAR(g), dNext_U2_STAR(g);
        DistMatrix<F,U2,STAR> dSubPrev_U2_STAR(g), dSubNext_U2_STAR(g);
        DistMatrix<F,U2,V2> XPrev(g), XNext(g);
        const Int colAlignPrev = (colAlign+colStride-1) % colStride;
        const Int colAlignNext = (colAlign+1) % colStride;
        dPrev_U2_STAR.AlignCols( colAlignPrev );
        dNext_U2_STAR.AlignCols( colAlignNext );
        dSubPrev_U2_STAR.AlignCols( colAlignPrev );
        dSubNext_U2_STAR.AlignCols( colAlignNext );
        XPrev.Align( colAlignPrev, rowAlign );
        XNext.Align( colAlignNext, rowAlign );
        dPrev_U2_STAR = d;
        dNext_U2_STAR = d;
        dSubPrev_U2_STAR = dSub;
        dSubNext_U2_STAR = dSub;
        XPrev = X;
        XNext = X;
        LeftQuasiDiagonalScale
        ( uplo, orientation, 
          d_U2_STAR, dPrev_U2_STAR, dNext_U2_STAR,
          dSub_U2_STAR, dSubPrev_U2_STAR, dSubNext_U2_STAR,
          X, XPrev, XNext, conjugated );
    }
    else
    {
        const Int rowStride = X.RowStride();
        DistMatrix<FMain,V2,STAR> d_V2_STAR(g);
        DistMatrix<F,V2,STAR> dSub_V2_STAR(g);
        d_V2_STAR.AlignWith( X );
        dSub_V2_STAR.AlignWith( X );
        d_V2_STAR = d;
        dSub_V2_STAR = dSub;
        if( rowStride == 1 )
        {
            QuasiDiagonalScale
            ( side, uplo, orientation, 
              d_V2_STAR.LockedMatrix(), dSub_V2_STAR.LockedMatrix(),
              X.Matrix(), conjugated );
            return;
        }

        DistMatrix<FMain,V2,STAR> dPrev_V2_STAR(g), dNext_V2_STAR(g);
        DistMatrix<F,V2,STAR> dSubPrev_V2_STAR(g), dSubNext_V2_STAR(g);
        DistMatrix<F,U2,V2> XPrev(g), XNext(g);
        const Int rowAlignPrev = (rowAlign+rowStride-1) % rowStride;
        const Int rowAlignNext = (rowAlign+1) % rowStride;
        dPrev_V2_STAR.AlignCols( rowAlignPrev );
        dNext_V2_STAR.AlignCols( rowAlignNext );
        dSubPrev_V2_STAR.AlignCols( rowAlignPrev );
        dSubNext_V2_STAR.AlignCols( rowAlignNext );
        XPrev.Align( colAlign, rowAlignPrev );
        XNext.Align( colAlign, rowAlignNext );
        dPrev_V2_STAR = d;
        dNext_V2_STAR = d;
        dSubPrev_V2_STAR = dSub;
        dSubNext_V2_STAR = dSub;
        XPrev = X;
        XNext = X;
        RightQuasiDiagonalScale
        ( uplo, orientation, 
          d_V2_STAR, dPrev_V2_STAR, dNext_V2_STAR,
          dSub_V2_STAR, dSubPrev_V2_STAR, dSubNext_V2_STAR,
          X, XPrev, XNext, conjugated );
    }
}

} // namespace elem

#endif // ifndef ELEM_BLAS_QUASIDIAGONALSOLVE_HPP