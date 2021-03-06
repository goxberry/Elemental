/*
   Copyright (c) 2009-2016, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#ifndef EL_SCHUR_HESS_MULTIBULGE_INTRA_BLOCK_CHASE_HPP
#define EL_SCHUR_HESS_MULTIBULGE_INTRA_BLOCK_CHASE_HPP

#include "./ComputeIntraBlockReflectors.hpp"
#include "./ApplyIntraBlockReflectors.hpp"

namespace El {
namespace hess_schur {
namespace multibulge {

// Chase the separated packets of tightly-packed 4x4 bulges from the top-left
// corners of the diagonal blocks down to the bottom-right corners. This is
// accomplished by locally accumulating the reflections into a dense matrix
// and then broadcasting/allgathering said matrix within the rows and columns
// of the process grid. See Fig. 3 of 
//
//   R. Granat, Bo Kagstrom, and D. Kressner, "LAPACK Working Note #216:
//   A novel parallel QR algorithm for hybrid distributed memory HPC systems",
//
// [CITATION] for a diagram.
//

// The following extends the discussion in Granat et al. to handle active
// windows. For example, ctrl.winBeg and ctrl.winEnd define index sets
//
//   ind0 = [0,ctrl.winBeg),
//   ind1 = [ctrl.winBeg,ctrl.winEnd),
//   ind2 = [ctrl.winEnd,n),
//
// and a partitioning
//
//    H = | H00 H01 H02 |
//        | H10 H11 H12 |
//        | 0   H21 H22 |
//
// such that H11 is the active submatrix, and H10 and H21 contain a single
// nonzero entry in their upper right corners (if they are non-empty). If
// a full Schur decomposition was requested, then the appropriate pieces of H01
// and H12 must be updated by the accumulated Householder transformations
// rather than just the off-diagonal blocks of H11. Note that the fact that
// [winBeg,winEnd) may take on arbitrary values implies that we must handle
// windows which begin in the middle of distribution blocks, but thankfully this
// only effects the *inter*-block chases and not the *intra*-block chases
// (Cf. the diagrams in the *inter*-block chase source for how these
// complications are handled).
//
// Each of the intra-block multibulge chases takes a single form (though the
// last diagonal block may have a different number of bulges): the bulges are
// locally chased from the top-left to the bottom-right of the diagonal block,
// e.g., if the distribution block size was 12 and there were two bulges in the
// diagonal block, we would have the transformation
//
//         ~ ~ ~ ~ ~ ~ ~ ~ ~ ~                  ~ ~ ~ ~ ~ ~ ~ ~ ~ ~  
//      ------------------------             -------------------------
//     | B B B B x x x x x x x x |          | x x x x x x x x x x x x |
//   ~ | B B B B x x x x x x x x |        ~ | x x x x x x x x x x x x |
//   ~ | B B B B x x x x x x x x |        ~ |   x x x x x x x x x x x |
//   ~ | B B B B B B B x x x x x |        ~ |     x x x x x x x x x x |
//   ~ |       B B B B x x x x x |        ~ |       x x x x x x x x x |
//   ~ |       B B B B x x x x x |  |->   ~ |         x B B B B x x x |
//   ~ |       B B B B x x x x x |        ~ |           B B B B x x x |
//   ~ |             x x x x x x |        ~ |           B B B B x x x |
//   ~ |               x x x x x |        ~ |           B B B B B B B |
//   ~ |                 x x x x |        ~ |                 B B B B |
//   ~ |                   x x x |        ~ |                 B B B B |
//     |                     x x |          |                 B B B B |
//      -------------------------            -------------------------
//
// It is worth noting that the accumulation of the ten 3x3 Householder
// reflections for this diagram effect all but the first and last rows when
// applied from the left, and all but the first and last columns when applied
// from the right. 
//
// It is also worth noting that none of the intra-block chases involve the last
// diagonal block, as inter-block chases that introduce bulges into the last
// diagonal block are immediately chased out of the window.
//

namespace intrablock {

// Form the list of accumulated Householder transformations, which should be
// applied as
//
//     \hat{U}_i' H_i \hat{U}_i,
//
// where \hat{U}_i = | 1, 0,   0 |
//                   | 0, U_i, 0 |
//                   | 0, 0,   1 |
//
// is the extension of U_i to the entire diagonal block (as the transformation
// leaves the first and last rows unchanged when applied from the left), and 
// H_i is the i'th locally-owned diagonal block of H.
//
template<typename F>
void LocalChase
(       DistMatrix<F,MC,MR,BLOCK>& H,
  const DistMatrix<Complex<Base<F>>,STAR,STAR>& shifts,
  const DistChaseState& state,
  const DistChaseContext& context,
  const HessenbergSchurCtrl& ctrl,
        vector<Matrix<F>>& UList )
{
    DEBUG_CSE
    const Int n = H.Height();
    const Grid& grid = H.Grid();

    Matrix<F> W;
    auto& HLoc = H.Matrix();
    const auto& shiftsLoc = shifts.LockedMatrix();

    // Count the number of diagonal blocks assigned to this process
    {
        // Only loop over the row blocks that are assigned to our process row
        // and occur within the active window.
        Int diagBlock = context.activeRowBlockBeg;
        Int numLocalBlocks = 0;
        while( diagBlock < state.activeBlockEnd )
        {
            const int ownerCol =
              Mod( context.winRowAlign+diagBlock, grid.Width() );
            if( ownerCol == grid.Col() )
                ++numLocalBlocks;
            diagBlock += grid.Height();
        }
        UList.resize(numLocalBlocks);
    }
    
    // Chase bulges down the local diagonal blocks and store the accumulations
    // of the Householder reflections. We only loop over the row blocks that
    // are assigned to our process row and filter based upon whether or not
    // we are in the correct process column.
    Int localDiagBlock = 0;
    Int diagBlock = context.activeRowBlockBeg;
    Zeros( W, 3, context.numBulgesPerBlock );
    while( diagBlock < state.activeBlockEnd )
    {
        const Int thisBlockHeight = 
          ( diagBlock == 0 ? context.firstBlockSize : context.blockSize );
        const Int numBlockBulges =
          ( diagBlock==state.activeBlockEnd-1 ?
            context.numBulgesInLastBlock :
            context.numBulgesPerBlock );

        const int ownerCol = Mod( context.winRowAlign+diagBlock, grid.Width() );
        if( ownerCol == grid.Col() )
        {
            const Int diagOffset = context.winBeg +
              ( diagBlock == 0 ?
                0 :
                context.firstBlockSize + (diagBlock-1)*context.blockSize );

            // View the local diagonal block of H
            const Int localRowOffset = H.LocalRowOffset( diagOffset );
            const Int localColOffset = H.LocalColOffset( diagOffset );
            auto HBlockLoc = 
              HLoc
              ( IR(0,thisBlockHeight)+localRowOffset,
                IR(0,thisBlockHeight)+localColOffset );

            // View the local shifts for this diagonal block
            const Int shiftOffset = state.shiftBeg +
              (2*context.numBulgesPerBlock)*(diagBlock-state.activeBlockBeg);
            auto shiftsBlockLoc =
              shiftsLoc( IR(0,2*numBlockBulges)+shiftOffset, ALL );

            // Initialize the accumulated reflection matrix; recall that it 
            // does not effect the first or last index of the block. For
            // example, consider the effects of a single 3x3 Householder 
            // similarity bulge chase step
            //
            //        ~ ~ ~                 ~ ~ ~
            //     -----------           -----------
            //    | B B B B x |  |->    | x x x x x |
            //  ~ | B B B B x |       ~ | x B B B B |
            //  ~ | B B B B x |       ~ |   B B B B |.
            //  ~ | B B B B x |       ~ |   B B B B |
            //    |       x x |         |   B B B B |
            //     -----------           -----------
            //
            auto& UBlock = UList[localDiagBlock];
            Identity( UBlock, thisBlockHeight-2, thisBlockHeight-2 );

            // Perform the diagonal block sweep and accumulate the
            // reflections in UBlock. The number of diagonal entries spanned
            // by numBlockBulges bulges is 1 + 3 numBlockBulges, so the number
            // of steps is thisBlockHeight - (1 + 3*numBlockBulges).
            const Int numSteps = thisBlockHeight - (1 + 3*numBlockBulges);
            for( Int step=0; step<numSteps; ++step )
            {
                ComputeIntraBlockReflectors
                ( step, numBlockBulges, HBlockLoc, shiftsBlockLoc, W,
                  ctrl.progress );
                ApplyIntraBlockReflectorsOpt
                ( step, numBlockBulges, HBlockLoc, UBlock, W,
                  ctrl.progress );
            }
            ++localDiagBlock;
        }
        diagBlock += grid.Height();
    }
}

template<typename F>
void ApplyAccumulatedReflections
(       DistMatrix<F,MC,MR,BLOCK>& H,
        DistMatrix<F,MC,MR,BLOCK>& Z,
  const DistMatrix<Complex<Base<F>>,STAR,STAR>& shifts,
  const DistChaseState& state,
  const DistChaseContext& context,
  const HessenbergSchurCtrl& ctrl,
  const vector<Matrix<F>>& UList )
{
    DEBUG_CSE
    const Grid& grid = H.Grid();

    // We will immediately apply the accumulated Householder transformations
    // after receiving them
    const bool immediatelyApply = true;

    auto& HLoc = H.Matrix();
    auto& ZLoc = Z.Matrix();
    const auto& shiftsLoc = shifts.LockedMatrix();

    // Broadcast/Allgather the accumulated reflections within rows
    if( immediatelyApply )
    {
        Matrix<F> UBlock;
        Matrix<F> tempMatrix;

        // Only loop over the row blocks assigned to this grid row
        Int diagBlock = context.activeRowBlockBeg;
        Int localDiagBlock = 0;
        while( diagBlock < state.activeBlockEnd )
        {
            const Int thisBlockHeight = 
              ( diagBlock == 0 ? context.firstBlockSize : context.blockSize );
            const Int numBlockBulges =
              ( diagBlock==state.activeBlockEnd-1 ?
                context.numBulgesInLastBlock :
                context.numBulgesPerBlock );

            const int ownerCol =
              Mod( context.winRowAlign+diagBlock, grid.Width() );
            if( ownerCol == grid.Col() )
            {
                UBlock = UList[localDiagBlock];
                ++localDiagBlock;
            }

            const Int diagOffset = context.winBeg +
              ( diagBlock == 0 ?
                0 :
                context.firstBlockSize + (diagBlock-1)*context.blockSize );
            const Int localRowOffset = H.LocalRowOffset( diagOffset );
            const Int localColOffset = H.LocalColOffset( diagOffset );
            El::Broadcast( UBlock, H.RowComm(), ownerCol );
            const auto applyRowInd = IR(1,thisBlockHeight-1) + localRowOffset;
            const auto applyColInd =
              IR(localColOffset+thisBlockHeight,context.localTransformColEnd);

            auto HLocRight = HLoc( applyRowInd, applyColInd );
            tempMatrix = HLocRight;
            Gemm( ADJOINT, NORMAL, F(1), UBlock, tempMatrix, HLocRight );

            diagBlock += grid.Height();
        }
    }
    else
    {
        // TODO(poulson): Add support for AllGather variant
        LogicError("This option is not yet supported");
    }

    // Broadcast/Allgather the accumulated reflections within columns
    if( immediatelyApply )
    {
        Matrix<F> UBlock;
        Matrix<F> tempMatrix;

        // Only loop over the row blocks assigned to this grid row
        Int diagBlock = context.activeColBlockBeg;
        Int localDiagBlock = 0;
        while( diagBlock < state.activeBlockEnd )
        {
            const Int thisBlockHeight = 
              ( diagBlock == 0 ? context.firstBlockSize : context.blockSize );
            const Int numBlockBulges =
              ( diagBlock==state.activeBlockEnd-1 ?
                context.numBulgesInLastBlock :
                context.numBulgesPerBlock );

            const int ownerRow =
              Mod( context.winColAlign+diagBlock, grid.Height() );
            if( ownerRow == grid.Row() )
            {
                UBlock = UList[localDiagBlock];
                ++localDiagBlock;
            }

            const Int diagOffset = context.winBeg +
              ( diagBlock == 0 ?
                0 :
                context.firstBlockSize + (diagBlock-1)*context.blockSize );
            const Int localRowOffset = H.LocalRowOffset( diagOffset );
            const Int localColOffset = H.LocalColOffset( diagOffset );
            El::Broadcast( UBlock, H.ColComm(), ownerRow );
            const auto applyRowInd =
              IR(context.localTransformRowBeg,localRowOffset);
            const auto applyColInd = IR(1,thisBlockHeight-1) + localColOffset;

            auto HLocAbove = HLoc( applyRowInd, applyColInd );
            tempMatrix = HLocAbove;
            Gemm( NORMAL, NORMAL, F(1), tempMatrix, UBlock, HLocAbove );
            if( ctrl.wantSchurVecs )
            {
                auto ZLocBlock = ZLoc( ALL, applyColInd ); 
                tempMatrix = ZLocBlock;
                Gemm( NORMAL, NORMAL, F(1), tempMatrix, UBlock, ZLocBlock );
            }

            diagBlock += grid.Width();
        }
    }
    else
    {
        // TODO(poulson): Add support for AllGather variant
        LogicError("This option is not yet supported");
    }
}

} // namespace intrablock

template<typename F>
void IntraBlockChase
(       DistMatrix<F,MC,MR,BLOCK>& H,
        DistMatrix<F,MC,MR,BLOCK>& Z,
  const DistMatrix<Complex<Base<F>>,STAR,STAR>& shifts,
  const DistChaseState& state,
  const HessenbergSchurCtrl& ctrl )
{
    DEBUG_CSE

    auto context = BuildDistChaseContext( H, shifts, state, ctrl );

    // Form the list of accumulated Householder transformations, which should be
    // applied as
    //
    //     \hat{U}_i' H_i \hat{U}_i,
    //
    // where \hat{U}_i = | 1, 0,   0 |
    //                   | 0, U_i, 0 |
    //                   | 0, 0,   1 |
    //
    // is the extension of U_i to the entire diagonal block (as the
    // transformation leaves the first and last rows unchanged when applied from
    // the left), and H_i is the i'th locally-owned diagonal block of H.
    //
    vector<Matrix<F>> UList;
    intrablock::LocalChase( H, shifts, state, context, ctrl, UList );

    // Broadcast the accumulated transformations from the owning diagonal block
    // over the entire process row/column teams and then apply them to Z from
    // the right (if the Schur vectors are desired), to the above-diagonal
    // portion of H from the right, and their adjoints to the relevant
    // right-of-diagonal portions of H from the left.
    intrablock::ApplyAccumulatedReflections
    ( H, Z, shifts, state, context, ctrl, UList );
}

} // namespace multibulge
} // namespace hess_schur
} // namespace El

#endif // ifndef EL_SCHUR_HESS_MULTIBULGE_INTRA_BLOCK_CHASE_HPP
