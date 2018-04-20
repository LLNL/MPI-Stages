#include <math.h>
#if USE_MPI
# include <mpi.h>
#endif
#if _OPENMP
#include <omp.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <cstdlib>
#include "lulesh.h"

/////////////////////////////////////////////////////////////////////
Domain::Domain() :
m_e_cut(Real_t(1.0e-7)),
   m_p_cut(Real_t(1.0e-7)),
   m_q_cut(Real_t(1.0e-7)),
   m_v_cut(Real_t(1.0e-10)),
   m_u_cut(Real_t(1.0e-7)),
   m_hgcoef(Real_t(3.0)),
   m_ss4o3(Real_t(4.0)/Real_t(3.0)),
   m_qstop(Real_t(1.0e+12)),
   m_monoq_max_slope(Real_t(1.0)),
   m_monoq_limiter_mult(Real_t(2.0)),
   m_qlc_monoq(Real_t(0.5)),
   m_qqc_monoq(Real_t(2.0)/Real_t(3.0)),
   m_qqc(Real_t(2.0)),
   m_eosvmax(Real_t(1.0e+9)),
   m_eosvmin(Real_t(1.0e-9)),
   m_pmin(Real_t(0.)),
   m_emin(Real_t(-1.0e+15)),
   m_dvovmax(Real_t(0.1)),
   m_refdens(Real_t(1.0))
{
}
Domain::Domain(Int_t numRanks, Index_t colLoc,
               Index_t rowLoc, Index_t planeLoc,
               Index_t nx, int tp, int nr, int balance, Int_t cost)
   :
   m_e_cut(Real_t(1.0e-7)),
   m_p_cut(Real_t(1.0e-7)),
   m_q_cut(Real_t(1.0e-7)),
   m_v_cut(Real_t(1.0e-10)),
   m_u_cut(Real_t(1.0e-7)),
   m_hgcoef(Real_t(3.0)),
   m_ss4o3(Real_t(4.0)/Real_t(3.0)),
   m_qstop(Real_t(1.0e+12)),
   m_monoq_max_slope(Real_t(1.0)),
   m_monoq_limiter_mult(Real_t(2.0)),
   m_qlc_monoq(Real_t(0.5)),
   m_qqc_monoq(Real_t(2.0)/Real_t(3.0)),
   m_qqc(Real_t(2.0)),
   m_eosvmax(Real_t(1.0e+9)),
   m_eosvmin(Real_t(1.0e-9)),
   m_pmin(Real_t(0.)),
   m_emin(Real_t(-1.0e+15)),
   m_dvovmax(Real_t(0.1)),
   m_refdens(Real_t(1.0))
{

   Index_t edgeElems = nx ;
   Index_t edgeNodes = edgeElems+1 ;
   this->cost() = cost;

   m_tp       = tp ;
   m_numRanks = numRanks ;

   ///////////////////////////////
   //   Initialize Sedov Mesh
   ///////////////////////////////

   // construct a uniform box for this processor

   m_colLoc   =   colLoc ;
   m_rowLoc   =   rowLoc ;
   m_planeLoc = planeLoc ;
   
   m_sizeX = edgeElems ;
   m_sizeY = edgeElems ;
   m_sizeZ = edgeElems ;
   m_numElem = edgeElems*edgeElems*edgeElems ;

   m_numNode = edgeNodes*edgeNodes*edgeNodes ;

   m_regNumList = new Index_t[numElem()] ;  // material indexset

   // Elem-centered 
   AllocateElemPersistent(numElem()) ;

   // Node-centered 
   AllocateNodePersistent(numNode()) ;

   SetupCommBuffers(edgeNodes);

   // Basic Field Initialization 
   for (Index_t i=0; i<numElem(); ++i) {
      e(i) =  Real_t(0.0) ;
      p(i) =  Real_t(0.0) ;
      q(i) =  Real_t(0.0) ;
      ss(i) = Real_t(0.0) ;
   }

   // Note - v initializes to 1.0, not 0.0!
   for (Index_t i=0; i<numElem(); ++i) {
      v(i) = Real_t(1.0) ;
   }

   for (Index_t i=0; i<numNode(); ++i) {
      xd(i) = Real_t(0.0) ;
      yd(i) = Real_t(0.0) ;
      zd(i) = Real_t(0.0) ;
   }

   for (Index_t i=0; i<numNode(); ++i) {
      xdd(i) = Real_t(0.0) ;
      ydd(i) = Real_t(0.0) ;
      zdd(i) = Real_t(0.0) ;
   }

   for (Index_t i=0; i<numNode(); ++i) {
      nodalMass(i) = Real_t(0.0) ;
   }

   BuildMesh(nx, edgeNodes, edgeElems);

#if _OPENMP
   SetupThreadSupportStructures();
#else
   // These arrays are not used if we're not threaded
   m_nodeElemStart = NULL;
   m_nodeElemCornerList = NULL;
#endif

   // Setup region index sets. For now, these are constant sized
   // throughout the run, but could be changed every cycle to 
   // simulate effects of ALE on the lagrange solver
   CreateRegionIndexSets(nr, balance);

   // Setup symmetry nodesets
   SetupSymmetryPlanes(edgeNodes);

   // Setup element connectivities
   SetupElementConnectivities(edgeElems);

   // Setup symmetry planes and free surface boundary arrays
   SetupBoundaryConditions(edgeElems);


   // Setup defaults

   // These can be changed (requires recompile) if you want to run
   // with a fixed timestep, or to a different end time, but it's
   // probably easier/better to just run a fixed number of timesteps
   // using the -i flag in 2.x

   dtfixed() = Real_t(-1.0e-6) ; // Negative means use courant condition
   stoptime()  = Real_t(1.0e-2); // *Real_t(edgeElems*tp/45.0) ;

   // Initial conditions
   deltatimemultlb() = Real_t(1.1) ;
   deltatimemultub() = Real_t(1.2) ;
   dtcourant() = Real_t(1.0e+20) ;
   dthydro()   = Real_t(1.0e+20) ;
   dtmax()     = Real_t(1.0e-2) ;
   time()    = Real_t(0.) ;
   cycle()   = Int_t(0) ;

   // initialize field data 
   for (Index_t i=0; i<numElem(); ++i) {
      Real_t x_local[8], y_local[8], z_local[8] ;
      Index_t *elemToNode = nodelist(i) ;
      for( Index_t lnode=0 ; lnode<8 ; ++lnode )
      {
        Index_t gnode = elemToNode[lnode];
        x_local[lnode] = x(gnode);
        y_local[lnode] = y(gnode);
        z_local[lnode] = z(gnode);
      }

      // volume calculations
      Real_t volume = CalcElemVolume(x_local, y_local, z_local );
      volo(i) = volume ;
      elemMass(i) = volume ;
      for (Index_t j=0; j<8; ++j) {
         Index_t idx = elemToNode[j] ;
         nodalMass(idx) += volume / Real_t(8.0) ;
      }
   }

   // deposit initial energy
   // An energy of 3.948746e+7 is correct for a problem with
   // 45 zones along a side - we need to scale it
   const Real_t ebase = Real_t(3.948746e+7);
   Real_t scale = (nx*m_tp)/Real_t(45.0);
   Real_t einit = ebase*scale*scale*scale;
   if (m_rowLoc + m_colLoc + m_planeLoc == 0) {
      // Dump into the first zone (which we know is in the corner)
      // of the domain that sits at the origin
      e(0) = einit;
   }
   //set initial deltatime base on analytic CFL calculation
   deltatime() = (Real_t(.5)*cbrt(volo(0)))/sqrt(Real_t(2.0)*einit);

} // End constructor


////////////////////////////////////////////////////////////////////////////////
void
Domain::BuildMesh(Int_t nx, Int_t edgeNodes, Int_t edgeElems)
{
  Index_t meshEdgeElems = m_tp*nx ;

  // initialize nodal coordinates 
  Index_t nidx = 0 ;
  Real_t tz = Real_t(1.125)*Real_t(m_planeLoc*nx)/Real_t(meshEdgeElems) ;
  for (Index_t plane=0; plane<edgeNodes; ++plane) {
    Real_t ty = Real_t(1.125)*Real_t(m_rowLoc*nx)/Real_t(meshEdgeElems) ;
    for (Index_t row=0; row<edgeNodes; ++row) {
      Real_t tx = Real_t(1.125)*Real_t(m_colLoc*nx)/Real_t(meshEdgeElems) ;
      for (Index_t col=0; col<edgeNodes; ++col) {
	x(nidx) = tx ;
	y(nidx) = ty ;
	z(nidx) = tz ;
	++nidx ;
	// tx += ds ; // may accumulate roundoff... 
	tx = Real_t(1.125)*Real_t(m_colLoc*nx+col+1)/Real_t(meshEdgeElems) ;
      }
      // ty += ds ;  // may accumulate roundoff... 
      ty = Real_t(1.125)*Real_t(m_rowLoc*nx+row+1)/Real_t(meshEdgeElems) ;
    }
    // tz += ds ;  // may accumulate roundoff... 
    tz = Real_t(1.125)*Real_t(m_planeLoc*nx+plane+1)/Real_t(meshEdgeElems) ;
  }


  // embed hexehedral elements in nodal point lattice 
  Index_t zidx = 0 ;
  nidx = 0 ;
  for (Index_t plane=0; plane<edgeElems; ++plane) {
    for (Index_t row=0; row<edgeElems; ++row) {
      for (Index_t col=0; col<edgeElems; ++col) {
	Index_t *localNode = nodelist(zidx) ;
	localNode[0] = nidx                                       ;
	localNode[1] = nidx                                   + 1 ;
	localNode[2] = nidx                       + edgeNodes + 1 ;
	localNode[3] = nidx                       + edgeNodes     ;
	localNode[4] = nidx + edgeNodes*edgeNodes                 ;
	localNode[5] = nidx + edgeNodes*edgeNodes             + 1 ;
	localNode[6] = nidx + edgeNodes*edgeNodes + edgeNodes + 1 ;
	localNode[7] = nidx + edgeNodes*edgeNodes + edgeNodes     ;
	++zidx ;
	++nidx ;
      }
      ++nidx ;
    }
    nidx += edgeNodes ;
  }
}


////////////////////////////////////////////////////////////////////////////////
void
Domain::SetupThreadSupportStructures()
{
#if _OPENMP
   Index_t numthreads = omp_get_max_threads();
#else
   Index_t numthreads = 1;
#endif

  if (numthreads > 1) {
    // set up node-centered indexing of elements 
    Index_t *nodeElemCount = new Index_t[numNode()] ;

    for (Index_t i=0; i<numNode(); ++i) {
      nodeElemCount[i] = 0 ;
    }

    for (Index_t i=0; i<numElem(); ++i) {
      Index_t *nl = nodelist(i) ;
      for (Index_t j=0; j < 8; ++j) {
	++(nodeElemCount[nl[j]] );
      }
    }

    m_nodeElemStart = new Index_t[numNode()+1] ;

    m_nodeElemStart[0] = 0;

    for (Index_t i=1; i <= numNode(); ++i) {
      m_nodeElemStart[i] =
	m_nodeElemStart[i-1] + nodeElemCount[i-1] ;
    }
       
    m_nodeElemCornerList = new Index_t[m_nodeElemStart[numNode()]];

    for (Index_t i=0; i < numNode(); ++i) {
      nodeElemCount[i] = 0;
    }

    for (Index_t i=0; i < numElem(); ++i) {
      Index_t *nl = nodelist(i) ;
      for (Index_t j=0; j < 8; ++j) {
	Index_t m = nl[j];
	Index_t k = i*8 + j ;
	Index_t offset = m_nodeElemStart[m] + nodeElemCount[m] ;
	m_nodeElemCornerList[offset] = k;
	++(nodeElemCount[m]) ;
      }
    }

    Index_t clSize = m_nodeElemStart[numNode()] ;
    for (Index_t i=0; i < clSize; ++i) {
      Index_t clv = m_nodeElemCornerList[i] ;
      if ((clv < 0) || (clv > numElem()*8)) {
	fprintf(stderr,
		"AllocateNodeElemIndexes(): nodeElemCornerList entry out of range!\n");
#if USE_MPI
	MPI_Abort(MPI_COMM_WORLD, -1);
#else
	exit(-1);
#endif
      }
    }

    delete [] nodeElemCount ;
  }
  else {
    // These arrays are not used if we're not threaded
    m_nodeElemStart = NULL;
    m_nodeElemCornerList = NULL;
  }
}


////////////////////////////////////////////////////////////////////////////////
void
Domain::SetupCommBuffers(Int_t edgeNodes)
{
  // allocate a buffer large enough for nodal ghost data 
  Index_t maxEdgeSize = MAX(this->sizeX(), MAX(this->sizeY(), this->sizeZ()))+1 ;
  m_maxPlaneSize = CACHE_ALIGN_REAL(maxEdgeSize*maxEdgeSize) ;
  m_maxEdgeSize = CACHE_ALIGN_REAL(maxEdgeSize) ;

  // assume communication to 6 neighbors by default 
  m_rowMin = (m_rowLoc == 0)        ? 0 : 1;
  m_rowMax = (m_rowLoc == m_tp-1)     ? 0 : 1;
  m_colMin = (m_colLoc == 0)        ? 0 : 1;
  m_colMax = (m_colLoc == m_tp-1)     ? 0 : 1;
  m_planeMin = (m_planeLoc == 0)    ? 0 : 1;
  m_planeMax = (m_planeLoc == m_tp-1) ? 0 : 1;

#if USE_MPI   
  // account for face communication 
  Index_t comBufSize =
    (m_rowMin + m_rowMax + m_colMin + m_colMax + m_planeMin + m_planeMax) *
    m_maxPlaneSize * MAX_FIELDS_PER_MPI_COMM ;

  // account for edge communication 
  comBufSize +=
    ((m_rowMin & m_colMin) + (m_rowMin & m_planeMin) + (m_colMin & m_planeMin) +
     (m_rowMax & m_colMax) + (m_rowMax & m_planeMax) + (m_colMax & m_planeMax) +
     (m_rowMax & m_colMin) + (m_rowMin & m_planeMax) + (m_colMin & m_planeMax) +
     (m_rowMin & m_colMax) + (m_rowMax & m_planeMin) + (m_colMax & m_planeMin)) *
    m_maxEdgeSize * MAX_FIELDS_PER_MPI_COMM ;

  // account for corner communication 
  // factor of 16 is so each buffer has its own cache line 
  comBufSize += ((m_rowMin & m_colMin & m_planeMin) +
		 (m_rowMin & m_colMin & m_planeMax) +
		 (m_rowMin & m_colMax & m_planeMin) +
		 (m_rowMin & m_colMax & m_planeMax) +
		 (m_rowMax & m_colMin & m_planeMin) +
		 (m_rowMax & m_colMin & m_planeMax) +
		 (m_rowMax & m_colMax & m_planeMin) +
		 (m_rowMax & m_colMax & m_planeMax)) * CACHE_COHERENCE_PAD_REAL ;

  this->commBufSize = comBufSize;

  this->commDataSend = new Real_t[comBufSize] ;
  this->commDataRecv = new Real_t[comBufSize] ;
  // prevent floating point exceptions 
  memset(this->commDataSend, 0, comBufSize*sizeof(Real_t)) ;
  memset(this->commDataRecv, 0, comBufSize*sizeof(Real_t)) ;
#endif   

  // Boundary nodesets
  if (m_colLoc == 0)
    m_symmX.resize(edgeNodes*edgeNodes);
  if (m_rowLoc == 0)
    m_symmY.resize(edgeNodes*edgeNodes);
  if (m_planeLoc == 0)
    m_symmZ.resize(edgeNodes*edgeNodes);
}


////////////////////////////////////////////////////////////////////////////////
void
Domain::CreateRegionIndexSets(Int_t nr, Int_t balance)
{
#if USE_MPI   
   Index_t myRank;
   MPI_Comm_rank(MPI_COMM_WORLD, &myRank) ;
   srand(myRank);
#else
   srand(0);
   Index_t myRank = 0;
#endif
   this->numReg() = nr;
   m_regElemSize = new Index_t[numReg()];
   m_regElemlist = new Index_t*[numReg()];
   Index_t nextIndex = 0;
   //if we only have one region just fill it
   // Fill out the regNumList with material numbers, which are always
   // the region index plus one 
   if(numReg() == 1) {
      while (nextIndex < numElem()) {
	 this->regNumList(nextIndex) = 1;
         nextIndex++;
      }
      regElemSize(0) = 0;
   }
   //If we have more than one region distribute the elements.
   else {
      Int_t regionNum;
      Int_t regionVar;
      Int_t lastReg = -1;
      Int_t binSize;
      Index_t elements;
      Index_t runto = 0;
      Int_t costDenominator = 0;
      Int_t* regBinEnd = new Int_t[numReg()];
      //Determine the relative weights of all the regions.  This is based off the -b flag.  Balance is the value passed into b.  
      for (Index_t i=0 ; i<numReg() ; ++i) {
         regElemSize(i) = 0;
	 costDenominator += pow((i+1), balance);  //Total sum of all regions weights
	 regBinEnd[i] = costDenominator;  //Chance of hitting a given region is (regBinEnd[i] - regBinEdn[i-1])/costDenominator
      }
      //Until all elements are assigned
      while (nextIndex < numElem()) {
	 //pick the region
	 regionVar = rand() % costDenominator;
	 Index_t i = 0;
         while(regionVar >= regBinEnd[i])
	    i++;
         //rotate the regions based on MPI rank.  Rotation is Rank % NumRegions this makes each domain have a different region with 
         //the highest representation
	 regionNum = ((i + myRank) % numReg()) + 1;
	 // make sure we don't pick the same region twice in a row
         while(regionNum == lastReg) {
	    regionVar = rand() % costDenominator;
	    i = 0;
            while(regionVar >= regBinEnd[i])
	       i++;
	    regionNum = ((i + myRank) % numReg()) + 1;
         }
	 //Pick the bin size of the region and determine the number of elements.
         binSize = rand() % 1000;
	 if(binSize < 773) {
	   elements = rand() % 15 + 1;
	 }
	 else if(binSize < 937) {
	   elements = rand() % 16 + 16;
	 }
	 else if(binSize < 970) {
	   elements = rand() % 32 + 32;
	 }
	 else if(binSize < 974) {
	   elements = rand() % 64 + 64;
	 } 
	 else if(binSize < 978) {
	   elements = rand() % 128 + 128;
	 }
	 else if(binSize < 981) {
	   elements = rand() % 256 + 256;
	 }
	 else
	    elements = rand() % 1537 + 512;
	 runto = elements + nextIndex;
	 //Store the elements.  If we hit the end before we run out of elements then just stop.
         while (nextIndex < runto && nextIndex < numElem()) {
	    this->regNumList(nextIndex) = regionNum;
	    nextIndex++;
	 }
	 lastReg = regionNum;
      } 
   }
   // Convert regNumList to region index sets
   // First, count size of each region 
   for (Index_t i=0 ; i<numElem() ; ++i) {
      int r = this->regNumList(i)-1; // region index == regnum-1
      regElemSize(r)++;
   }
   // Second, allocate each region index set
   for (Index_t i=0 ; i<numReg() ; ++i) {
      m_regElemlist[i] = new Index_t[regElemSize(i)];
      regElemSize(i) = 0;
   }
   // Third, fill index sets
   for (Index_t i=0 ; i<numElem() ; ++i) {
      Index_t r = regNumList(i)-1;       // region index == regnum-1
      Index_t regndx = regElemSize(r)++; // Note increment
      regElemlist(r,regndx) = i;
   }
   
}

/////////////////////////////////////////////////////////////
void 
Domain::SetupSymmetryPlanes(Int_t edgeNodes)
{
  Index_t nidx = 0 ;
  for (Index_t i=0; i<edgeNodes; ++i) {
    Index_t planeInc = i*edgeNodes*edgeNodes ;
    Index_t rowInc   = i*edgeNodes ;
    for (Index_t j=0; j<edgeNodes; ++j) {
      if (m_planeLoc == 0) {
	m_symmZ[nidx] = rowInc   + j ;
      }
      if (m_rowLoc == 0) {
	m_symmY[nidx] = planeInc + j ;
      }
      if (m_colLoc == 0) {
	m_symmX[nidx] = planeInc + j*edgeNodes ;
      }
      ++nidx ;
    }
  }
}



/////////////////////////////////////////////////////////////
void
Domain::SetupElementConnectivities(Int_t edgeElems)
{
   lxim(0) = 0 ;
   for (Index_t i=1; i<numElem(); ++i) {
      lxim(i)   = i-1 ;
      lxip(i-1) = i ;
   }
   lxip(numElem()-1) = numElem()-1 ;

   for (Index_t i=0; i<edgeElems; ++i) {
      letam(i) = i ; 
      letap(numElem()-edgeElems+i) = numElem()-edgeElems+i ;
   }
   for (Index_t i=edgeElems; i<numElem(); ++i) {
      letam(i) = i-edgeElems ;
      letap(i-edgeElems) = i ;
   }

   for (Index_t i=0; i<edgeElems*edgeElems; ++i) {
      lzetam(i) = i ;
      lzetap(numElem()-edgeElems*edgeElems+i) = numElem()-edgeElems*edgeElems+i ;
   }
   for (Index_t i=edgeElems*edgeElems; i<numElem(); ++i) {
      lzetam(i) = i - edgeElems*edgeElems ;
      lzetap(i-edgeElems*edgeElems) = i ;
   }
}

/////////////////////////////////////////////////////////////
void
Domain::SetupBoundaryConditions(Int_t edgeElems) 
{
  Index_t ghostIdx[6] ;  // offsets to ghost locations

  // set up boundary condition information
  for (Index_t i=0; i<numElem(); ++i) {
     elemBC(i) = Int_t(0) ;
  }

  for (Index_t i=0; i<6; ++i) {
    ghostIdx[i] = INT_MIN ;
  }

  Int_t pidx = numElem() ;
  if (m_planeMin != 0) {
    ghostIdx[0] = pidx ;
    pidx += sizeX()*sizeY() ;
  }

  if (m_planeMax != 0) {
    ghostIdx[1] = pidx ;
    pidx += sizeX()*sizeY() ;
  }

  if (m_rowMin != 0) {
    ghostIdx[2] = pidx ;
    pidx += sizeX()*sizeZ() ;
  }

  if (m_rowMax != 0) {
    ghostIdx[3] = pidx ;
    pidx += sizeX()*sizeZ() ;
  }

  if (m_colMin != 0) {
    ghostIdx[4] = pidx ;
    pidx += sizeY()*sizeZ() ;
  }

  if (m_colMax != 0) {
    ghostIdx[5] = pidx ;
  }

  // symmetry plane or free surface BCs 
  for (Index_t i=0; i<edgeElems; ++i) {
    Index_t planeInc = i*edgeElems*edgeElems ;
    Index_t rowInc   = i*edgeElems ;
    for (Index_t j=0; j<edgeElems; ++j) {
      if (m_planeLoc == 0) {
	elemBC(rowInc+j) |= ZETA_M_SYMM ;
      }
      else {
	elemBC(rowInc+j) |= ZETA_M_COMM ;
	lzetam(rowInc+j) = ghostIdx[0] + rowInc + j ;
      }

      if (m_planeLoc == m_tp-1) {
	elemBC(rowInc+j+numElem()-edgeElems*edgeElems) |=
	  ZETA_P_FREE;
      }
      else {
	elemBC(rowInc+j+numElem()-edgeElems*edgeElems) |=
	  ZETA_P_COMM ;
	lzetap(rowInc+j+numElem()-edgeElems*edgeElems) =
	  ghostIdx[1] + rowInc + j ;
      }

      if (m_rowLoc == 0) {
	elemBC(planeInc+j) |= ETA_M_SYMM ;
      }
      else {
	elemBC(planeInc+j) |= ETA_M_COMM ;
	letam(planeInc+j) = ghostIdx[2] + rowInc + j ;
      }

      if (m_rowLoc == m_tp-1) {
	elemBC(planeInc+j+edgeElems*edgeElems-edgeElems) |= 
	  ETA_P_FREE ;
      }
      else {
	elemBC(planeInc+j+edgeElems*edgeElems-edgeElems) |= 
	  ETA_P_COMM ;
	letap(planeInc+j+edgeElems*edgeElems-edgeElems) =
	  ghostIdx[3] +  rowInc + j ;
      }

      if (m_colLoc == 0) {
	elemBC(planeInc+j*edgeElems) |= XI_M_SYMM ;
      }
      else {
	elemBC(planeInc+j*edgeElems) |= XI_M_COMM ;
	lxim(planeInc+j*edgeElems) = ghostIdx[4] + rowInc + j ;
      }

      if (m_colLoc == m_tp-1) {
	elemBC(planeInc+j*edgeElems+edgeElems-1) |= XI_P_FREE ;
      }
      else {
	elemBC(planeInc+j*edgeElems+edgeElems-1) |= XI_P_COMM ;
	lxip(planeInc+j*edgeElems+edgeElems-1) =
	  ghostIdx[5] + rowInc + j ;
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////
void InitMeshDecomp(Int_t numRanks, Int_t myRank,
                    Int_t *col, Int_t *row, Int_t *plane, Int_t *side)
{
   Int_t testProcs;
   Int_t dx, dy, dz;
   Int_t myDom;
   
   // Assume cube processor layout for now 
   testProcs = Int_t(cbrt(Real_t(numRanks))+0.5) ;
   if (testProcs*testProcs*testProcs != numRanks) {
      printf("Num processors must be a cube of an integer (1, 8, 27, ...)\n") ;
#if USE_MPI      
      MPI_Abort(MPI_COMM_WORLD, -1) ;
#else
      exit(-1);
#endif
   }
   if (sizeof(Real_t) != 4 && sizeof(Real_t) != 8) {
      printf("MPI operations only support float and double right now...\n");
#if USE_MPI      
      MPI_Abort(MPI_COMM_WORLD, -1) ;
#else
      exit(-1);
#endif
   }
   if (MAX_FIELDS_PER_MPI_COMM > CACHE_COHERENCE_PAD_REAL) {
      printf("corner element comm buffers too small.  Fix code.\n") ;
#if USE_MPI      
      MPI_Abort(MPI_COMM_WORLD, -1) ;
#else
      exit(-1);
#endif
   }

   dx = testProcs ;
   dy = testProcs ;
   dz = testProcs ;

   // temporary test
   if (dx*dy*dz != numRanks) {
      printf("error -- must have as many domains as procs\n") ;
#if USE_MPI      
      MPI_Abort(MPI_COMM_WORLD, -1) ;
#else
      exit(-1);
#endif
   }
   Int_t remainder = dx*dy*dz % numRanks ;
   if (myRank < remainder) {
      myDom = myRank*( 1+ (dx*dy*dz / numRanks)) ;
   }
   else {
      myDom = remainder*( 1+ (dx*dy*dz / numRanks)) +
         (myRank - remainder)*(dx*dy*dz/numRanks) ;
   }

   *col = myDom % dx ;
   *row = (myDom / dx) % dy ;
   *plane = myDom / (dx*dy) ;
   *side = testProcs;

   return;
}

void write(Domain& locDom, struct cmdLineOpts &opts, int rank, int epoch) {
	std::ofstream t;
	char buf[20];
	sprintf(buf, "check_%d_%d", rank, epoch);
	t.open(buf, std::ios::out | std::ios::binary);

	int size;
	size = locDom.m_x.size();
	t.write(reinterpret_cast<char *>(&size), sizeof(int));
	for(Real_t i : locDom.m_x) {
		t.write(reinterpret_cast<char *>(&i), sizeof(double));
	}

	size = locDom.m_y.size();
	t.write(reinterpret_cast<char *>(&size), sizeof(int));
	for(Real_t i : locDom.m_y) {
		t.write(reinterpret_cast<char *>(&i), sizeof(double));
	}

	size = locDom.m_z.size();
	t.write(reinterpret_cast<char *>(&size), sizeof(int));
	for(Real_t i : locDom.m_z) {
		t.write(reinterpret_cast<char *>(&i), sizeof(double));
	}

	size = locDom.m_xd.size();
	t.write(reinterpret_cast<char *>(&size), sizeof(int));
	for(Real_t i : locDom.m_xd) {
		t.write(reinterpret_cast<char *>(&i), sizeof(double));
	}

	size = locDom.m_yd.size();
	t.write(reinterpret_cast<char *>(&size), sizeof(int));
	for(Real_t i : locDom.m_yd) {
		t.write(reinterpret_cast<char *>(&i), sizeof(double));
	}

	size = locDom.m_zd.size();
	t.write(reinterpret_cast<char *>(&size), sizeof(int));
	for(Real_t i : locDom.m_zd) {
		t.write(reinterpret_cast<char *>(&i), sizeof(double));
	}

	size = locDom.m_xdd.size();
	t.write(reinterpret_cast<char *>(&size), sizeof(int));
	for(Real_t i : locDom.m_xdd) {
		t.write(reinterpret_cast<char *>(&i), sizeof(double));
	}

	size = locDom.m_ydd.size();
	t.write(reinterpret_cast<char *>(&size), sizeof(int));
	for(Real_t i : locDom.m_ydd) {
		t.write(reinterpret_cast<char *>(&i), sizeof(double));
	}

	size = locDom.m_zdd.size();
	t.write(reinterpret_cast<char *>(&size), sizeof(int));
	for(Real_t i : locDom.m_zdd) {
		t.write(reinterpret_cast<char *>(&i), sizeof(double));
	}

	size = locDom.m_fx.size();
	t.write(reinterpret_cast<char *>(&size), sizeof(int));
	for(Real_t i : locDom.m_fx) {
		t.write(reinterpret_cast<char *>(&i), sizeof(double));
	}

	size = locDom.m_fy.size();
	t.write(reinterpret_cast<char *>(&size), sizeof(int));
	for(Real_t i : locDom.m_fy) {
		t.write(reinterpret_cast<char *>(&i), sizeof(double));
	}

	size = locDom.m_fz.size();
	t.write(reinterpret_cast<char *>(&size), sizeof(int));
	for(Real_t i : locDom.m_fz) {
		t.write(reinterpret_cast<char *>(&i), sizeof(double));
	}

	size = locDom.m_nodalMass.size();
	t.write(reinterpret_cast<char *>(&size), sizeof(int));
	for(Real_t i : locDom.m_nodalMass) {
		t.write(reinterpret_cast<char *>(&i), sizeof(double));
	}

	size = locDom.m_symmX.size();
	t.write(reinterpret_cast<char *>(&size), sizeof(int));
	for(Index_t i : locDom.m_symmX) {
		t.write(reinterpret_cast<char *>(&i), sizeof(int));
	}

	size = locDom.m_symmY.size();
	t.write(reinterpret_cast<char *>(&size), sizeof(int));
	for(Index_t i : locDom.m_symmY) {
		t.write(reinterpret_cast<char *>(&i), sizeof(int));
	}

	size = locDom.m_symmZ.size();
	t.write(reinterpret_cast<char *>(&size), sizeof(int));
	for(Index_t i : locDom.m_symmZ) {
		t.write(reinterpret_cast<char *>(&i), sizeof(int));
	}
	int numReg = locDom.m_numReg;
	t.write(reinterpret_cast<char *>(&numReg), sizeof(int));
	int cost = locDom.m_cost;
	t.write(reinterpret_cast<char *>(&cost), sizeof(int));
	t.write(reinterpret_cast<char *>(locDom.m_regElemSize), sizeof(int) * locDom.m_numReg);
	int  nElem = locDom.m_numElem;
	t.write(reinterpret_cast<char *>(&nElem), sizeof(int));
	t.write(reinterpret_cast<char *>(locDom.m_regNumList), sizeof(int) * locDom.m_numElem);

	for(int i = 0; i < locDom.m_numReg; i++)
	{
		t.write(reinterpret_cast<char *>(locDom.m_regElemlist[i]), sizeof(int) * locDom.regElemSize(i));
	}

	size = locDom.m_nodelist.size();
	t.write(reinterpret_cast<char *>(&size), sizeof(int));
	for(Index_t i : locDom.m_nodelist) {
		t.write(reinterpret_cast<char *>(&i), sizeof(int));
	}

	size = locDom.m_lxim.size();
	t.write(reinterpret_cast<char *>(&size), sizeof(int));
	for(Index_t i : locDom.m_lxim) {
		t.write(reinterpret_cast<char *>(&i), sizeof(int));
	}

	size = locDom.m_lxip.size();
	t.write(reinterpret_cast<char *>(&size), sizeof(int));
	for(Index_t i : locDom.m_lxip) {
		t.write(reinterpret_cast<char *>(&i), sizeof(int));
	}

	size = locDom.m_letam.size();
	t.write(reinterpret_cast<char *>(&size), sizeof(int));
	for(Index_t i : locDom.m_letam) {
		t.write(reinterpret_cast<char *>(&i), sizeof(int));
	}

	size = locDom.m_letap.size();
	t.write(reinterpret_cast<char *>(&size), sizeof(int));
	for(Index_t i : locDom.m_letap) {
		t.write(reinterpret_cast<char *>(&i), sizeof(int));
	}

	size = locDom.m_lzetam.size();
	t.write(reinterpret_cast<char *>(&size), sizeof(int));
	for(Index_t i : locDom.m_lzetam) {
		t.write(reinterpret_cast<char *>(&i), sizeof(int));
	}

	size = locDom.m_lzetap.size();
	t.write(reinterpret_cast<char *>(&size), sizeof(int));
	for(Index_t i : locDom.m_lzetap) {
		t.write(reinterpret_cast<char *>(&i), sizeof(int));
	}

	size = locDom.m_elemBC.size();
	t.write(reinterpret_cast<char *>(&size), sizeof(int));
	for(Int_t i : locDom.m_elemBC) {
		t.write(reinterpret_cast<char *>(&i), sizeof(int));
	}

	size = locDom.m_dxx.size();
	t.write(reinterpret_cast<char *>(&size), sizeof(int));
	for(Real_t i : locDom.m_dxx) {
		t.write(reinterpret_cast<char *>(&i), sizeof(double));
	}

	size = locDom.m_dyy.size();
	t.write(reinterpret_cast<char *>(&size), sizeof(int));
	for(Real_t i : locDom.m_dyy) {
		t.write(reinterpret_cast<char *>(&i), sizeof(double));
	}

	size = locDom.m_dzz.size();
	t.write(reinterpret_cast<char *>(&size), sizeof(int));
	for(Real_t i : locDom.m_dzz) {
		t.write(reinterpret_cast<char *>(&i), sizeof(double));
	}

	size = locDom.m_delv_xi.size();
	t.write(reinterpret_cast<char *>(&size), sizeof(int));
	for(Real_t i : locDom.m_delv_xi) {
		t.write(reinterpret_cast<char *>(&i), sizeof(double));
	}

	size = locDom.m_delv_eta.size();
	t.write(reinterpret_cast<char *>(&size), sizeof(int));
	for(Real_t i : locDom.m_delv_eta) {
		t.write(reinterpret_cast<char *>(&i), sizeof(double));
	}

	size = locDom.m_delv_zeta.size();
	t.write(reinterpret_cast<char *>(&size), sizeof(int));
	for(Real_t i : locDom.m_delv_zeta) {
		t.write(reinterpret_cast<char *>(&i), sizeof(double));
	}

	size = locDom.m_delx_xi.size();
	t.write(reinterpret_cast<char *>(&size), sizeof(int));
	for(Real_t i : locDom.m_delx_xi) {
		t.write(reinterpret_cast<char *>(&i), sizeof(double));
	}

	size = locDom.m_delx_eta.size();
	t.write(reinterpret_cast<char *>(&size), sizeof(int));
	for(Real_t i : locDom.m_delx_eta) {
		t.write(reinterpret_cast<char *>(&i), sizeof(double));
	}

	size = locDom.m_delx_zeta.size();
	t.write(reinterpret_cast<char *>(&size), sizeof(int));
	for(Real_t i : locDom.m_delx_zeta) {
		t.write(reinterpret_cast<char *>(&i), sizeof(double));
	}

	size = locDom.m_e.size();
	t.write(reinterpret_cast<char *>(&size), sizeof(int));
	for(Real_t i : locDom.m_e) {
		t.write(reinterpret_cast<char *>(&i), sizeof(double));
	}

	size = locDom.m_p.size();
	t.write(reinterpret_cast<char *>(&size), sizeof(int));
	for(Real_t i : locDom.m_p) {
		t.write(reinterpret_cast<char *>(&i), sizeof(double));
	}

	size = locDom.m_q.size();
	t.write(reinterpret_cast<char *>(&size), sizeof(int));
	for(Real_t i : locDom.m_q) {
		t.write(reinterpret_cast<char *>(&i), sizeof(double));
	}

	size = locDom.m_ql.size();
	t.write(reinterpret_cast<char *>(&size), sizeof(int));
	for(Real_t i : locDom.m_ql) {
		t.write(reinterpret_cast<char *>(&i), sizeof(double));
	}

	size = locDom.m_qq.size();
	t.write(reinterpret_cast<char *>(&size), sizeof(int));
	for(Real_t i : locDom.m_qq) {
		t.write(reinterpret_cast<char *>(&i), sizeof(double));
	}

	size = locDom.m_v.size();
	t.write(reinterpret_cast<char *>(&size), sizeof(int));
	for(Real_t i : locDom.m_v) {
		t.write(reinterpret_cast<char *>(&i), sizeof(double));
	}

	size = locDom.m_volo.size();
	t.write(reinterpret_cast<char *>(&size), sizeof(int));
	for(Real_t i : locDom.m_volo) {
		t.write(reinterpret_cast<char *>(&i), sizeof(double));
	}

	size = locDom.m_vnew.size();
	t.write(reinterpret_cast<char *>(&size), sizeof(int));
	for(Real_t i : locDom.m_vnew) {
		t.write(reinterpret_cast<char *>(&i), sizeof(double));
	}

	size = locDom.m_delv.size();
	t.write(reinterpret_cast<char *>(&size), sizeof(int));
	for(Real_t i : locDom.m_delv) {
		t.write(reinterpret_cast<char *>(&i), sizeof(double));
	}

	size = locDom.m_vdov.size();
	t.write(reinterpret_cast<char *>(&size), sizeof(int));
	for(Real_t i : locDom.m_vdov) {
		t.write(reinterpret_cast<char *>(&i), sizeof(double));
	}

	size = locDom.m_arealg.size();
	t.write(reinterpret_cast<char *>(&size), sizeof(int));
	for(Real_t i : locDom.m_arealg) {
		t.write(reinterpret_cast<char *>(&i), sizeof(double));
	}

	size = locDom.m_ss.size();
	t.write(reinterpret_cast<char *>(&size), sizeof(int));
	for(Real_t i : locDom.m_ss) {
		t.write(reinterpret_cast<char *>(&i), sizeof(double));
	}

	size = locDom.m_elemMass.size();
	t.write(reinterpret_cast<char *>(&size), sizeof(int));
	for(Real_t i : locDom.m_elemMass) {
		t.write(reinterpret_cast<char *>(&i), sizeof(double));
	}

	Real_t time = locDom.time();
	printf("Time.... %lf\n", (double)locDom.time());
	t.write(reinterpret_cast<char *>(&locDom.m_dtcourant), sizeof(double));
	t.write(reinterpret_cast<char *>(&locDom.m_dthydro), sizeof(double));
	t.write(reinterpret_cast<char *>(&locDom.m_cycle), sizeof(int));
	t.write(reinterpret_cast<char *>(&locDom.m_dtfixed), sizeof(double));
	t.write(reinterpret_cast<char *>(&time), sizeof(double));
	t.write(reinterpret_cast<char *>(&locDom.m_deltatime), sizeof(double));
	t.write(reinterpret_cast<char *>(&locDom.m_deltatimemultlb), sizeof(double));
	t.write(reinterpret_cast<char *>(&locDom.m_deltatimemultub), sizeof(double));
	t.write(reinterpret_cast<char *>(&locDom.m_dtmax), sizeof(double));
	t.write(reinterpret_cast<char *>(&locDom.m_stoptime), sizeof(double));

	t.write(reinterpret_cast<char *>(&locDom.m_numRanks), sizeof(int));

	t.write(reinterpret_cast<char *>(&locDom.m_colLoc), sizeof(int));
	t.write(reinterpret_cast<char *>(&locDom.m_rowLoc), sizeof(int));
	t.write(reinterpret_cast<char *>(&locDom.m_planeLoc), sizeof(int));
	t.write(reinterpret_cast<char *>(&locDom.m_tp), sizeof(int));

	t.write(reinterpret_cast<char *>(&locDom.m_sizeX), sizeof(int));
	t.write(reinterpret_cast<char *>(&locDom.m_sizeY), sizeof(int));
	t.write(reinterpret_cast<char *>(&locDom.m_sizeZ), sizeof(int));
	t.write(reinterpret_cast<char *>(&locDom.m_numElem), sizeof(int));
	t.write(reinterpret_cast<char *>(&locDom.m_numNode), sizeof(int));

	t.write(reinterpret_cast<char *>(&locDom.m_maxPlaneSize), sizeof(int));
	t.write(reinterpret_cast<char *>(&locDom.m_maxEdgeSize), sizeof(int));

	//t.write(reinterpret_cast<char *>(m_nodeElemStart), sizeof(Index_t) * m_numReg);
	//t.write(reinterpret_cast<char *>(m_nodeElemCornerList), sizeof(Index_t) * m_numElem);

	t.write(reinterpret_cast<char *>(&locDom.m_rowMin), sizeof(int));
	t.write(reinterpret_cast<char *>(&locDom.m_rowMax), sizeof(int));
	t.write(reinterpret_cast<char *>(&locDom.m_colMin), sizeof(int));
	t.write(reinterpret_cast<char *>(&locDom.m_colMax), sizeof(int));
	t.write(reinterpret_cast<char *>(&locDom.m_planeMin), sizeof(int));
	t.write(reinterpret_cast<char *>(&locDom.m_planeMax), sizeof(int));

	//Size of comm send/recv buffer
	t.write(reinterpret_cast<char *>(&locDom.commBuffSize), sizeof(int));
	t.write(reinterpret_cast<char *>(locDom.commDataSend), sizeof(double) * locDom.commBuffSize);
	t.write(reinterpret_cast<char *>(locDom.commDataRecv), sizeof(double) * locDom.commBuffSize);

	//struct
	t.write(reinterpret_cast<char *>(&opts), sizeof(opts));

	t.close();
}

void read(Domain& locDom, struct cmdLineOpts &opts, int rank, int epoch) {
	std::ifstream t;
	char buf[20];
	sprintf(buf, "check_%d_%d", rank, epoch);
	t.open(buf, std::ios::in | std::ios::binary);

	int sz;
	t.read(reinterpret_cast<char *>(&sz), sizeof(int));
	locDom.m_x.resize(sz);
	double val;
	for (int i = 0; i < sz; i++) {
		t.read(reinterpret_cast<char *>(&val), sizeof(double));
		locDom.m_x[i] = val;
	}

	t.read(reinterpret_cast<char *>(&sz), sizeof(int));
	locDom.m_y.resize(sz);
	for (int i = 0; i < sz; i++) {
		t.read(reinterpret_cast<char *>(&val), sizeof(double));
		locDom.m_y[i] = val;
	}

	t.read(reinterpret_cast<char *>(&sz), sizeof(int));
	locDom.m_z.resize(sz);
	for (int i = 0; i < sz; i++) {
		t.read(reinterpret_cast<char *>(&val), sizeof(double));
		locDom.m_z[i] = val;
	}

	t.read(reinterpret_cast<char *>(&sz), sizeof(int));
	locDom.m_xd.resize(sz);
	for (int i = 0; i < sz; i++) {
		t.read(reinterpret_cast<char *>(&val), sizeof(double));
		locDom.m_xd[i] = val;
	}

	t.read(reinterpret_cast<char *>(&sz), sizeof(int));
	locDom.m_yd.resize(sz);
	for (int i = 0; i < sz; i++) {
		t.read(reinterpret_cast<char *>(&val), sizeof(double));
		locDom.m_yd[i] = val;
	}

	t.read(reinterpret_cast<char *>(&sz), sizeof(int));
	locDom.m_zd.resize(sz);
	for (int i = 0; i < sz; i++) {
		t.read(reinterpret_cast<char *>(&val), sizeof(double));
		locDom.m_zd[i] = val;
	}

	t.read(reinterpret_cast<char *>(&sz), sizeof(int));
	locDom.m_xdd.resize(sz);
	for (int i = 0; i < sz; i++) {
		t.read(reinterpret_cast<char *>(&val), sizeof(double));
		locDom.m_xdd[i] = val;
	}

	t.read(reinterpret_cast<char *>(&sz), sizeof(int));
	locDom.m_ydd.resize(sz);
	for (int i = 0; i < sz; i++) {
		t.read(reinterpret_cast<char *>(&val), sizeof(double));
		locDom.m_ydd[i] = val;
	}

	t.read(reinterpret_cast<char *>(&sz), sizeof(int));
	locDom.m_zdd.resize(sz);
	for (int i = 0; i < sz; i++) {
		t.read(reinterpret_cast<char *>(&val), sizeof(double));
		locDom.m_zdd[i] = val;
	}

	t.read(reinterpret_cast<char *>(&sz), sizeof(int));
	locDom.m_fx.resize(sz);
	for (int i = 0; i < sz; i++) {
		t.read(reinterpret_cast<char *>(&val), sizeof(double));
		locDom.m_fx[i] = val;
	}

	t.read(reinterpret_cast<char *>(&sz), sizeof(int));
	locDom.m_fy.resize(sz);
	for (int i = 0; i < sz; i++) {
		t.read(reinterpret_cast<char *>(&val), sizeof(double));
		locDom.m_fy[i] = val;
	}

	t.read(reinterpret_cast<char *>(&sz), sizeof(int));
	locDom.m_fz.resize(sz);
	for (int i = 0; i < sz; i++) {
		t.read(reinterpret_cast<char *>(&val), sizeof(double));
		locDom.m_fz[i] = val;
	}

	t.read(reinterpret_cast<char *>(&sz), sizeof(int));
	locDom.m_nodalMass.resize(sz);
	for (int i = 0; i < sz; i++) {
		t.read(reinterpret_cast<char *>(&val), sizeof(double));
		locDom.m_nodalMass[i] = val;
	}

	Index_t value;
	t.read(reinterpret_cast<char *>(&sz), sizeof(int));
	locDom.m_symmX.resize(sz);
	for (int i = 0; i < sz; i++) {
		t.read(reinterpret_cast<char *>(&value), sizeof(int));
		locDom.m_symmX[i] = value;
	}

	t.read(reinterpret_cast<char *>(&sz), sizeof(int));
	locDom.m_symmY.resize(sz);
	for (int i = 0; i < sz; i++) {
		t.read(reinterpret_cast<char *>(&value), sizeof(int));
		locDom.m_symmY[i] = value;
	}

	t.read(reinterpret_cast<char *>(&sz), sizeof(int));
	locDom.m_symmZ.resize(sz);
	for (int i = 0; i < sz; i++) {
		t.read(reinterpret_cast<char *>(&value), sizeof(int));
		locDom.m_symmZ[i] = value;
	}

	Int_t numReg;
	t.read(reinterpret_cast<char *>(&numReg), sizeof(int));
	locDom.m_numReg = numReg;

	Int_t cost;
	t.read(reinterpret_cast<char *>(&cost), sizeof(int));
	locDom.m_cost = cost;

	locDom.m_regElemSize = new Index_t[locDom.m_numReg];
	t.read(reinterpret_cast<char *>(locDom.m_regElemSize), sizeof(int) * locDom.m_numReg);

	Index_t nElem;
	t.read(reinterpret_cast<char *>(&nElem), sizeof(int));
	locDom.m_regNumList = new Index_t[nElem];
	t.read(reinterpret_cast<char *>(locDom.m_regNumList), sizeof(int) * nElem);

	locDom.m_regElemlist = new Index_t*[locDom.m_numReg];
	for (int i = 0; i < locDom.m_numReg; i++) {
		locDom.m_regElemlist[i] = new Index_t[locDom.regElemSize(i)];
	}
	for(int i = 0; i < locDom.m_numReg; i++)
	{
		t.read(reinterpret_cast<char *>(locDom.m_regElemlist[i]), sizeof(int) * locDom.regElemSize(i));
	}

	t.read(reinterpret_cast<char *>(&sz), sizeof(int));
	locDom.m_nodelist.resize(sz);
	for (int i = 0; i < sz; i++) {
		t.read(reinterpret_cast<char *>(&value), sizeof(int));
		locDom.m_nodelist[i] = value;
	}

	t.read(reinterpret_cast<char *>(&sz), sizeof(int));
	locDom.m_lxim.resize(sz);
	for (int i = 0; i < sz; i++) {
		t.read(reinterpret_cast<char *>(&value), sizeof(int));
		locDom.m_lxim[i] = value;
	}

	t.read(reinterpret_cast<char *>(&sz), sizeof(int));
	locDom.m_lxip.resize(sz);
	for (int i = 0; i < sz; i++) {
		t.read(reinterpret_cast<char *>(&value), sizeof(int));
		locDom.m_lxip[i] = value;
	}

	t.read(reinterpret_cast<char *>(&sz), sizeof(int));
	locDom.m_letam.resize(sz);
	for (int i = 0; i < sz; i++) {
		t.read(reinterpret_cast<char *>(&value), sizeof(int));
		locDom.m_letam[i] = value;
	}

	t.read(reinterpret_cast<char *>(&sz), sizeof(int));
	locDom.m_letap.resize(sz);
	for (int i = 0; i < sz; i++) {
		t.read(reinterpret_cast<char *>(&value), sizeof(int));
		locDom.m_letap[i] = value;
	}

	t.read(reinterpret_cast<char *>(&sz), sizeof(int));
	locDom.m_lzetam.resize(sz);
	for (int i = 0; i < sz; i++) {
		t.read(reinterpret_cast<char *>(&value), sizeof(int));
		locDom.m_lzetam[i] = value;
	}

	t.read(reinterpret_cast<char *>(&sz), sizeof(int));
	locDom.m_lzetap.resize(sz);
	for (int i = 0; i < sz; i++) {
		t.read(reinterpret_cast<char *>(&value), sizeof(int));
		locDom.m_lzetap[i] = value;
	}

	Int_t v;
	t.read(reinterpret_cast<char *>(&sz), sizeof(int));
	locDom.m_elemBC.resize(sz);
	for (int i = 0; i < sz; i++) {
		t.read(reinterpret_cast<char *>(&v), sizeof(int));
		locDom.m_elemBC[i] = v;
	}

	t.read(reinterpret_cast<char *>(&sz), sizeof(int));
	locDom.m_dxx.resize(sz);
	for (int i = 0; i < sz; i++) {
		t.read(reinterpret_cast<char *>(&val), sizeof(double));
		locDom.m_dxx[i] = val;
	}

	t.read(reinterpret_cast<char *>(&sz), sizeof(int));
	locDom.m_dyy.resize(sz);
	for (int i = 0; i < sz; i++) {
		t.read(reinterpret_cast<char *>(&val), sizeof(double));
		locDom.m_dyy[i] = val;
	}

	t.read(reinterpret_cast<char *>(&sz), sizeof(int));
	locDom.m_dzz.resize(sz);
	for (int i = 0; i < sz; i++) {
		t.read(reinterpret_cast<char *>(&val), sizeof(double));
		locDom.m_dzz[i] = val;
	}

	t.read(reinterpret_cast<char *>(&sz), sizeof(int));
	locDom.m_delv_xi.resize(sz);
	for (int i = 0; i < sz; i++) {
		t.read(reinterpret_cast<char *>(&val), sizeof(double));
		locDom.m_delv_xi[i] = val;
	}

	t.read(reinterpret_cast<char *>(&sz), sizeof(int));
	locDom.m_delv_eta.resize(sz);
	for (int i = 0; i < sz; i++) {
		t.read(reinterpret_cast<char *>(&val), sizeof(double));
		locDom.m_delv_eta[i] = val;
	}

	t.read(reinterpret_cast<char *>(&sz), sizeof(int));
	locDom.m_delv_zeta.resize(sz);
	for (int i = 0; i < sz; i++) {
		t.read(reinterpret_cast<char *>(&val), sizeof(double));
		locDom.m_delv_zeta[i] = val;
	}

	t.read(reinterpret_cast<char *>(&sz), sizeof(int));
	locDom.m_delx_xi.resize(sz);
	for (int i = 0; i < sz; i++) {
		t.read(reinterpret_cast<char *>(&val), sizeof(double));
		locDom.m_delx_xi[i] = val;
	}

	t.read(reinterpret_cast<char *>(&sz), sizeof(int));
	locDom.m_delx_eta.resize(sz);
	for (int i = 0; i < sz; i++) {
		t.read(reinterpret_cast<char *>(&val), sizeof(double));
		locDom.m_delx_eta[i] = val;
	}

	t.read(reinterpret_cast<char *>(&sz), sizeof(int));
	locDom.m_delx_zeta.resize(sz);
	for (int i = 0; i < sz; i++) {
		t.read(reinterpret_cast<char *>(&val), sizeof(double));
		locDom.m_delx_zeta[i] = val;
	}

	t.read(reinterpret_cast<char *>(&sz), sizeof(int));
	locDom.m_e.resize(sz);
	for (int i = 0; i < sz; i++) {
		t.read(reinterpret_cast<char *>(&val), sizeof(double));
		locDom.m_e[i] = val;
	}

	t.read(reinterpret_cast<char *>(&sz), sizeof(int));
	locDom.m_p.resize(sz);
	for (int i = 0; i < sz; i++) {
		t.read(reinterpret_cast<char *>(&val), sizeof(double));
		locDom.m_p[i] = val;
	}

	t.read(reinterpret_cast<char *>(&sz), sizeof(int));
	locDom.m_q.resize(sz);
	for (int i = 0; i < sz; i++) {
		t.read(reinterpret_cast<char *>(&val), sizeof(double));
		locDom.m_q[i] = val;
	}

	t.read(reinterpret_cast<char *>(&sz), sizeof(int));
	locDom.m_ql.resize(sz);
	for (int i = 0; i < sz; i++) {
		t.read(reinterpret_cast<char *>(&val), sizeof(double));
		locDom.m_ql[i] = val;
	}

	t.read(reinterpret_cast<char *>(&sz), sizeof(int));
	locDom.m_qq.resize(sz);
	for (int i = 0; i < sz; i++) {
		t.read(reinterpret_cast<char *>(&val), sizeof(double));
		locDom.m_qq[i] = val;
	}

	t.read(reinterpret_cast<char *>(&sz), sizeof(int));
	locDom.m_v.resize(sz);
	for (int i = 0; i < sz; i++) {
		t.read(reinterpret_cast<char *>(&val), sizeof(double));
		locDom.m_v[i] = val;
	}

	t.read(reinterpret_cast<char *>(&sz), sizeof(int));
	locDom.m_volo.resize(sz);
	for (int i = 0; i < sz; i++) {
		t.read(reinterpret_cast<char *>(&val), sizeof(double));
		locDom.m_volo[i] = val;
	}

	t.read(reinterpret_cast<char *>(&sz), sizeof(int));
	locDom.m_vnew.resize(sz);
	for (int i = 0; i < sz; i++) {
		t.read(reinterpret_cast<char *>(&val), sizeof(double));
		locDom.m_vnew[i] = val;
	}

	t.read(reinterpret_cast<char *>(&sz), sizeof(int));
	locDom.m_delv.resize(sz);
	for (int i = 0; i < sz; i++) {
		t.read(reinterpret_cast<char *>(&val), sizeof(double));
		locDom.m_delv[i] = val;
	}

	t.read(reinterpret_cast<char *>(&sz), sizeof(int));
	locDom.m_vdov.resize(sz);
	for (int i = 0; i < sz; i++) {
		t.read(reinterpret_cast<char *>(&val), sizeof(double));
		locDom.m_vdov[i] = val;
	}

	t.read(reinterpret_cast<char *>(&sz), sizeof(int));
	locDom.m_arealg.resize(sz);
	for (int i = 0; i < sz; i++) {
		t.read(reinterpret_cast<char *>(&val), sizeof(double));
		locDom.m_arealg[i] = val;
	}

	t.read(reinterpret_cast<char *>(&sz), sizeof(int));
	locDom.m_ss.resize(sz);
	for (int i = 0; i < sz; i++) {
		t.read(reinterpret_cast<char *>(&val), sizeof(double));
		locDom.m_ss[i] = val;
	}

	t.read(reinterpret_cast<char *>(&sz), sizeof(int));
	locDom.m_elemMass.resize(sz);
	for (int i = 0; i < sz; i++) {
		t.read(reinterpret_cast<char *>(&val), sizeof(double));
		locDom.m_elemMass[i] = val;
	}
	Real_t var;

	t.read(reinterpret_cast<char *>(&var), sizeof(double));
	locDom.m_dtcourant = var;

	t.read(reinterpret_cast<char *>(&var), sizeof(double));
	locDom.m_dthydro = var;
	Int_t cycle;
	t.read(reinterpret_cast<char *>(&cycle), sizeof(int));
	locDom.m_cycle = cycle;

	t.read(reinterpret_cast<char *>(&var), sizeof(double));
	locDom.m_dtfixed = var;

	t.read(reinterpret_cast<char *>(&var), sizeof(double));
	locDom.m_time = var;
	printf("Time after read...%e\n", (double)locDom.m_time);

	t.read(reinterpret_cast<char *>(&var), sizeof(double));
	locDom.m_deltatime = var;

	t.read(reinterpret_cast<char *>(&var), sizeof(double));
	locDom.m_deltatimemultlb = var;

	t.read(reinterpret_cast<char *>(&var), sizeof(double));
	locDom.m_deltatimemultub = var;

	t.read(reinterpret_cast<char *>(&var), sizeof(double));
	locDom.m_dtmax = var;

	t.read(reinterpret_cast<char *>(&var), sizeof(double));
	locDom.m_stoptime = var;

	Int_t numRanks;
	t.read(reinterpret_cast<char *>(&numRanks), sizeof(int));
	locDom.m_numRanks = numRanks;

	t.read(reinterpret_cast<char *>(&value), sizeof(int));
	locDom.m_colLoc = value;

	t.read(reinterpret_cast<char *>(&value), sizeof(int));
	locDom.m_rowLoc = value;

	t.read(reinterpret_cast<char *>(&value), sizeof(int));
	locDom.m_planeLoc = value;

	t.read(reinterpret_cast<char *>(&value), sizeof(int));
	locDom.m_tp = value;

	t.read(reinterpret_cast<char *>(&value), sizeof(int));
	locDom.m_sizeX = value;

	t.read(reinterpret_cast<char *>(&value), sizeof(int));
	locDom.m_sizeY = value;

	t.read(reinterpret_cast<char *>(&value), sizeof(int));
	locDom.m_sizeZ = value;

	t.read(reinterpret_cast<char *>(&value), sizeof(int));
	locDom.m_numElem = value;

	t.read(reinterpret_cast<char *>(&value), sizeof(int));
	locDom.m_numNode = value;

	t.read(reinterpret_cast<char *>(&value), sizeof(int));
	locDom.m_maxPlaneSize = value;

	t.read(reinterpret_cast<char *>(&value), sizeof(int));
	locDom.m_maxEdgeSize = value;

	t.read(reinterpret_cast<char *>(&value), sizeof(int));
	locDom.m_rowMin = value;

	t.read(reinterpret_cast<char *>(&value), sizeof(int));
	locDom.m_rowMax = value;

	t.read(reinterpret_cast<char *>(&value), sizeof(int));
	locDom.m_colMin = value;

	t.read(reinterpret_cast<char *>(&value), sizeof(int));
	locDom.m_colMax = value;

	t.read(reinterpret_cast<char *>(&value), sizeof(int));
	locDom.m_planeMin = value;

	t.read(reinterpret_cast<char *>(&value), sizeof(int));
	locDom.m_planeMax = value;

	Index_t comm_buf_size;
	t.read(reinterpret_cast<char *>(&comm_buf_size), sizeof(int));
	locDom.commBuffSize = comm_buf_size;
	locDom.commDataSend = new Real_t[locDom.commBuffSize];
	t.read(reinterpret_cast<char *>(locDom.commDataSend), sizeof(double) * locDom.commBuffSize);
	locDom.commDataRecv = new Real_t[locDom.commBuffSize];
	t.read(reinterpret_cast<char *>(locDom.commDataRecv), sizeof(double) * locDom.commBuffSize);

	//struct
	t.read(reinterpret_cast<char *>(&opts), sizeof(opts));

	t.close();
}

