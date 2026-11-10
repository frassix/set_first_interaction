/*
 * MCForcedFirstInteractionOperator.cc
 *
 * Copyright (C) by the MEGAlib contributors.
 *
 * This file is part of MEGAlib.
 *
 * MEGAlib is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * MEGAlib is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 * License (License.md) for more details.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

// edited by Andrea Frassà, 11/10/26: new class (forced first interaction mode)


// Cosima:
#include "MCForcedFirstInteractionOperator.hh"
#include "MCTrackInformation.hh"
#include "MCSource.hh"

// Geant4:
#include "G4Track.hh"
#include "G4LogicalVolume.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4VPhysicalVolume.hh"
#include "G4BiasingProcessInterface.hh"
#include "G4Navigator.hh"
#include "G4TransportationManager.hh"
#include "G4Gamma.hh"
#include "G4ProcessManager.hh"
#include "G4ProcessVector.hh"
#include "G4VProcess.hh"
#include "G4VDiscreteProcess.hh"
#include "G4VEmProcess.hh"
#include "G4HadronicProcess.hh"
#include "G4MaterialCutsCouple.hh"
#include "G4Material.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"
#include "geomdefs.hh"

// MEGAlib:
#include "MStreams.h"

// Standard lib:
#include <cmath>
#include <cfloat>
#include <algorithm>
using namespace std;


/******************************************************************************
 * Default constructor
 */
MCForcedFirstInteractionOperator::MCForcedFirstInteractionOperator()
  : G4VBiasingOperator("MCForcedFirstInteractionOperator"), m_Navigator(nullptr), m_NImpossible(0)
{
  m_Operation = new MCForcedFirstInteractionOperation();
}


/******************************************************************************
 * Default destructor
 */
MCForcedFirstInteractionOperator::~MCForcedFirstInteractionOperator()
{
  delete m_Operation;
  for (auto& F: m_Flights) delete F.second;
  delete m_Navigator;
}


/******************************************************************************
 * Attach this operator to all logical volumes of the geometry
 * (the first interaction can happen anywhere, including the world volume)
 */
void MCForcedFirstInteractionOperator::AttachToAllVolumes()
{
  G4LogicalVolumeStore* Store = G4LogicalVolumeStore::GetInstance();
  for (unsigned int v = 0; v < Store->size(); ++v) {
    AttachTo((*Store)[v]);
  }
  mout<<"Forced first interaction: biasing operator attached to "<<Store->size()<<" logical volumes"<<endl;
}


/******************************************************************************
 * Return true if the track is armed and its first interaction point has been sampled
 */
bool MCForcedFirstInteractionOperator::IsArmed(const G4Track* Track) const
{
  MCTrackInformation* Info = dynamic_cast<MCTrackInformation*>(Track->GetUserInformation());
  if (Info == nullptr) return false;
  if (Info->IsForcedFirstInteractionPending() == false) return false;
  if (Info->GetForcedFirstInteractionDistance() < 0.0) return false;

  return true;
}


/******************************************************************************
 * Called by Geant4 at the start of each track (after MCTrackingAction::PreUserTrackingAction,
 * which arms the primaries): sample the first interaction point of armed primaries
 */
void MCForcedFirstInteractionOperator::StartTracking(const G4Track* Track)
{
  MCTrackInformation* Info = dynamic_cast<MCTrackInformation*>(Track->GetUserInformation());
  if (Info == nullptr || Info->IsForcedFirstInteractionPending() == false) return;

  double Distance = -1.0;
  double Weight = 0.0;
  if (SampleFirstInteraction(Track, Info->GetForcedFirstInteractionProcessID(), Distance, Weight) == true) {
    Info->SetForcedFirstInteractionPoint(Distance, Weight);
  } else {
    // The forced process is impossible along the whole path (e.g. PAIR below threshold):
    // this primary is simulated analog and reported as FAILED (weight 0)
    if (m_NImpossible == 0) {
      mout<<"Forced first interaction: The forced process ("
          <<MCSource::GetForcedFirstInteractionAsString(Info->GetForcedFirstInteractionProcessID())
          <<") is impossible along the whole path of a primary (E="<<Track->GetKineticEnergy()/keV<<" keV)."<<endl
          <<"  Such primaries are simulated analog and are marked with \"FAILED\" in the CC line. This message is only shown once."<<endl;
    }
    m_NImpossible++;
    Info->SetForcedFirstInteractionDone(false);
  }
}


/******************************************************************************
 * Trace the straight path of the track through the geometry and sample the
 * first interaction point of the forced process
 *
 * Before its first interaction a photon flies straight and does not lose energy,
 * thus its path is a sequence of segments with constant cross sections.
 * We sample exactly from p(x) ~ sigma_X(x) * exp(-integral_0^x sigma_tot)
 * via inverse transform sampling - no rejection, every primary gets its point.
 */
bool MCForcedFirstInteractionOperator::SampleFirstInteraction(const G4Track* Track, const int ForcedID, double& Distance, double& Weight)
{
  Distance = -1.0;
  Weight = 0.0;

  // Step 0: Initialization at the first call (physics and geometry are ready then)

  // The real physics processes of the gamma: they are all wrapped by biasing interfaces
  if (m_GammaProcesses.size() == 0) {
    G4ProcessVector* Processes = G4Gamma::Gamma()->GetProcessManager()->GetProcessList();
    for (G4int p = 0; p < (G4int) Processes->size(); ++p) {
      G4BiasingProcessInterface* Interface = dynamic_cast<G4BiasingProcessInterface*>((*Processes)[p]);
      if (Interface != nullptr && Interface->GetWrappedProcess() != nullptr) {
        m_GammaProcesses.push_back(Interface->GetWrappedProcess());
      }
    }
    mout<<"Forced first interaction: gamma processes used for the total cross section:";
    for (unsigned int p = 0; p < m_GammaProcesses.size(); ++p) mout<<" "<<m_GammaProcesses[p]->GetProcessName();
    mout<<endl;
  }

  // Our own navigator - the tracking navigator must not be touched
  if (m_Navigator == nullptr) {
    m_Navigator = new G4Navigator();
    m_Navigator->SetWorldVolume(G4TransportationManager::GetTransportationManager()->GetNavigatorForTracking()->GetWorldVolume());
  }


  // Step 1: Trace the path through the geometry and compute P_k for each segment
  const double Energy = Track->GetKineticEnergy();
  G4ThreeVector Position = Track->GetPosition();
  const G4ThreeVector Direction = Track->GetMomentumDirection();

  vector<double> SegmentStart;      // distance of the segment start from the track start
  vector<double> SegmentLength;     // length of the segment
  vector<double> SegmentSigmaTotal; // total macroscopic cross section
  vector<double> SegmentP;          // probability that the first interaction is X in this segment

  double Start = 0.0;
  double OpticalDepth = 0.0; // T_k: sum of sigma_tot * L before the current segment
  double PTotal = 0.0;

  G4VPhysicalVolume* Volume = m_Navigator->LocateGlobalPointAndSetup(Position, &Direction, false, false);
  const unsigned int MaxSegments = 100000; // protection against geometry hick-ups
  unsigned int NSegments = 0;
  while (Volume != nullptr && NSegments < MaxSegments) {
    ++NSegments;

    G4double Safety = 0.0;
    G4double Length = m_Navigator->ComputeStep(Position, Direction, kInfinity, Safety);
    if (Length >= kInfinity) break; // should not happen inside a closed world volume

    const G4MaterialCutsCouple* Couple = Volume->GetLogicalVolume()->GetMaterialCutsCouple();
    if (Length > 0.0 && Couple != nullptr) {
      double SigmaTotal = 0.0;
      double SigmaForced = 0.0;
      for (unsigned int p = 0; p < m_GammaProcesses.size(); ++p) {
        double Sigma = GetCrossSection(m_GammaProcesses[p], Energy, Couple);
        if (Sigma <= 0.0 || std::isfinite(Sigma) == false) continue;
        SigmaTotal += Sigma;
        if (MCForcedFirstInteractionOperation::GetGammaProcessID(m_GammaProcesses[p]->GetProcessName()) == ForcedID) {
          SigmaForced += Sigma;
        }
      }

      double P = 0.0;
      if (SigmaTotal > 0.0 && SigmaForced > 0.0) {
        // P_k = sigma_X/sigma_tot * exp(-T_k) * (1 - exp(-sigma_tot*L_k))
        P = SigmaForced/SigmaTotal * exp(-OpticalDepth) * (-expm1(-SigmaTotal*Length));
      }

      SegmentStart.push_back(Start);
      SegmentLength.push_back(Length);
      SegmentSigmaTotal.push_back(SigmaTotal);
      SegmentP.push_back(P);

      PTotal += P;
      OpticalDepth += SigmaTotal*Length;
    }

    // Move to the next volume
    Start += Length;
    Position += Length*Direction;
    m_Navigator->SetGeometricallyLimitedStep();
    Volume = m_Navigator->LocateGlobalPointAndSetup(Position, &Direction, true);
  }

  if (PTotal <= 0.0 || std::isfinite(PTotal) == false) return false;


  // Step 2: Choose the segment k with probability P_k / P
  double U = G4UniformRand()*PTotal;
  unsigned int k = 0;
  unsigned int LastPossible = 0;
  for (k = 0; k < SegmentP.size(); ++k) {
    if (SegmentP[k] <= 0.0) continue;
    LastPossible = k;
    if (U < SegmentP[k]) break;
    U -= SegmentP[k];
  }
  if (k >= SegmentP.size()) { // only by rounding
    k = LastPossible;
    U = SegmentP[k];
  }


  // Step 3: Choose the position inside the segment: the density there is ~exp(-sigma_tot*d),
  //         truncated to the segment length. Inverse transform:
  //         d = -ln(1 - V*(1 - exp(-sigma_tot*L)))/sigma_tot with V uniform in [0, 1)
  double V = min(max(U/SegmentP[k], 0.0), 1.0 - 1E-15);
  double SigmaTotal = SegmentSigmaTotal[k];
  double Length = SegmentLength[k];
  double d = -log1p(-V*(-expm1(-SigmaTotal*Length)))/SigmaTotal;
  d = min(max(d, 0.0), Length);

  Distance = SegmentStart[k] + d;
  Weight = PTotal;

  return true;
}


/******************************************************************************
 * Return the macroscopic cross section (1/length) of a gamma process in the given material
 * EM processes: the same tables as used during tracking
 */
double MCForcedFirstInteractionOperator::GetCrossSection(G4VProcess* Process, const double Energy, const G4MaterialCutsCouple* Couple)
{
  G4VEmProcess* EmProcess = dynamic_cast<G4VEmProcess*>(Process);
  if (EmProcess != nullptr) {
    return EmProcess->GetCrossSection(Energy, Couple);
  }

  G4HadronicProcess* HadronicProcess = dynamic_cast<G4HadronicProcess*>(Process);
  if (HadronicProcess != nullptr) {
    return HadronicProcess->ComputeCrossSection(G4Gamma::Gamma(), Couple->GetMaterial(), Energy);
  }

  G4VDiscreteProcess* DiscreteProcess = dynamic_cast<G4VDiscreteProcess*>(Process);
  if (DiscreteProcess != nullptr) {
    return DiscreteProcess->GetCrossSection(Energy, Couple);
  }

  return 0.0;
}


/******************************************************************************
 * No non-physics biasing
 */
G4VBiasingOperation* MCForcedFirstInteractionOperator::ProposeNonPhysicsBiasingOperation(const G4Track*,
                                                                                         const G4BiasingProcessInterface*)
{
  return nullptr;
}


/******************************************************************************
 * Occurrence biasing for armed tracks, called for each process in each step:
 * the forced process interacts after the remaining distance to the sampled point,
 * all other processes are switched off (their survival is already contained
 * in the sampling of the point)
 */
G4VBiasingOperation* MCForcedFirstInteractionOperator::ProposeOccurenceBiasingOperation(const G4Track* Track,
                                                                                        const G4BiasingProcessInterface* CallingProcess)
{
  if (IsArmed(Track) == false) return nullptr;

  MCTrackInformation* Info = dynamic_cast<MCTrackInformation*>(Track->GetUserInformation());

  // One operation per process, since each needs its own interaction law in this step
  MCForcedFirstInteractionFlight* Flight = nullptr;
  auto Iter = m_Flights.find(CallingProcess);
  if (Iter == m_Flights.end()) {
    Flight = new MCForcedFirstInteractionFlight();
    m_Flights[CallingProcess] = Flight;
  } else {
    Flight = Iter->second;
  }

  // The physical cross section in this step (all wrapped processes have already
  // done their PostStepGPIL in this step) - only used to keep Geant4 weights at 1
  G4VProcess* Wrapped = CallingProcess->GetWrappedProcess();
  double Lambda = Wrapped->GetCurrentInteractionLength();
  double Sigma = (Lambda > 0.0 && Lambda < DBL_MAX) ? 1.0/Lambda : 0.0;

  if (MCForcedFirstInteractionOperation::GetGammaProcessID(Wrapped->GetProcessName()) == Info->GetForcedFirstInteractionProcessID()) {
    // The photon flies straight, thus the track length is the distance from the start point
    Flight->SetInteractionAt(Info->GetForcedFirstInteractionDistance() - Track->GetTrackLength(), Sigma);
  } else {
    Flight->SetNoInteraction(Sigma);
  }

  return Flight;
}


/******************************************************************************
 * Final state biasing: called for the process which interacts at the end of
 * this step. For an armed track this is the forced process at the sampled point.
 */
G4VBiasingOperation* MCForcedFirstInteractionOperator::ProposeFinalStateBiasingOperation(const G4Track* Track,
                                                                                         const G4BiasingProcessInterface*)
{
  if (IsArmed(Track) == true) {
    return m_Operation;
  }

  return nullptr;
}


/*
 * MCForcedFirstInteractionOperator.cc: the end...
 ******************************************************************************/
