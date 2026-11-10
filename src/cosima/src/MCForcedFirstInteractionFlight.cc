/*
 * MCForcedFirstInteractionFlight.cc
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
#include "MCForcedFirstInteractionFlight.hh"

// Geant4:
#include "G4Step.hh"

// Standard lib:
#include <cfloat>
using namespace std;


/******************************************************************************
 * Default constructor
 */
MCForcedFirstInteractionFlight::MCForcedFirstInteractionFlight()
  : G4VBiasingOperation("MCForcedFirstInteractionFlight")
{
  m_Law = new MCForcedFirstInteractionLaw();
}


/******************************************************************************
 * Default destructor
 */
MCForcedFirstInteractionFlight::~MCForcedFirstInteractionFlight()
{
  delete m_Law;
}


/******************************************************************************
 * Prepare this step for the forced process: interact after the remaining distance
 */
void MCForcedFirstInteractionFlight::SetInteractionAt(const G4double RemainingDistance, const G4double PhysicalCrossSection)
{
  m_Law->SetInteractionAt(RemainingDistance);
  m_Law->SetPhysicalCrossSection(PhysicalCrossSection);
}


/******************************************************************************
 * Prepare this step for any other process: no interaction
 */
void MCForcedFirstInteractionFlight::SetNoInteraction(const G4double PhysicalCrossSection)
{
  m_Law->SetNoInteraction();
  m_Law->SetPhysicalCrossSection(PhysicalCrossSection);
}


/******************************************************************************
 * Return the interaction law prepared for this step.
 * The force condition of the wrapped process (NotForced for gammas) is kept.
 */
const G4VBiasingInteractionLaw* MCForcedFirstInteractionFlight::ProvideOccurenceBiasingInteractionLaw(const G4BiasingProcessInterface*,
                                                                                                      G4ForceCondition&)
{
  // Store the distance to interaction as "sampled interaction length" in the law
  // --> this is what the biasing interface returns as PostStepGPIL
  m_Law->Sample();
  return m_Law;
}


/******************************************************************************
 * Keep the remaining distance up to date (it is anyway recalculated
 * from the track length in each step by the operator)
 */
void MCForcedFirstInteractionFlight::AlongMoveBy(const G4BiasingProcessInterface*,
                                                 const G4Step* Step,
                                                 G4double)
{
  m_Law->UpdateForStep(Step->GetStepLength());
}


/******************************************************************************
 * Not used: the final state is handled by MCForcedFirstInteractionOperation
 */
G4VParticleChange* MCForcedFirstInteractionFlight::ApplyFinalStateBiasing(const G4BiasingProcessInterface*,
                                                                          const G4Track*,
                                                                          const G4Step*,
                                                                          G4bool&)
{
  return nullptr;
}


/******************************************************************************
 * Not used: no non-physics biasing
 */
G4double MCForcedFirstInteractionFlight::DistanceToApplyOperation(const G4Track*,
                                                                  G4double,
                                                                  G4ForceCondition* Condition)
{
  *Condition = NotForced;
  return DBL_MAX;
}


/******************************************************************************
 * Not used: no non-physics biasing
 */
G4VParticleChange* MCForcedFirstInteractionFlight::GenerateBiasingFinalState(const G4Track*,
                                                                             const G4Step*)
{
  return nullptr;
}


/*
 * MCForcedFirstInteractionFlight.cc: the end...
 ******************************************************************************/
