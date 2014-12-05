/*
   Copyright (c) 2009-2014, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#pragma once
#ifndef EL_OPTIMIZATION_HPP
#define EL_OPTIMIZATION_HPP

namespace El {

namespace RegularizationNS {
enum Regularization {
  NO_PENALTY,
  L1_PENALTY,
  L2_PENALTY
};
}
using namespace RegularizationNS;

// TODO: Modify the following routines to use control structures instead

// Basis pursuit: min || z ||_1 such that A z = b
// ==============================================
template<typename F>
Int BasisPursuit
( const Matrix<F>& A, const Matrix<F>& b,
  Matrix<F>& z,
  Base<F> rho=1., Base<F> alpha=1.2, Int maxIter=500, Base<F> absTol=1e-6, 
  Base<F> relTol=1e-4, bool usePinv=false, Base<F> pinvTol=0, 
  bool progress=true );
template<typename F>
Int BasisPursuit
( const AbstractDistMatrix<F>& A, const AbstractDistMatrix<F>& b,
        AbstractDistMatrix<F>& z,
  Base<F> rho=1., Base<F> alpha=1.2, Int maxIter=500, Base<F> absTol=1e-6, 
  Base<F> relTol=1e-4, bool usePinv=false, Base<F> pinvTol=0,
  bool progress=true );

// Coherence
// =========
template<typename F>
Base<F> Coherence( const Matrix<F>& A );
template<typename F>
Base<F> Coherence( const AbstractDistMatrix<F>& A );

// Least Absolute Shrinkage and Selection Operator (LASSO)
// =======================================================
template<typename F>
Int Lasso
( const Matrix<F>& A, const Matrix<F>& b, Base<F> lambda,
  Matrix<F>& z,
  Base<F> rho=1, Base<F> alpha=1.2, Int maxIter=500, Base<F> absTol=1e-6, 
  Base<F> relTol=1e-4, bool inv=true, bool progress=true );
template<typename F>
Int Lasso
( const AbstractDistMatrix<F>& A, const AbstractDistMatrix<F>& b, 
  Base<F> lambda, AbstractDistMatrix<F>& z,
  Base<F> rho=1, Base<F> alpha=1.2, Int maxIter=500, Base<F> absTol=1e-6, 
  Base<F> relTol=1e-4, bool inv=true, bool progress=true );

// Linear program
// ==============

namespace KKTSystemNS {
enum KKTSystem {
  FULL_KKT,
  AUGMENTED_KKT,
  NORMAL_KKT
};
}
using namespace KKTSystemNS;

namespace lin_prog {

template<typename Real>
struct IPFLineSearchCtrl {
    Real gamma;
    Real beta;
    Real psi;
    bool print;

    IPFLineSearchCtrl()
    : gamma(1e-3), beta(2), psi(100), print(false)
    { }
};

template<typename Real>
struct IPFCtrl {
    Real tol;
    Int maxIts;
    Real centering; 
    KKTSystem system;

    IPFLineSearchCtrl<Real> lineSearchCtrl;

    bool print;

    IPFCtrl( bool isSparse=true ) 
    : tol(1e-8), maxIts(1000), centering(0.9), print(false)
    {
        system = ( isSparse ? AUGMENTED_KKT : NORMAL_KKT );
    }
};

template<typename Real>
struct MehrotraCtrl {
    Real tol;
    Int maxIts;
    Real maxStepRatio;
    KKTSystem system;
    bool print;

    // TODO: Add a user-definable (muAff,mu) -> sigma function to replace
    //       the default, (muAff/mu)^3 

    MehrotraCtrl( bool isSparse=true )
    : tol(1e-8), maxIts(1000), maxStepRatio(0.99), print(false)
    { 
        system = ( isSparse ? AUGMENTED_KKT : NORMAL_KKT );
    }
};

template<typename Real>
struct ADMMCtrl
{
    Real rho;
    Real alpha;
    Int maxIter;
    Real absTol;
    Real relTol;
    bool inv;
    bool print;

    ADMMCtrl()
    : rho(1), alpha(1.2), maxIter(500), absTol(1e-6), relTol(1e-4), inv(true),
      print(true)
    { }
};

} // namespace lin_prog

namespace LinProgAlgNS {
enum LinProgAlg {
    LIN_PROG_ADMM,
    LIN_PROG_IPF,
    LIN_PROG_MEHROTRA
};
} // namespace LinProgAlgNS
using namespace LinProgAlgNS;

template<typename Real>
struct LinProgCtrl 
{
    LinProgAlg alg;
    lin_prog::ADMMCtrl<Real> admmCtrl;
    lin_prog::IPFCtrl<Real> ipfCtrl;
    lin_prog::MehrotraCtrl<Real> mehrotraCtrl;

    LinProgCtrl( bool isSparse=true ) 
    : alg(LIN_PROG_MEHROTRA), ipfCtrl(isSparse), mehrotraCtrl(isSparse)
    { }
};

template<typename Real>
void LinearProgram
( const Matrix<Real>& A, const Matrix<Real>& b, const Matrix<Real>& c,
  Matrix<Real>& x, const LinProgCtrl<Real>& ctrl=LinProgCtrl<Real>(false) );
template<typename Real>
void LinearProgram
( const AbstractDistMatrix<Real>& A, const AbstractDistMatrix<Real>& b,
  const AbstractDistMatrix<Real>& c,       AbstractDistMatrix<Real>& x,
  const LinProgCtrl<Real>& ctrl=LinProgCtrl<Real>(false) );
template<typename Real>
void LinearProgram
( const SparseMatrix<Real>& A, const Matrix<Real>& b, const Matrix<Real>& c,
  Matrix<Real>& x, const LinProgCtrl<Real>& ctrl=LinProgCtrl<Real>(true) );
template<typename Real>
void LinearProgram
( const DistSparseMatrix<Real>& A, 
  const DistMultiVec<Real>& b, const DistMultiVec<Real>& c,
  DistMultiVec<Real>& x, 
  const LinProgCtrl<Real>& ctrl=LinProgCtrl<Real>(true) );

namespace lin_prog {

// Mehrotra's Predictor-Corrector Infeasible Interior Point Method
// ---------------------------------------------------------------
template<typename Real>
void Mehrotra
( const Matrix<Real>& A,
  const Matrix<Real>& b, const Matrix<Real>& c,
  Matrix<Real>& s, Matrix<Real>& x, Matrix<Real>& l,
  const MehrotraCtrl<Real>& ctrl=MehrotraCtrl<Real>(false) );
template<typename Real>
void Mehrotra
( const AbstractDistMatrix<Real>& A,
  const AbstractDistMatrix<Real>& b, const AbstractDistMatrix<Real>& c,
  AbstractDistMatrix<Real>& s, AbstractDistMatrix<Real>& x, 
  AbstractDistMatrix<Real>& l,
  const MehrotraCtrl<Real>& ctrl=MehrotraCtrl<Real>(false) );
template<typename Real>
void Mehrotra
( const SparseMatrix<Real>& A,
  const Matrix<Real>& b, const Matrix<Real>& c,
  Matrix<Real>& s, Matrix<Real>& x, Matrix<Real>& l,
  const MehrotraCtrl<Real>& ctrl=MehrotraCtrl<Real>(true) );
template<typename Real>
void Mehrotra
( const DistSparseMatrix<Real>& A,
  const DistMultiVec<Real>& b, const DistMultiVec<Real>& c,
  DistMultiVec<Real>& s, DistMultiVec<Real>& x, DistMultiVec<Real>& l,
  const MehrotraCtrl<Real>& ctrl=MehrotraCtrl<Real>(true) );

// Infeasible Path-Following Interior Point Method (IPF)
// -----------------------------------------------------
template<typename Real>
void IPF
( const Matrix<Real>& A,
  const Matrix<Real>& b, const Matrix<Real>& c,
  Matrix<Real>& s, Matrix<Real>& x, Matrix<Real>& l,
  const IPFCtrl<Real>& ctrl=IPFCtrl<Real>(false) );
template<typename Real>
void IPF
( const AbstractDistMatrix<Real>& A,
  const AbstractDistMatrix<Real>& b,  const AbstractDistMatrix<Real>& c,
  AbstractDistMatrix<Real>& s, AbstractDistMatrix<Real>& x, 
  AbstractDistMatrix<Real>& l, 
  const IPFCtrl<Real>& ctrl=IPFCtrl<Real>(false) );
template<typename Real>
void IPF
( const SparseMatrix<Real>& A,
  const Matrix<Real>& b,  const Matrix<Real>& c,
  Matrix<Real>& s, Matrix<Real>& x, Matrix<Real>& l,
  const IPFCtrl<Real>& ctrl=IPFCtrl<Real>(true) );
template<typename Real>
void IPF
( const DistSparseMatrix<Real>& A,
  const DistMultiVec<Real>& b,  const DistMultiVec<Real>& c,
  DistMultiVec<Real>& s, DistMultiVec<Real>& x, DistMultiVec<Real>& l,
  const IPFCtrl<Real>& ctrl=IPFCtrl<Real>(true) );

// Alternating Direction Method of Multipliers (ADMM)
// --------------------------------------------------
template<typename Real>
Int ADMM
( const Matrix<Real>& A, const Matrix<Real>& b, const Matrix<Real>& c,
  Matrix<Real>& z,
  const ADMMCtrl<Real>& ctrl=ADMMCtrl<Real>() );
template<typename Real>
Int ADMM
( const AbstractDistMatrix<Real>& A, const AbstractDistMatrix<Real>& b,
  const AbstractDistMatrix<Real>& c,       AbstractDistMatrix<Real>& z,
  const ADMMCtrl<Real>& ctrl=ADMMCtrl<Real>() );

} // namespace lin_prog

// Logistic Regression
// ===================
template<typename Real>
Int LogisticRegression
( const Matrix<Real>& G, const Matrix<Real>& q, Matrix<Real>& z,
  Real gamma, Regularization penalty=L1_PENALTY,
  Real rho=1, Int maxIter=500, bool inv=true, bool progress=true );
template<typename Real>
Int LogisticRegression
( const AbstractDistMatrix<Real>& G, const AbstractDistMatrix<Real>& q, 
        AbstractDistMatrix<Real>& z,
  Real gamma, Regularization penalty=L1_PENALTY,
  Real rho=1, Int maxIter=500, bool inv=true, bool progress=true );

// Fit a model with using a loss function plus regularization
// ==========================================================
// TODO: Implement these functions
template<typename Real>
Int ModelFit
( std::function<void(Matrix<Real>&,Real)> lossProx,
  std::function<void(Matrix<Real>&,Real)> regProx,
  const Matrix<Real>& A, const Matrix<Real>& b, Matrix<Real>& w,
  Real rho, Int maxIter=1000, bool inv=true, bool progress=true );
template<typename Real>
Int ModelFit
( std::function<void(DistMatrix<Real>&,Real)> lossProx,
  std::function<void(DistMatrix<Real>&,Real)> regProx,
  const AbstractDistMatrix<Real>& A, const AbstractDistMatrix<Real>& b, 
        AbstractDistMatrix<Real>& w,
  Real rho, Int maxIter=1000, bool inv=true, bool progress=true );

// Non-negative matrix factorization
// =================================
// TODO: Generalize to complex
template<typename Real>
void NMF( const Matrix<Real>& A, Matrix<Real>& X, Matrix<Real>& Y );
template<typename Real>
void NMF
( const AbstractDistMatrix<Real>& A, AbstractDistMatrix<Real>& X, 
        AbstractDistMatrix<Real>& Y );

// Non-negative least squares
// ==========================
// TODO: Generalize to complex
template<typename Real>
Int NonNegativeLeastSquares
( const Matrix<Real>& A, const Matrix<Real>& Y, Matrix<Real>& Z,
  Real rho=1., Real alpha=1.2, Int maxIter=500, Real absTol=1e-6,
  Real relTol=1e-4, bool inv=true, bool progress=true );
template<typename Real>
Int NonNegativeLeastSquares
( const AbstractDistMatrix<Real>& A, const AbstractDistMatrix<Real>& Y, 
        AbstractDistMatrix<Real>& Z, 
  Real rho=1., Real alpha=1.2, Int maxIter=500, Real absTol=1e-6,
  Real relTol=1e-4, bool inv=true, bool progress=true );

// Quadratic program
// =================

namespace quad_prog {

template<typename Real>
struct IPFLineSearchCtrl {
    Real gamma;
    Real beta;
    Real psi;
    bool print;

    IPFLineSearchCtrl()
    : gamma(1e-3), beta(2), psi(100), print(false)
    { }
};

template<typename Real>
struct IPFCtrl {
    Real tol;
    Int maxIts;
    Real centering;
    KKTSystem system;

    IPFLineSearchCtrl<Real> lineSearchCtrl;

    bool print;

    IPFCtrl()
    : tol(1e-8), maxIts(1000), centering(0.9), system(AUGMENTED_KKT), 
      print(false)
    { }
};

template<typename Real>
struct MehrotraCtrl {
    Real tol;
    Int maxIts;
    Real maxStepRatio;
    KKTSystem system;
    bool print;

    // TODO: Add a user-definable (muAff,mu) -> sigma function to replace
    //       the default, (muAff/mu)^3 

    MehrotraCtrl()
    : tol(1e-8), maxIts(1000), maxStepRatio(0.99), system(AUGMENTED_KKT), 
      print(false)
    { }
};

template<typename Real>
struct ADMMCtrl
{
    Real rho;
    Real alpha;
    Int maxIter;
    Real absTol;
    Real relTol;
    bool inv;
    bool print;

    ADMMCtrl()
    : rho(1), alpha(1.2), maxIter(500), absTol(1e-6), relTol(1e-4), inv(true),
      print(true)
    { }
};

} // namespace quad_prog

namespace QuadProgAlgNS {
enum QuadProgAlg {
    QUAD_PROG_ADMM, // NOTE: There is no conic-form ADMM code as of yet
    QUAD_PROG_IPF,
    QUAD_PROG_MEHROTRA
};
} // namespace QuadProgAlgNS
using namespace QuadProgAlgNS;

template<typename Real>
struct QuadProgCtrl 
{
    QuadProgAlg alg;
    quad_prog::ADMMCtrl<Real> admmCtrl;
    quad_prog::IPFCtrl<Real> ipfCtrl;
    quad_prog::MehrotraCtrl<Real> mehrotraCtrl;

    QuadProgCtrl() 
    : alg(QUAD_PROG_MEHROTRA)
    { }
};

// Solve the following (conic form) quadratic program:
//   min 1/2 x' Q x + c' x, subject to A x = b and x >= 0
//    x
// using an Interior Point Method
template<typename Real>
void QuadraticProgram
( const Matrix<Real>& Q, const Matrix<Real>& A, 
  const Matrix<Real>& b, const Matrix<Real>& c,
        Matrix<Real>& x,
  const QuadProgCtrl<Real>& ctrl=QuadProgCtrl<Real>() );
template<typename Real>
void QuadraticProgram
( const AbstractDistMatrix<Real>& Q, const AbstractDistMatrix<Real>& A, 
  const AbstractDistMatrix<Real>& b, const AbstractDistMatrix<Real>& c,
        AbstractDistMatrix<Real>& x,
  const QuadProgCtrl<Real>& ctrl=QuadProgCtrl<Real>() );
template<typename Real>
void QuadraticProgram
( const SparseMatrix<Real>& Q, const SparseMatrix<Real>& A, 
  const Matrix<Real>& b, const Matrix<Real>& c,
        Matrix<Real>& x,
  const QuadProgCtrl<Real>& ctrl=QuadProgCtrl<Real>() );
template<typename Real>
void QuadraticProgram
( const DistSparseMatrix<Real>& Q, const DistSparseMatrix<Real>& A, 
  const DistMultiVec<Real>& b, const DistMultiVec<Real>& c,
        DistMultiVec<Real>& x,
  const QuadProgCtrl<Real>& ctrl=QuadProgCtrl<Real>() );

namespace quad_prog {

// Mehrotra's Predictor-Corrector Infeasible Interior Point Method
// ---------------------------------------------------------------
template<typename Real>
void Mehrotra
( const Matrix<Real>& Q, const Matrix<Real>& A,
  const Matrix<Real>& b, const Matrix<Real>& c,
  Matrix<Real>& s, Matrix<Real>& x, Matrix<Real>& l,
  const MehrotraCtrl<Real>& ctrl=MehrotraCtrl<Real>() );
template<typename Real>
void Mehrotra
( const AbstractDistMatrix<Real>& Q, const AbstractDistMatrix<Real>& A,
  const AbstractDistMatrix<Real>& b, const AbstractDistMatrix<Real>& c,
  AbstractDistMatrix<Real>& s, AbstractDistMatrix<Real>& x, 
  AbstractDistMatrix<Real>& l,
  const MehrotraCtrl<Real>& ctrl=MehrotraCtrl<Real>() );
template<typename Real>
void Mehrotra
( const SparseMatrix<Real>& Q, const SparseMatrix<Real>& A,
  const Matrix<Real>& b, const Matrix<Real>& c,
  Matrix<Real>& s, Matrix<Real>& x, Matrix<Real>& l,
  const MehrotraCtrl<Real>& ctrl=MehrotraCtrl<Real>() );
template<typename Real>
void Mehrotra
( const DistSparseMatrix<Real>& Q, const DistSparseMatrix<Real>& A,
  const DistMultiVec<Real>& b, const DistMultiVec<Real>& c,
  DistMultiVec<Real>& s, DistMultiVec<Real>& x, DistMultiVec<Real>& l,
  const MehrotraCtrl<Real>& ctrl=MehrotraCtrl<Real>() );

// Infeasible Path-Following Interior Point Method (IPF)
// -----------------------------------------------------
template<typename Real>
void IPF
( const Matrix<Real>& Q, const Matrix<Real>& A,
  const Matrix<Real>& b, const Matrix<Real>& c,
  Matrix<Real>& s, Matrix<Real>& x, Matrix<Real>& l,
  const IPFCtrl<Real>& ctrl=IPFCtrl<Real>() );
template<typename Real>
void IPF
( const AbstractDistMatrix<Real>& Q, const AbstractDistMatrix<Real>& A,
  const AbstractDistMatrix<Real>& b, const AbstractDistMatrix<Real>& c,
  AbstractDistMatrix<Real>& s, AbstractDistMatrix<Real>& x, 
  AbstractDistMatrix<Real>& l, 
  const IPFCtrl<Real>& ctrl=IPFCtrl<Real>() );
template<typename Real>
void IPF
( const SparseMatrix<Real>& Q, const SparseMatrix<Real>& A,
  const Matrix<Real>& b, const Matrix<Real>& c,
  Matrix<Real>& s, Matrix<Real>& x, Matrix<Real>& l,
  const IPFCtrl<Real>& ctrl=IPFCtrl<Real>() );
template<typename Real>
void IPF
( const DistSparseMatrix<Real>& Q, const DistSparseMatrix<Real>& A,
  const DistMultiVec<Real>& b, const DistMultiVec<Real>& c,
  DistMultiVec<Real>& s, DistMultiVec<Real>& x, DistMultiVec<Real>& l,
  const IPFCtrl<Real>& ctrl=IPFCtrl<Real>() );

// Solve a set of quadratic programs of the form 
//   min 1/2 x' Q x + c' x, subject to l_b <= x <= u_b
//    x
template<typename Real>
Int ADMM
( const Matrix<Real>& Q, const Matrix<Real>& C, 
  Real lb, Real ub, Matrix<Real>& Z, 
  const ADMMCtrl<Real>& ctrl=ADMMCtrl<Real>() );
template<typename Real>
Int ADMM
( const AbstractDistMatrix<Real>& Q, const AbstractDistMatrix<Real>& C, 
  Real lb, Real ub, AbstractDistMatrix<Real>& Z,
  const ADMMCtrl<Real>& ctrl=ADMMCtrl<Real>() );

} // namespace quad_prog

// Robust Principal Component Analysis (RPCA)
// ==========================================

template<typename Real>
struct RPCACtrl
{
    bool useALM;
    bool usePivQR;
    bool progress;

    Int numPivSteps;
    Int maxIts;

    Real tau;
    Real beta;
    Real rho;
    Real tol;

    RPCACtrl() 
    : useALM(true), usePivQR(false), progress(true), 
      numPivSteps(75), maxIts(1000),
      tau(0), beta(1), rho(6), tol(1e-5)
    { }
};

template<typename F>
void RPCA
( const Matrix<F>& M, Matrix<F>& L, Matrix<F>& S,
  const RPCACtrl<Base<F>>& ctrl=RPCACtrl<Base<F>>() );

template<typename F>
void RPCA
( const AbstractDistMatrix<F>& M, AbstractDistMatrix<F>& L, 
        AbstractDistMatrix<F>& S,
  const RPCACtrl<Base<F>>& ctrl=RPCACtrl<Base<F>>() );

// Sparse inverse covariance selection
// ===================================
template<typename F>
Int SparseInvCov
( const Matrix<F>& D, Base<F> lambda, Matrix<F>& Z,
  Base<F> rho=1., Base<F> alpha=1.2, Int maxIter=500,
  Base<F> absTol=1e-6, Base<F> relTol=1e-4, bool progress=true );
template<typename F>
Int SparseInvCov
( const AbstractDistMatrix<F>& D, Base<F> lambda, AbstractDistMatrix<F>& Z,
  Base<F> rho=1., Base<F> alpha=1.2, Int maxIter=500,
  Base<F> absTol=1e-6, Base<F> relTol=1e-4, bool progress=true );

// Support Vector Machine
// ======================
template<typename Real>
Int SVM
( const Matrix<Real>& G, const Matrix<Real>& q, Matrix<Real>& z,
  Real gamma, Real rho=1, Int maxIter=500, bool inv=true, bool progress=true );
template<typename Real>
Int SVM
( const AbstractDistMatrix<Real>& G, const AbstractDistMatrix<Real>& q, 
        AbstractDistMatrix<Real>& z,
  Real gamma, Real rho=1, Int maxIter=500, bool inv=true, bool progress=true );

// Proximal maps
// =============

// Clipping
// --------
template<typename Real>
void LowerClip( Matrix<Real>& X, Real lowerBound=0 );
template<typename Real>
void LowerClip( AbstractDistMatrix<Real>& X, Real lowerBound=0 );

template<typename Real>
void UpperClip( Matrix<Real>& X, Real upperBound=0 );
template<typename Real>
void UpperClip( AbstractDistMatrix<Real>& X, Real upperBound=0 );

template<typename Real>
void Clip( Matrix<Real>& X, Real lowerBound=0, Real upperBound=1 );
template<typename Real>
void Clip( AbstractDistMatrix<Real>& X, Real lowerBound=0, Real upperBound=1 );

// Frobenius-norm proximal map
// ---------------------------
// The Frobenius norm prox returns the solution to
//     arg min || A ||_F + rho/2 || A - A0 ||_F^2
//        A
// where A0 in the input matrix.
template<typename F>
void FrobeniusProx( Matrix<F>& A, Base<F> rho );
template<typename F>
void FrobeniusProx( AbstractDistMatrix<F>& A, Base<F> rho );

// Hinge-loss proximal map
// -----------------------
// TODO: Description
template<typename Real>
void HingeLossProx( Matrix<Real>& A, Real rho );
template<typename Real>
void HingeLossProx( AbstractDistMatrix<Real>& A, Real rho );

// Logistic proximal map
// ---------------------
// The logistic proximal map returns the solution to
//    arg min sum_{i,j}[ log(1+exp(-A_{i,j})) ] + rho/2 || A - A0 ||_F^2
//       A
// where A0 is the input matrix.
template<typename Real>
void LogisticProx( Matrix<Real>& A, Real rho, Int numIts=5 );
template<typename Real>
void LogisticProx( AbstractDistMatrix<Real>& A, Real rho, Int numIts=5 );

// Singular-value soft thresholding
// --------------------------------
template<typename F>
Int SVT( Matrix<F>& A, Base<F> rho, bool relative=false );
template<typename F>
Int SVT( AbstractDistMatrix<F>& A, Base<F> rho, bool relative=false );
template<typename F>
Int SVT( Matrix<F>& A, Base<F> rho, Int relaxedRank, bool relative=false );
template<typename F>
Int SVT
( AbstractDistMatrix<F>& A, Base<F> rho, Int relaxedRank, bool relative=false );
template<typename F,Dist U>
Int SVT( DistMatrix<F,U,STAR>& A, Base<F> rho, bool relative=false );

namespace svt {

// TODO: Add SVT control structure

template<typename F>
Int Cross( Matrix<F>& A, Base<F> rho, bool relative=false );
template<typename F>
Int Cross( AbstractDistMatrix<F>& A, Base<F> rho, bool relative=false );
template<typename F>
Int Cross( DistMatrix<F,VC,STAR>& A, Base<F> rho, bool relative=false );

template<typename F>
Int Normal( Matrix<F>& A, Base<F> rho, bool relative=false );
template<typename F>
Int Normal( AbstractDistMatrix<F>& A, Base<F> rho, bool relative=false );

template<typename F>
Int PivotedQR
( Matrix<F>& A, Base<F> rho, Int numSteps, bool relative=false );
template<typename F>
Int PivotedQR
( AbstractDistMatrix<F>& A, Base<F> rho, Int numSteps, bool relative=false );

template<typename F>
Int TSQR( AbstractDistMatrix<F>& A, Base<F> rho, bool relative=false );

} // namespace svt

// Soft-thresholding
// -----------------
// Returns the solution to
//     arg min || vec(A) ||_1 + rho/2 || A - A0 ||_F^2
//        A 
// where A0 is the input matrix.
template<typename F>
F SoftThreshold( F alpha, Base<F> rho );

template<typename F>
void SoftThreshold
( Matrix<F>& A, Base<F> rho, bool relative=false );
template<typename F>
void SoftThreshold
( AbstractDistMatrix<F>& A, Base<F> rho, bool relative=false );

// Utilities
// =========

// Covariance
// ----------
template<typename F>
void Covariance( const Matrix<F>& D, Matrix<F>& S );
template<typename F>
void Covariance( const AbstractDistMatrix<F>& D, AbstractDistMatrix<F>& S );

// Log barrier
// -----------
template<typename F>
Base<F> LogBarrier( UpperOrLower uplo, const Matrix<F>& A );
template<typename F>
Base<F> LogBarrier( UpperOrLower uplo, const AbstractDistMatrix<F>& A );

template<typename F>
Base<F> LogBarrier
( UpperOrLower uplo, Matrix<F>& A, bool canOverwrite=false );
template<typename F>
Base<F> LogBarrier
( UpperOrLower uplo, AbstractDistMatrix<F>& A, bool canOverwrite=false );

// Log-det divergence
// ------------------
template<typename F>
Base<F> LogDetDiv
( UpperOrLower uplo, const Matrix<F>& A, const Matrix<F>& B );
template<typename F>
Base<F> LogDetDiv
( UpperOrLower uplo, 
  const AbstractDistMatrix<F>& A, const AbstractDistMatrix<F>& B );

// Regularized LDL
// ---------------
// NOTE: If the pivot candidate is not at least as large as the pivot tolerance
//       and with the implied sign, then it is increased by the specified value.
template<typename F>
void RegularizedLDL
( Matrix<F>& A, Base<F> pivTol,
  const Matrix<Base<F>>& regCand, 
        Matrix<Base<F>>& reg );
template<typename F>
void RegularizedLDL
( AbstractDistMatrix<F>& A, Base<F> pivTol,
  const AbstractDistMatrix<Base<F>>& regCand, 
        AbstractDistMatrix<Base<F>>& reg );

template<typename F>
void RegularizedLDL
( DistSymmInfo& info, DistSymmFrontTree<F>& L,
  Base<F> pivTol,
  const DistNodalMultiVec<Base<F>>& regCand, 
        DistNodalMultiVec<Base<F>>& reg,
  SymmFrontType newFrontType=LDL_1D );

} // namespace El

#endif // ifndef EL_OPTIMIZATION_HPP
