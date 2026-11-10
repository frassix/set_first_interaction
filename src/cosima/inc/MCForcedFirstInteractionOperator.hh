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
 * Geant4 generic biasing operator for the forced first interaction mode.
 * It is attached to ALL logical volumes, and only acts on armed primaries
 * (MCTrackInformation::IsForcedFirstInteractionPending()):
 *
 * 1) StartTracking: before its first step, the straight path of the primary
 *    through the whole geometry is traced. In each segment k (constant material,
 *    constant energy, since nothing happened yet) the probability that the first
 *    interaction is of the forced type X and happens in this segment is:
 *      P_k = sigma_X/sigma_tot * exp(-T_k) * (1 - exp(-sigma_tot * L_k))
 *    with T_k the optical depth (sum of sigma_tot*L) before the segment.
 *    The interaction point is sampled exactly from
 *      p(x) ~ sigma_X(x) * exp(-integral_0^x sigma_tot)
 *    by inverse transform (no rejection): first the segment with P_k/P, then the
 *    position inside the segment from the truncated exponential.
 *    The weight P = sum P_k is the probability that the first interaction of
 *    this photon along its path is of type X.
 * 2) During the flight to the sampled point: occurrence biasing
 *    (MCForcedFirstInteractionFlight): the forced process interacts exactly at
 *    the sampled point, all others never.
 * 3) At the sampled point: final state biasing (MCForcedFirstInteractionOperation):
 *    the forced process generates its real final state, and the track is disarmed.
 * Afterwards everything is analog.
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
#include "MCForcedFirstInteractionFlight.hh"

// Standard lib:
#include <map>
#include <vector>
using namespace std;

// Forward declarations:
class G4Navigator;
class G4VProcess;
class G4MaterialCutsCouple;


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

  /// Called by Geant4 at the start of each track: samples the first interaction point of armed primaries
  virtual void StartTracking(const G4Track* Track);

  // protected methods:
protected:
  /// No non-physics biasing
  virtual G4VBiasingOperation* ProposeNonPhysicsBiasingOperation(const G4Track* Track,
                                                                 const G4BiasingProcessInterface* CallingProcess);
  /// Occurrence biasing for armed tracks: the forced process interacts at the sampled point, all others never
  virtual G4VBiasingOperation* ProposeOccurenceBiasingOperation(const G4Track* Track,
                                                                const G4BiasingProcessInterface* CallingProcess);
  /// Final state biasing for armed tracks: the real final state of the forced process, then disarm
  virtual G4VBiasingOperation* ProposeFinalStateBiasingOperation(const G4Track* Track,
                                                                 const G4BiasingProcessInterface* CallingProcess);

  /// Trace the straight path of the track through the geometry and sample the
  /// first interaction point of the forced process. Returns false if the forced
  /// process is impossible along the whole path
  bool SampleFirstInteraction(const G4Track* Track, const int ForcedID, double& Distance, double& Weight);

  /// Return the macroscopic cross section (1/length) of a gamma process in the given material
  double GetCrossSection(G4VProcess* Process, const double Energy, const G4MaterialCutsCouple* Couple);

  /// Return true if the track is armed and its first interaction point has been sampled
  bool IsArmed(const G4Track* Track) const;

  // protected members:
protected:


  // private members:
private:
  /// The final state operation (one for all processes)
  MCForcedFirstInteractionOperation* m_Operation;
  /// The occurrence operations (one per wrapped process)
  map<const G4BiasingProcessInterface*, MCForcedFirstInteractionFlight*> m_Flights;

  /// Our own navigator to trace the path (never use the tracking navigator for this!)
  G4Navigator* m_Navigator;
  /// The (wrapped = real) physics processes of the gamma
  vector<G4VProcess*> m_GammaProcesses;

  /// Counter of primaries for which the forced process is impossible along the path - for a warning only
  long m_NImpossible;
};

#endif


/*
 * MCForcedFirstInteractionOperator.hh: the end...
 ******************************************************************************/
