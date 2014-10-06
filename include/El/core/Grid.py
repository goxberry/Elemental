#
#  Copyright (c) 2009-2014, Jack Poulson
#  All rights reserved.
#
#  This file is part of Elemental and is under the BSD 2-Clause License, 
#  which can be found in the LICENSE file in the root directory, or at 
#  http://opensource.org/licenses/BSD-2-Clause
#
from environment import *

# Grid
# ====

lib.ElDefaultGrid.argtypes = [POINTER(c_void_p)]
lib.ElDefaultGrid.restype = c_uint

lib.ElGridCreate.argtypes = [MPI_Comm,c_uint,POINTER(c_void_p)]
lib.ElGridCreate.restype = c_uint

lib.ElGridCreateSpecific.argtypes = [MPI_Comm,c_int,c_uint,POINTER(c_void_p)]
lib.ElGridCreateSpecific.restype = c_uint

lib.ElGridDestroy.argtypes = [c_void_p]
lib.ElGridDestroy.restype = c_uint

lib.ElGridRow.argtypes = [c_void_p,POINTER(c_int)]
lib.ElGridRow.restype = c_uint

lib.ElGridCol.argtypes = [c_void_p,POINTER(c_int)]
lib.ElGridCol.restype = c_uint

lib.ElGridRank.argtypes = [c_void_p,POINTER(c_int)]
lib.ElGridRank.restype = c_uint

lib.ElGridHeight.argtypes = [c_void_p,POINTER(c_int)]
lib.ElGridHeight.restype = c_uint

lib.ElGridWidth.argtypes = [c_void_p,POINTER(c_int)]
lib.ElGridWidth.restype = c_uint

lib.ElGridSize.argtypes = [c_void_p,POINTER(c_int)]
lib.ElGridSize.restype = c_uint

lib.ElGridOrder.argtypes = [c_void_p,POINTER(c_uint)]
lib.ElGridOrder.restype = c_uint

lib.ElGridColComm.argtypes = [c_void_p,POINTER(MPI_Comm)]
lib.ElGridColComm.restype = c_uint

lib.ElGridRowComm.argtypes = [c_void_p,POINTER(MPI_Comm)]
lib.ElGridRowComm.restype = c_uint

lib.ElGridComm.argtypes = [c_void_p,POINTER(MPI_Comm)]
lib.ElGridComm.restype = c_uint

lib.ElGridMCRank.argtypes = [c_void_p,POINTER(c_int)]
lib.ElGridMCRank.restype = c_uint

lib.ElGridMRRank.argtypes = [c_void_p,POINTER(c_int)]
lib.ElGridMRRank.restype = c_uint

lib.ElGridVCRank.argtypes = [c_void_p,POINTER(c_int)]
lib.ElGridVCRank.restype = c_uint

lib.ElGridVRRank.argtypes = [c_void_p,POINTER(c_int)]
lib.ElGridVRRank.restype = c_uint

lib.ElGridMCSize.argtypes = [c_void_p,POINTER(c_int)]
lib.ElGridMCSize.restype = c_uint

lib.ElGridMRSize.argtypes = [c_void_p,POINTER(c_int)]
lib.ElGridMRSize.restype = c_uint

lib.ElGridVCSize.argtypes = [c_void_p,POINTER(c_int)]
lib.ElGridVCSize.restype = c_uint

lib.ElGridVRSize.argtypes = [c_void_p,POINTER(c_int)]
lib.ElGridVRSize.restype = c_uint

lib.ElGridMCComm.argtypes = [c_void_p,POINTER(MPI_Comm)]
lib.ElGridMCComm.restype = c_uint

lib.ElGridMRComm.argtypes = [c_void_p,POINTER(MPI_Comm)]
lib.ElGridMRComm.restype = c_uint

lib.ElGridVCComm.argtypes = [c_void_p,POINTER(MPI_Comm)]
lib.ElGridVCComm.restype = c_uint

lib.ElGridVRComm.argtypes = [c_void_p,POINTER(MPI_Comm)]
lib.ElGridVRComm.restype = c_uint

lib.ElGridMDComm.argtypes = [c_void_p,POINTER(MPI_Comm)]
lib.ElGridMDComm.restype = c_uint

lib.ElGridMDPerpComm.argtypes = [c_void_p,POINTER(MPI_Comm)]
lib.ElGridMDPerpComm.restype = c_uint

lib.ElGridGCD.argtypes = [c_void_p,POINTER(c_int)]
lib.ElGridGCD.restype = c_uint

lib.ElGridLCM.argtypes = [c_void_p,POINTER(c_int)]
lib.ElGridLCM.restype = c_uint

lib.ElGridInGrid.argtypes = [c_void_p,POINTER(bType)]
lib.ElGridInGrid.restype = c_uint

lib.ElGridHaveViewers.argtypes = [c_void_p,POINTER(bType)]
lib.ElGridHaveViewers.restype = c_uint

lib.ElGridOwningRank.argtypes = [c_void_p,POINTER(c_int)]
lib.ElGridOwningRank.restype = c_uint

lib.ElGridViewingRank.argtypes = [c_void_p,POINTER(c_int)]
lib.ElGridViewingRank.restype = c_uint

lib.ElGridVCToViewingMap.argtypes = [c_void_p,c_int,POINTER(c_int)]
lib.ElGridVCToViewingMap.restype = c_uint

lib.ElGridOwningGroup.argtypes = [c_void_p,POINTER(MPI_Group)]
lib.ElGridOwningGroup.restype = c_uint

lib.ElGridOwningComm.argtypes = [c_void_p,POINTER(MPI_Comm)]
lib.ElGridOwningComm.restype = c_uint

lib.ElGridViewingComm.argtypes = [c_void_p,POINTER(MPI_Comm)]
lib.ElGridViewingComm.restype = c_uint

lib.ElGridDiagPath.argtypes = [c_void_p,c_int,POINTER(c_int)]
lib.ElGridDiagPath.restype = c_uint

lib.ElGridDiagPathRank.argtypes = [c_void_p,c_int,POINTER(c_int)]
lib.ElGridDiagPathRank.restype = c_uint

lib.ElGridFirstVCRank.argtypes = [c_void_p,c_int,POINTER(c_int)]
lib.ElGridFirstVCRank.restype = c_uint

lib.ElGridFindFactor.argtypes = [c_int,POINTER(c_int)]
lib.ElGridFindFactor.restype = c_uint

class Grid(object):
  def __init__(self,create=True):
    self.obj = c_void_p()
    if create:
      lib.ElDefaultGrid(pointer(self.obj))
  @classmethod
  def FromComm(cls,comm,order):
    g = cls(False)
    lib.ElGridCreate(comm,order,pointer(g.obj))
  @classmethod
  def FromCommSpecific(cls,comm,height,order):
    g = cls(False)
    lib.ElGridCreateSpecific(comm,height,order,pointer(g.obj))
  def Destroy(self): 
    lib.ElGridDestroy(self.obj)
  def Row(self):
    row = c_int()
    lib.ElGridRow(self.obj,pointer(row))
    return row
  def Col(self):
    col = c_int()
    lib.ElGridCol(self.obj,pointer(col))
    return col
  def Rank(self):
    rank = c_int()
    lib.ElGridRank(self.obj,pointer(rank))
    return rank
  def Height(self):
    height = c_int()
    lib.ElGridHeight(self.obj,pointer(height))
    return height
  def Width(self):
    width = c_int()
    lib.ElGridWidth(self.obj,pointer(width))
    return width
  def Size(self):
    size = c_int()
    lib.ElGridSize(self.obj,pointer(size))
    return size
  def Order(self):
    order = c_uint()
    lib.ElGridOrder(self.obj,pointer(order))
    return order
  def ColComm(self):
    colComm = MPI_Comm()
    lib.ElGridColComm(self.obj,pointer(colComm))
    return colComm
  def RowComm(self):
    rowComm = MPI_Comm()
    lib.ElGridColComm(self.obj,pointer(rowComm))
    return rowComm
  def Comm(self):
    comm = MPI_Comm()
    lib.ElGridComm(self.obj,pointer(comm))
    return comm
  def MCRank(self):
    rank = c_int()
    lib.ElGridMCRank(self.obj,pointer(rank))
    return rank
  def MRRank(self):
    rank = c_int()
    lib.ElGridMRRank(self.obj,pointer(rank))
    return rank
  def VCRank(self):
    rank = c_int()
    lib.ElGridVCRank(self.obj,pointer(rank))
    return rank
  def VRRank(self):
    rank = c_int()
    lib.ElGridVRRank(self.obj,pointer(rank))
    return rank
  def MCSize(self):
    size = c_int()
    lib.ElGridMCSize(self.obj,pointer(size))
    return size
  def MRSize(self):
    size = c_int()
    lib.ElGridMRSize(self.obj,pointer(size))
    return size
  def VCSize(self):
    size = c_int()
    lib.ElGridVCSize(self.obj,pointer(size))
    return size
  def VRSize(self):
    size = c_int()
    lib.ElGridVRSize(self.obj,pointer(size))
    return size
  def MCComm(self):
    comm = MPI_Comm()
    lib.ElGridMCComm(self.obj,pointer(comm))
    return comm
  def MRComm(self):
    comm = MPI_Comm()
    lib.ElGridMRComm(self.obj,pointer(comm))
    return comm
  def VCComm(self):
    comm = MPI_Comm()
    lib.ElGridVCComm(self.obj,pointer(comm))
    return comm
  def VRComm(self):
    comm = MPI_Comm()
    lib.ElGridVRComm(self.obj,pointer(comm))
    return comm
  def MDComm(self):
    comm = MPI_Comm()
    lib.ElGridMDComm(self.obj,pointer(comm))
    return comm
  def MDPerpComm(self):
    comm = MPI_Comm()
    lib.ElGridMDPerpComm(self.obj,pointer(comm))
  def GCD(self):
    gcd = c_int()
    lib.ElGridGCD(self.obj,pointer(gcd))
    return gcd
  def LCM(self):
    lcm = c_int()
    lib.ElGridLCM(self.obj,pointer(lcm))
    return lcm
  def InGrid(self):
    inGrid = bType()
    lib.ElGridInGrid(self.obj,pointer(inGrid))
    return inGrid
  def HaveViewers(self):
    haveViewers = bType() 
    lib.ElGridHaveViewers(self.obj,pointer(haveViewers))
    return haveViewers
  def OwningRank(self):
    rank = c_int()
    lib.ElGridOwningRank(self.obj,pointer(rank))
    return rank
  def ViewingRank(self):
    rank = c_int()
    lib.ElGridViewingRank(self.obj,pointer(rank))
    return rank
  def VCToViewingMap(self,vcRank):
    viewingRank = c_int()
    lib.ElGridVCToViewingMap(self.obj,vcRank,pointer(viewingRank))
    return viewingRank
  def OwningGroup(self):
    group = MPI_Group()
    lib.ElGridOwningGroup(self.obj,pointer(group))
    return group
  def OwningComm(self):
    comm = MPI_Comm()
    lib.ElGridOwningComm(self.obj,pointer(comm))
    return comm
  def ViewingComm(self):
    comm = MPI_Comm()
    lib.ElGridViewingComm(self.obj,pointer(comm))
    return comm
  def DiagPath(self,vcRank):
    diagPath = c_int()
    lib.ElGridDiagPath(self.obj,vcRank,pointer(diagPath))
    return diagPath
  def DiagPathRank(self,vcRank):
    pathRank = c_int()
    lib.ElGridDiagPathRank(self.obj,vcRank,pointer(pathRank))
    return pathRank
  def FirstVCRank(self,diagPath):
    firstVCRank = c_int()
    lib.ElGridFirstVCRank(self.obj,diagPath,pointer(firstVCRank))
    return firstVCRank
  # NOTE: The following method is static
  def FindFactor(numProcs):
    factor = c_int()
    lib.ElGridFindFactor(numProcs,pointer(factor))
    return factor

def DefaultGrid():
  return Grid()