/*
 * MCTrackInformation.cc
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
#include "MCTrackInformation.hh"


/******************************************************************************
 * Default constructor
 */
MCTrackInformation::MCTrackInformation() 
  : m_IsDigitized(false), m_Id(-1), m_OriginId(-1),
    // edited by Andrea Frassà, 11/10/26: by default nothing is forced
    m_ForcedFirstInteractionPending(false), m_ForcedFirstInteractionProcessID(-1),
    m_ForcedFirstInteractionReportPending(false), m_ForcedFirstInteractionApplied(false),
    m_ForcedFirstInteractionWeight(0.0), m_ForcedFirstInteractionNaturalProcessID(-1)
{
  // Intentionally left blank
}


/******************************************************************************
 * Extended constructor
 */
MCTrackInformation::MCTrackInformation(int Id, int OriginId) 
  : m_IsDigitized(false), m_Id(Id), m_OriginId(OriginId),
    // edited by Andrea Frassà, 11/10/26: by default nothing is forced 
    // (this constructor is used for all secondaries, thus they are never forced)
    m_ForcedFirstInteractionPending(false), m_ForcedFirstInteractionProcessID(-1),
    m_ForcedFirstInteractionReportPending(false), m_ForcedFirstInteractionApplied(false),
    m_ForcedFirstInteractionWeight(0.0), m_ForcedFirstInteractionNaturalProcessID(-1)
{
  // Intentionally left blank
}


/******************************************************************************
 * Default destructor
 */
MCTrackInformation::~MCTrackInformation()
{
  // Intentionally left blank
}


/******************************************************************************
 * Set, if the digitization of the last hit was successful
 */
void MCTrackInformation::SetDigitized(const bool IsDigitizedFlag)
{
  m_IsDigitized = IsDigitizedFlag;
}


/******************************************************************************
 * Return true, if the digitization of the last hit was successful
 */
bool MCTrackInformation::IsDigitized() const
{
  return m_IsDigitized;
}


/******************************************************************************
 * Set the id if this track
 */
void MCTrackInformation::SetId(const int Id)
{
  m_Id = Id;
}


/******************************************************************************
 * Return the id of this track
 */
int MCTrackInformation::GetId() const
{
  return m_Id;
}



/******************************************************************************
 * Set the originating track Id 
 */
void MCTrackInformation::SetOriginId(const int OriginId)
{
  m_OriginId = OriginId;
}


/******************************************************************************
 * Return the originating track Id 
 */
int MCTrackInformation::GetOriginId() const
{
  return m_OriginId;
}


/******************************************************************************
 * Arm the forced first interaction
 * edited by Andrea Frassà, 11/10/26
 */
void MCTrackInformation::ArmForcedFirstInteraction(const int ForcedProcessID)
{
  m_ForcedFirstInteractionPending = true;
  m_ForcedFirstInteractionProcessID = ForcedProcessID;
  m_ForcedFirstInteractionReportPending = false;
  m_ForcedFirstInteractionApplied = false;
  m_ForcedFirstInteractionWeight = 0.0;
  m_ForcedFirstInteractionNaturalProcessID = -1;
}


/******************************************************************************
 * Disarm the forcing after the first interaction happened 
 * and remember the result until it is reported 
 * edited by Andrea Frassà, 11/10/26
 */
void MCTrackInformation::SetForcedFirstInteractionDone(const bool IsForced, const double Weight, const int NaturalProcessID)
{
  m_ForcedFirstInteractionPending = false;
  m_ForcedFirstInteractionReportPending = true;
  m_ForcedFirstInteractionApplied = IsForced;
  m_ForcedFirstInteractionWeight = Weight;
  m_ForcedFirstInteractionNaturalProcessID = NaturalProcessID;
}


/*
 * MCTrackInformation.cc: the end...
 ******************************************************************************/
