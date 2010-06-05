/*
   This file is part of elemental, a library for distributed-memory dense 
   linear algebra.

   Copyright (C) 2009-2010 Jack Poulson <jack.poulson@gmail.com>

   This program is released under the terms of the license contained in the 
   file LICENSE.
*/
#ifndef ELEMENTAL_DIST_MATRIX_STAR_MD_HPP
#define ELEMENTAL_DIST_MATRIX_STAR_MD_HPP 1

#include "elemental/dist_matrix.hpp"

namespace elemental {

// Partial specialization to A[* ,MD]
// 
// The rows of these distributed matrices will be distributed like 
// "Matrix Diagonals" (MD). It is important to recognize that the diagonal
// of a sufficiently large distributed matrix is distributed amongst the 
// entire process grid if and only if the dimensions of the process grid
// are coprime.

template<typename T>
class DistMatrixBase<T,Star,MD> : public AbstractDistMatrix<T>
{
protected:
    typedef AbstractDistMatrix<T> ADM;

    bool _inDiagonal;

    DistMatrixBase
    ( int height,
      int width,
      bool constrainedRowAlignment,
      int rowAlignment,
      int rowShift,
      const Grid& grid );

    ~DistMatrixBase();

public:
    //------------------------------------------------------------------------//
    // Fulfillments of abstract virtual func's from AbstractDistMatrixBase    //
    //------------------------------------------------------------------------//

    //
    // Non-collective routines
    //

    // (empty)

    //
    // Collective routines
    //

    T Get( int i, int j ) const;
    void Set( int i, int j, T alpha );

    void MakeTrapezoidal
    ( Side side, Shape shape, int offset = 0 );

    void Print( const std::string& s ) const;
    void ResizeTo( int height, int width );
    void SetToIdentity();
    void SetToRandom();

    //------------------------------------------------------------------------//
    // Routines specific to [* ,MD] distribution                              //
    //------------------------------------------------------------------------//

    bool InDiagonal() const;

    // Aligns all of our DistMatrix's distributions that match a distribution
    // of the argument DistMatrix.
    void AlignWith( const DistMatrixBase<T,MD,  Star>& A );
    void AlignWith( const DistMatrixBase<T,Star,MD>& A );
    void AlignWith( const DistMatrixBase<T,Star,MC  >& A ) {}
    void AlignWith( const DistMatrixBase<T,Star,MR  >& A ) {}
    void AlignWith( const DistMatrixBase<T,Star,VC  >& A ) {}
    void AlignWith( const DistMatrixBase<T,Star,VR  >& A ) {}
    void AlignWith( const DistMatrixBase<T,Star,Star>& A ) {}
    void AlignWith( const DistMatrixBase<T,MC,  Star>& A ) {}
    void AlignWith( const DistMatrixBase<T,MR,  Star>& A ) {}
    void AlignWith( const DistMatrixBase<T,VC,  Star>& A ) {}
    void AlignWith( const DistMatrixBase<T,VR,  Star>& A ) {}

    // Aligns our column distribution (i.e., Star) with the matching 
    // distribution of the argumnet. These are all no-ops and exist solely
    // to allow for templating over distribution parameters.
    void AlignColsWith( const DistMatrixBase<T,Star,MC  >& A ) {}
    void AlignColsWith( const DistMatrixBase<T,Star,MR  >& A ) {}
    void AlignColsWith( const DistMatrixBase<T,Star,MD  >& A ) {}
    void AlignColsWith( const DistMatrixBase<T,Star,VC  >& A ) {}
    void AlignColsWith( const DistMatrixBase<T,Star,VR  >& A ) {}
    void AlignColsWith( const DistMatrixBase<T,Star,Star>& A ) {}
    void AlignColsWith( const DistMatrixBase<T,MC,  Star>& A ) {}
    void AlignColsWith( const DistMatrixBase<T,MR,  Star>& A ) {}
    void AlignColsWith( const DistMatrixBase<T,MD,  Star>& A ) {}
    void AlignColsWith( const DistMatrixBase<T,VC,  Star>& A ) {}
    void AlignColsWith( const DistMatrixBase<T,VR,  Star>& A ) {}

    // Aligns our row distribution (i.e., MD) with the matching distribution
    // of the argument. 
    void AlignRowsWith( const DistMatrixBase<T,MD,  Star>& A );
    void AlignRowsWith( const DistMatrixBase<T,Star,MD  >& A );

    void AlignWithDiag
    ( const DistMatrixBase<T,MC,MR>& A, int offset = 0 );

    void AlignWithDiag
    ( const DistMatrixBase<T,MR,MC>& A, int offset = 0 );

    // (Immutable) view of a distributed matrix
    void View( DistMatrixBase<T,Star,MD>& A );
    void LockedView( const DistMatrixBase<T,Star,MD>& A );

    // (Immutable) view of a portion of a distributed matrix
    void View
    ( DistMatrixBase<T,Star,MD>& A,
      int i, int j, int height, int width );

    void LockedView
    ( const DistMatrixBase<T,Star,MD>& A,
      int i, int j, int height, int width );

    // (Immutable) view of two horizontally contiguous partitions of a
    // distributed matrix
    void View1x2
    ( DistMatrixBase<T,Star,MD>& AL, DistMatrixBase<T,Star,MD>& AR );

    void LockedView1x2
    ( const DistMatrixBase<T,Star,MD>& AL, 
      const DistMatrixBase<T,Star,MD>& AR );

    // (Immutable) view of two vertically contiguous partitions of a
    // distributed matrix
    void View2x1
    ( DistMatrixBase<T,Star,MD>& AT,
      DistMatrixBase<T,Star,MD>& AB );

    void LockedView2x1
    ( const DistMatrixBase<T,Star,MD>& AT,
      const DistMatrixBase<T,Star,MD>& AB );

    // (Immutable) view of a contiguous 2x2 set of partitions of a
    // distributed matrix
    void View2x2
    ( DistMatrixBase<T,Star,MD>& ATL, DistMatrixBase<T,Star,MD>& ATR,
      DistMatrixBase<T,Star,MD>& ABL, DistMatrixBase<T,Star,MD>& ABR );

    void LockedView2x2
    ( const DistMatrixBase<T,Star,MD>& ATL, 
      const DistMatrixBase<T,Star,MD>& ATR,
      const DistMatrixBase<T,Star,MD>& ABL, 
      const DistMatrixBase<T,Star,MD>& ABR );

    const DistMatrixBase<T,Star,MD>&
    operator=( const DistMatrixBase<T,MC,MR>& A );

    const DistMatrixBase<T,Star,MD>&
    operator=( const DistMatrixBase<T,MC,Star>& A );

    const DistMatrixBase<T,Star,MD>&
    operator=( const DistMatrixBase<T,Star,MR>& A );

    const DistMatrixBase<T,Star,MD>&
    operator=( const DistMatrixBase<T,MD,Star>& A );

    const DistMatrixBase<T,Star,MD>&
    operator=( const DistMatrixBase<T,Star,MD>& A );

    const DistMatrixBase<T,Star,MD>&
    operator=( const DistMatrixBase<T,MR,MC>& A );

    const DistMatrixBase<T,Star,MD>&
    operator=( const DistMatrixBase<T,MR,Star>& A );

    const DistMatrixBase<T,Star,MD>&
    operator=( const DistMatrixBase<T,Star,MC>& A );

    const DistMatrixBase<T,Star,MD>&
    operator=( const DistMatrixBase<T,VC,Star>& A );

    const DistMatrixBase<T,Star,MD>&
    operator=( const DistMatrixBase<T,Star,VC>& A );

    const DistMatrixBase<T,Star,MD>&
    operator=( const DistMatrixBase<T,VR,Star>& A );

    const DistMatrixBase<T,Star,MD>&
    operator=( const DistMatrixBase<T,Star,VR>& A );
    
    const DistMatrixBase<T,Star,MD>&
    operator=( const DistMatrixBase<T,Star,Star>& A );
};

template<typename R>
class DistMatrix<R,Star,MD> : public DistMatrixBase<R,Star,MD>
{
protected:
    typedef DistMatrixBase<R,Star,MD> DMB;

public:
    DistMatrix
    ( const Grid& grid );

    DistMatrix
    ( int height, int width, const Grid& grid );

    DistMatrix
    ( bool constrainedRowAlignment, int rowAlignment, const Grid& grid );

    DistMatrix
    ( int height, int width,
      bool constrainedRowAlignment, int rowAlignment, const Grid& grid );

    DistMatrix
    ( const DistMatrix<R,Star,MD>& A );

    ~DistMatrix();
    
    const DistMatrix<R,Star,MD>&
    operator=( const DistMatrixBase<R,MC,MR>& A );

    const DistMatrix<R,Star,MD>&
    operator=( const DistMatrixBase<R,MC,Star>& A );

    const DistMatrix<R,Star,MD>&
    operator=( const DistMatrixBase<R,Star,MR>& A );

    const DistMatrix<R,Star,MD>&
    operator=( const DistMatrixBase<R,MD,Star>& A );

    const DistMatrix<R,Star,MD>&
    operator=( const DistMatrixBase<R,Star,MD>& A );

    const DistMatrix<R,Star,MD>&
    operator=( const DistMatrixBase<R,MR,MC>& A );

    const DistMatrix<R,Star,MD>&
    operator=( const DistMatrixBase<R,MR,Star>& A );

    const DistMatrix<R,Star,MD>&
    operator=( const DistMatrixBase<R,Star,MC>& A );

    const DistMatrix<R,Star,MD>&
    operator=( const DistMatrixBase<R,VC,Star>& A );

    const DistMatrix<R,Star,MD>&
    operator=( const DistMatrixBase<R,Star,VC>& A );

    const DistMatrix<R,Star,MD>&
    operator=( const DistMatrixBase<R,VR,Star>& A );

    const DistMatrix<R,Star,MD>&
    operator=( const DistMatrixBase<R,Star,VR>& A );
    
    const DistMatrix<R,Star,MD>&
    operator=( const DistMatrixBase<R,Star,Star>& A );

    //------------------------------------------------------------------------//
    // Fulfillments of abstract virtual func's from AbstractDistMatrixBase    //
    //------------------------------------------------------------------------//

    //
    // Non-collective routines
    //

    // (empty)

    //
    // Collective routines
    //

    void SetToRandomHPD();

    //------------------------------------------------------------------------//
    // Routines specific to real [* ,MD] distribution                         //
    //------------------------------------------------------------------------//
    void AlignWithDiag
    ( const DistMatrixBase<R,MC,MR>& A, int offset = 0 );

    void AlignWithDiag
    ( const DistMatrixBase<R,MR,MC>& A, int offset = 0 );

#ifndef WITHOUT_COMPLEX
    void AlignWithDiag
    ( const DistMatrixBase<std::complex<R>,MC,MR>& A, int offset = 0 );

    void AlignWithDiag
    ( const DistMatrixBase<std::complex<R>,MR,MC>& A, int offset = 0 );
#endif
};

#ifndef WITHOUT_COMPLEX
template<typename R>
class DistMatrix<std::complex<R>,Star,MD>
: public DistMatrixBase<std::complex<R>,Star,MD>
{
protected:
    typedef std::complex<R> C;
    typedef DistMatrixBase<C,Star,MD> DMB;

public:
    DistMatrix
    ( const Grid& grid );

    DistMatrix
    ( int height, int width, const Grid& grid );

    DistMatrix
    ( bool constrainedRowAlignment, int rowAlignment, const Grid& grid );

    DistMatrix
    ( int height, int width,
      bool constrainedRowAlignment, int rowAlignment, const Grid& grid );

    DistMatrix
    ( const DistMatrix<C,Star,MD>& A );

    ~DistMatrix();
    
    const DistMatrix<C,Star,MD>&
    operator=( const DistMatrixBase<C,MC,MR>& A );

    const DistMatrix<C,Star,MD>&
    operator=( const DistMatrixBase<C,MC,Star>& A );

    const DistMatrix<C,Star,MD>&
    operator=( const DistMatrixBase<C,Star,MR>& A );

    const DistMatrix<C,Star,MD>&
    operator=( const DistMatrixBase<C,MD,Star>& A );

    const DistMatrix<C,Star,MD>&
    operator=( const DistMatrixBase<C,Star,MD>& A );

    const DistMatrix<C,Star,MD>&
    operator=( const DistMatrixBase<C,MR,MC>& A );

    const DistMatrix<C,Star,MD>&
    operator=( const DistMatrixBase<C,MR,Star>& A );

    const DistMatrix<C,Star,MD>&
    operator=( const DistMatrixBase<C,Star,MC>& A );

    const DistMatrix<C,Star,MD>&
    operator=( const DistMatrixBase<C,VC,Star>& A );

    const DistMatrix<C,Star,MD>&
    operator=( const DistMatrixBase<C,Star,VC>& A );

    const DistMatrix<C,Star,MD>&
    operator=( const DistMatrixBase<C,VR,Star>& A );

    const DistMatrix<C,Star,MD>&
    operator=( const DistMatrixBase<C,Star,VR>& A );
    
    const DistMatrix<C,Star,MD>&
    operator=( const DistMatrixBase<C,Star,Star>& A );

    //------------------------------------------------------------------------//
    // Fulfillments of abstract virtual func's from AbstractDistMatrixBase    //
    //------------------------------------------------------------------------//

    //
    // Non-collective routines
    //

    // (empty)

    //
    // Collective routines
    //

    void SetToRandomHPD();

    //------------------------------------------------------------------------//
    // Fulfillments of abstract virtual func's from AbstractDistMatrix        //
    //------------------------------------------------------------------------//

    //
    // Non-collective routines
    //

    // (empty)

    //
    // Collective routines
    //

    R GetReal( int i, int j ) const;
    R GetImag( int i, int j ) const;
    void SetReal( int i, int j, R u );
    void SetImag( int i, int j, R u );
};
#endif

//----------------------------------------------------------------------------//
// Implementation begins here                                                 //
//----------------------------------------------------------------------------//

//
// DistMatrixBase[* ,MD]
//

template<typename T>
inline
DistMatrixBase<T,Star,MD>::DistMatrixBase
( int height,
  int width,
  bool constrainedRowAlignment,
  int rowAlignment,
  int rowShift,
  const Grid& grid )
: ADM(height,width,false,constrainedRowAlignment,0,rowAlignment,0,rowShift,grid)
{ }

template<typename T>
inline
DistMatrixBase<T,Star,MD>::~DistMatrixBase()
{ }

template<typename T>
inline bool
DistMatrixBase<T,Star,MD>::InDiagonal() const
{ return _inDiagonal; }

//
// Real DistMatrix[* ,MD]
//

template<typename R>
inline
DistMatrix<R,Star,MD>::DistMatrix
( const Grid& grid )
: DMB(0,0,false,0,0,grid)
{
#ifndef RELEASE
    PushCallStack("DistMatrix[* ,MD]::DistMatrix");
#endif
    const int lcm = grid.LCM();
    const int myDiagPath = grid.DiagPath();
    const int ownerDiagPath = grid.DiagPath( 0 );

    if( myDiagPath == ownerDiagPath )
    {
        DMB::_inDiagonal = true;

        const int myDiagPathRank = grid.DiagPathRank();
        const int ownerDiagPathRank = grid.DiagPathRank( 0 );
        DMB::_rowShift = (myDiagPathRank+lcm-ownerDiagPathRank) % lcm;
    }
    else
    {
        DMB::_inDiagonal = false;
    }
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename R>
inline
DistMatrix<R,Star,MD>::DistMatrix
( int height, int width, const Grid& grid )
: DMB(height,width,false,0,0,grid)
{
#ifndef RELEASE
    PushCallStack("DistMatrix[* ,MD]::DistMatrix");
    if( height < 0 || width < 0 )
        throw "Height and width must be non-negative.";
#endif
    const int lcm = grid.LCM();
    const int myDiagPath = grid.DiagPath();
    const int ownerDiagPath = grid.DiagPath( 0 );

    if( myDiagPath == ownerDiagPath )
    {
        DMB::_inDiagonal = true;

        const int myDiagPathRank = grid.DiagPathRank();
        const int ownerDiagPathRank = grid.DiagPathRank( 0 );
        DMB::_rowShift = (myDiagPathRank+lcm-ownerDiagPathRank) % lcm;
        DMB::_localMatrix.ResizeTo
        ( height, utilities::LocalLength(width,DMB::_rowShift,lcm) );
    }
    else
    {
        DMB::_inDiagonal = false;
    }
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename R>
inline
DistMatrix<R,Star,MD>::DistMatrix
( bool constrainedRowAlignment, int rowAlignment, const Grid& grid )
: DMB(0,0,constrainedRowAlignment,rowAlignment,0,grid)
{
#ifndef RELEASE
    PushCallStack("DistMatrix[* ,MD]::DistMatrix");
    if( rowAlignment < 0 || rowAlignment >= grid.Size() )
        throw "alignment for [* ,MD] must be in [0,p-1] (rxc grid,p=r*c).";
#endif
    const int lcm = grid.LCM();
    const int myDiagPath = grid.DiagPath();
    const int ownerDiagPath = grid.DiagPath( rowAlignment );

    if( myDiagPath == ownerDiagPath )
    {
        DMB::_inDiagonal = true;

        const int myDiagPathRank = grid.DiagPathRank();
        const int ownerDiagPathRank = grid.DiagPathRank( rowAlignment );
        DMB::_rowShift = (myDiagPathRank+lcm-ownerDiagPathRank) % lcm;
    }
    else
    {
        DMB::_inDiagonal = false;
    }
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename R>
inline
DistMatrix<R,Star,MD>::DistMatrix
( const DistMatrix<R,Star,MD>& A )
: DMB(A.Height(),A.Width(),A.ConstrainedRowAlignment(),A.RowAlignment(),
      A.RowShift(),A.GetGrid())
{
#ifndef RELEASE
    PushCallStack("DistMatrix[* ,MD]::DistMatrix");
#endif
    DMB::_inDiagonal = A.InDiagonal();

    if( &A != this )
        *this = A;
    else
        throw "Attempted to construct a [* ,MD] with itself.";
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename R>
inline
DistMatrix<R,Star,MD>::~DistMatrix()
{ }

template<typename R>
inline const DistMatrix<R,Star,MD>&
DistMatrix<R,Star,MD>::operator=
( const DistMatrixBase<R,MC,MR>& A )
{ DMB::operator=( A ); return *this; }

template<typename R>
inline const DistMatrix<R,Star,MD>&
DistMatrix<R,Star,MD>::operator=
( const DistMatrixBase<R,MC,Star>& A )
{ DMB::operator=( A ); return *this; }

template<typename R>
inline const DistMatrix<R,Star,MD>&
DistMatrix<R,Star,MD>::operator=
( const DistMatrixBase<R,Star,MR>& A )
{ DMB::operator=( A ); return *this; }

template<typename R>
inline const DistMatrix<R,Star,MD>&
DistMatrix<R,Star,MD>::operator=
( const DistMatrixBase<R,MD,Star>& A )
{ DMB::operator=( A ); return *this; }

template<typename R>
inline const DistMatrix<R,Star,MD>&
DistMatrix<R,Star,MD>::operator=
( const DistMatrixBase<R,Star,MD>& A )
{ DMB::operator=( A ); return *this; }

template<typename R>
inline const DistMatrix<R,Star,MD>&
DistMatrix<R,Star,MD>::operator=
( const DistMatrixBase<R,MR,MC>& A )
{ DMB::operator=( A ); return *this; }

template<typename R>
inline const DistMatrix<R,Star,MD>&
DistMatrix<R,Star,MD>::operator=
( const DistMatrixBase<R,MR,Star>& A )
{ DMB::operator=( A ); return *this; }

template<typename R>
inline const DistMatrix<R,Star,MD>&
DistMatrix<R,Star,MD>::operator=
( const DistMatrixBase<R,Star,MC>& A )
{ DMB::operator=( A ); return *this; }

template<typename R>
inline const DistMatrix<R,Star,MD>&
DistMatrix<R,Star,MD>::operator=
( const DistMatrixBase<R,VC,Star>& A )
{ DMB::operator=( A ); return *this; }

template<typename R>
inline const DistMatrix<R,Star,MD>&
DistMatrix<R,Star,MD>::operator=
( const DistMatrixBase<R,Star,VC>& A )
{ DMB::operator=( A ); return *this; }

template<typename R>
inline const DistMatrix<R,Star,MD>&
DistMatrix<R,Star,MD>::operator=
( const DistMatrixBase<R,VR,Star>& A )
{ DMB::operator=( A ); return *this; }

template<typename R>
inline const DistMatrix<R,Star,MD>&
DistMatrix<R,Star,MD>::operator=
( const DistMatrixBase<R,Star,VR>& A )
{ DMB::operator=( A ); return *this; }

template<typename R>
inline const DistMatrix<R,Star,MD>&
DistMatrix<R,Star,MD>::operator=
( const DistMatrixBase<R,Star,Star>& A )
{ DMB::operator=( A ); return *this; }

//
// Complex DistMatrix[* ,MD]
//

#ifndef WITHOUT_COMPLEX
template<typename R>
inline
DistMatrix<std::complex<R>,Star,MD>::DistMatrix
( const Grid& grid )
: DMB(0,0,false,0,0,grid)
{
#ifndef RELEASE
    PushCallStack("DistMatrix[* ,MD]::DistMatrix");
#endif
    const int lcm = grid.LCM();
    const int myDiagPath = grid.DiagPath();
    const int ownerDiagPath = grid.DiagPath( 0 );

    if( myDiagPath == ownerDiagPath )
    {
        DMB::_inDiagonal = true;

        const int myDiagPathRank = grid.DiagPathRank();
        const int ownerDiagPathRank = grid.DiagPathRank( 0 );
        DMB::_rowShift = (myDiagPathRank+lcm-ownerDiagPathRank) % lcm;
    }
    else
    {
        DMB::_inDiagonal = false;
    }
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename R>
inline
DistMatrix<std::complex<R>,Star,MD>::DistMatrix
( int height, int width, const Grid& grid )
: DMB(height,width,false,0,0,grid)
{
#ifndef RELEASE
    PushCallStack("DistMatrix[* ,MD]::DistMatrix");
    if( height < 0 || width < 0 )
        throw "Height and width must be non-negative.";
#endif
    const int lcm = grid.LCM();
    const int myDiagPath = grid.DiagPath();
    const int ownerDiagPath = grid.DiagPath( 0 );

    if( myDiagPath == ownerDiagPath )
    {
        DMB::_inDiagonal = true;

        const int myDiagPathRank = grid.DiagPathRank();
        const int ownerDiagPathRank = grid.DiagPathRank( 0 );
        DMB::_rowShift = (myDiagPathRank+lcm-ownerDiagPathRank) % lcm;
        DMB::_localMatrix.ResizeTo
        ( height, utilities::LocalLength(width,DMB::_rowShift,lcm) );
    }
    else
    {
        DMB::_inDiagonal = false;
    }
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename R>
inline
DistMatrix<std::complex<R>,Star,MD>::DistMatrix
( bool constrainedRowAlignment, int rowAlignment, const Grid& grid )
: DMB(0,0,constrainedRowAlignment,rowAlignment,0,grid)
{
#ifndef RELEASE
    PushCallStack("DistMatrix[* ,MD]::DistMatrix");
    if( rowAlignment < 0 || rowAlignment >= grid.Size() )
        throw "alignment for [* ,MD] must be in [0,p-1] (rxc grid,p=r*c).";
#endif
    const int lcm = grid.LCM();
    const int myDiagPath = grid.DiagPath();
    const int ownerDiagPath = grid.DiagPath( rowAlignment );

    if( myDiagPath == ownerDiagPath )
    {
        DMB::_inDiagonal = true;

        const int myDiagPathRank = grid.DiagPathRank();
        const int ownerDiagPathRank = grid.DiagPathRank( rowAlignment );
        DMB::_rowShift = (myDiagPathRank+lcm-ownerDiagPathRank) % lcm;
    }
    else
    {
        DMB::_inDiagonal = false;
    }
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename R>
inline
DistMatrix<std::complex<R>,Star,MD>::DistMatrix
( const DistMatrix<std::complex<R>,Star,MD>& A )
: DMB(A.Height(),A.Width(),A.ConstrainedRowAlignment(),A.RowAlignment(),
      A.RowShift(),A.GetGrid())
{
#ifndef RELEASE
    PushCallStack("DistMatrix[* ,MD]::DistMatrix");
#endif
    DMB::_inDiagonal = A.InDiagonal();

    if( &A != this )
        *this = A;
    else
        throw "Attempted to construct a [* ,MD] with itself.";
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename R>
inline
DistMatrix<std::complex<R>,Star,MD>::~DistMatrix()
{ }

template<typename R>
inline const DistMatrix<std::complex<R>,Star,MD>&
DistMatrix<std::complex<R>,Star,MD>::operator=
( const DistMatrixBase<std::complex<R>,MC,MR>& A )
{ DMB::operator=( A ); return *this; }

template<typename R>
inline const DistMatrix<std::complex<R>,Star,MD>&
DistMatrix<std::complex<R>,Star,MD>::operator=
( const DistMatrixBase<std::complex<R>,MC,Star>& A )
{ DMB::operator=( A ); return *this; }

template<typename R>
inline const DistMatrix<std::complex<R>,Star,MD>&
DistMatrix<std::complex<R>,Star,MD>::operator=
( const DistMatrixBase<std::complex<R>,Star,MR>& A )
{ DMB::operator=( A ); return *this; }

template<typename R>
inline const DistMatrix<std::complex<R>,Star,MD>&
DistMatrix<std::complex<R>,Star,MD>::operator=
( const DistMatrixBase<std::complex<R>,MD,Star>& A )
{ DMB::operator=( A ); return *this; }

template<typename R>
inline const DistMatrix<std::complex<R>,Star,MD>&
DistMatrix<std::complex<R>,Star,MD>::operator=
( const DistMatrixBase<std::complex<R>,Star,MD>& A )
{ DMB::operator=( A ); return *this; }

template<typename R>
inline const DistMatrix<std::complex<R>,Star,MD>&
DistMatrix<std::complex<R>,Star,MD>::operator=
( const DistMatrixBase<std::complex<R>,MR,MC>& A )
{ DMB::operator=( A ); return *this; }

template<typename R>
inline const DistMatrix<std::complex<R>,Star,MD>&
DistMatrix<std::complex<R>,Star,MD>::operator=
( const DistMatrixBase<std::complex<R>,MR,Star>& A )
{ DMB::operator=( A ); return *this; }

template<typename R>
inline const DistMatrix<std::complex<R>,Star,MD>&
DistMatrix<std::complex<R>,Star,MD>::operator=
( const DistMatrixBase<std::complex<R>,Star,MC>& A )
{ DMB::operator=( A ); return *this; }

template<typename R>
inline const DistMatrix<std::complex<R>,Star,MD>&
DistMatrix<std::complex<R>,Star,MD>::operator=
( const DistMatrixBase<std::complex<R>,VC,Star>& A )
{ DMB::operator=( A ); return *this; }

template<typename R>
inline const DistMatrix<std::complex<R>,Star,MD>&
DistMatrix<std::complex<R>,Star,MD>::operator=
( const DistMatrixBase<std::complex<R>,Star,VC>& A )
{ DMB::operator=( A ); return *this; }

template<typename R>
inline const DistMatrix<std::complex<R>,Star,MD>&
DistMatrix<std::complex<R>,Star,MD>::operator=
( const DistMatrixBase<std::complex<R>,VR,Star>& A )
{ DMB::operator=( A ); return *this; }

template<typename R>
inline const DistMatrix<std::complex<R>,Star,MD>&
DistMatrix<std::complex<R>,Star,MD>::operator=
( const DistMatrixBase<std::complex<R>,Star,VR>& A )
{ DMB::operator=( A ); return *this; }

template<typename R>
inline const DistMatrix<std::complex<R>,Star,MD>&
DistMatrix<std::complex<R>,Star,MD>::operator=
( const DistMatrixBase<std::complex<R>,Star,Star>& A )
{ DMB::operator=( A ); return *this; }
#endif // WITHOUT_COMPLEX

} // elemental

#endif /* ELEMENTAL_DIST_MATRIX_STAR_MD_HPP */

