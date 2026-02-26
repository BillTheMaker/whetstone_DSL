#!/usr/bin/env python3
import argparse
import json
from dataclasses import dataclass, asdict
from pathlib import Path
from typing import Dict, List


@dataclass
class FallbackRecord:
    run_id: str
    input_file: str
    timestamp: str
    mode: str
    enabled: bool
    fallback_applied: bool
    skipped_reason: str
    native_task_count: int
    native_semantic_signal_count: int
    expanded_task_count: int
    gap_class: str
    likely_root_cause: str
    recommended_tools: List[str]
    recommended_taskitem_constraints: List[str]


def load_json(path: Path) -> Dict:
    with path.open("r", encoding="utf-8") as f:
        return json.load(f)


def classify_root_cause(native_task_count: int, native_semantic_signal_count: int) -> str:
    if native_task_count <= 2 and native_semantic_signal_count == 0:
        return "native_decomposition_and_semantic_signal_deficit"
    if native_task_count <= 2:
        return "native_decomposition_too_shallow"
    if native_semantic_signal_count == 0:
        return "semantic_rationale_missing"
    return "native_decomposition_sufficient"


def classify_gap(expansion: Dict) -> str:
    enabled = bool(expansion.get("enabled", False))
    fallback_applied = bool(expansion.get("fallback_applied", False))
    mode = str(expansion.get("mode", ""))
    if not enabled:
        return "semantic_expansion_disabled"
    if fallback_applied:
        return "semantic_fallback_triggered"
    if mode == "fallback_only":
        return "native_first_pass"
    return "semantic_expansion_not_applied"


def recommendations(gap_class: str, root_cause: str) -> Dict[str, List[str]]:
    if gap_class == "semantic_fallback_triggered":
        tools = [
            "whetstone_architect_intake",
            "whetstone_generate_taskitems",
            "whetstone_validate_taskitem",
            "whetstone_queue_ready",
        ]
        constraints = [
            "min_native_task_count>=5",
            "require_semantic_reason_tokens=true",
            "require_execution_specificity_score>=85",
            "require_deterministic_rollback_replay_contract=true",
        ]
        if root_cause == "semantic_rationale_missing":
            tools.append("whetstone_validate_taskitem")
            constraints.append("require_reason_contains_semantic_risk_contract_capability=true")
        return {"tools": tools, "constraints": constraints}

    if gap_class == "semantic_expansion_disabled":
        return {
            "tools": ["whetstone_generate_taskitems", "whetstone_queue_ready"],
            "constraints": ["set_WSTONE_SEMANTIC_TASK_EXPANSION=1_for_complex_specs"],
        }

    return {
        "tools": ["whetstone_validate_taskitem"],
        "constraints": ["monitor_semantic_fallback_rate<=0.35"],
    }


def load_records(runs_root: Path, include_glob: str) -> List[FallbackRecord]:
    records: List[FallbackRecord] = []
    for summary_path in sorted(runs_root.glob(f"{include_glob}/00_summary.json")):
        try:
            summary = load_json(summary_path)
        except Exception:
            continue
        expansion = summary.get("semantic_task_expansion") or {}
        if not isinstance(expansion, dict):
            continue

        native_task_count = int(expansion.get("native_task_count", 0) or 0)
        native_semantic_signal_count = int(expansion.get("native_semantic_signal_count", 0) or 0)
        gap_class = classify_gap(expansion)
        root_cause = classify_root_cause(native_task_count, native_semantic_signal_count)
        rec = recommendations(gap_class, root_cause)

        records.append(
            FallbackRecord(
                run_id=summary_path.parent.name,
                input_file=str(summary.get("input_file", "")),
                timestamp=str(summary.get("timestamp", "")),
                mode=str(expansion.get("mode", "")),
                enabled=bool(expansion.get("enabled", False)),
                fallback_applied=bool(expansion.get("fallback_applied", False)),
                skipped_reason=str(expansion.get("skipped_reason", "")),
                native_task_count=native_task_count,
                native_semantic_signal_count=native_semantic_signal_count,
                expanded_task_count=int(expansion.get("expanded_task_count", 0) or 0),
                gap_class=gap_class,
                likely_root_cause=root_cause,
                recommended_tools=rec["tools"],
                recommended_taskitem_constraints=rec["constraints"],
            )
        )

    return records


def write_json(path: Path, obj: Dict) -> None:
    with path.open("w", encoding="utf-8") as f:
        json.dump(obj, f, indent=2, sort_keys=True)
        f.write("\n")


def write_jsonl(path: Path, rows: List[Dict]) -> None:
    with path.open("w", encoding="utf-8") as f:
        for row in rows:
            f.write(json.dumps(row, sort_keys=True))
            f.write("\n")


def summarize(records: List[FallbackRecord]) -> Dict:
    total = len(records)
    fallback_count = sum(1 for r in records if r.fallback_applied)
    enabled_count = sum(1 for r in records if r.enabled)
    by_gap: Dict[str, int] = {}
    by_root: Dict[str, int] = {}
    tool_frequency: Dict[str, int] = {}

    for r in records:
        by_gap[r.gap_class] = by_gap.get(r.gap_class, 0) + 1
        by_root[r.likely_root_cause] = by_root.get(r.likely_root_cause, 0) + 1
        for t in r.recommended_tools:
            tool_frequency[t] = tool_frequency.get(t, 0) + 1

    fallback_rate = (fallback_count / enabled_count) if enabled_count else 0.0
    return {
        "record_count": total,
        "enabled_count": enabled_count,
        "fallback_applied_count": fallback_count,
        "fallback_rate": round(fallback_rate, 4),
        "gap_class_counts": by_gap,
        "root_cause_counts": by_root,
        "recommended_tool_frequency": tool_frequency,
        "status": "ok" if total > 0 else "no_data",
    }


def main() -> int:
    parser = argparse.ArgumentParser(description="Analyze semantic fallback usage and emit remediation metadata.")
    parser.add_argument("--runs-root", default="logs/taskitem_runs", help="Run root containing per-run 00_summary.json files")
    parser.add_argument("--include-glob", default="*", help="Folder glob under runs-root to include")
    parser.add_argument("--out-dir", required=True, help="Output directory for audit artifacts")
    args = parser.parse_args()

    runs_root = Path(args.runs_root)
    out_dir = Path(args.out_dir)
    out_dir.mkdir(parents=True, exist_ok=True)

    records = load_records(runs_root, args.include_glob)
    rows = [asdict(r) for r in records]
    write_jsonl(out_dir / "semantic_fallback_records.jsonl", rows)

    summary = summarize(records)
    write_json(out_dir / "semantic_fallback_summary.json", summary)

    recommendations_payload = {
        "top_recommendations": sorted(
            summary.get("recommended_tool_frequency", {}).items(),
            key=lambda kv: (-int(kv[1]), str(kv[0])),
        ),
        "records": rows,
    }
    write_json(out_dir / "semantic_fallback_tooling_recommendations.json", recommendations_payload)

    print(
        json.dumps(
            {
                "status": summary.get("status", "no_data"),
                "record_count": summary.get("record_count", 0),
                "fallback_rate": summary.get("fallback_rate", 0.0),
                "out_dir": str(out_dir),
            },
            sort_keys=True,
        )
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
