/*
 * MCForcedFirstInteractionOperator.hh
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
 * Geant4 generic biasing operator for the forced first interaction mode:
 *
 * It is attached to ALL logical volumes (the first interaction can happen
 * anywhere) and proposes the final state operation
 * MCForcedFirstInteractionOperation only for tracks which are still armed
 * (MCTrackInformation::IsForcedFirstInteractionPending()), i.e. primaries of a
 * source with "ForceFirstInteraction" before their first interaction.
 * For all other tracks and steps Geant4 runs completely analog.
 *
 * edited by Andrea Frassà, 11/10/26: new class (forced first interaction mode)
 *
 */

#ifndef ___MCForcedFirstInteractionOperator___
#define ___MCForcedFirstInteractionOperator___

// Geant4:
#include "globals.hh"
#include "G4VBiasingOperator.hh"

// Cosima:
#include "MCForcedFirstInteractionOperation.hh"


/******************************************************************************/

class MCForcedFirstInteractionOperator : public G4VBiasingOperator
{
  // public interface:
public:
  /// Default constructor
  MCForcedFirstInteractionOperator();
  /// Default destructor
  virtual ~MCForcedFirstInteractionOperator();

  /// Attach this operator to all logical volumes of the geometry
  void AttachToAllVolumes();

  // protected methods:
protected:
  /// No non-physics biasing
  virtual G4VBiasingOperation* ProposeNonPhysicsBiasingOperation(const G4Track* Track,
                                                                 const G4BiasingProcessInterface* CallingProcess);
  /// No occurrence biasing: the interaction location is sampled from the analog total cross section
  virtual G4VBiasingOperation* ProposeOccurenceBiasingOperation(const G4Track* Track,
                                                                const G4BiasingProcessInterface* CallingProcess);
  /// Final state biasing: only for armed tracks
  virtual G4VBiasingOperation* ProposeFinalStateBiasingOperation(const G4Track* Track,
                                                                 const G4BiasingProcessInterface* CallingProcess);

  // protected members:
protected:


  // private members:
private:
  /// The one and only operation
  MCForcedFirstInteractionOperation* m_Operation;
};

#endif


/*
 * MCForcedFirstInteractionOperator.hh: the end...
 ******************************************************************************/
