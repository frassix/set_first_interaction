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

// Geant4:
#include "G4Track.hh"
#include "G4LogicalVolume.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4BiasingProcessInterface.hh"

// MEGAlib:
#include "MStreams.h"


/******************************************************************************
 * Default constructor
 */
MCForcedFirstInteractionOperator::MCForcedFirstInteractionOperator()
  : G4VBiasingOperator("MCForcedFirstInteractionOperator")
{
  m_Operation = new MCForcedFirstInteractionOperation();
}


/******************************************************************************
 * Default destructor
 */
MCForcedFirstInteractionOperator::~MCForcedFirstInteractionOperator()
{
  delete m_Operation;
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
 * No non-physics biasing
 */
G4VBiasingOperation* MCForcedFirstInteractionOperator::ProposeNonPhysicsBiasingOperation(const G4Track*,
                                                                                         const G4BiasingProcessInterface*)
{
  return nullptr;
}


/******************************************************************************
 * No occurrence biasing: where the first interaction happens is NOT changed
 */
G4VBiasingOperation* MCForcedFirstInteractionOperator::ProposeOccurenceBiasingOperation(const G4Track*,
                                                                                        const G4BiasingProcessInterface*)
{
  return nullptr;
}


/******************************************************************************
 * Final state biasing: this is called for the process which Geant4 selected
 * for the interaction at the end of this step. If the track is still armed,
 * this is its first interaction, and we replace its final state.
 */
G4VBiasingOperation* MCForcedFirstInteractionOperator::ProposeFinalStateBiasingOperation(const G4Track* Track,
                                                                                         const G4BiasingProcessInterface*)
{
  MCTrackInformation* Info = dynamic_cast<MCTrackInformation*>(Track->GetUserInformation());
  if (Info != nullptr && Info->IsForcedFirstInteractionPending() == true) {
    return m_Operation;
  }

  return nullptr;
}


/*
 * MCForcedFirstInteractionOperator.cc: the end...
 ******************************************************************************/
