#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
import re
from pathlib import Path
from typing import Dict, List


def first_heading(text: str) -> str:
    for line in text.splitlines():
        s = line.strip()
        if s.startswith("#"):
            return s.lstrip("#").strip()
    return ""


def collect_bullets(text: str, limit: int = 12) -> List[str]:
    out: List[str] = []
    for line in text.splitlines():
        s = line.strip()
        if s.startswith("- "):
            out.append(s[2:].strip())
        if len(out) >= limit:
            break
    return out


def infer_complexity(text: str) -> Dict[str, object]:
    lower = text.lower()
    score = 0
    markers = {
        "multi_file": ["multi-file", "multi file", "artifact", "cross-file"],
        "concurrency": ["concurrency", "thread", "lock", "mutex", "async"],
        "migration": ["migration", "rollback", "backfill"],
        "fullstack": ["fullstack", "frontend", "backend", "openapi", "sdk"],
        "projection": ["projection", "target", "cross-target", "environment"],
    }
    active: List[str] = []
    for k, pats in markers.items():
        if any(p in lower for p in pats):
            score += 1
            active.append(k)

    level = "low"
    if score >= 4:
        level = "high"
    elif score >= 2:
        level = "medium"

    return {"level": level, "score": score, "markers": active}


def infer_risks(text: str) -> List[str]:
    lower = text.lower()
    risks: List[str] = []
    if any(k in lower for k in ["rollback", "migration", "data_loss", "backfill"]):
        risks.append("data_integrity")
    if any(k in lower for k in ["security", "auth", "deny_by_default", "permission"]):
        risks.append("security_regression")
    if any(k in lower for k in ["p95", "latency", "throughput", "slo", "performance"]):
        risks.append("performance_regression")
    if any(k in lower for k in ["concurrency", "thread", "race", "lock"]):
        risks.append("concurrency_bug")
    if any(k in lower for k in ["compatibility", "backward", "deprecation", "rollout"]):
        risks.append("compatibility_break")
    return sorted(set(risks))


def extract_contract_fields(text: str) -> Dict[str, object]:
    lines = text.splitlines()
    artifacts: List[str] = []
    budgets: List[str] = []
    for line in lines:
        s = line.strip()
        if not s:
            continue
        if re.search(r"[A-Za-z0-9_./*-]+", s) and any(tok in s for tok in ["/", "*.", ".md", ".yaml", ".sql", ".go", ".rs", ".cpp", ".py"]):
            if s.startswith("-"):
                artifacts.append(s.lstrip("- ").strip())
        if re.search(r"\b\d+\s*(ms|s|sec|mb|gb|rps|qps|%)\b", s, re.IGNORECASE):
            budgets.append(s)

    constraints: List[str] = []
    lower = text.lower()
    for c in ["rollback_required", "deny_by_default", "staged", "auto_abort", "deterministic", "replay"]:
        if c in lower:
            constraints.append(c)

    return {
        "required_artifacts": artifacts[:24],
        "budget_clauses": budgets[:24],
        "constraints": constraints,
    }


def infer_tags(text: str) -> List[str]:
    lower = text.lower()
    tags: List[str] = []
    mapping = {
        "@network": ["api", "http", "gateway", "service"],
        "@io": ["file", "storage", "database", "migration"],
        "@concurrency": ["thread", "async", "worker", "queue", "lock"],
        "@crypto": ["crypto", "encrypt", "hash", "sign"],
        "@test": ["test", "acceptance", "validation"],
        "@ui": ["ui", "frontend", "compose", "react"],
    }
    for tag, keys in mapping.items():
        if any(k in lower for k in keys):
            tags.append(tag)
    return sorted(set(tags))


def infer_capability_requirements(text: str) -> List[Dict[str, str]]:
    lower = text.lower()
    reqs: List[Dict[str, str]] = []
    if "arduino" in lower or "embedded" in lower:
        reqs.append({"capability": "embedded_profile", "reason": "embedded target constraints detected"})
    if "fullstack" in lower or ("frontend" in lower and "backend" in lower):
        reqs.append({"capability": "fullstack_contract_propagation", "reason": "cross-layer requirements detected"})
    if "migration" in lower or "rollback" in lower:
        reqs.append({"capability": "state_migration_guardrails", "reason": "migration/rollback requirements detected"})
    if "concurrency" in lower or "thread" in lower or "lock" in lower:
        reqs.append({"capability": "concurrency_safety", "reason": "concurrency semantics detected"})
    return reqs


def build_packet(path: Path) -> Dict[str, object]:
    text = path.read_text(encoding="utf-8")
    heading = first_heading(text)
    bullets = collect_bullets(text)
    complexity = infer_complexity(text)
    risks = infer_risks(text)
    contract = extract_contract_fields(text)
    tags = infer_tags(text)
    caps = infer_capability_requirements(text)

    annotations: List[Dict[str, object]] = [
        {
            "type": "intent",
            "fields": {
                "summary": heading if heading else "spec_without_heading",
                "key_points": bullets[:8],
            },
        },
        {
            "type": "complexity",
            "fields": complexity,
        },
        {
            "type": "risk",
            "fields": {
                "risks": risks,
                "risk_level": "high" if len(risks) >= 3 else ("medium" if len(risks) >= 1 else "low"),
            },
        },
        {
            "type": "contract",
            "fields": contract,
        },
        {
            "type": "tags",
            "fields": {
                "tags": tags,
            },
        },
    ]

    for req in caps:
        annotations.append({"type": "capability_requirement", "fields": req})

    required_types = {"intent", "complexity", "risk", "contract", "tags"}
    present_types = {a["type"] for a in annotations}
    missing_types = sorted(required_types - present_types)

    coverage = {
        "required_types": sorted(required_types),
        "present_types": sorted(present_types),
        "missing_types": missing_types,
        "coverage_ratio": round((len(required_types) - len(missing_types)) / len(required_types), 4),
    }

    return {
        "source_file": str(path),
        "packet_version": "planning_semantic_packet_v1",
        "annotations": annotations,
        "coverage": coverage,
    }


def main() -> None:
    ap = argparse.ArgumentParser(description="Convert markdown plan/spec into semantic planning annotation packet.")
    ap.add_argument("--spec", required=True)
    ap.add_argument("--out", required=True)
    args = ap.parse_args()

    packet = build_packet(Path(args.spec))
    out = Path(args.out)
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(json.dumps(packet, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(packet, indent=2))


if __name__ == "__main__":
    main()
