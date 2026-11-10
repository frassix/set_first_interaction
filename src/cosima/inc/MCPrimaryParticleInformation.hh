/*
 * MCPrimaryParticleInformation.hh
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
 * Objects of this class belong to a G4PrimaryParticle
 * They transport source-specific information (e.g. which process has to be
 * forced as first interaction) from the source (MCSource/MCRun) to the
 * G4Track of the primary (MCTrackingAction)
 *
 * edited by Andrea Frassà, 11/10/26: new class (forced first interaction mode)
 *
 */

#ifndef ___MCPrimaryParticleInformation___
#define ___MCPrimaryParticleInformation___

// Geant4:
#include "G4VUserPrimaryParticleInformation.hh"


/******************************************************************************/

class MCPrimaryParticleInformation : public G4VUserPrimaryParticleInformation
{
  // public interface:
public:
  /// Default constructor
  MCPrimaryParticleInformation();
  /// Extended constructor
  explicit MCPrimaryParticleInformation(int ForcedFirstInteraction);
  /// Default destructor
  virtual ~MCPrimaryParticleInformation();

  /// Set the process ID (MCSteppingAction::c_ProcessIDXYZ) which has to be
  /// forced as first interaction of this primary, or c_ForcedFirstInteractionNone
  void SetForcedFirstInteraction(const int ForcedFirstInteraction);
  /// Return the process ID which has to be forced as first interaction of this primary
  int GetForcedFirstInteraction() const;
  /// Return true if the first interaction of this primary has to be forced
  bool HasForcedFirstInteraction() const;

  /// Print the content (required by Geant4)
  virtual void Print() const;

  // protected methods:
protected:


  // protected members:
protected:


  // private members:
private:
  /// The process ID (MCSteppingAction::c_ProcessIDXYZ) to be forced as first interaction
  int m_ForcedFirstInteraction;
};

#endif


/*
 * MCPrimaryParticleInformation.hh: the end...
 ******************************************************************************/
