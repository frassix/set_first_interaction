/*
 * MCEventAction.hh
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
 * The event action implementation
 *
 */

#ifndef ___MCEventAction___
#define ___MCEventAction___

// Geant4:
#include "G4UserEventAction.hh"
#include "globals.hh"
#include "G4ParticleDefinition.hh" // edited by Andrea Frassà, 11/16/26

// Standard lib:
#include <fstream>
#include <map>
#include <string>
#include <vector>
using namespace std;

// Cosima:
#include "MC2DStripHit.hh"
#include "MCScintillatorHit.hh"
#include "MCCalorBarHit.hh"
#include "MCDriftChamberHit.hh"
#include "MCAngerCameraHit.hh"
#include "MCVoxel3DHit.hh"
#include "MCParameterFile.hh"

// MEGAlib:
#include "MFile.h"
#include "MTimer.h"
#include "MTransceiverTcpIp.h"
#include "MSimEvent.h"
#include "MDTriggerUnit.h"


/******************************************************************************/

class MCEventAction : public G4UserEventAction
{
  // public interface:
public:
  /// Constructor taking the run parameters as argument
  MCEventAction(MCParameterFile& RunParameters, const bool Zip, 
                const long Seed, const bool m_NoTimeOut);
  /// Default destructor
  virtual ~MCEventAction();

  /// Set the collection ID of the hits
  void SetCollectionName(G4String Name, int Type);

  /// Set the function to which we relegate the events
  void SetRelegator(void (Relegator)(MSimEvent*));
  
  /// Prepare for next run: Open the output file and reset
  bool NextRun();

  /// Abort the current event
  void AbortEvent();
  /// Test if the evnt has been aborted
  bool IsAborted() const { return m_IsAborted; }

  /// Things to do before the start of a new event (reset last event data) 
  void BeginOfEventAction(const G4Event* Event);
  /// Test for trigger and store all events
  void EndOfEventAction(const G4Event* Event);

  /// Remove the last events data
  void Reset();
  /// Add an IA to this event:
  void AddIA(G4String ProcessID, 
             int InteractionID,
             int OriginID,
             int DetectorID,
             double Time,
             G4ThreeVector Position,
             int MotherParticleID,
             G4ThreeVector NewMotherMomentumDirection,
             G4ThreeVector NewMotherPolarization,
             double NewMotherKineticEnergy,
             int SecondaryParticleID,
             G4ThreeVector SecondaryMomentumDirection,
             G4ThreeVector SecondaryPolarization,
             double SecondaryEnergy);
  ///! Set the Galactic pointing 
  void SetGalacticPointing(double XLat, double XLong, double ZLat, double ZLong);
  ///! Set the Detector orientation 
  void SetDetectorPointing(double XTheta, double XPhi, double ZTheta, double ZPhi);
  /// Add a passive material output information
  void AddDepositPassiveMaterial(double Energy, string MaterialName);
  /// Add a comment
  void AddComment(string Comment);
  /// Add an energy loss
  void AddEnergyLoss(double Energy);

  // edited by Andrea Frassà, 11/16/26: energy window mode -->
  /// Return true if only events with a deposit inside the energy window are stored
  bool UsesEnergyWindow() const { return m_UseEnergyWindow; }
  /// Add a primary particle of this event to the energy budget (called at generation)
  void AddEnergyWindowPrimary(const G4ParticleDefinition* Particle, const double KineticEnergy);
  /// Check a newly created secondary: if it can release energy (e.g. nuclear processes)
  /// the energy budget is no longer an upper limit and the early abort "from below" is switched off
  void CheckEnergyWindowSecondary(const G4ParticleDefinition* Particle);
  /// Add a deposit which has been stored in a hit of a sensitive detector
  /// (called by the sensitive detectors, see MCSD::ReportDeposit)
  void AddSensitiveDeposit(const G4String& SDName, const double Energy, const bool IsGuardring, const bool IsNeverTriggering);
  // <-- edited by Andrea Frassà, 11/16/26
  
  /// Interrupts the executon at the end of the next event
  void Interrupt() { m_Interrupt = true; }
  /// Return if there should be an interrupt
  bool GetInterrupt() const { return m_Interrupt; }
  
  // protected methods:
protected:
  /// Write the file header 
  bool WriteFileHeader(double ObservationStartTime);

  // edited by Andrea Frassà, 11/16/26: energy window mode -->
  /// Add energy which can no longer contribute to the energy window deposit
  /// (escaped, passive material, guard rings, not counted detectors)
  void AddEnergyWindowLoss(const double Energy);
  /// Return true if the particle cannot release energy (gamma, e-, e+, neutrinos)
  bool IsEnergyWindowSafeParticle(const G4ParticleDefinition* Particle) const;
  /// Return true if deposits in this sensitive detector count for the energy window
  bool IsEnergyWindowDetector(const G4String& SDName, const bool IsNeverTriggering) const;
  // <-- edited by Andrea Frassà, 11/16/26

  /// Save the event to file (only saves the event if we really want to)
  bool SaveEventToFile(MSimEvent* Event);
  /// Transmit event via TCP/IP (only transmit it if we have an open transceiver)
  bool TransmitEvent(MSimEvent* Event);

  // protected members:
protected:


  // private members:
private:
  /// Class containing all run parameter 
  MCParameterFile& m_RunParameters;

  /// Invalid collection ID 
  static const int c_InvalidCollID;

  /// Id of the tracker collection
  vector<G4String> m_2DStripCollNames;
  /// Id of the calorimeter collection
  vector<G4String> m_CalorimeterCollNames;
  /// Id of the 3Dstrip collection
  vector<G4String> m_3DStripCollNames;
  /// Id of the Voxel3D collection
  vector<G4String> m_Voxel3DCollNames;
  /// Id of the szintillator collection
  vector<G4String> m_ScintillatorCollNames;
  /// Id of the drift chamber collection
  vector<G4String> m_DriftChamberCollNames;
  /// Id of the Anger camera collection
  vector<G4String> m_AngerCameraCollNames;

  /// True if the evnts should be saved to file
  bool m_SaveEvents;
  /// Stream to the output file
  MFile m_OutFile;
  /// Name of the current iutput file
  string m_OutFileName;
  /// Id of the output file, if the maximum file size has been exceeded
  int m_FileNumber;
  /// Incarnation Id of the output file, if the base file name already exists
  int m_Incarnation;

  /// True if the evnts should be saved to file
  bool m_TransmitEvents;
  /// TCP/IP: The name of the output host
  string m_Host;
  /// TCP/IP: The port on the host
  //int m_Port;
  /// The transceiver
  MTransceiverTcpIp m_Transceiver;
  
  /// True if the events should be handed over to a function
  bool m_RelegateEvents;
  /// The function pointer for relegation
  void (*m_Relegator)(MSimEvent*);
  
  /// Seed of the random number generator at the beginning of the event
  long m_Seed;

  /// Parallel ID: Additional ID assigned from outside to uniquely identify sim files from one batch of parallel started simulations 
  int m_ParallelID;
  /// Assigned fixed incarnation ID of the output file(s)
  //int m_IncarnationID;

  /// ID of the current event
  long m_ID;
  
  
  /// The temporary store of the simulated event
  MSimEvent* m_Event;

  /// Determines if we store the data in binary format
  bool m_StoreBinary;
  /// Determines how much detail shall be stored in the simulations file
  unsigned int m_StoreSimulationInfo;
  /// Determines in which version the output file shall be stored
  unsigned int m_StoreSimulationInfoVersion;
  /// True if hits as posiiton/enrgy and not as bars/strips should be written to the file  
  G4bool m_StoreCalibrated;
  /// True if each hit in its own event
  G4bool m_StoreOneHitPerEvent;
  /// The minimum energy an event must have before it is stored
  double m_StoreMinimumEnergy;
  /// The maximum lost energy allowed
  double m_StoreMaximumEnergyLoss;
  
  /// Precision (number of digits after '.') for scientific storage
  int m_StoreScientificPrecision;

  /// The pre-trigger mode (either store everything, only events with hits, or fully pretriggered events)
  int m_PreTriggerMode;
  
  /// A timer stopping the real event action time
  MTimer m_Timer;
  /// The total time spent during all event actions:
  double m_TotalTime;
  /// Flag indicating if the timer has started
  bool m_TimerStarted;
  /// Flag indicating that we should not consider time outs
  bool m_NoTimeOut;
  /// A time out - if no events are stored after this time stop the simulation
  double m_TimeOut;
  /// Flag indicating that we can ignore the time out since events have been stored
  bool m_IgnoreTimeOut;

  /// Map of passive material name and deposited energy
  map<string, double> m_PassiveMaterialMap; 

  /// The lost energy (escapes or passive material)
  double m_LostEnergy;
  /// True if this event has been aborted
  bool m_IsAborted;
  
  /// True, if the run should be terminated at the end of this event
  bool m_Interrupt;
  /// True, if the sim file should be zip after its generation
  bool m_Zip;

  /// Store the trigger unit for fast access
  MDTriggerUnit* m_TriggerUnit;

  // edited by Andrea Frassà, 11/16/26: energy window mode -->
  /// True if only events with a deposit inside the energy window are stored
  bool m_UseEnergyWindow;
  /// Lower edge of the energy window
  double m_EnergyWindowMinimum;
  /// Upper edge of the energy window
  double m_EnergyWindowMaximum;
  /// Names of the sensitive detectors (detector name + "SD") which count - empty: all triggering detectors
  vector<G4String> m_EnergyWindowDetectors;
  /// True if events which can no longer end up in the window are aborted early
  bool m_EnergyWindowEarlyAbort;

  /// Per event: deposit in the counted detectors (without guard rings)
  double m_EnergyWindowDeposit;
  /// Per event: energy budget: kinetic energy of all primaries (+ 2 m_e c^2 for each primary positron)
  double m_EnergyWindowBudget;
  /// Per event: energy which can no longer reach the counted detectors
  double m_EnergyWindowLoss;
  /// Per event: true as long as the budget is a strict upper limit for the deposit
  /// (false as soon as a particle exists which can release energy, e.g. neutrons or ions)
  bool m_EnergyWindowBudgetValid;

  /// Statistics: events aborted since the deposit exceeded the upper edge
  long m_EnergyWindowNAbortedAbove;
  /// Statistics: events aborted since the deposit can no longer reach the lower edge
  long m_EnergyWindowNAbortedBelow;
  /// Statistics: events which ended outside the window
  long m_EnergyWindowNRejected;
  /// Statistics: events which ended inside the window
  long m_EnergyWindowNAccepted;
  // <-- edited by Andrea Frassà, 11/16/26

};

#endif

   
/*
 * MCEventAction.hh: the end...
 ******************************************************************************/
 
