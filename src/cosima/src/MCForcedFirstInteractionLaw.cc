/*
 * MCForcedFirstInteractionLaw.cc
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
#include "MCForcedFirstInteractionLaw.hh"

// Standard lib:
#include <cmath>
#include <cfloat>
#include <algorithm>
using namespace std;


/******************************************************************************
 * Default constructor
 */
MCForcedFirstInteractionLaw::MCForcedFirstInteractionLaw()
  : G4VBiasingInteractionLaw("MCForcedFirstInteractionLaw"),
    m_Interacts(false), m_RemainingDistance(DBL_MAX), m_PhysicalCrossSection(0.0)
{
  // Intentionally left blank
}


/******************************************************************************
 * Default destructor
 */
MCForcedFirstInteractionLaw::~MCForcedFirstInteractionLaw()
{
  // Intentionally left blank
}


/******************************************************************************
 * Set this law as "interact at the given distance" (forced process)
 */
void MCForcedFirstInteractionLaw::SetInteractionAt(const G4double RemainingDistance)
{
  m_Interacts = true;
  m_RemainingDistance = max(RemainingDistance, 0.0);
}


/******************************************************************************
 * Set this law as "never interact" (all other processes)
 */
void MCForcedFirstInteractionLaw::SetNoInteraction()
{
  m_Interacts = false;
  m_RemainingDistance = DBL_MAX;
}


/******************************************************************************
 * Return the PHYSICAL non-interaction probability:
 * Geant4 multiplies the track weight by p_physical/p_biased, which thus stays 1
 */
G4double MCForcedFirstInteractionLaw::ComputeNonInteractionProbabilityAt(G4double Length) const
{
  return exp(-m_PhysicalCrossSection*Length);
}


/******************************************************************************
 * Return the PHYSICAL cross section (only used for Geant4 weights, which stay 1)
 */
G4double MCForcedFirstInteractionLaw::ComputeEffectiveCrossSectionAt(G4double) const
{
  return m_PhysicalCrossSection;
}


/******************************************************************************
 * Return the distance to the interaction
 */
G4double MCForcedFirstInteractionLaw::SampleInteractionLength()
{
  return (m_Interacts == true) ? m_RemainingDistance : DBL_MAX;
}


/******************************************************************************
 * Reduce the remaining distance by the step length
 */
G4double MCForcedFirstInteractionLaw::UpdateInteractionLengthForStep(G4double TruePathLength)
{
  if (m_Interacts == false) return DBL_MAX;

  m_RemainingDistance = max(m_RemainingDistance - TruePathLength, 0.0);
  return m_RemainingDistance;
}


/*
 * MCForcedFirstInteractionLaw.cc: the end...
 ******************************************************************************/
