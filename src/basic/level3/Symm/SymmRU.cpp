/*
   Copyright (c) 2009-2011, Jack Poulson
   All rights reserved.

   This file is part of Elemental.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

    - Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.

    - Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.

    - Neither the name of the owner nor the names of its contributors
      may be used to endorse or promote products derived from this software
      without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
*/
#include "elemental/basic_internal.hpp"
using namespace std;
using namespace elemental;

// Template conventions:
//   G: general datatype
//
//   T: any ring, e.g., the (Gaussian) integers and the real/complex numbers
//   Z: representation of a real ring, e.g., the integers or real numbers
//   std::complex<Z>: representation of a complex ring, e.g. Gaussian integers
//                    or complex numbers
//
//   F: representation of real or complex number
//   R: representation of real number
//   std::complex<R>: representation of complex number

template<typename T>
void
elemental::basic::internal::SymmRU
( T alpha, const DistMatrix<T,MC,MR>& A,
           const DistMatrix<T,MC,MR>& B,
  T beta,        DistMatrix<T,MC,MR>& C )
{
#ifndef RELEASE
    PushCallStack("basic::internal::SymmRU");
#endif
    // TODO: Come up with a better routing mechanism
    if( A.Height() > 5*B.Height() )
        basic::internal::SymmRUA( alpha, A, B, beta, C );
    else
        basic::internal::SymmRUC( alpha, A, B, beta, C );
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T>
void
elemental::basic::internal::SymmRUA
( T alpha, const DistMatrix<T,MC,MR>& A,
           const DistMatrix<T,MC,MR>& B,
  T beta,        DistMatrix<T,MC,MR>& C )
{
#ifndef RELEASE
    PushCallStack("basic::internal::SymmRUA");
    if( A.Grid() != B.Grid() || B.Grid() != C.Grid() )
        throw logic_error( "{A,B,C} must be distributed over the same grid." );
#endif
    const Grid& g = A.Grid();

    DistMatrix<T,MC,MR>
        BT(g),  B0(g),
        BB(g),  B1(g),
                B2(g);

    DistMatrix<T,MC,MR>
        CT(g),  C0(g),
        CB(g),  C1(g),
                C2(g);

    DistMatrix<T,MR,  STAR> B1Trans_MR_STAR(g);
    DistMatrix<T,VC,  STAR> B1Trans_VC_STAR(g);
    DistMatrix<T,STAR,MC  > B1_STAR_MC(g);
    DistMatrix<T,MC,  STAR> Z1Trans_MC_STAR(g);
    DistMatrix<T,MR,  STAR> Z1Trans_MR_STAR(g);
    DistMatrix<T,MC,  MR  > Z1Trans(g);
    DistMatrix<T,MR,  MC  > Z1Trans_MR_MC(g);

    Matrix<T> Z1Local;

    basic::Scal( beta, C );
    LockedPartitionDown
    ( B, BT,
         BB, 0 );
    PartitionDown
    ( C, CT,
         CB, 0 );
    while( CT.Height() < C.Height() )
    {
        LockedRepartitionDown
        ( BT,  B0, 
         /**/ /**/
               B1,
          BB,  B2 );

        RepartitionDown
        ( CT,  C0,
         /**/ /**/
               C1,
          CB,  C2 );

        B1Trans_MR_STAR.AlignWith( A );
        B1Trans_VC_STAR.AlignWith( A );
        B1_STAR_MC.AlignWith( A );
        Z1Trans_MC_STAR.AlignWith( A );
        Z1Trans_MR_STAR.AlignWith( A );
        Z1Trans_MR_MC.AlignWith( C1 );
        Z1Trans_MC_STAR.ResizeTo( C1.Width(), C1.Height() );
        Z1Trans_MR_STAR.ResizeTo( C1.Width(), C1.Height() );
        //--------------------------------------------------------------------//
        B1Trans_MR_STAR.TransposeFrom( B1 );
        B1Trans_VC_STAR = B1Trans_MR_STAR;
        B1_STAR_MC.TransposeFrom( B1Trans_VC_STAR );
        Z1Trans_MC_STAR.SetToZero();
        Z1Trans_MR_STAR.SetToZero();
        basic::internal::LocalSymmetricAccumulateRU
        ( TRANSPOSE, alpha, A, B1_STAR_MC, B1Trans_MR_STAR, 
          Z1Trans_MC_STAR, Z1Trans_MR_STAR );

        Z1Trans.SumScatterFrom( Z1Trans_MC_STAR );
        Z1Trans_MR_MC = Z1Trans;
        Z1Trans_MR_MC.SumScatterUpdate( (T)1, Z1Trans_MR_STAR );
        basic::Transpose( Z1Trans_MR_MC.LockedLocalMatrix(), Z1Local );
        basic::Axpy( (T)1, Z1Local, C1.LocalMatrix() );
        //--------------------------------------------------------------------//
        B1Trans_MR_STAR.FreeAlignments();
        B1Trans_VC_STAR.FreeAlignments();
        B1_STAR_MC.FreeAlignments();
        Z1Trans_MC_STAR.FreeAlignments();
        Z1Trans_MR_STAR.FreeAlignments();
        Z1Trans_MR_MC.FreeAlignments();

        SlideLockedPartitionDown
        ( BT,  B0,
               B1,
         /**/ /**/
          BB,  B2 );

        SlidePartitionDown
        ( CT,  C0,
               C1,
         /**/ /**/
          CB,  C2 );
    }
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T>
void
elemental::basic::internal::SymmRUC
( T alpha, const DistMatrix<T,MC,MR>& A,
           const DistMatrix<T,MC,MR>& B,
  T beta,        DistMatrix<T,MC,MR>& C )
{
#ifndef RELEASE
    PushCallStack("basic::internal::SymmRUC");
    if( A.Grid() != B.Grid() || B.Grid() != C.Grid() )
        throw logic_error( "{A,B,C} must be distributed on the same grid." );
#endif
    const Grid& g = A.Grid();

    // Matrix views
    DistMatrix<T,MC,MR> 
        ATL(g), ATR(g),  A00(g), A01(g), A02(g),  AColPan(g),
        ABL(g), ABR(g),  A10(g), A11(g), A12(g),  ARowPan(g),
                         A20(g), A21(g), A22(g);

    DistMatrix<T,MC,MR> BL(g), BR(g),
                        B0(g), B1(g), B2(g);

    DistMatrix<T,MC,MR> CL(g), CR(g),
                        C0(g), C1(g), C2(g),
                        CLeft(g), CRight(g);

    // Temporary distributions
    DistMatrix<T,MC,  STAR> B1_MC_STAR(g);
    DistMatrix<T,VR,  STAR> AColPan_VR_STAR(g);
    DistMatrix<T,STAR,MR  > AColPanTrans_STAR_MR(g);
    DistMatrix<T,MR,  STAR> ARowPanTrans_MR_STAR(g);

    // Start the algorithm
    basic::Scal( beta, C );
    LockedPartitionDownDiagonal
    ( A, ATL, ATR,
         ABL, ABR, 0 );
    LockedPartitionRight( B, BL, BR, 0 );
    PartitionRight( C, CL, CR, 0 );
    while( CR.Width() > 0 )
    {
        LockedRepartitionDownDiagonal
        ( ATL, /**/ ATR,  A00, /**/ A01, A02,
         /*************/ /******************/
               /**/       A10, /**/ A11, A12,
          ABL, /**/ ABR,  A20, /**/ A21, A22 );

        LockedRepartitionRight
        ( BL, /**/ BR,
          B0, /**/ B1, B2 );

        RepartitionRight
        ( CL, /**/ CR,
          C0, /**/ C1, C2 );

        ARowPan.LockedView1x2( A11, A12 );

        AColPan.LockedView2x1
        ( A01,
          A11 );

        CLeft.View1x2( C0, C1 );

        CRight.View1x2( C1, C2 );

        B1_MC_STAR.AlignWith( C );
        AColPan_VR_STAR.AlignWith( CLeft );
        AColPanTrans_STAR_MR.AlignWith( CLeft );
        ARowPanTrans_MR_STAR.AlignWith( CRight );
        //--------------------------------------------------------------------//
        B1_MC_STAR = B1;

        AColPan_VR_STAR = AColPan;
        AColPanTrans_STAR_MR.TransposeFrom( AColPan_VR_STAR );
        ARowPanTrans_MR_STAR.TransposeFrom( ARowPan );
        ARowPanTrans_MR_STAR.MakeTrapezoidal( LEFT, LOWER );
        AColPanTrans_STAR_MR.MakeTrapezoidal( RIGHT, LOWER, -1 );

        basic::internal::LocalGemm
        ( NORMAL, TRANSPOSE, 
          alpha, B1_MC_STAR, ARowPanTrans_MR_STAR, (T)1, CRight );

        basic::internal::LocalGemm
        ( NORMAL, NORMAL,
          alpha, B1_MC_STAR, AColPanTrans_STAR_MR, (T)1, CLeft );
        //--------------------------------------------------------------------//
        B1_MC_STAR.FreeAlignments();
        AColPan_VR_STAR.FreeAlignments();
        AColPanTrans_STAR_MR.FreeAlignments();
        ARowPanTrans_MR_STAR.FreeAlignments();

        SlideLockedPartitionDownDiagonal
        ( ATL, /**/ ATR,  A00, A01, /**/ A02,
               /**/       A10, A11, /**/ A12,
         /*************/ /******************/
          ABL, /**/ ABR,  A20, A21, /**/ A22 );

        SlideLockedPartitionRight
        ( BL,     /**/ BR,
          B0, B1, /**/ B2 );

        SlidePartitionRight
        ( CL,     /**/ CR,
          C0, C1, /**/ C2 );
    }
#ifndef RELEASE
    PopCallStack();
#endif
}

template void elemental::basic::internal::SymmRU
( float alpha, const DistMatrix<float,MC,MR>& A,
               const DistMatrix<float,MC,MR>& B,
  float beta,        DistMatrix<float,MC,MR>& C );

template void elemental::basic::internal::SymmRU
( double alpha, const DistMatrix<double,MC,MR>& A,
                const DistMatrix<double,MC,MR>& B,
  double beta,        DistMatrix<double,MC,MR>& C );

#ifndef WITHOUT_COMPLEX
template void elemental::basic::internal::SymmRU
( scomplex alpha, const DistMatrix<scomplex,MC,MR>& A,
                  const DistMatrix<scomplex,MC,MR>& B,
  scomplex beta,        DistMatrix<scomplex,MC,MR>& C );

template void elemental::basic::internal::SymmRU
( dcomplex alpha, const DistMatrix<dcomplex,MC,MR>& A,
                  const DistMatrix<dcomplex,MC,MR>& B,
  dcomplex beta,        DistMatrix<dcomplex,MC,MR>& C );
#endif

