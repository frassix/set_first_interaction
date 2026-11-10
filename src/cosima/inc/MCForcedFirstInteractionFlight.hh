/*
 * MCForcedFirstInteractionFlight.hh
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
 * Geant4 occurrence biasing operation for the forced first interaction mode:
 *
 * It controls WHERE a process of an armed primary interacts, while the primary
 * flies to its pre-sampled first interaction point:
 * - forced process: exactly at the sampled point
 * - all other processes: never
 *
 * There is one object per (wrapped) process, since each process needs its own
 * interaction law in each step.
 *
 * edited by Andrea Frassà, 11/10/26: new class (forced first interaction mode)
 *
 */

#ifndef ___MCForcedFirstInteractionFlight___
#define ___MCForcedFirstInteractionFlight___

// Geant4:
#include "globals.hh"
#include "G4VBiasingOperation.hh"

// Cosima:
#include "MCForcedFirstInteractionLaw.hh"


/******************************************************************************/

class MCForcedFirstInteractionFlight : public G4VBiasingOperation
{
  // public interface:
public:
  /// Default constructor
  MCForcedFirstInteractionFlight();
  /// Default destructor
  virtual ~MCForcedFirstInteractionFlight();

  /// Prepare this step for the forced process: interact after the remaining distance
  void SetInteractionAt(const G4double RemainingDistance, const G4double PhysicalCrossSection);
  /// Prepare this step for any other process: no interaction
  void SetNoInteraction(const G4double PhysicalCrossSection);

  /// Return the interaction law prepared for this step
  virtual const G4VBiasingInteractionLaw* ProvideOccurenceBiasingInteractionLaw(const G4BiasingProcessInterface* CallingProcess,
                                                                                G4ForceCondition& ProposeForceCondition);
  /// Keep the remaining distance up to date
  virtual void AlongMoveBy(const G4BiasingProcessInterface* CallingProcess,
                           const G4Step* Step,
                           G4double WeightForNonInteraction);

  /// Not used: the final state is handled by MCForcedFirstInteractionOperation
  virtual G4VParticleChange* ApplyFinalStateBiasing(const G4BiasingProcessInterface* CallingProcess,
                                                    const G4Track* Track,
                                                    const G4Step* Step,
                                                    G4bool& ForceBiasedFinalState);
  /// Not used: no non-physics biasing
  virtual G4double DistanceToApplyOperation(const G4Track* Track,
                                            G4double PreviousStepSize,
                                            G4ForceCondition* Condition);
  /// Not used: no non-physics biasing
  virtual G4VParticleChange* GenerateBiasingFinalState(const G4Track* Track,
                                                       const G4Step* Step);

  // protected methods:
protected:


  // protected members:
protected:


  // private members:
private:
  /// The interaction law of this process
  MCForcedFirstInteractionLaw* m_Law;
};

#endif


/*
 * MCForcedFirstInteractionFlight.hh: the end...
 ******************************************************************************/
