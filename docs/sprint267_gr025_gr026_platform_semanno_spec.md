# Sprint 267 — GR-025/026: Platform SemAnno Annotation Family

## Goals

Add a `@platform.*` annotation family to the Whetstone SemAnno system so the
projector can distinguish pure logic nodes from platform-specific I/O boundaries.
This unblocks cross-platform projection (Python→Kotlin, Python→Swift, etc.).

Two sub-goals, sequenced:

**GR-025**: Add annotation classes, emitter, parser, visitor dispatch.
**GR-026**: Add a platform profile registry that maps `@platform.*` tags to
target-SDK API substitutions. Provide two built-in profiles:
`python-stdlib` and `android-kotlin`.

## Constraints

- New annotation classes go in `editor/src/ast/Annotation.h` — no new files for
  annotation types.
- Emitter cases added to `editor/src/semanno/SemannoEmitterBody.h`.
- Parser cases added to `editor/src/semanno/SemannoParserSection.h`.
- Visitor pure-virtuals added to `editor/src/ast/AnnotationVisitors.h`.
- Default implementations added to `editor/src/SemannoAnnotationImpl.h` (CRTP mixin).
- Dispatch cases added to `editor/src/ast/ProjectionGenerator.h`.
- Platform profile registry: new file `editor/src/PlatformProfileRegistry.h`,
  ≤ 300 lines, JSON-driven, ships two built-in profiles.
- All headers must stay ≤ 600 lines. `Annotation.h` is currently ~550 lines;
  add new classes at the end in a new section.
- No changes to CMakeLists needed (header-only additions).
- TDD: write step tests (step1863–step1867) before implementing.

## Acceptance Criteria

AC-1: Five new annotation classes exist and construct correctly:
  `PureLogicAnnotation`, `PlatformStorageAnnotation`, `PlatformCryptoAnnotation`,
  `PlatformNetworkAnnotation`, `PlatformOsAnnotation`.

AC-2: SemannoEmitter emits each as `@semanno:pure.logic()`,
  `@semanno:platform.storage(adapter="sqlite3")`, etc.

AC-3: SemannoParser parses those strings back to the correct annotation class
  with the `adapter` field populated.

AC-4: All five visitor methods are dispatched through ProjectionGenerator and
  handled by SemannoAnnotationImpl (emitting the semanno comment via the
  language-specific comment prefix).

AC-5 (GR-026): `PlatformProfileRegistry` loads built-in profiles and resolves
  a `@platform.storage` annotation on `python-stdlib` to `"sqlite3"` and on
  `android-kotlin` to `"androidx.room.RoomDatabase"`.

AC-6: All existing step tests continue to pass (no regressions).
