/*
 * MCPrimaryParticleInformation.cc
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
#include "MCPrimaryParticleInformation.hh"
#include "MCSource.hh"

// MEGAlib:
#include "MStreams.h"


/******************************************************************************
 * Default constructor: no forced first interaction
 */
MCPrimaryParticleInformation::MCPrimaryParticleInformation()
  : m_ForcedFirstInteraction(MCSource::c_ForcedFirstInteractionNone)
{
  // Intentionally left blank
}


/******************************************************************************
 * Extended constructor
 */
MCPrimaryParticleInformation::MCPrimaryParticleInformation(int ForcedFirstInteraction)
  : m_ForcedFirstInteraction(ForcedFirstInteraction)
{
  // Intentionally left blank
}


/******************************************************************************
 * Default destructor
 */
MCPrimaryParticleInformation::~MCPrimaryParticleInformation()
{
  // Intentionally left blank
}


/******************************************************************************
 * Set the process ID which has to be forced as first interaction
 */
void MCPrimaryParticleInformation::SetForcedFirstInteraction(const int ForcedFirstInteraction)
{
  m_ForcedFirstInteraction = ForcedFirstInteraction;
}


/******************************************************************************
 * Return the process ID which has to be forced as first interaction
 */
int MCPrimaryParticleInformation::GetForcedFirstInteraction() const
{
  return m_ForcedFirstInteraction;
}


/******************************************************************************
 * Return true if the first interaction of this primary has to be forced
 */
bool MCPrimaryParticleInformation::HasForcedFirstInteraction() const
{
  return m_ForcedFirstInteraction != MCSource::c_ForcedFirstInteractionNone;
}


/******************************************************************************
 * Print the content (required by Geant4)
 */
void MCPrimaryParticleInformation::Print() const
{
  mout<<"MCPrimaryParticleInformation: forced first interaction ID: "<<m_ForcedFirstInteraction<<endl;
}


/*
 * MCPrimaryParticleInformation.cc: the end...
 ******************************************************************************/
