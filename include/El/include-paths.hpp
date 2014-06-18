/*
   Copyright (c) 2009-2014, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#pragma once
#ifndef EL_INCLUDEPATHS_HPP
#define EL_INCLUDEPATHS_HPP

// LAPACK-like routines
// ====================
#define EL_LAPACK_INC      "El/lapack-like.hpp"

// Special matrices
// ================
#define EL_MATRICES_INC      "El/matrices.hpp"
#define EL_MATRICES_DECL_INC "El/matrices/decl.hpp"
#define EL_MATRICES_IMPL_INC "El/matrices/impl.hpp"

// Deterministic matrices
// ----------------------
#define EL_EGOROV_INC           "El/matrices/Egorov.hpp"
#define EL_GEAR_INC             "El/matrices/Gear.hpp"
#define EL_GKS_INC              "El/matrices/GKS.hpp"
#define EL_GRCAR_INC            "El/matrices/Grcar.hpp"
#define EL_HANKEL_INC           "El/matrices/Hankel.hpp"
#define EL_HANOWA_INC           "El/matrices/Hanowa.hpp"
#define EL_HELMHOLTZ_INC        "El/matrices/Helmholtz.hpp"
#define EL_HELMHOLTZPML_INC     "El/matrices/HelmholtzPML.hpp"
#define EL_HERMITIANFROMEVD_INC "El/matrices/HermitianFromEVD.hpp"
#define EL_KAHAN_INC            "El/matrices/Kahan.hpp"
#define EL_KMS_INC              "El/matrices/KMS.hpp"
#define EL_LAPLACIAN_INC        "El/matrices/Laplacian.hpp"
#define EL_LAUCHLI_INC          "El/matrices/Lauchli.hpp"
#define EL_LEGENDRE_INC         "El/matrices/Legendre.hpp"
#define EL_LEHMER_INC           "El/matrices/Lehmer.hpp"
#define EL_LOTKIN_INC           "El/matrices/Lotkin.hpp"
#define EL_MINIJ_INC            "El/matrices/MinIJ.hpp"
#define EL_NORMALFROMEVD_INC    "El/matrices/NormalFromEVD.hpp"
#define EL_ONETWOONE_INC        "El/matrices/OneTwoOne.hpp"
#define EL_PARTER_INC           "El/matrices/Parter.hpp"
#define EL_PEI_INC              "El/matrices/Pei.hpp"
#define EL_REDHEFFER_INC        "El/matrices/Redheffer.hpp"
#define EL_RIEMANN_INC          "El/matrices/Riemann.hpp"
#define EL_RIFFLE_INC           "El/matrices/Riffle.hpp"
#define EL_RIS_INC              "El/matrices/Ris.hpp"
#define EL_TOEPLITZ_INC         "El/matrices/Toeplitz.hpp"
#define EL_TREFETHEN_INC        "El/matrices/Trefethen.hpp"
#define EL_TRIANGLE_INC         "El/matrices/Triangle.hpp"
#define EL_TRIW_INC             "El/matrices/TriW.hpp"
#define EL_WALSH_INC            "El/matrices/Walsh.hpp"
#define EL_WHALE_INC            "El/matrices/Whale.hpp"
#define EL_WILKINSON_INC        "El/matrices/Wilkinson.hpp"

// Random matrices
// ---------------
#define EL_HATANONELSON_INC \
  "El/matrices/HatanoNelson.hpp"
#define EL_HERMITIANUNIFORMSPECTRUM_INC \
  "El/matrices/HermitianUniformSpectrum.hpp"
#define EL_NORMALUNIFORMSPECTRUM_INC \
  "El/matrices/NormalUniformSpectrum.hpp"
#define EL_UNIFORMHELMHOLTZGREENS_INC \
  "El/matrices/UniformHelmholtzGreens.hpp"

#endif // ifndef EL_INCLUDEPATHS_HPP
