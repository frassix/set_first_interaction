/*
 * MCTrackInformation.hh
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
 * Objects of this class belong to a track
 * This class stores temporary additional information of this track
 *
 */

#ifndef ___MCTrackInformation___
#define ___MCTrackInformation___

// Geant4:
#include "G4VUserTrackInformation.hh"


/******************************************************************************/

class MCTrackInformation : public G4VUserTrackInformation
{
  // public interface:
public:
  /// Default constructor
  MCTrackInformation();
  /// Extended constructor
  MCTrackInformation(int Id, int OriginId);
  /// Default destructor
  virtual ~MCTrackInformation();

  /// Set, if the digitization of the last hit was successful
  void SetDigitized(const bool IsDigitized);
  /// Return true, if the digitization of the last hit was successful
  bool IsDigitized() const;

  /// Set the id if this track
  void SetId(const int Id);
  /// Return the id of this track
  int GetId() const;
  
  /// Set the originating track Id 
  void SetOriginId(const int OriginId);
  /// Return the originating track Id 
  int GetOriginId() const;

  // edited by Andrea Frassà, 11/10/26: forced first interaction mode -->
  /// Arm the forced first interaction: the next discrete interaction of this 
  /// track will be replaced by the process with the given ID (MCSteppingAction::c_ProcessIDXYZ)
  /// Only call this for primaries - secondaries are never forced
  void ArmForcedFirstInteraction(const int ForcedProcessID);
  /// Return true if the first interaction of this track still has to be forced
  bool IsForcedFirstInteractionPending() const { return m_ForcedFirstInteractionPending; }
  /// Return the process ID which has to be forced as first interaction
  int GetForcedFirstInteractionProcessID() const { return m_ForcedFirstInteractionProcessID; }
  /// Disarm the forcing after the first interaction happened:
  /// IsForced: true if the forced process was applied, false if this was not possible (sigma_forced == 0)
  /// Weight: sigma_forced / sigma_total at the interaction point
  /// NaturalProcessID: process ID Geant4 originally selected for this interaction
  void SetForcedFirstInteractionDone(const bool IsForced, const double Weight, const int NaturalProcessID);
  /// Return true if the result of the forcing has not yet been reported to the event (CC line)
  bool HasForcedFirstInteractionReport() const { return m_ForcedFirstInteractionReportPending; }
  /// Mark the result of the forcing as reported
  void ClearForcedFirstInteractionReport() { m_ForcedFirstInteractionReportPending = false; }
  /// Return true if the forced process was really applied
  bool WasForcedFirstInteractionApplied() const { return m_ForcedFirstInteractionApplied; }
  /// Return the weight sigma_forced / sigma_total of the forced interaction
  double GetForcedFirstInteractionWeight() const { return m_ForcedFirstInteractionWeight; }
  /// Return the process ID Geant4 originally selected for the first interaction
  int GetForcedFirstInteractionNaturalProcessID() const { return m_ForcedFirstInteractionNaturalProcessID; }
  // <-- edited by Andrea Frassà, 11/10/26

  /// Not used
  void Print() const {}

  // protected methods:
protected:
  

  // protected members:
protected:


  // private members:
private:
  /// Track if a digitization happened for the LAST energy deposit
  bool m_IsDigitized;

  /// Id of the track
  int m_Id;

  /// Id of the origin track
  int m_OriginId;

  // edited by Andrea Frassà, 11/10/26: forced first interaction mode -->
  /// True if the first interaction of this track still has to be forced
  bool m_ForcedFirstInteractionPending;
  /// The process ID which has to be forced as first interaction
  int m_ForcedFirstInteractionProcessID;
  /// True if the result of the forcing still has to be written to the event
  bool m_ForcedFirstInteractionReportPending;
  /// True if the forced process has really been applied
  bool m_ForcedFirstInteractionApplied;
  /// The weight sigma_forced / sigma_total at the interaction point
  double m_ForcedFirstInteractionWeight;
  /// The process ID Geant4 originally selected for the first interaction
  int m_ForcedFirstInteractionNaturalProcessID;
  // <-- edited by Andrea Frassà, 11/10/26
};

#endif


/*
 * MCTrackInformation.hh: the end...
 ******************************************************************************/
