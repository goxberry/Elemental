/*
   This file is part of elemental, a library for distributed-memory dense 
   linear algebra.

   Copyright (C) 2009-2010 Jack Poulson <jack.poulson@gmail.com>

   This program is released under the terms of the license contained in the 
   file LICENSE.
*/
#include "elemental/dist_matrix.hpp"
using namespace std;
using namespace elemental;
using namespace elemental::utilities;
using namespace elemental::wrappers::mpi;

//----------------------------------------------------------------------------//
// DistMatrixBase                                                             //
//----------------------------------------------------------------------------//

template<typename T>
void
elemental::DistMatrixBase<T,Star,MC>::Print( const string& s ) const
{
#ifndef RELEASE
    PushCallStack("[* ,MC]::Print");
#endif
    const Grid& grid = this->GetGrid();
    if( grid.VCRank() == 0 && s != "" )
        cout << s << endl;

    const int height     = this->Height();
    const int width      = this->Width();
    const int localWidth = this->LocalWidth();
    const int r          = grid.Height();
    const int rowShift   = this->RowShift();

    if( height == 0 || width == 0 )
    {
#ifndef RELEASE
        PopCallStack();
#endif
        return;
    }

    // Only one process col needs to participate
    if( grid.MRRank() == 0 )
    {
        T* sendBuf = new T[height*width];
        for( int i=0; i<height*width; ++i )
            sendBuf[i] = (T)0;
        for( int i=0; i<height; ++i )
            for( int j=0; j<localWidth; ++j )
                sendBuf[i+(rowShift+j*r)*height] = this->LocalEntry(i,j);

        // If we are the root, fill the receive buffer
        T* recvBuf = 0;
        if( grid.MCRank() == 0 )
        {
            recvBuf = new T[height*width];
            for( int i=0; i<height*width; ++i )
                recvBuf[i] = (T)0;
        }

        // Sum the contributions and send to the root
        Reduce
        ( sendBuf, recvBuf, height*width, MPI_SUM, 0, grid.MCComm() );
        delete[] sendBuf;

        if( grid.MCRank() == 0 )
        {
            // Print the data
            for( int i=0; i<height; ++i )
            {
                for( int j=0; j<width; ++j )
                    cout << recvBuf[i+j*height] << " ";
                cout << endl;
            }
            cout << endl;
            delete[] recvBuf;
        }
    }
    Barrier( grid.VCComm() );

#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T>
void
elemental::DistMatrixBase<T,Star,MC>::AlignWith
( const DistMatrixBase<T,MR,MC>& A )
{
#ifndef RELEASE
    PushCallStack("[* ,MC]::AlignWith([MR,MC])");
    this->AssertFreeRowAlignment();
    this->AssertSameGrid( A );
#endif
    this->_rowAlignment = A.RowAlignment();
    this->_rowShift = A.RowShift();
    this->_constrainedRowAlignment = true;
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T>
void
elemental::DistMatrixBase<T,Star,MC>::AlignWith
( const DistMatrixBase<T,Star,MC>& A )
{
#ifndef RELEASE
    PushCallStack("[* ,MC]::AlignWith([* ,MC])");
    this->AssertFreeRowAlignment();
    this->AssertSameGrid( A );
#endif
    this->_rowAlignment = A.RowAlignment();
    this->_rowShift = A.RowShift();
    this->_constrainedRowAlignment = true;
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T>
void
elemental::DistMatrixBase<T,Star,MC>::AlignWith
( const DistMatrixBase<T,MC,MR>& A )
{
#ifndef RELEASE
    PushCallStack("[* ,MC]::AlignWith([MC,MR])");
    this->AssertFreeRowAlignment();
    this->AssertSameGrid( A );
#endif
    this->_rowAlignment = A.ColAlignment();
    this->_rowShift = A.ColShift();
    this->_constrainedRowAlignment = true;
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T>
void
elemental::DistMatrixBase<T,Star,MC>::AlignWith
( const DistMatrixBase<T,MC,Star>& A )
{
#ifndef RELEASE
    PushCallStack("[* ,MC]::AlignWith([MC,* ])");
    this->AssertFreeRowAlignment();
    this->AssertSameGrid( A );
#endif
    this->_rowAlignment = A.ColAlignment();
    this->_rowShift = A.ColShift();
    this->_constrainedRowAlignment = true;
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T>
void
elemental::DistMatrixBase<T,Star,MC>::AlignRowsWith
( const DistMatrixBase<T,MC,MR>& A )
{ AlignWith( A ); }

template<typename T>
void
elemental::DistMatrixBase<T,Star,MC>::AlignRowsWith
( const DistMatrixBase<T,MC,Star>& A )
{ AlignWith( A ); }

template<typename T>
void
elemental::DistMatrixBase<T,Star,MC>::AlignRowsWith
( const DistMatrixBase<T,Star,MC>& A )
{ AlignWith( A ); }

template<typename T>
void
elemental::DistMatrixBase<T,Star,MC>::AlignRowsWith
( const DistMatrixBase<T,MR,MC>& A )
{ AlignWith( A ); }

template<typename T>
void
elemental::DistMatrixBase<T,Star,MC>::View
( DistMatrixBase<T,Star,MC>& A )
{
#ifndef RELEASE
    PushCallStack("[* ,MC]::View");
    this->AssertFreeRowAlignment();
    this->AssertNotStoringData();
    if( this->Viewing() )
        this->AssertSameGrid( A );
#endif
    this->_height = A.Height();
    this->_width = A.Width();
    this->_rowAlignment = A.RowAlignment();
    this->_rowShift = A.RowShift();
    this->_localMatrix.View( A.LocalMatrix() );
    this->_viewing = true;
    this->_lockedView = false;
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T>
void
elemental::DistMatrixBase<T,Star,MC>::LockedView
( const DistMatrixBase<T,Star,MC>& A )
{
#ifndef RELEASE 
    PushCallStack("[* ,MC]::LockedView");
    this->AssertFreeRowAlignment();
    this->AssertNotStoringData();
    this->AssertSameGrid( A );
#endif
    this->_height = A.Height();
    this->_width = A.Width();
    this->_rowAlignment = A.RowAlignment();
    this->_rowShift = A.RowShift();
    this->_localMatrix.LockedView( A.LockedLocalMatrix() );
    this->_viewing = true;
    this->_lockedView = true;
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T>
void
elemental::DistMatrixBase<T,Star,MC>::View
( DistMatrixBase<T,Star,MC>& A,
  int i, int j, int height, int width )
{
#ifndef RELEASE
    PushCallStack("[* ,MC]::View");
    this->AssertFreeRowAlignment();
    this->AssertNotStoringData();
    this->AssertSameGrid( A );
    this->AssertValidSubmatrix( A, i, j, height, width );
#endif
    this->_height = height;
    this->_width = width;
    {
        const Grid& grid = this->GetGrid();
        const int r   = grid.Height();
        const int row = grid.MCRank();

        this->_rowAlignment = (A.RowAlignment()+j) % r;
        this->_rowShift = Shift( row, this->RowAlignment(), r ); 

        const int localWidthBefore = LocalLength( j, A.RowShift(), r );
        const int localWidth = LocalLength( width, this->RowShift(), r );

        this->_localMatrix.View
        ( A.LocalMatrix(), i, localWidthBefore, height, localWidth );
    }
    this->_viewing = true;
    this->_lockedView = false;
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T>
void
elemental::DistMatrixBase<T,Star,MC>::LockedView
( const DistMatrixBase<T,Star,MC>& A,
  int i, int j, int height, int width )
{
#ifndef RELEASE
    PushCallStack("[* ,MC]::LockedView");
    this->AssertFreeRowAlignment();
    this->AssertNotStoringData();
    this->AssertSameGrid( A );
    this->AssertValidSubmatrix( A, i, j, height, width );
#endif
    this->_height = height;
    this->_width = width;
    {
        const Grid& grid = this->GetGrid();
        const int r   = grid.Height();
        const int row = grid.MCRank();

        this->_rowAlignment = (A.RowAlignment()+j) % r;
        this->_rowShift = Shift( row, this->RowAlignment(), r );

        const int localWidthBefore = LocalLength( j, A.RowShift(), r );
        const int localWidth = LocalLength( width, this->RowShift(), r );

        this->_localMatrix.LockedView
        ( A.LockedLocalMatrix(), i, localWidthBefore, height, localWidth );
    }
    this->_viewing = true;
    this->_lockedView = true;
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T>
void
elemental::DistMatrixBase<T,Star,MC>::View1x2
( DistMatrixBase<T,Star,MC>& AL, DistMatrixBase<T,Star,MC>& AR )
{
#ifndef RELEASE
    PushCallStack("[* ,MC]::View1x2");
    this->AssertFreeRowAlignment();
    this->AssertNotStoringData();
    this->AssertSameGrid( AL );
    this->AssertSameGrid( AR );
    this->AssertConforming1x2( AL, AR );
#endif
    this->_height = AL.Height();
    this->_width = AL.Width() + AR.Width();
    this->_rowAlignment = AL.RowAlignment();
    this->_rowShift = AL.RowShift();
    this->_localMatrix.View1x2( AL.LocalMatrix(), AR.LocalMatrix() );
    this->_viewing = true;
    this->_lockedView = false;
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T>
void
elemental::DistMatrixBase<T,Star,MC>::LockedView1x2
( const DistMatrixBase<T,Star,MC>& AL, const DistMatrixBase<T,Star,MC>& AR )
{
#ifndef RELEASE
    PushCallStack("[* ,MC]::LockedView1x2");
    this->AssertFreeRowAlignment();
    this->AssertNotStoringData();
    this->AssertSameGrid( AL );
    this->AssertSameGrid( AR );
    this->AssertConforming1x2( AL, AR );
#endif
    this->_height = AL.Height();
    this->_width = AL.Width() + AR.Width();
    this->_rowAlignment = AL.RowAlignment();
    this->_rowShift = AL.RowShift();
    this->_localMatrix.LockedView1x2
    ( AL.LockedLocalMatrix(), AR.LockedLocalMatrix() );
    this->_viewing = true;
    this->_lockedView = true;
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T>
void
elemental::DistMatrixBase<T,Star,MC>::View2x1
( DistMatrixBase<T,Star,MC>& AT,
  DistMatrixBase<T,Star,MC>& AB )
{
#ifndef RELEASE
    PushCallStack("[* ,MC]::View2x1");
    this->AssertFreeRowAlignment();
    this->AssertNotStoringData();
    this->AssertSameGrid( AT );
    this->AssertSameGrid( AB );
    this->AssertConforming2x1( AT, AB );
#endif
    this->_height = AT.Height() + AB.Height();
    this->_width = AT.Width();
    this->_rowAlignment = AT.RowAlignment();
    this->_rowShift = AT.RowShift();
    this->_localMatrix.View2x1
    ( AT.LocalMatrix(),
      AB.LocalMatrix() );
    this->_viewing = true;
    this->_lockedView = false;
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T>
void
elemental::DistMatrixBase<T,Star,MC>::LockedView2x1
( const DistMatrixBase<T,Star,MC>& AT,
  const DistMatrixBase<T,Star,MC>& AB )
{
#ifndef RELEASE
    PushCallStack("[* ,MC]::LockedView2x1");
    this->AssertFreeRowAlignment();
    this->AssertNotStoringData();
    this->AssertSameGrid( AT );
    this->AssertSameGrid( AB );
    this->AssertConforming2x1( AT, AB );
#endif
    this->_height = AT.Height() + AB.Height();
    this->_width = AT.Width();
    this->_rowAlignment = AT.RowAlignment();
    this->_rowShift = AT.RowShift();
    this->_localMatrix.LockedView2x1
    ( AT.LockedLocalMatrix(),
      AB.LockedLocalMatrix() );
    this->_viewing = true;
    this->_lockedView = true;
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T>
void
elemental::DistMatrixBase<T,Star,MC>::View2x2
( DistMatrixBase<T,Star,MC>& ATL,
  DistMatrixBase<T,Star,MC>& ATR,
  DistMatrixBase<T,Star,MC>& ABL,
  DistMatrixBase<T,Star,MC>& ABR )
{
#ifndef RELEASE
    PushCallStack("[* ,MC]::View2x2");
    this->AssertFreeRowAlignment();
    this->AssertNotStoringData();
    this->AssertSameGrid( ATL );
    this->AssertSameGrid( ATR );
    this->AssertSameGrid( ABL );
    this->AssertSameGrid( ABR );
    this->AssertConforming2x2( ATL, ATR, ABL, ABR );
#endif
    this->_height = ATL.Height() + ABL.Height();
    this->_width = ATL.Width() + ATR.Width();
    this->_rowAlignment = ATL.RowAlignment();
    this->_rowShift = ATL.RowShift();
    this->_localMatrix.View2x2
    ( ATL.LocalMatrix(), ATR.LocalMatrix(),
      ABL.LocalMatrix(), ABR.LocalMatrix() );
    this->_viewing = true;
    this->_lockedView = false;
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T>
void
elemental::DistMatrixBase<T,Star,MC>::LockedView2x2
( const DistMatrixBase<T,Star,MC>& ATL,
  const DistMatrixBase<T,Star,MC>& ATR,
  const DistMatrixBase<T,Star,MC>& ABL,
  const DistMatrixBase<T,Star,MC>& ABR )
{
#ifndef RELEASE
    PushCallStack("[* ,MC]::LockedView2x2");
    this->AssertFreeRowAlignment();
    this->AssertNotStoringData();
    this->AssertSameGrid( ATL );
    this->AssertSameGrid( ATR );
    this->AssertSameGrid( ABL );
    this->AssertSameGrid( ABR );
    this->AssertConforming2x2( ATL, ATR, ABL, ABR );
#endif
    this->_height = ATL.Height() + ABL.Height();
    this->_width = ATL.Width() + ATR.Width();
    this->_rowAlignment = ATL.RowAlignment();
    this->_rowShift = ATL.RowShift();
    this->_localMatrix.LockedView2x2
    ( ATL.LockedLocalMatrix(), ATR.LockedLocalMatrix(),
      ABL.LockedLocalMatrix(), ABR.LockedLocalMatrix() );
    this->_viewing = true;
    this->_lockedView = true;
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T>
void
elemental::DistMatrixBase<T,Star,MC>::ResizeTo
( int height, int width )
{
#ifndef RELEASE
    PushCallStack("[* ,MC]::ResizeTo");
    this->AssertNotLockedView();
    if( height < 0 || width < 0 )
        throw "Height and width must be non-negative.";
#endif
    this->_height = height;
    this->_width = width;
    this->_localMatrix.ResizeTo
    ( height, LocalLength(width,this->RowShift(),this->GetGrid().Height()) );
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T>
T
elemental::DistMatrixBase<T,Star,MC>::Get
( int i, int j ) const
{
#ifndef RELEASE
    PushCallStack("[* ,MR]::Get");
    this->AssertValidEntry( i, j );
#endif
    // We will determine the owner row of entry (i,j) and broadcast from that
    // row within each process column
    const Grid& grid = this->GetGrid();
    const int ownerRow = (j + this->RowAlignment()) % grid.Height();

    T u;
    if( grid.MCRank() == ownerRow )
    {
        const int jLoc = (j-this->RowShift()) / grid.Height();
        u = this->LocalEntry(i,jLoc);
    }
    Broadcast( &u, 1, ownerRow, grid.MCComm() );

#ifndef RELEASE
    PopCallStack();
#endif
    return u;
}

template<typename T>
void
elemental::DistMatrixBase<T,Star,MC>::Set
( int i, int j, T u )
{
#ifndef RELEASE
    PushCallStack("[* ,MC]::Set");
    this->AssertValidEntry( i, j );
#endif
    const Grid& grid = this->GetGrid();
    const int ownerRow = (j + this->RowAlignment()) % grid.Height();

    if( grid.MCRank() == ownerRow )
    {
        const int jLoc = (j-this->RowShift()) / grid.Height();
        this->LocalEntry(i,jLoc) = u;
    }
#ifndef RELEASE
    PopCallStack();
#endif
}

//
// Utility functions, e.g., SetToIdentity and MakeTrapezoidal
//

template<typename T>
void
elemental::DistMatrixBase<T,Star,MC>::MakeTrapezoidal
( Side side, Shape shape, int offset )
{
#ifndef RELEASE
    PushCallStack("[* ,MC]::MakeTrapezoidal");
    this->AssertNotLockedView();
#endif
    const int height = this->Height();
    const int width = this->Width();
    const int localWidth = this->LocalWidth();
    const int r = this->GetGrid().Height();
    const int rowShift = this->RowShift();

    if( shape == Lower )
    {
        for( int jLoc=0; jLoc<localWidth; ++jLoc )
        {
            const int j = rowShift + jLoc*r;
            int firstNonzero_i;
            if( side == Left )
                firstNonzero_i = max(j-offset,0);
            else
                firstNonzero_i = max(j-offset+height-width,0);

            const int boundary = min(height,firstNonzero_i);
            for( int i=0; i<boundary; ++i )
                this->LocalEntry(i,jLoc) = (T)0;
        }
    }
    else
    {
        for( int jLoc=0; jLoc<localWidth; ++jLoc )
        {
            const int j = rowShift + jLoc*r;
            int firstZero_i;
            if( side == Left )
                firstZero_i = max(j-offset+1,0);
            else
                firstZero_i = max(j-offset+height-width+1,0);
            for( int i=firstZero_i; i<height; ++i )
                this->LocalEntry(i,jLoc) = (T)0;
        }
    }
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T>
void
elemental::DistMatrixBase<T,Star,MC>::SetToIdentity()
{
#ifndef RELEASE
    PushCallStack("[* ,MC]::SetToIdentity");
    this->AssertNotLockedView();
#endif
    const int height = this->Height();
    const int localWidth = this->LocalWidth();
    const int r = this->GetGrid().Height();
    const int rowShift = this->RowShift();

    this->SetToZero();
    for( int jLoc=0; jLoc<localWidth; ++jLoc )
    {
        const int j = rowShift + jLoc*r;
        if( j < height )
            this->LocalEntry(j,jLoc) = (T)1;
    }
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T>
void
elemental::DistMatrixBase<T,Star,MC>::SetToRandom()
{
#ifndef RELEASE
    PushCallStack("[* ,MC]::SetToRandom");
    this->AssertNotLockedView();
#endif
    const Grid& grid = this->GetGrid();
    const int height     = this->Height();    
    const int localWidth = this->LocalWidth();
    const int bufSize    = height*localWidth;

    this->_auxMemory.Require( bufSize );

    // Create a random matrix on process column 0, then broadcast
    T* buffer = this->_auxMemory.Buffer();
    if( grid.MRRank() == 0 )
    {
        for( int j=0; j<localWidth; ++j )
            for( int i=0; i<height; ++i )
                buffer[i+j*height] = Random<T>();
    }
    Broadcast( buffer, bufSize, 0, grid.MRComm() );

    // Unpack
    for( int j=0; j<localWidth; ++j )
        for( int i=0; i<height; ++i )
            this->LocalEntry(i,j) = buffer[i+j*height];

    this->_auxMemory.Release();
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T>
void
elemental::DistMatrixBase<T,Star,MC>::SumOverRow()
{
#ifndef RELEASE
    PushCallStack("[* ,MC]::SumOverRow");
    this->AssertNotLockedView();
#endif
    const int localHeight = this->LocalHeight();
    const int localWidth = this->LocalWidth();
    const int localSize = max( localHeight*localWidth, MinCollectContrib );

    this->_auxMemory.Require( 2*localSize );
    T* buffer = this->_auxMemory.Buffer();
    T* sendBuf = &buffer[0];
    T* recvBuf = &buffer[localSize];

    // Pack
    for( int j=0; j<localWidth; ++j )
        for( int i=0; i<localHeight; ++i )
            sendBuf[i+j*localHeight] = this->LocalEntry(i,j);

    // AllReduce sum
    AllReduce
    ( sendBuf, recvBuf, localSize, MPI_SUM, this->GetGrid().MRComm() );

    // Unpack
    for( int j=0; j<localWidth; ++j )
        for( int i=0; i<localHeight; ++i )
            this->LocalEntry(i,j) = recvBuf[i+j*localHeight];

    this->_auxMemory.Release();
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T>
void
elemental::DistMatrixBase<T,Star,MC>::ConjugateTransposeFrom
( const DistMatrixBase<T,VC,Star>& A )
{ 
#ifndef RELEASE
    PushCallStack("[* ,MC]::ConjugateTransposeFrom");
    this->AssertNotLockedView();
    this->AssertSameGrid( A );
    if( this->Viewing() )
        this->AssertSameSizeAsTranspose( A );
#endif
    const Grid& grid = this->GetGrid();
    if( !this->Viewing() )
    {
        if( !this->ConstrainedRowAlignment() )
        {
            this->_rowAlignment = A.ColAlignment() % grid.Height();
            this->_rowShift = 
                Shift( grid.MCRank(), this->RowAlignment(), grid.Height() );
        }
        this->ResizeTo( A.Width(), A.Height() );
    }

    if( this->RowAlignment() == A.ColAlignment() % grid.Height() )
    {
        const int r = grid.Height();
        const int c = grid.Width();
        const int p = grid.Size();
        const int row = grid.MCRank();

        const int height = this->Height();
        const int width = this->Width();
        const int localHeightOfA = A.LocalHeight();
        const int maxLocalHeightOfA = MaxLocalLength(width,p);

        const int portionSize = 
            max(height*maxLocalHeightOfA,MinCollectContrib);

        this->_auxMemory.Require( (c+1)*portionSize );

        T* buffer = this->_auxMemory.Buffer();
        T* originalData = &buffer[0];
        T* gatheredData = &buffer[portionSize];

        // Pack
        for( int j=0; j<localHeightOfA; ++j )
            for( int i=0; i<height; ++i )
                originalData[i+j*height] = Conj( A.LocalEntry(j,i) );

        // Communicate
        AllGather
        ( originalData, portionSize,
          gatheredData, portionSize, grid.MRComm() );

        // Unpack
        const int rowShift = this->RowShift();
        const int colAlignmentOfA = A.ColAlignment();
        for( int k=0; k<c; ++k )
        {
            const T* data = &gatheredData[k*portionSize];

            const int colShiftOfA = Shift( row+k*r, colAlignmentOfA, p );
            const int rowOffset = (colShiftOfA-rowShift) / r;
            const int localWidth = LocalLength( width, colShiftOfA, p );

            for( int j=0; j<localWidth; ++j )
                for( int i=0; i<height; ++i )
                    this->LocalEntry(i,rowOffset+j*c) = data[i+j*height];
        }

        this->_auxMemory.Release();
    }
    else
    {
#ifndef RELEASE
        if( grid.VCRank() == 0 )
            cout << "Unaligned [* ,MC]::ConjugateTransposeFrom." << endl;
#endif
        const int r = grid.Height();
        const int c = grid.Width();
        const int p = grid.Size();
        const int row = grid.MCRank();
        const int rank = grid.VCRank();

        // Perform the SendRecv to make A have the same rowAlignment
        const int rowAlignment = this->RowAlignment();
        const int colAlignmentOfA = A.ColAlignment();
        const int rowShift = this->RowShift();

        const int sendRank = (rank+p+rowAlignment-colAlignmentOfA) % p;
        const int recvRank = (rank+p+colAlignmentOfA-rowAlignment) % p;

        const int height = this->Height();
        const int width = this->Width();
        const int localHeightOfA = A.LocalHeight();
        const int maxLocalHeightOfA = MaxLocalLength(width,p);

        const int portionSize = 
            max(height*maxLocalHeightOfA,MinCollectContrib);

        this->_auxMemory.Require( (c+1)*portionSize );

        T* buffer = this->_auxMemory.Buffer();
        T* firstBuffer = &buffer[0];
        T* secondBuffer = &buffer[portionSize];

        // Pack
        for( int j=0; j<localHeightOfA; ++j )
            for( int i=0; i<height; ++i )
                secondBuffer[i+j*height] = Conj( A.LocalEntry(j,i) );

        // Perform the SendRecv: puts the new data into the first buffer
        SendRecv
        ( secondBuffer, portionSize, sendRank, 0,
          firstBuffer,  portionSize, recvRank, 0, grid.VCComm() );

        // Use the SendRecv as input to the AllGather
        AllGather
        ( firstBuffer,  portionSize,
          secondBuffer, portionSize, grid.MRComm() );

        // Unpack
        for( int k=0; k<c; ++k )
        {
            const T* data = &secondBuffer[k*portionSize];

            const int colShiftOfA = Shift(row+r*k,rowAlignment,p);
            const int rowOffset = (colShiftOfA-rowShift) / r;
            const int localWidth = LocalLength( width, colShiftOfA, p );
            
            for( int j=0; j<localWidth; ++j )
                for( int i=0; i<height; ++i )
                    this->LocalEntry(i,rowOffset+j*c) = data[i+j*height];
        }

        this->_auxMemory.Release();
    }
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T>
void
elemental::DistMatrixBase<T,Star,MC>::TransposeFrom
( const DistMatrixBase<T,VC,Star>& A )
{ 
#ifndef RELEASE
    PushCallStack("[* ,MC]::TransposeFrom");
    this->AssertNotLockedView();
    this->AssertSameGrid( A );
    if( this->Viewing() )
        this->AssertSameSizeAsTranspose( A );
#endif
    const Grid& grid = this->GetGrid();
    if( !this->Viewing() )
    {
        if( !this->ConstrainedRowAlignment() )
        {
            this->_rowAlignment = A.ColAlignment() % grid.Height();
            this->_rowShift = 
                Shift( grid.MCRank(), this->RowAlignment(), grid.Height() );
        }
        this->ResizeTo( A.Width(), A.Height() );
    }

    if( this->RowAlignment() == A.ColAlignment() % grid.Height() )
    {
        const int r = grid.Height();
        const int c = grid.Width();
        const int p = grid.Size();
        const int row = grid.MCRank();

        const int height = this->Height();
        const int width = this->Width();
        const int localHeightOfA = A.LocalHeight();
        const int maxLocalHeightOfA = MaxLocalLength(width,p);

        const int portionSize = 
            max(height*maxLocalHeightOfA,MinCollectContrib);

        this->_auxMemory.Require( (c+1)*portionSize );

        T* buffer = this->_auxMemory.Buffer();
        T* originalData = &buffer[0];
        T* gatheredData = &buffer[portionSize];

        // Pack
        for( int j=0; j<localHeightOfA; ++j )
            for( int i=0; i<height; ++i )
                originalData[i+j*height] = A.LocalEntry(j,i);

        // Communicate
        AllGather
        ( originalData, portionSize,
          gatheredData, portionSize, grid.MRComm() );

        // Unpack
        const int rowShift = this->RowShift();
        const int colAlignmentOfA = A.ColAlignment();
        for( int k=0; k<c; ++k )
        {
            const T* data = &gatheredData[k*portionSize];

            const int colShiftOfA = Shift( row+k*r, colAlignmentOfA, p );
            const int rowOffset = (colShiftOfA-rowShift) / r;
            const int localWidth = LocalLength( width, colShiftOfA, p );

            for( int j=0; j<localWidth; ++j )
                for( int i=0; i<height; ++i )
                    this->LocalEntry(i,rowOffset+j*c) = data[i+j*height];
        }

        this->_auxMemory.Release();
    }
    else
    {
#ifndef RELEASE
        if( grid.VCRank() == 0 )
            cout << "Unaligned [* ,MC]::TransposeFrom." << endl;
#endif
        const int r = grid.Height();
        const int c = grid.Width();
        const int p = grid.Size();
        const int row = grid.MCRank();
        const int rank = grid.VCRank();

        // Perform the SendRecv to make A have the same rowAlignment
        const int rowAlignment = this->RowAlignment();
        const int colAlignmentOfA = A.ColAlignment();
        const int rowShift = this->RowShift();

        const int sendRank = (rank+p+rowAlignment-colAlignmentOfA) % p;
        const int recvRank = (rank+p+colAlignmentOfA-rowAlignment) % p;

        const int height = this->Height();
        const int width = this->Width();
        const int localHeightOfA = A.LocalHeight();
        const int maxLocalHeightOfA = MaxLocalLength(width,p);

        const int portionSize = 
            max(height*maxLocalHeightOfA,MinCollectContrib);

        this->_auxMemory.Require( (c+1)*portionSize );

        T* buffer = this->_auxMemory.Buffer();
        T* firstBuffer = &buffer[0];
        T* secondBuffer = &buffer[portionSize];

        // Pack
        for( int j=0; j<localHeightOfA; ++j )
            for( int i=0; i<height; ++i )
                secondBuffer[i+j*height] = A.LocalEntry(j,i);

        // Perform the SendRecv: puts the new data into the first buffer
        SendRecv
        ( secondBuffer, portionSize, sendRank, 0,
          firstBuffer,  portionSize, recvRank, 0, grid.VCComm() );

        // Use the SendRecv as input to the AllGather
        AllGather
        ( firstBuffer,  portionSize,
          secondBuffer, portionSize, grid.MRComm() );

        // Unpack
        for( int k=0; k<c; ++k )
        {
            const T* data = &secondBuffer[k*portionSize];

            const int colShiftOfA = Shift(row+r*k,rowAlignment,p);
            const int rowOffset = (colShiftOfA-rowShift) / r;
            const int localWidth = LocalLength( width, colShiftOfA, p );
            
            for( int j=0; j<localWidth; ++j )
                for( int i=0; i<height; ++i )
                    this->LocalEntry(i,rowOffset+j*c) = data[i+j*height];
        }

        this->_auxMemory.Release();
    }
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T>
const DistMatrixBase<T,Star,MC>&
elemental::DistMatrixBase<T,Star,MC>::operator=
( const DistMatrixBase<T,MC,MR>& A )
{ 
#ifndef RELEASE
    PushCallStack("[* ,MC] = [MC,MR]");
    this->AssertNotLockedView();
    this->AssertSameGrid( A );
    if( this->Viewing() )
        this->AssertSameSize( A );
#endif
    const Grid& grid = this->GetGrid();
    auto_ptr< DistMatrix<T,Star,VR> > A_Star_VR
    ( new DistMatrix<T,Star,VR>(grid) );
    *A_Star_VR = A;

    auto_ptr< DistMatrix<T,Star,VC> > A_Star_VC
    ( new DistMatrix<T,Star,VC>(true,this->RowAlignment(),grid) );
    *A_Star_VC = *A_Star_VR;
    delete A_Star_VR.release(); // lowers memory highwater

    *this = *A_Star_VC;
#ifndef RELEASE
    PopCallStack();
#endif
    return *this;
}

template<typename T>
const DistMatrixBase<T,Star,MC>&
elemental::DistMatrixBase<T,Star,MC>::operator=
( const DistMatrixBase<T,MC,Star>& A )
{ 
#ifndef RELEASE
    PushCallStack("[* ,MC] = [MC,* ]");
    this->AssertNotLockedView();
    this->AssertSameGrid( A );
    if( this->Viewing() )
        this->AssertSameSize( A );
#endif
    const Grid& grid = this->GetGrid();
    auto_ptr< DistMatrix<T,MC,MR> > A_MC_MR
    ( new DistMatrix<T,MC,MR>(grid) );
    *A_MC_MR   = A;

    auto_ptr< DistMatrix<T,Star,VR> > A_Star_VR
    ( new DistMatrix<T,Star,VR>(grid) );
    *A_Star_VR = *A_MC_MR;
    delete A_MC_MR.release(); // lowers memory highwater

    auto_ptr< DistMatrix<T,Star,VC> > A_Star_VC
    ( new DistMatrix<T,Star,VC>(true,this->RowAlignment(),grid) );
    *A_Star_VC = *A_Star_VR;
    delete A_Star_VR.release(); // lowers memory highwater

    *this = *A_Star_VC;
#ifndef RELEASE
    PopCallStack();
#endif
    return *this;
}

template<typename T>
const DistMatrixBase<T,Star,MC>&
elemental::DistMatrixBase<T,Star,MC>::operator=
( const DistMatrixBase<T,Star,MR>& A )
{ 
#ifndef RELEASE
    PushCallStack("[* ,MC] = [* ,MR]");
    this->AssertNotLockedView();
    this->AssertSameGrid( A );
    if( this->Viewing() )
        this->AssertSameSize( A );
#endif
    const Grid& grid = this->GetGrid();
    if( A.Height() == 1 )
    {
        if( !this->Viewing() )
            this->ResizeTo( 1, A.Width() );

        const int r = grid.Height();
        const int c = grid.Width();
        const int p = grid.Size();
        const int myRow = grid.MCRank();
        const int rankCM = grid.VCRank();
        const int rankRM = grid.VRRank();
        const int rowAlignment = this->RowAlignment();
        const int rowAlignmentOfA = A.RowAlignment();
        const int rowShiftOfA = A.RowShift();

        const int width = this->Width();
        const int maxLocalVectorWidth = MaxLocalLength(width,p);
        const int portionSize = max(maxLocalVectorWidth,MinCollectContrib);

        const int rowShiftVC = Shift(rankCM,rowAlignment,p);
        const int rowShiftVROfA = Shift(rankRM,rowAlignmentOfA,p);
        const int sendRankCM = (rankCM+(p+rowShiftVROfA-rowShiftVC)) % p;
        const int recvRankRM = (rankRM+(p+rowShiftVC-rowShiftVROfA)) % p;
        const int recvRankCM = (recvRankRM/c)+r*(recvRankRM%c);

        this->_auxMemory.Require( (c+1)*portionSize );
        T* buffer = this->_auxMemory.Buffer();
        T* sendBuf = &buffer[0];
        T* recvBuf = &buffer[c*portionSize];

        // A[* ,VR] <- A[* ,MR]
        {
            const int shift = Shift(rankRM,rowAlignmentOfA,p);
            const int offset = (shift-rowShiftOfA) / c;
            const int thisLocalWidth = LocalLength(width,shift,p);

            for( int j=0; j<thisLocalWidth; ++j )
                sendBuf[j] = A.LocalEntry(0,offset+j*r);
        }

        // A[* ,VC] <- A[* ,VR]
        SendRecv
        ( sendBuf, portionSize, sendRankCM, 0,
          recvBuf, portionSize, recvRankCM, MPI_ANY_TAG, grid.VCComm() );

        // A[* ,MC] <- A[* ,VC]
        AllGather
        ( recvBuf, portionSize,
          sendBuf, portionSize, grid.MRComm() );

        // Unpack
        for( int k=0; k<c; ++k )
        {
            const T* data = &sendBuf[k*portionSize];

            const int shift = Shift(myRow+r*k,rowAlignment,p);
            const int offset = (shift-this->RowShift()) / r;
            const int thisLocalWidth = LocalLength(width,shift,p);

            for( int j=0; j<thisLocalWidth; ++j )
                this->LocalEntry(0,offset+j*c) = data[j];
        }

        this->_auxMemory.Release();
    }
    else
    {
        auto_ptr< DistMatrix<T,Star,VR> > A_Star_VR
        ( new DistMatrix<T,Star,VR>(grid) );
        *A_Star_VR = A;

        auto_ptr< DistMatrix<T,Star,VC> > A_Star_VC
        ( new DistMatrix<T,Star,VC>(true,this->RowAlignment(),grid) );
        *A_Star_VC = *A_Star_VR;
        delete A_Star_VR.release(); // lowers memory highwater

        *this = *A_Star_VC;
    }
#ifndef RELEASE
    PopCallStack();
#endif
    return *this;
}

template<typename T>
const DistMatrixBase<T,Star,MC>&
elemental::DistMatrixBase<T,Star,MC>::operator=
( const DistMatrixBase<T,MD,Star>& A )
{
#ifndef RELEASE
    PushCallStack("[* ,MC] = [MD,* ]");
    this->AssertNotLockedView();
    this->AssertSameGrid( A );
    if( this->Viewing() )
        this->AssertSameSize( A );
#endif
    throw "[* ,MC] = [MD,* ] not yet implemented.";
#ifndef RELEASE
    PopCallStack();
#endif
    return *this;
}

template<typename T>
const DistMatrixBase<T,Star,MC>&
elemental::DistMatrixBase<T,Star,MC>::operator=
( const DistMatrixBase<T,Star,MD>& A )
{ 
#ifndef RELEASE
    PushCallStack("[* ,MC] = [* ,MD]");
    this->AssertNotLockedView();
    this->AssertSameGrid( A );
    if( this->Viewing() )
        this->AssertSameSize( A );
#endif
    throw "[* ,MC] = [MD,* ] not yet implemented.";
#ifndef RELEASE
    PopCallStack();
#endif
    return *this;
}

template<typename T>
const DistMatrixBase<T,Star,MC>&
elemental::DistMatrixBase<T,Star,MC>::operator=
( const DistMatrixBase<T,MR,MC>& A )
{ 
#ifndef RELEASE
    PushCallStack("[* ,MC] = [MR,MC]");
    this->AssertNotLockedView();
    this->AssertSameGrid( A );
    if( this->Viewing() )
        this->AssertSameSize( A );
#endif
    const Grid& grid = this->GetGrid();
    if( !this->Viewing() )
    {
        if( !this->ConstrainedRowAlignment() )
        {
            this->_rowAlignment = A.RowAlignment();
            this->_rowShift = 
                Shift( grid.MCRank(), this->RowAlignment(), grid.Height() );
        }
        this->ResizeTo( A.Height(), A.Width() );
    }

    if( this->RowAlignment() == A.RowAlignment() )
    {
        const int c = grid.Width();
        const int height = this->Height();
        const int localWidth = this->LocalWidth();
        const int localHeightOfA = A.LocalHeight();
        const int maxLocalHeightOfA = MaxLocalLength(height,c);

        const int portionSize = 
            max(maxLocalHeightOfA*localWidth,MinCollectContrib);

        this->_auxMemory.Require( (c+1)*portionSize );

        T* buffer = this->_auxMemory.Buffer();
        T* originalData = &buffer[0];
        T* gatheredData = &buffer[portionSize];

        // Pack
        for( int j=0; j<localWidth; ++j )
            for( int i=0; i<localHeightOfA; ++i )
                originalData[i+j*localHeightOfA] = A.LocalEntry(i,j);

        // Communicate
        AllGather
        ( originalData, portionSize,
          gatheredData, portionSize, grid.MRComm() );

        // Unpack
        const int colAlignmentOfA = A.ColAlignment();
        for( int k=0; k<c; ++k )
        {
            const T* data = &gatheredData[k*portionSize];

            const int colShift = Shift( k, colAlignmentOfA, c );
            const int localHeight = LocalLength( height, colShift, c );

            for( int j=0; j<localWidth; ++j )
                for( int i=0; i<localHeight; ++i )
                    this->LocalEntry(colShift+i*c,j) = data[i+j*localHeight];
        }

        this->_auxMemory.Release();
    }
    else
    {
#ifndef RELEASE
        if( grid.VCRank() == 0 )
            cout << "Unaligned [* ,MC] <- [MR,MC]." << endl;
#endif
        const int r = grid.Height();
        const int c = grid.Width();
        const int row = grid.MCRank();

        const int rowAlignment = this->RowAlignment();
        const int rowAlignmentOfA = A.RowAlignment();
        const int sendRow = (row+r+rowAlignment-rowAlignmentOfA) % r;
        const int recvRow = (row+r+rowAlignmentOfA-rowAlignment) % r;

        const int height = this->Height();
        const int width = this->Width();
        const int localWidth = this->LocalWidth();
        const int localHeightOfA = A.LocalHeight();
        const int localWidthOfA = A.LocalWidth();
        const int maxLocalHeightOfA = MaxLocalLength(height,c);
        const int maxLocalWidth = MaxLocalLength(width,r);

        const int portionSize = 
            max(maxLocalHeightOfA*maxLocalWidth,MinCollectContrib);

        this->_auxMemory.Require( (c+1)*portionSize );

        T* buffer = this->_auxMemory.Buffer();
        T* firstBuffer = &buffer[0];
        T* secondBuffer = &buffer[portionSize];

        // Pack
        for( int j=0; j<localWidthOfA; ++j )
            for( int i=0; i<localHeightOfA; ++i )
                secondBuffer[i+j*localHeightOfA] = A.LocalEntry(i,j);

        // Perform the SendRecv: puts the new data into the first buffer
        SendRecv
        ( secondBuffer, portionSize, sendRow, 0,
          firstBuffer,  portionSize, recvRow, MPI_ANY_TAG, grid.MCComm() );

        // Use the output of the SendRecv as input to the AllGather
        AllGather
        ( firstBuffer,  portionSize,
          secondBuffer, portionSize, grid.MRComm() );

        // Unpack the contents of each member of the process row
        const int colAlignmentOfA = A.ColAlignment();
        for( int k=0; k<c; ++k )
        {
            const T* data = &secondBuffer[k*portionSize];

            const int colShift = Shift( k, colAlignmentOfA, c );
            const int localHeight = LocalLength( height, colShift, c );
            for( int j=0; j<localWidth; ++j )
                for( int i=0; i<localHeight; ++i )
                    this->LocalEntry(colShift+i*c,j) = data[i+j*localHeight];
        }

        this->_auxMemory.Release();
    }
#ifndef RELEASE
    PopCallStack();
#endif
    return *this;
}

template<typename T>
const DistMatrixBase<T,Star,MC>&
elemental::DistMatrixBase<T,Star,MC>::operator=
( const DistMatrixBase<T,MR,Star>& A )
{ 
#ifndef RELEASE
    PushCallStack("[* ,MC] = [MR,* ]");
    this->AssertNotLockedView();
    this->AssertSameGrid( A );
    if( this->Viewing() )
        this->AssertSameSize( A );
#endif
    const Grid& grid = this->GetGrid();
    DistMatrix<T,MR,MC> A_MR_MC(grid);

    A_MR_MC = A;
    *this = A_MR_MC;
#ifndef RELEASE
    PopCallStack();
#endif
    return *this;
}

template<typename T>
const DistMatrixBase<T,Star,MC>&
elemental::DistMatrixBase<T,Star,MC>::operator=
( const DistMatrixBase<T,Star,MC>& A )
{ 
#ifndef RELEASE
    PushCallStack("[* ,MC] = [* ,MC]");
    this->AssertNotLockedView();
    this->AssertSameGrid( A );
    if( this->Viewing() )
        this->AssertSameSize( A );
#endif
    const Grid& grid = this->GetGrid();
    if( !this->Viewing() )
    {
        if( !this->ConstrainedRowAlignment() )
        {
            this->_rowAlignment = A.RowAlignment();
            this->_rowShift = A.RowShift();
        }
        this->ResizeTo( A.Height(), A.Width() );
    }

    if( this->RowAlignment() == A.RowAlignment() )
    {
        this->_localMatrix = A.LockedLocalMatrix();
    }
    else
    {
#ifndef RELEASE
        if( grid.VCRank() == 0 )
            cout << "Unaligned [* ,MC] <- [* ,MC]." << endl;
#endif
        const int rank = grid.MCRank();
        const int r = grid.Height();

        const int rowAlignment = this->RowAlignment();
        const int rowAlignmentOfA = A.RowAlignment();

        const int sendRank = (rank+r+rowAlignment-rowAlignmentOfA) % r;
        const int recvRank = (rank+r+rowAlignmentOfA-rowAlignment) % r;

        const int height = this->Height();
        const int localWidth = this->LocalWidth();
        const int localWidthOfA = A.LocalWidth();

        const int sendSize = height * localWidthOfA;
        const int recvSize = height * localWidth;

        this->_auxMemory.Require( sendSize + recvSize );

        T* buffer = this->_auxMemory.Buffer();
        T* sendBuffer = &buffer[0];
        T* recvBuffer = &buffer[sendSize];

        // Pack
        for( int j=0; j<localWidthOfA; ++j )
            for( int i=0; i<height; ++i )
                sendBuffer[i+j*height] = A.LocalEntry(i,j);

        // Communicate
        SendRecv
        ( sendBuffer, sendSize, sendRank, 0,
          recvBuffer, recvSize, recvRank, MPI_ANY_TAG, grid.MCComm() );

        // Unpack
        for( int j=0; j<localWidth; ++j )
            for( int i=0; i<height; ++i )
                this->LocalEntry(i,j) = recvBuffer[i+j*height];

        this->_auxMemory.Release();
    }
#ifndef RELEASE
    PopCallStack();
#endif
    return *this;
}

template<typename T>
const DistMatrixBase<T,Star,MC>&
elemental::DistMatrixBase<T,Star,MC>::operator=
( const DistMatrixBase<T,VC,Star>& A )
{ 
#ifndef RELEASE
    PushCallStack("[* ,MC] = [VC,* ]");
    this->AssertNotLockedView();
    this->AssertSameGrid( A );
    if( this->Viewing() )
        this->AssertSameSize( A );
#endif
    const Grid& grid = this->GetGrid();
    auto_ptr< DistMatrix<T,VR,Star> > A_VR_Star
    ( new DistMatrix<T,VR,Star>(grid) );
    *A_VR_Star = A;

    auto_ptr< DistMatrix<T,MR,MC> > A_MR_MC
    ( new DistMatrix<T,MR,MC>(false,true,0,this->RowAlignment(),grid) );
    *A_MR_MC = *A_VR_Star;
    delete A_VR_Star.release();

    *this = *A_MR_MC;
#ifndef RELEASE
    PopCallStack();
#endif
    return *this;
}

template<typename T>
const DistMatrixBase<T,Star,MC>&
elemental::DistMatrixBase<T,Star,MC>::operator=
( const DistMatrixBase<T,Star,VC>& A )
{ 
#ifndef RELEASE
    PushCallStack("[* ,MC] = [* ,VC]");
    this->AssertNotLockedView();
    this->AssertSameGrid( A );
    if( this->Viewing() )
        this->AssertSameSize( A );
#endif
    const Grid& grid = this->GetGrid();
    if( !this->Viewing() )
    {
        if( !this->ConstrainedRowAlignment() )
        {
            this->_rowAlignment = A.RowAlignment() % grid.Height();
            this->_rowShift = 
                Shift( grid.MCRank(), this->RowAlignment(), grid.Height() );
        }
        this->ResizeTo( A.Height(), A.Width() );
    }

    if( this->RowAlignment() == A.RowAlignment() % grid.Height() )
    {
        const int r = grid.Height();
        const int c = grid.Width();
        const int p = grid.Size();
        const int row = grid.MCRank();

        const int height = this->Height();
        const int width = this->Width();
        const int localWidthOfA = A.LocalWidth();
        const int maxLocalWidthOfA = MaxLocalLength(width,p);

        const int portionSize = 
            max(height*maxLocalWidthOfA,MinCollectContrib);

        this->_auxMemory.Require( (c+1)*portionSize );

        T* buffer = this->_auxMemory.Buffer();
        T* originalData = &buffer[0];
        T* gatheredData = &buffer[portionSize];

        // Pack
        for( int j=0; j<localWidthOfA; ++j )
            for( int i=0; i<height; ++i )
                originalData[i+j*height] = A.LocalEntry(i,j);

        // Communicate
        AllGather
        ( originalData, portionSize,
          gatheredData, portionSize, grid.MRComm() );

        // Unpack
        const int rowShift = this->RowShift();
        const int rowAlignmentOfA = A.RowAlignment();
        for( int k=0; k<c; ++k )
        {
            const T* data = &gatheredData[k*portionSize];

            const int rowShiftOfA = Shift( row+k*r, rowAlignmentOfA, p );
            const int rowOffset = (rowShiftOfA-rowShift) / r;
            const int localWidth = LocalLength( width, rowShiftOfA, p );

            for( int j=0; j<localWidth; ++j )
                for( int i=0; i<height; ++i )
                    this->LocalEntry(i,rowOffset+j*c) = data[i+j*height];
        }

        this->_auxMemory.Release();
    }
    else
    {
#ifndef RELEASE
        if( grid.VCRank() == 0 )
            cout << "Unaligned [* ,MC] <- [* ,VC]." << endl;
#endif
        const int r = grid.Height();
        const int c = grid.Width();
        const int p = grid.Size();
        const int row = grid.MCRank();
        const int rank = grid.VCRank();

        // Perform the SendRecv to make A have the same rowAlignment
        const int rowAlignment = this->RowAlignment();
        const int rowAlignmentOfA = A.RowAlignment();
        const int rowShift = this->RowShift();

        const int sendRank = (rank+p+rowAlignment-rowAlignmentOfA) % p;
        const int recvRank = (rank+p+rowAlignmentOfA-rowAlignment) % p;

        const int height = this->Height();
        const int width = this->Width();
        const int localWidthOfA = A.LocalWidth();
        const int maxLocalWidthOfA = MaxLocalLength(width,p);

        const int portionSize = 
            max(height*maxLocalWidthOfA,MinCollectContrib);

        this->_auxMemory.Require( (c+1)*portionSize );

        T* buffer = this->_auxMemory.Buffer();
        T* firstBuffer = &buffer[0];
        T* secondBuffer = &buffer[portionSize];

        // Pack
        for( int j=0; j<localWidthOfA; ++j )
            for( int i=0; i<height; ++i )
                secondBuffer[i+j*height] = A.LocalEntry(i,j);

        // Perform the SendRecv: puts the new data into the first buffer
        SendRecv
        ( secondBuffer, portionSize, sendRank, 0,
          firstBuffer,  portionSize, recvRank, 0, grid.VCComm() );

        // Use the SendRecv as input to the AllGather
        AllGather
        ( firstBuffer,  portionSize,
          secondBuffer, portionSize, grid.MRComm() );

        // Unpack
        for( int k=0; k<c; ++k )
        {
            const T* data = &secondBuffer[k*portionSize];

            const int rowShiftOfA = Shift(row+r*k,rowAlignment,p);
            const int rowOffset = (rowShiftOfA-rowShift) / r;
            const int localWidth = LocalLength( width, rowShiftOfA, p );
            
            for( int j=0; j<localWidth; ++j )
                for( int i=0; i<height; ++i )
                    this->LocalEntry(i,rowOffset+j*c) = data[i+j*height];
        }

        this->_auxMemory.Release();
    }
#ifndef RELEASE
    PopCallStack();
#endif
    return *this;
}

template<typename T>
const DistMatrixBase<T,Star,MC>&
elemental::DistMatrixBase<T,Star,MC>::operator=
( const DistMatrixBase<T,VR,Star>& A )
{ 
#ifndef RELEASE
    PushCallStack("[* ,MC] = [VR,* ]");
    this->AssertNotLockedView();
    this->AssertSameGrid( A );
    if( this->Viewing() )
        this->AssertSameSize( A );
#endif
    const Grid& grid = this->GetGrid();
    DistMatrix<T,MR,MC> A_MR_MC(grid);

    A_MR_MC = A;
    *this = A_MR_MC;
#ifndef RELEASE
    PopCallStack();
#endif
    return *this;
}

template<typename T>
const DistMatrixBase<T,Star,MC>&
elemental::DistMatrixBase<T,Star,MC>::operator=
( const DistMatrixBase<T,Star,VR>& A )
{ 
#ifndef RELEASE
    PushCallStack("[* ,MC] = [* ,VR]");
    this->AssertNotLockedView();
    this->AssertSameGrid( A );
    if( this->Viewing() )
        this->AssertSameSize( A );
#endif
    const Grid& grid = this->GetGrid();
    DistMatrix<T,Star,VC> A_Star_VC(true,this->RowAlignment(),grid);
    *this = A_Star_VC = A;
#ifndef RELEASE
    PopCallStack();
#endif
    return *this;
}

template<typename T>
const DistMatrixBase<T,Star,MC>&
elemental::DistMatrixBase<T,Star,MC>::operator=
( const DistMatrixBase<T,Star,Star>& A )
{ 
#ifndef RELEASE
    PushCallStack("[* ,MC] = [* ,* ]");
    this->AssertNotLockedView();
    this->AssertSameGrid( A );
    if( this->Viewing() )
        this->AssertSameSize( A );
#endif
    if( !this->Viewing() )
        this->ResizeTo( A.Height(), A.Width() );

    const int r = this->GetGrid().Height();
    const int rowShift = this->RowShift();

    const int localHeight = this->LocalHeight();
    const int localWidth = this->LocalWidth();
    for( int j=0; j<localWidth; ++j )
        for( int i=0; i<localHeight; ++i )
            this->LocalEntry(i,j) = A.LocalEntry(i,rowShift+j*r);
#ifndef RELEASE
    PopCallStack();
#endif
    return *this;
}

//----------------------------------------------------------------------------//
// DistMatrix                                                                 //
//----------------------------------------------------------------------------//

template<typename R>
void
elemental::DistMatrix<R,Star,MC>::SetToRandomHPD()
{
#ifndef RELEASE
    PushCallStack("[* ,MC]::SetToRandomHPD");
    this->AssertNotLockedView();
    if( this->Height() != this->Width() )
        throw "Positive-definite matrices must be square.";
#endif
    const int height = this->Height();
    const int localWidth = this->LocalWidth();
    const int r = this->GetGrid().Height();
    const int rowShift = this->RowShift();

    this->SetToRandom();
    for( int jLoc=0; jLoc<localWidth; ++jLoc )
    {
        const int j = rowShift + jLoc*r;
        if( j < height )
            this->LocalEntry(j,jLoc) += (R)this->Width();
    }
#ifndef RELEASE
    PopCallStack();
#endif
}

#ifndef WITHOUT_COMPLEX
template<typename R>
void
elemental::DistMatrix<complex<R>,Star,MC>::SetToRandomHPD()
{
#ifndef RELEASE
    PushCallStack("[* ,MC]::SetToRandomHPD");
    this->AssertNotLockedView();
    if( this->Height() != this->Width() )
        throw "Positive-definite matrices must be square.";
#endif
    const int height = this->Height();
    const int localWidth = this->LocalWidth();
    const int r = this->GetGrid().Height();
    const int rowShift = this->RowShift();

    this->SetToRandom();
    for( int jLoc=0; jLoc<localWidth; ++jLoc )
    {
        const int j = rowShift + jLoc*r;
        if( j < height )
            this->LocalEntry(j,jLoc) = 
                real(this->LocalEntry(j,jLoc)) + (R)this->Width();
    }
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename R>
R
elemental::DistMatrix<complex<R>,Star,MC>::GetReal
( int i, int j ) const
{
#ifndef RELEASE
    PushCallStack("[* ,MR]::GetReal");
    this->AssertValidEntry( i, j );
#endif
    // We will determine the owner row of entry (i,j) and broadcast from that
    // row within each process column
    const Grid& grid = this->GetGrid();
    const int ownerRow = (j + this->RowAlignment()) % grid.Height();

    R u;
    if( grid.MCRank() == ownerRow )
    {
        const int jLoc = (j-this->RowShift()) / grid.Height();
        u = real(this->LocalEntry(i,jLoc));
    }
    Broadcast( &u, 1, ownerRow, grid.MCComm() );

#ifndef RELEASE
    PopCallStack();
#endif
    return u;
}

template<typename R>
R
elemental::DistMatrix<complex<R>,Star,MC>::GetImag
( int i, int j ) const
{
#ifndef RELEASE
    PushCallStack("[* ,MR]::GetImag");
    this->AssertValidEntry( i, j );
#endif
    // We will determine the owner row of entry (i,j) and broadcast from that
    // row within each process column
    const Grid& grid = this->GetGrid();
    const int ownerRow = (j + this->RowAlignment()) % grid.Height();

    R u;
    if( grid.MCRank() == ownerRow )
    {
        const int jLoc = (j-this->RowShift()) / grid.Height();
        u = imag(this->LocalEntry(i,jLoc));
    }
    Broadcast( &u, 1, ownerRow, grid.MCComm() );

#ifndef RELEASE
    PopCallStack();
#endif
    return u;
}

template<typename R>
void
elemental::DistMatrix<complex<R>,Star,MC>::SetReal
( int i, int j, R u )
{
#ifndef RELEASE
    PushCallStack("[* ,MC]::SetReal");
    this->AssertValidEntry( i, j );
#endif
    const Grid& grid = this->GetGrid();
    const int ownerRow = (j + this->RowAlignment()) % grid.Height();

    if( grid.MCRank() == ownerRow )
    {
        const int jLoc = (j-this->RowShift()) / grid.Height();
        real(this->LocalEntry(i,jLoc)) = u;
    }
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename R>
void
elemental::DistMatrix<complex<R>,Star,MC>::SetImag
( int i, int j, R u )
{
#ifndef RELEASE
    PushCallStack("[* ,MC]::SetImag");
    this->AssertValidEntry( i, j );
#endif
    const Grid& grid = this->GetGrid();
    const int ownerRow = (j + this->RowAlignment()) % grid.Height();

    if( grid.MCRank() == ownerRow )
    {
        const int jLoc = (j-this->RowShift()) / grid.Height();
        imag(this->LocalEntry(i,jLoc)) = u;
    }
#ifndef RELEASE
    PopCallStack();
#endif
}
#endif // WITHOUT_COMPLEX

template class elemental::DistMatrixBase<int,   Star,MC>;
template class elemental::DistMatrixBase<float, Star,MC>;
template class elemental::DistMatrixBase<double,Star,MC>;
#ifndef WITHOUT_COMPLEX
template class elemental::DistMatrixBase<scomplex,Star,MC>;
template class elemental::DistMatrixBase<dcomplex,Star,MC>;
#endif

template class elemental::DistMatrix<int,     Star,MC>;
template class elemental::DistMatrix<float,   Star,MC>;
template class elemental::DistMatrix<double,  Star,MC>;
#ifndef WITHOUT_COMPLEX
template class elemental::DistMatrix<scomplex,Star,MC>;
template class elemental::DistMatrix<dcomplex,Star,MC>;
#endif

