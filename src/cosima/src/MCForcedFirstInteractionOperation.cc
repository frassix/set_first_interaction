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
#include "G4VProcess.hh"
#include "G4Track.hh"
#include "G4Step.hh"
#include "G4Material.hh"
#include "G4SystemOfUnits.hh"

// MEGAlib:
#include "MStreams.h"

// Standard lib:
#include <cfloat>
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
 * Apply the forcing
 *
 * When we are here, the armed primary has reached its pre-sampled first
 * interaction point (MCForcedFirstInteractionOperator::StartTracking): during
 * the flight all processes but the forced one were switched off
 * (MCForcedFirstInteractionFlight), thus the calling process is the forced one,
 * and we just have to let it generate its real, analog final state.
 */
G4VParticleChange* MCForcedFirstInteractionOperation::ApplyFinalStateBiasing(const G4BiasingProcessInterface* CallingProcess,
                                                                             const G4Track* Track,
                                                                             const G4Step* Step,
                                                                             G4bool& ForceBiasedFinalState)
{
  // We return the particle change "as is", the framework should not touch weights:
  // cosima does not use Geant4 track weights, our weight is stored as CC line instead
  ForceBiasedFinalState = true;

  G4VProcess* Process = CallingProcess->GetWrappedProcess();

  // Safety check: the operator only proposes this operation for armed tracks
  MCTrackInformation* Info = dynamic_cast<MCTrackInformation*>(Track->GetUserInformation());
  if (Info == nullptr || Info->IsForcedFirstInteractionPending() == false) {
    return Process->PostStepDoIt(*Track, *Step);
  }

  int ForcedID = Info->GetForcedFirstInteractionProcessID();


  // Case 1 (the normal case): the forced process interacts at the sampled point
  if (GetGammaProcessID(Process->GetProcessName()) == ForcedID) {
    // The current interaction length is the inverse of the macroscopic cross section
    // of the material at this point (DBL_MAX if the process cannot happen)
    double Lambda = Process->GetCurrentInteractionLength();
    if (Lambda > 0.0 && Lambda < DBL_MAX) {
      // The real final state of the forced process
      G4VParticleChange* ParticleChange = Process->PostStepDoIt(*Track, *Step);
      // Disarm: all following interactions of this track are analog
      // (Geant4 resets the interaction lengths of all other processes in the next step)
      Info->SetForcedFirstInteractionDone(true);
      return ParticleChange;
    }

    // Case 2: the forced process is not possible here (sigma == 0). This can only happen
    // by rounding, if the sampled point is exactly on a boundary to a material in which
    // the process is impossible. Then nothing happens here and the track continues analog.
    if (m_NFailed == 0) {
      mout<<"Forced first interaction: The forced process ("<<MCSource::GetForcedFirstInteractionAsString(ForcedID)
          <<") has a zero cross section at the sampled interaction point (E="<<Track->GetKineticEnergy()/CLHEP::keV
          <<" keV, material: "<<Track->GetMaterial()->GetName()<<")."<<endl
          <<"  Such primaries are simulated analog and are marked with \"FAILED\" in the CC line. This message is only shown once."<<endl;
    }
    m_NFailed++;
    Info->SetForcedFirstInteractionDone(false);
    m_ParticleChange.Initialize(*Track);
    return &m_ParticleChange;
  }


  // Case 3: Another process interacts - should never happen, since all other processes
  // are switched off during the flight. Do the analog interaction and disarm
  if (m_NFailed == 0) {
    mout<<"Forced first interaction: Unexpected first interaction "<<Process->GetProcessName()
        <<" instead of "<<MCSource::GetForcedFirstInteractionAsString(ForcedID)<<"."<<endl
        <<"  Such primaries are simulated analog and are marked with \"FAILED\" in the CC line. This message is only shown once."<<endl;
  }
  m_NFailed++;
  Info->SetForcedFirstInteractionDone(false);
  return Process->PostStepDoIt(*Track, *Step);
}


/******************************************************************************
 * Not used: the occurrence biasing is done by MCForcedFirstInteractionFlight
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
