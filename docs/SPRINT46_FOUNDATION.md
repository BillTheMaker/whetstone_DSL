# Sprint 46 Foundation

This document captures the baseline artifacts introduced for Sprint 46.

## Components

- Semantic IR schema: `editor/src/SemanticCoreIR.h`
- Support tiers and gate mapping: `editor/src/LanguageSupportTier.h`
- Capability matrix model: `editor/src/LanguageCapabilityMatrix.h`
- Migration contract checks: `editor/src/MigrationAcceptanceContract.h`

## Example Artifacts

- Semantic IR example: `docs/examples/sprint46_semantic_core_example.json`

## Notes

- All schema models are deterministic JSON serializable.
- Unknown tags/fields are preserved where possible for forward compatibility.
