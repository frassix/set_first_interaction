/*
 * MCTrackingAction.cc
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


// Cosima:
#include "MCTrackingAction.hh"
#include "MCTrackInformation.hh"
#include "MCRunManager.hh"
#include "MCPhysicsList.hh"
#include "MCPrimaryParticleInformation.hh" // edited by Andrea Frassà, 11/10/26

// MEGAlib:
#include "MStreams.h"

// Geant4:
#include "G4DynamicParticle.hh" // edited by Andrea Frassà, 11/10/26
#include "G4PrimaryParticle.hh" // edited by Andrea Frassà, 11/10/26

// Standard lib:


/******************************************************************************
 * Default constructor
 */
MCTrackingAction::MCTrackingAction()
{
  // Intentionally left blank
}


/******************************************************************************
 * Default destructor
 */
MCTrackingAction::~MCTrackingAction()
{
  // Intentionally left blank
}


/******************************************************************************
 * Actions before tracking starts
 */
void MCTrackingAction::PreUserTrackingAction(const G4Track* Track)
{
  G4Track* MyTrack = const_cast<G4Track*>(Track); 

  if (MyTrack->GetUserInformation() == 0) {
    MCTrackInformation* Info = new MCTrackInformation(m_NPrimaries, m_NPrimaries);
    MyTrack->SetUserInformation(Info);
    m_NPrimaries--;
    
    // edited by Andrea Frassà, 11/10/26: forced first interaction mode -->
    // Only primaries (parent ID == 0) can carry the forcing: the information is
    // attached to the G4PrimaryParticle by the source (MCRun::GeneratePrimaries).
    // This is called before the processes' StartTracking, thus the biasing operator
    // sees the armed track already at the first step
    if (MyTrack->GetParentID() == 0 && MyTrack->GetDynamicParticle()->GetPrimaryParticle() != nullptr) {
      MCPrimaryParticleInformation* PrimaryInfo = 
        dynamic_cast<MCPrimaryParticleInformation*>(MyTrack->GetDynamicParticle()->GetPrimaryParticle()->GetUserInformation());
      if (PrimaryInfo != nullptr && PrimaryInfo->HasForcedFirstInteraction() == true) {
        Info->ArmForcedFirstInteraction(PrimaryInfo->GetForcedFirstInteraction());
      }
    }
    // <-- edited by Andrea Frassà, 11/10/26
  }
  
//   if (Track->GetDefinition()->GetParticleName() == "gamma") {
//     MCRunManager::GetMCRunManager()->GetPhysicsList()->SetGammaCuts();
//   }

}


/******************************************************************************
 * Set the number of generated particles
 */
void MCTrackingAction::SetNGeneratedParticles(int NGeneratedParticles)
{
  m_NPrimaries = NGeneratedParticles;
}


/*
 * MCTrackingAction.cc: the end...
 ******************************************************************************/
