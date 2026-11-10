/*
 * MCForcedFirstInteractionOperation.cc
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
#include "MCForcedFirstInteractionOperation.hh"
#include "MCTrackInformation.hh"
#include "MCSteppingAction.hh"
#include "MCSource.hh"

// Geant4:
#include "G4BiasingProcessInterface.hh"
#include "G4BiasingProcessSharedData.hh"
#include "G4VProcess.hh"
#include "G4Track.hh"
#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4Material.hh"
#include "G4SystemOfUnits.hh"

// MEGAlib:
#include "MStreams.h"

// Standard lib:
#include <cfloat>
#include <vector>
using namespace std;


/******************************************************************************
 * Default constructor
 */
MCForcedFirstInteractionOperation::MCForcedFirstInteractionOperation()
  : G4VBiasingOperation("MCForcedFirstInteractionOperation"), m_NFailed(0)
{
  // Intentionally left blank
}


/******************************************************************************
 * Default destructor
 */
MCForcedFirstInteractionOperation::~MCForcedFirstInteractionOperation()
{
  // Intentionally left blank
}


/******************************************************************************
 * Return the cosima process ID of a gamma interaction from the Geant4 name
 * Attention: keep this list in sync with the list in the MCSteppingAction constructor!
 */
int MCForcedFirstInteractionOperation::GetGammaProcessID(const G4String& Name)
{
  if (Name == "compt" || Name == "polarLowEnCompt" || Name == "PenCompton" || Name == "LowEnCompton") {
    return MCSteppingAction::c_ProcessIDCompton;
  } else if (Name == "phot" || Name == "LowEnPhotoElec" || Name == "PenPhotoElec") {
    return MCSteppingAction::c_ProcessIDPhoto;
  } else if (Name == "conv" || Name == "PenConversion" || Name == "LowEnConversion" || Name == "LowEnPolarizConversion") {
    return MCSteppingAction::c_ProcessIDPair;
  } else if (Name == "Rayl" || Name == "PenRayleigh" || Name == "LowEnRayleigh") {
    return MCSteppingAction::c_ProcessIDRayleigh;
  } else if (Name == "photonNuclear" || Name == "PhotonInelastic") {
    return MCSteppingAction::c_ProcessIDInelasticScattering;
  }

  return MCSteppingAction::c_ProcessIDUncovered;
}


/******************************************************************************
 * Apply the forcing.
 *
 * When we are here, Geant4 has selected the natural process "CallingProcess" to
 * interact at the end of this step. The location was sampled from the analog
 * total cross section, thus it is not modified.
 *
 * All wrapped processes have computed their PostStepGPIL in this step (done by
 * the first G4BiasingProcessInterface), thus their current interaction lengths
 * are up to date and can be used to compute the cross sections at this point.
 */
G4VParticleChange* MCForcedFirstInteractionOperation::ApplyFinalStateBiasing(const G4BiasingProcessInterface* CallingProcess,
                                                                             const G4Track* Track,
                                                                             const G4Step* Step,
                                                                             G4bool& ForceBiasedFinalState)
{
  // We return the particle change "as is", the framework should not touch weights:
  // cosima does not use Geant4 track weights, the weight is stored as CC line instead
  ForceBiasedFinalState = true;

  G4VProcess* NaturalProcess = CallingProcess->GetWrappedProcess();

  // Safety check: the operator only proposes this operation for armed tracks
  MCTrackInformation* Info = dynamic_cast<MCTrackInformation*>(Track->GetUserInformation());
  if (Info == nullptr || Info->IsForcedFirstInteractionPending() == false) {
    return NaturalProcess->PostStepDoIt(*Track, *Step);
  }

  int ForcedID = Info->GetForcedFirstInteractionProcessID();
  int NaturalID = GetGammaProcessID(NaturalProcess->GetProcessName());


  // Step 1: Determine the cross sections of all physics processes at this point
  //         and find the forced process
  G4VProcess* ForcedProcess = nullptr;
  double SigmaForced = 0.0;
  double SigmaTotal = 0.0;

  const vector<const G4BiasingProcessInterface*>& Interfaces = CallingProcess->GetSharedData()->GetPhysicsBiasingProcessInterfaces();
  for (unsigned int i = 0; i < Interfaces.size(); ++i) {
    G4VProcess* Wrapped = Interfaces[i]->GetWrappedProcess();
    if (Wrapped == nullptr) continue;

    // The current interaction length is the inverse of the macroscopic cross section
    // (DBL_MAX if the process cannot happen, e.g. below threshold)
    double Lambda = Wrapped->GetCurrentInteractionLength();
    double Sigma = (Lambda > 0.0 && Lambda < DBL_MAX) ? 1.0/Lambda : 0.0;
    SigmaTotal += Sigma;

    if (ForcedProcess == nullptr && GetGammaProcessID(Wrapped->GetProcessName()) == ForcedID) {
      ForcedProcess = Wrapped;
      SigmaForced = Sigma;
    }
  }


  // Step 2: Option (a): if the forced process is not possible here, the natural process happens
  //         The first interaction is done anyway, thus the track is disarmed
  if (ForcedProcess == nullptr || SigmaForced <= 0.0 || SigmaTotal <= 0.0) {
    if (m_NFailed == 0) {
      mout<<"Forced first interaction: The forced process ("<<MCSource::GetForcedFirstInteractionAsString(ForcedID)
          <<") has a zero cross section at the first interaction point of a primary (E="<<Track->GetKineticEnergy()/CLHEP::keV
          <<" keV, material: "<<Track->GetMaterial()->GetName()<<")."<<endl
          <<"  Such primaries are simulated analog and are marked with \"FAILED\" in the CC line. This message is only shown once."<<endl;
    }
    m_NFailed++;
    Info->SetForcedFirstInteractionDone(false, 0.0, NaturalID);
    return NaturalProcess->PostStepDoIt(*Track, *Step);
  }

  double Weight = SigmaForced/SigmaTotal;


  // Step 3: Generate the final state
  G4VParticleChange* ParticleChange = nullptr;
  if (ForcedProcess == NaturalProcess) {
    // Nothing to replace - Geant4 picked the forced process anyway
    ParticleChange = NaturalProcess->PostStepDoIt(*Track, *Step);
  } else {
    // The real final state of the forced process (it resets its own number of interaction lengths)
    ParticleChange = ForcedProcess->PostStepDoIt(*Track, *Step);

    // The natural process did not do its PostStepDoIt, thus its number of interaction
    // lengths left would be ~0 and it would interact again immediately in the next step.
    // StartTracking sets it to -1, i.e. it is resampled from scratch at the next step -
    // exactly what its own PostStepDoIt would have done (memoryless exponential law)
    NaturalProcess->StartTracking(const_cast<G4Track*>(Track));

    // Tell the stepping action which process REALLY happened, this way it records
    // e.g. "PHOT" and not "COMP" (same trick as used by G4GammaGeneralProcess)
    Step->GetPostStepPoint()->SetProcessDefinedStep(ForcedProcess);
  }


  // Step 4: Disarm: all following interactions of this track are analog
  Info->SetForcedFirstInteractionDone(true, Weight, NaturalID);

  return ParticleChange;
}


/******************************************************************************
 * Not used: no occurrence biasing
 */
const G4VBiasingInteractionLaw* MCForcedFirstInteractionOperation::ProvideOccurenceBiasingInteractionLaw(const G4BiasingProcessInterface*,
                                                                                                         G4ForceCondition&)
{
  return nullptr;
}


/******************************************************************************
 * Not used: no non-physics biasing
 */
G4double MCForcedFirstInteractionOperation::DistanceToApplyOperation(const G4Track*,
                                                                     G4double,
                                                                     G4ForceCondition* Condition)
{
  *Condition = NotForced;
  return DBL_MAX;
}


/******************************************************************************
 * Not used: no non-physics biasing
 */
G4VParticleChange* MCForcedFirstInteractionOperation::GenerateBiasingFinalState(const G4Track*,
                                                                                const G4Step*)
{
  return nullptr;
}


/*
 * MCForcedFirstInteractionOperation.cc: the end...
 ******************************************************************************/
