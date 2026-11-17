# Cosima updates

*Andrea Frassà*

Two optional modes, both off unless their keyword is in the `.source` file.

**Versions:** MEGAlib's setup requires ROOT 6.40 and Geant4 10.2 (p03), which `setup.sh` downloads and builds when no `--root`/`--geant` is given (see `config/Allowed*Versions.txt`). The new code compiles against Geant4 10.2 and also 11.x.

The two modes:

1. [Forced first interaction](#1-forced-first-interaction): `<Source>.ForceFirstInteraction`
2. [Energy window](#2-energy-window): `StoreEnergyWindow`

They can be used together.

---

# 1. Forced first interaction

*11/10/26*

## What it does

Add this to any gamma source in a `.source` file:

```
MySource.ForceFirstInteraction  PHOT     # COMP | PHOT | PAIR | RAYL | NONE
```

For every primary of that source, the **first interaction is of the selected type, at a physically correct position**. Everything after it is analog: later interactions of the same photon, all secondaries (including photons), and primaries of other sources.

No events are discarded: every primary does exactly one forced interaction, unless the forced process is impossible along its whole path.

## How the position is chosen

Before its first interaction a photon flies straight and keeps its energy. So its path through the geometry is a list of segments (one per volume), each with constant cross sections. The real position of a first interaction of type X follows:

```
p(x) ∝ σ_X(x) · exp(−∫₀ˣ σ_tot)
         ↑ X here     ↑ reach x without any interaction
```

At the start of each forced primary:

1. The straight path is traced through the geometry with its own `G4Navigator`. In each segment k this gives σ_tot, σ_X and the probability that the first interaction is X there:
   `P_k = σ_X/σ_tot · e^(−T_k) · (1 − e^(−σ_tot·L_k))`, where T_k is the optical depth before the segment.
2. The point is chosen by **inverse transform sampling, with no rejection**:
   - pick segment k with probability P_k / ΣP;
   - pick the depth inside it from the truncated exponential, `d = −ln(1 − V·(1 − e^(−σ_tot·L)))/σ_tot`, where V is uniform in [0, 1).
3. The photon flies to that point without interacting. Only X can happen there, and it produces its real Geant4 final state. After that the track is released.

The cross sections come from the same Geant4 tables used during tracking. A toy check of the sampling against a brute-force analog simulation agreed to statistical precision.

## Output

Each forced primary writes one comment line into its event:

```
CC ForcedFirstInteraction <PrimaryID> <Forced> <OK|FAILED> <Weight>
```

`Weight = ΣP_k`: the probability that this photon's first interaction would have been of type X in an analog run.

## Assumptions / critical points

- **Gamma only.** Parsing fails for other particles or with `PhysicsListEM None`.
- **Every gamma physics process counts toward the total cross section** (including Rayleigh and photo-nuclear), so they all compete with X on the way to the point.
- **When the weight matters:**
  - Shapes need no weight when all forced photons share the same path, e.g. a pencil beam.
  - Absolute rates always need it.
  - With different paths (wide beams, far-field sources, many directions) the weight differs per photon, so weight the events.
  - A photon that misses the detector is still forced somewhere along its path, e.g. in the world material or air, with a tiny weight. With extended beams, always use the weight.
- **If X is impossible along the whole path** (e.g. PAIR below 1.022 MeV), the primary is simulated analog and marked `FAILED` with weight 0. A warning is printed once.
- **Physics setup:**
  - Gamma processes are wrapped by `G4GenericBiasingPhysics`. With Geant4 ≥ 10.5, `G4GammaGeneralProcess` is also switched off (it doesn't exist in 10.2). This only happens when the keyword is used; the physics is unchanged.
  - Geant4 track weights are kept at 1. Our weight is only in the CC line.
- **Process-name list is duplicated.** `MCForcedFirstInteractionOperation::GetGammaProcessID` repeats the gamma process names from `MCSteppingAction`. Keep them in sync.
- **Not compiled yet.** The code was checked against the Geant4 10.2.3 and 11.2.2 sources, but not built against a full ROOT + Geant4 install.

## Code changes (`src/cosima`)

| File | Change |
|---|---|
| `MCForcedFirstInteractionOperator` (new) | Attached to all volumes. Traces the path, samples the point, proposes the operations below for armed primaries only. |
| `MCForcedFirstInteractionFlight` + `MCForcedFirstInteractionLaw` (new) | During the flight, X interacts exactly at the point and all other processes never do. |
| `MCForcedFirstInteractionOperation` (new) | At the point: real final state of X, then disarms the track. |
| `MCPrimaryParticleInformation` (new) | Carries the tag from the source to the primary. |
| `MCSource`, `MCParameterFile` | Keyword and validation. |
| `MCRun::GeneratePrimaries` | Tags the primaries of forcing sources. |
| `MCTrackingAction` | Arms primary tracks (parent ID 0) that carry the tag. |
| `MCTrackInformation` | Per-track forcing state. Secondaries are never armed. |
| `MCPhysicsList` | Biasing physics; general gamma process off. |
| `MCDetectorConstruction::ConstructSDandField` | Attaches the operator. |
| `MCSteppingAction` | Unwraps `biasWrapper(...)` process names and writes the CC line. |

Example: `resource/examples/cosima/source/ForcedFirstInteraction.source`

## Suggested validation

Run an analog simulation and keep only events whose first interaction is X. With a pencil beam, the forced run should reproduce, without weights:
- the first-interaction positions;
- the energy spectra;
- the fraction of events with first interaction X, compared with the mean weight.

---

# 2. Energy window

*11/16/26*

## What it does

One global line in the `.source` file (it applies to all runs and sources):

```
StoreEnergyWindow  <Emin>  <Emax>  [<Detector> ...]     # keV
```

- An event is **saved only if its deposit in the active detector volumes lies between Emin and Emax.** The deposit is the true energy, before detector resolution.
- **Which deposits count:**
  - by default, all detectors that can trigger; detectors used only as vetoes (e.g. anti-coincidence shields) don't count;
  - with detector names (as in the geometry file), only those detectors count;
  - guard rings never count.
- **It adds to the normal trigger.** An event must trigger and be inside the window.
- **`Triggers N` counts saved events only**, because the window is checked before the event is counted.
- **The `TS` line still holds all simulated events**, so efficiency = saved / TS.

It combines the behaviour of the existing `StoreMinimumEnergy` (lower cut) and `StoreMaximumEnergyLoss` (early abort), and adds an upper cut.

## Early abort: why it is faster than "simulate everything and filter"

An event is stopped as soon as it can no longer end up in the window:

- **Above:** deposits only grow. Once the counted deposit passes Emax, the event is aborted immediately.
- **Below:** the deposit can never exceed
  `budget − lost`, where
  - `budget` = kinetic energy of the primaries, plus 1022 keV for each primary positron;
  - `lost` = energy that escaped the world, was killed in a black absorber, or went into passive material, guard rings or detectors that don't count.

  Once `budget − lost < Emin`, the event is aborted.

  Example: a Compton-scattered photon leaves the detector. Geant4 tracks the photon before its recoil electron, so the escape is known first, and the expensive electron tracking is skipped.

**When the lower abort is switched off.** The budget is a true upper limit only while every particle in the event is a gamma, e±, or neutrino. As soon as any other particle exists (neutron, proton, ion, muon, …), energy can be released (capture, decay, nuclear reactions), so the lower abort is switched off for the rest of that event. This happens:
- from the start, for such primaries;
- at the moment of creation, for such secondaries (e.g. photo-nuclear reactions).

The upper abort is always exact and stays on.

To switch both aborts off (e.g. to validate that saved results are identical with and without them):

```
StoreEnergyWindowEarlyAbort  false
```

At the end of each run cosima prints how many events were stored, rejected, and aborted above or below.

## Critical points

- **Detector resolution is not included.** Cosima stores true energies; resolution is applied when revan or sivan reads the `.sim` file. Set the window about 3σ of your resolution wider on each side than your analysis window, then apply the exact cut in revan or mimrec.
- **Normalisation:** saved / TS is the probability for one simulated primary to give a saved event. Together with the forced first interaction mode, also multiply by the CC weight.
- **Detector names:** a name not found in the geometry stops cosima with an error.
- **Not compiled yet.** Checked by reading the code; not built against a full ROOT + Geant4 install.

## Code changes (`src/cosima`)

| File | Change |
|---|---|
| `MCParameterFile` | `StoreEnergyWindow` and `StoreEnergyWindowEarlyAbort` keywords. |
| `MCEventAction` | Running deposit, budget and loss per event; early aborts; window check before the event is counted; run statistics. |
| `MCSD` + all 6 sensitive detectors | `ReportDeposit()`: each deposit stored in a hit is reported with its detector and guard-ring flag. |
| `MCSteppingAction` | Checks new secondaries for particles that can release energy. |
| `MCRun::GeneratePrimaries` | Adds each primary to the energy budget. |
| `MCDetectorConstruction` | Checks that the named detectors exist. |

Example: `resource/examples/cosima/source/EnergyWindow.source`
