# Sprint 232 Plan: Semantic Annotation Bridge For Planning

## Goal
Make semantic annotations first-class in spec-to-taskitem planning by bridging markdown specs into typed semantic planning packets.

## Steps
- Step 2199: Add `markdown_to_semantic_annotations` bridge tool.
- Step 2200: Emit semantic annotation packet types aligned with existing semantic annotation vocabulary (`intent`, `complexity`, `risk`, `contract`, `tags`, `capability_requirement`).
- Step 2201: Integrate bridge output into sprint taskitem pipeline summary artifacts.
- Step 2202: Add env control to enable/disable semantic planning bridge in pipeline.
- Step 2203: Produce baseline packet artifacts and update tracker.
- Step 2204: Add `Sprint232IntegrationSummary.h`.
