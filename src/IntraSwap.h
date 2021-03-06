/*******************************************************************************
GPU OPTIMIZED MONTE CARLO (GOMC) 1.70 (Serial version)
Copyright (C) 2015  GOMC Group
A copy of the GNU General Public License can be found in the COPYRIGHT.txt
along with this program, also can be found at <http://www.gnu.org/licenses/>.
********************************************************************************/
#ifndef INTRASWAP_H
#define INTRASWAP_H


#include "MoveBase.h"
#include "cbmc/TrialMol.h"

//#define DEBUG_MOVES

class IntraSwap : public MoveBase
{
 public:

   IntraSwap(System &sys, StaticVals const& statV) : 
      ffRef(statV.forcefield), molLookRef(sys.molLookupRef), 
      MoveBase(sys, statV) {}

   virtual uint Prep(const double subDraw, const double movPerc);
   virtual uint Transform();
   virtual void CalcEn();
   virtual void Accept(const uint earlyReject, const uint step);
 private:
   uint GetBoxAndMol(const double subDraw, const double movPerc);
   MolPick molPick;
   uint sourceBox, destBox;
   uint pStart, pLen;
   uint molIndex, kindIndex;

   double W_tc, oldVirial_LJ, oldVirial_Real;
   cbmc::TrialMol oldMol, newMol;
   Intermolecular tcLose, tcGain;
   MoleculeLookup & molLookRef;
   Forcefield const& ffRef;
};

inline uint IntraSwap::GetBoxAndMol
(const double subDraw, const double movPerc)
{

#if ENSEMBLE == GCMC
    sourceBox = mv::BOX0;
   uint state = prng.PickMol(molIndex, kindIndex, sourceBox, subDraw, movPerc);
#else
   uint state = prng.PickMolAndBox(molIndex, kindIndex, sourceBox, subDraw,
				   movPerc);
#endif

   //molecule will be removed and insert in same box
   destBox = sourceBox;
 
   if ( state != mv::fail_state::NO_MOL_OF_KIND_IN_BOX)
   {
      pStart = pLen = 0;
      molRef.GetRangeStartLength(pStart, pLen, molIndex);
   }
   return state;
}

inline uint IntraSwap::Prep(const double subDraw,
				   const double movPerc)
{
   uint state = GetBoxAndMol(subDraw, movPerc);
   newMol = cbmc::TrialMol(molRef.kinds[kindIndex], boxDimRef,
			   destBox);
   oldMol = cbmc::TrialMol(molRef.kinds[kindIndex], boxDimRef,
			   sourceBox);
   oldMol.SetCoords(coordCurrRef, pStart);
   W_tc = 1.0;
   return state;
}


inline uint IntraSwap::Transform()
{
   oldVirial_LJ = 0.0; 
   oldVirial_Real = 0.0;
   calcEnRef.MoleculeVirial(oldVirial_LJ, oldVirial_Real, molIndex, sourceBox);
#ifdef CELL_LIST
   cellList.RemoveMol(molIndex, sourceBox, coordCurrRef);
#endif
   subPick = mv::GetMoveSubIndex(mv::INTRA_SWAP, sourceBox);
   molRef.kinds[kindIndex].Build(oldMol, newMol, molIndex);
   return mv::fail_state::NO_FAIL;
}

inline void IntraSwap::CalcEn()
{
   if (ffRef.useLRC)
   {
      // since number of molecule would not change in box,
      //there is no change in Tc
      W_tc = 1.0;
   }
}


inline void IntraSwap::Accept(const uint rejectState, const uint step)
{
   bool result;
   //If we didn't skip the move calculation
   if(rejectState == mv::fail_state::NO_FAIL)
   {
      double molTransCoeff = 1.0;
      double Wo = oldMol.GetWeight();
      double Wn = newMol.GetWeight();
      double Wrat = Wn / Wo * W_tc;

      result = prng() < molTransCoeff * Wrat;
      if (result)
      {

         //Add rest of energy.
         sysPotRef.boxEnergy[sourceBox] -= oldMol.GetEnergy();
         sysPotRef.boxEnergy[destBox] += newMol.GetEnergy();
         sysPotRef.boxVirial[sourceBox].inter -= oldVirial_LJ;
	 sysPotRef.boxVirial[sourceBox].real -= oldVirial_Real;

	 //Set coordinates, new COM; shift index to new box's list
         newMol.GetCoords().CopyRange(coordCurrRef, 0, pStart, pLen);
         comCurrRef.SetNew(molIndex, destBox);
         //molLookRef.ShiftMolBox(molIndex, sourceBox, destBox,
	 //			kindIndex);
#ifdef CELL_LIST
	 cellList.AddMol(molIndex, destBox, coordCurrRef);
#endif

	 //Calculate the fresh virial.
         double newVirial_LJ = 0.0, newVirial_Real = 0.0;
	 calcEnRef.MoleculeVirial(newVirial_LJ, newVirial_Real,
				  molIndex,destBox);
         sysPotRef.boxVirial[destBox].inter += newVirial_LJ;
	 sysPotRef.boxVirial[destBox].real += newVirial_Real;

	 //Zero out box energies to prevent small number 
	 //errors in double.
	 if (molLookRef.NumInBox(sourceBox) == 1)
	 {
	    sysPotRef.boxEnergy[sourceBox].inter = 0;
	    sysPotRef.boxVirial[sourceBox].inter = 0;
	 }

	 //Retotal
         sysPotRef.Total();
      }
#ifdef CELL_LIST
      else
      {
	cellList.AddMol(molIndex, sourceBox, coordCurrRef);
      }
#endif
   }
   else  //else we didn't even try because we knew it would fail
      result = false;
   subPick = mv::GetMoveSubIndex(mv::INTRA_SWAP, sourceBox);
   moveSetRef.Update(result, subPick, step);
}

#endif
