# Hybrid Pipeline Contract

This editor supports two entry paths:

1. `language-first`: operate from source text in a target language.
2. `ast-first`: operate directly on canonical AST/IR structures.

Both paths must converge on shared canonical semantics before projection to a target language.

## Contract Rules

1. Canonical-first convergence
- Every sprint result should include canonical signals (for example `AST`, `IR`, `Schema`, `Canonical`, `Semantic`, `Model`) in integration artifacts.

2. Projection neutrality
- Every sprint should show evidence that behavior is not locked to C++ only.
- Acceptable evidence includes non-C++ language/projection signals (for example `Rust`, `Go`, `Java`, `Python`, `TypeScript`, `Wasm`, SQL-family adapters).

3. C++ as backend, not source of truth
- C++ implementation artifacts are valid runtime host/projection work.
- They are not sufficient alone to satisfy hybrid pipeline quality.

## Validation Script

Use:

```bash
./tools/mcp/validate_hybrid_pipeline_contract.sh --start 50 --end 90
```

Strict enforcement (fail CI/run if checks fail):

```bash
./tools/mcp/validate_hybrid_pipeline_contract.sh \
  --start 50 --end 90 --strict --enforce-non-cpp
```

Write machine-readable report:

```bash
./tools/mcp/validate_hybrid_pipeline_contract.sh \
  --start 50 --end 90 --json-out logs/taskitem_runs/hybrid_contract_50_90.json
```

## Suggested Workflow Hook

After finishing a sprint batch:

1. Run taskitem pipeline.
2. Export LoRA capture.
3. Run hybrid contract validator.
4. Treat failures as blocking when preparing training-ready data.

