/*
 * MCSD.cc
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
#include "MCSD.hh"

// MEGAlib:
#include "MDDetector.h"

// Geant4:
#include "G4EventManager.hh" // edited by Andrea Frassà, 11/16/26

// Cosima:
#include "MCEventAction.hh" // edited by Andrea Frassà, 11/16/26


/******************************************************************************/


const int MCSD::c_Unknown      = MDDetector::c_NoDetectorType;
const int MCSD::c_2DStrip      = MDDetector::c_Strip2D;
const int MCSD::c_Calorimeter  = MDDetector::c_Calorimeter;
const int MCSD::c_3DStrip      = MDDetector::c_Strip3D;
const int MCSD::c_Scintillator = MDDetector::c_Scintillator;
const int MCSD::c_DriftChamber = MDDetector::c_DriftChamber;
const int MCSD::c_Voxel3D      = MDDetector::c_Voxel3D;
const int MCSD::c_AngerCamera  = MDDetector::c_AngerCamera;


/******************************************************************************
 * Default constructor, which only sets the name and the type to Unknown
 */
MCSD::MCSD(G4String Name) : G4VSensitiveDetector(Name), 
                            m_Type(c_Unknown),
                            m_DiscretizeHits(true),
                            m_UseEnergyLoss(false),
                            m_HasTimeResolution(false),
                            m_IsNeverTriggering(false),
                            m_Epsilon(1E-10)
{
  // Intentionally left blank

}


/******************************************************************************
 * Not yet implemented
 */
MCSD::~MCSD()
{
  // Intentionally left blank
}


/******************************************************************************
 * Return the type of the detector:
 * c_Unknown = 0;
 * c_2DStrip = 1;
 * c_Calorimeter = 2;
 * c_Scintillator = 4;
 */
int MCSD::GetType()
{
  return m_Type;
}

/******************************************************************************
 * Report a deposit, which has just been stored in a hit, to the event action:
 * used by the energy window mode to know the deposit in the active volumes 
 * during the event (for the early abort)
 * edited by Andrea Frassà, 11/16/26
 */
void MCSD::ReportDeposit(const G4double Energy, const bool IsGuardring)
{
  MCEventAction* EventAction = dynamic_cast<MCEventAction*>(G4EventManager::GetEventManager()->GetUserEventAction());
  if (EventAction != nullptr && EventAction->UsesEnergyWindow() == true) {
    EventAction->AddSensitiveDeposit(GetName(), Energy, IsGuardring, m_IsNeverTriggering);
  }
}


/*
 * MCSD.cc: the end...
 ******************************************************************************/
