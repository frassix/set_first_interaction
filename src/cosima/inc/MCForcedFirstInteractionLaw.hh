/*
 * MCForcedFirstInteractionLaw.hh
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
 * Geant4 biasing interaction law for the forced first interaction mode.
 *
 * While a primary flies to its (pre-sampled) first interaction point, each of
 * its physics processes gets one of these laws instead of its analog law:
 * - the forced process: it interacts exactly at the remaining distance to the
 *   sampled interaction point,
 * - all other processes: they never interact (free flight).
 *
 * The survival probability of the photon up to the interaction point is already
 * contained in the sampling of the point (see MCForcedFirstInteractionOperator).
 *
 * The non-interaction probability and effective cross section return the
 * PHYSICAL values of the process. This way the Geant4 occurrence biasing
 * weights are exactly 1, i.e. Geant4 track weights stay untouched.
 * Our weight is stored separately (CC line).
 *
 * edited by Andrea Frassà, 11/10/26: new class (forced first interaction mode)
 *
 */

#ifndef ___MCForcedFirstInteractionLaw___
#define ___MCForcedFirstInteractionLaw___

// Geant4:
#include "globals.hh"
#include "G4VBiasingInteractionLaw.hh"


/******************************************************************************/

class MCForcedFirstInteractionLaw : public G4VBiasingInteractionLaw
{
  // public interface:
public:
  /// Default constructor
  MCForcedFirstInteractionLaw();
  /// Default destructor
  virtual ~MCForcedFirstInteractionLaw();

  /// Set this law as "interact at the given distance" (forced process)
  void SetInteractionAt(const G4double RemainingDistance);
  /// Set this law as "never interact" (all other processes)
  void SetNoInteraction();
  /// Set the physical cross section of the process in the current step
  /// (only used to keep the Geant4 weights untouched)
  void SetPhysicalCrossSection(const G4double CrossSection) { m_PhysicalCrossSection = CrossSection; }

  /// Return the PHYSICAL non-interaction probability (--> Geant4 weight stays 1)
  virtual G4double ComputeNonInteractionProbabilityAt(G4double Length) const;
  /// Return the PHYSICAL cross section (--> Geant4 weight stays 1)
  virtual G4double ComputeEffectiveCrossSectionAt(G4double Length) const;

  // protected methods:
protected:
  /// Return the distance to the interaction: the remaining distance or DBL_MAX
  virtual G4double SampleInteractionLength();
  /// Reduce the remaining distance by the step length
  virtual G4double UpdateInteractionLengthForStep(G4double TruePathLength);

  // protected members:
protected:


  // private members:
private:
  /// True if this process has to interact at m_RemainingDistance
  bool m_Interacts;
  /// Remaining distance to the interaction point
  G4double m_RemainingDistance;
  /// Physical cross section of the process in the current step
  G4double m_PhysicalCrossSection;
};

#endif


/*
 * MCForcedFirstInteractionLaw.hh: the end...
 ******************************************************************************/
