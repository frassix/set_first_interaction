# Cosima update: forced first interaction

*Andrea Frassà, 11/10/26*

## What it does

Add this to any gamma source in a `.source` file:

```
MySource.ForceFirstInteraction  PHOT     # COMP | PHOT | PAIR | RAYL | NONE
```

For every primary of that source:

- **Where** the first interaction happens is unchanged. It follows the natural total cross section, so attenuation and transmission are analog.
- **What** happens there is forced. The final state is produced by the real Geant4 model of the chosen process.
- **Everything after** is analog: the later interactions of the same photon, all secondaries (including secondary photons), and primaries from other sources.

No events are thrown away: every event is simulated once.

## Output

Each forced primary writes one comment line into its event:

```
CC ForcedFirstInteraction <PrimaryID> <Forced> <OK|FAILED> <Natural> <Weight>
```

- `Natural` is the process Geant4 originally picked at that point.
- `Weight` = σ_forced / σ_total at the interaction point. It is the probability that this first interaction would have been the forced type in an analog run.

## Assumptions / critical points

- **Gamma only.** Parsing fails for other particle types or with `PhysicsListEM None`.
- **Every discrete gamma interaction counts as "first"**, including Rayleigh and photo-nuclear.
- **Unweighted results are not the true "first interaction = X" sample.** The location follows σ_total rather than σ_X. Multiply by `Weight` to recover the analog conditional distributions and rates.
- **If σ_forced = 0 at the interaction point** (e.g. PAIR below 1.022 MeV), the natural process happens. The CC line then says `FAILED` with weight 0. A warning is printed once.
- **Gamma general process is disabled.** When the mode is used, the Geant4 "gamma general process" (`G4GammaGeneralProcess`) is switched off and the gamma processes are wrapped by the generic biasing (`G4GenericBiasingPhysics`). The physics is unchanged; it is slightly slower. Runs without the keyword are untouched.
- **Process-name list is duplicated.** `MCForcedFirstInteractionOperation::GetGammaProcessID` repeats the gamma process names from `MCSteppingAction`. Keep the two in sync.
- **Not compiled yet.** The code was checked against the Geant4 11.2.2 sources but not built against a full ROOT + Geant4 install.

## Code changes (`src/cosima`)

| File | Change |
|---|---|
| `MCForcedFirstInteractionOperator` (new) | Biasing operator attached to all volumes. It acts only on armed tracks. |
| `MCForcedFirstInteractionOperation` (new) | Swaps the final state, computes the weight, disarms the track. |
| `MCPrimaryParticleInformation` (new) | Carries the tag from the source to the primary. |
| `MCSource`, `MCParameterFile` | New keyword, plus validation. |
| `MCRun::GeneratePrimaries` | Tags the primaries of forcing sources. |
| `MCTrackingAction` | Arms primary tracks (parent ID 0) that carry the tag. |
| `MCTrackInformation` | Holds the per-track forcing state. Secondaries are never armed. |
| `MCPhysicsList` | Registers the biasing physics and disables the general process. |
| `MCDetectorConstruction::ConstructSDandField` | Attaches the operator. |
| `MCSteppingAction` | Unwraps `biasWrapper(...)` process names and writes the CC line. |

Example: `resource/examples/cosima/source/ForcedFirstInteraction.source`

## Suggested validation

1. Compare with an analog run, keeping only events whose first interaction is X. The forced run, weighted by `Weight`, should reproduce the first-interaction positions and the energy spectra.
2. The fraction of primaries that never interact must be identical with and without forcing.
