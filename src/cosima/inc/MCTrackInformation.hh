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
  /// Arm the forced first interaction: the first interaction of this track will be 
  /// of the process with the given ID (MCSteppingAction::c_ProcessIDXYZ)
  /// Only call this for primaries - secondaries are never forced
  void ArmForcedFirstInteraction(const int ForcedProcessID);
  /// Return true if the first interaction of this track still has to be forced
  bool IsForcedFirstInteractionPending() const { return m_ForcedFirstInteractionPending; }
  /// Return the process ID which has to be forced as first interaction
  int GetForcedFirstInteractionProcessID() const { return m_ForcedFirstInteractionProcessID; }
  /// Set the sampled first interaction point:
  /// Distance: distance along the (straight) path from the start point of the track
  /// Weight: probability that the first interaction along this path is of the forced type
  void SetForcedFirstInteractionPoint(const double Distance, const double Weight);
  /// Return the sampled distance of the first interaction from the start point of the track
  double GetForcedFirstInteractionDistance() const { return m_ForcedFirstInteractionDistance; }
  /// Disarm the forcing: 
  /// IsForced: true if the forced process was applied, false if this was not possible 
  /// (then the track continues analog, and the weight is set to zero)
  void SetForcedFirstInteractionDone(const bool IsForced);
  /// Return true if the result of the forcing has not yet been reported to the event (CC line)
  bool HasForcedFirstInteractionReport() const { return m_ForcedFirstInteractionReportPending; }
  /// Mark the result of the forcing as reported
  void ClearForcedFirstInteractionReport() { m_ForcedFirstInteractionReportPending = false; }
  /// Return true if the forced process was really applied
  bool WasForcedFirstInteractionApplied() const { return m_ForcedFirstInteractionApplied; }
  /// Return the weight: probability that the first interaction along the path is of the forced type
  double GetForcedFirstInteractionWeight() const { return m_ForcedFirstInteractionWeight; }
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
  /// The sampled distance of the first interaction from the start point of the track
  double m_ForcedFirstInteractionDistance;
  /// True if the result of the forcing still has to be written to the event
  bool m_ForcedFirstInteractionReportPending;
  /// True if the forced process has really been applied
  bool m_ForcedFirstInteractionApplied;
  /// The probability that the first interaction along the path is of the forced type
  double m_ForcedFirstInteractionWeight;
  // <-- edited by Andrea Frassà, 11/10/26
};

#endif


/*
 * MCTrackInformation.hh: the end...
 ******************************************************************************/
