/*
 * MCForcedFirstInteractionOperation.hh
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



/******************************************************************************
 *
 * Geant4 generic biasing operation (final state biasing) for the forced first
 * interaction mode:
 *
 * The armed primary has been transported to its pre-sampled first interaction
 * point (see MCForcedFirstInteractionOperator and MCForcedFirstInteractionFlight),
 * where the forced process is the only one which can interact. This operation
 * lets the forced process generate its real (analog) final state, and disarms
 * the track: all following interactions are analog.
 *
 * If the forced process is not possible at this point (sigma_forced == 0, only
 * possible by rounding on a boundary), nothing happens and the primary
 * continues analog (reported as FAILED).
 *
 * edited by Andrea Frassà, 11/10/26: new class (forced first interaction mode)
 *
 */

#ifndef ___MCForcedFirstInteractionOperation___
#define ___MCForcedFirstInteractionOperation___

// Geant4:
#include "globals.hh"
#include "G4VBiasingOperation.hh"
#include "G4ParticleChange.hh"


/******************************************************************************/

class MCForcedFirstInteractionOperation : public G4VBiasingOperation
{
  // public interface:
public:
  /// Default constructor
  MCForcedFirstInteractionOperation();
  /// Default destructor
  virtual ~MCForcedFirstInteractionOperation();

  /// Apply the forcing: let the forced process generate its real final state and disarm the track
  virtual G4VParticleChange* ApplyFinalStateBiasing(const G4BiasingProcessInterface* CallingProcess,
                                                    const G4Track* Track,
                                                    const G4Step* Step,
                                                    G4bool& ForceBiasedFinalState);

  /// Not used: the occurrence biasing is done by MCForcedFirstInteractionFlight
  virtual const G4VBiasingInteractionLaw* ProvideOccurenceBiasingInteractionLaw(const G4BiasingProcessInterface* CallingProcess,
                                                                                G4ForceCondition& ProposeForceCondition);
  /// Not used: no non-physics biasing
  virtual G4double DistanceToApplyOperation(const G4Track* Track,
                                            G4double PreviousStepSize,
                                            G4ForceCondition* Condition);
  /// Not used: no non-physics biasing
  virtual G4VParticleChange* GenerateBiasingFinalState(const G4Track* Track,
                                                       const G4Step* Step);

  /// Return the cosima process ID (MCSteppingAction::c_ProcessIDXYZ) of a gamma
  /// interaction from the Geant4 process name, or c_ProcessIDUncovered.
  /// Attention: keep this list in sync with the list in the MCSteppingAction constructor!
  static int GetGammaProcessID(const G4String& ProcessName);

  // protected methods:
protected:


  // protected members:
protected:


  // private members:
private:
  /// Counter of failed forcings - for a warning only
  long m_NFailed;
  /// Particle change for "nothing happens"
  G4ParticleChange m_ParticleChange;
};

#endif


/*
 * MCForcedFirstInteractionOperation.hh: the end...
 ******************************************************************************/
