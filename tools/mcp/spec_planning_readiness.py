#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
import re
from dataclasses import dataclass
from pathlib import Path
from typing import Dict, List


SECTION_PATTERNS = {
    "goals": [r"^#*\s*goals?\b", r"^#*\s*objective\b", r"^#*\s*problem\b"],
    "constraints": [r"^#*\s*constraints?\b", r"^#*\s*non-functional\b", r"^#*\s*requirements\b"],
    "acceptance": [r"^#*\s*acceptance\b", r"^#*\s*done criteria\b", r"^#*\s*definition of done\b"],
    "environment": [r"^#*\s*environment\b", r"^#*\s*target(s)?\b", r"^#*\s*platform\b"],
    "rollout": [r"^#*\s*rollout\b", r"^#*\s*migration\b", r"^#*\s*release\b"],
    "observability": [r"^#*\s*observability\b", r"^#*\s*telemetry\b", r"^#*\s*monitoring\b"],
    "security": [r"^#*\s*security\b", r"^#*\s*auth\b", r"^#*\s*threat\b"],
    "performance": [r"^#*\s*performance\b", r"^#*\s*slo\b", r"^#*\s*latency\b"],
}

KEYWORD_PATTERNS = {
    "numeric_budget": [r"\b\d+\s*(ms|s|sec|mb|gb|rps|qps|%)\b", r"\bp\d{2}\b", r"\berror budget\b"],
    "determinism": [r"\bdetermin(istic|ism)\b", r"\breplay\b", r"\brollback\b", r"\bidempotent\b"],
    "multifile": [r"\bmulti[- ]file\b", r"\bartifact\b", r"\bopenapi\b", r"\bmigration\b"],
    "tests": [r"\btest\b", r"\bassert\b", r"\bscenario\b", r"\bacceptance criteria\b"],
}


@dataclass
class Signal:
    key: str
    present: bool
    evidence: List[str]


def find_section_signals(lines: List[str]) -> List[Signal]:
    out: List[Signal] = []
    for key, pats in SECTION_PATTERNS.items():
        evidence: List[str] = []
        for line in lines:
            for pat in pats:
                if re.search(pat, line.strip(), flags=re.IGNORECASE):
                    evidence.append(line.strip())
                    break
            if evidence:
                break
        out.append(Signal(key=key, present=bool(evidence), evidence=evidence))
    return out


def find_keyword_signals(text: str) -> List[Signal]:
    out: List[Signal] = []
    for key, pats in KEYWORD_PATTERNS.items():
        matches: List[str] = []
        for pat in pats:
            m = re.search(pat, text, flags=re.IGNORECASE)
            if m:
                matches.append(m.group(0))
        out.append(Signal(key=key, present=bool(matches), evidence=matches[:5]))
    return out


def score(section_signals: List[Signal], keyword_signals: List[Signal]) -> Dict[str, int]:
    section_count = sum(1 for s in section_signals if s.present)
    keyword_count = sum(1 for s in keyword_signals if s.present)
    section_score = int(round((section_count / max(1, len(section_signals))) * 70))
    keyword_score = int(round((keyword_count / max(1, len(keyword_signals))) * 30))
    total = min(100, section_score + keyword_score)
    return {
        "section_score": section_score,
        "keyword_score": keyword_score,
        "total": total,
    }


def build_missing_actions(section_signals: List[Signal], keyword_signals: List[Signal]) -> List[Dict[str, str]]:
    missing: List[Dict[str, str]] = []
    section_actions = {
        "goals": "Add explicit product/behavior goals and out-of-scope boundaries.",
        "constraints": "Add hard constraints: language/runtime limits, prohibited APIs, determinism and safety policies.",
        "acceptance": "Add acceptance criteria with pass/fail checks and expected artifacts.",
        "environment": "Add environment targets/projections and per-target constraints.",
        "rollout": "Add rollout/migration/rollback choreography and compatibility window.",
        "observability": "Add logging/metrics/tracing expectations and alert thresholds.",
        "security": "Add threat/security constraints, default-deny policy, and authn/authz expectations.",
        "performance": "Add quantitative budgets (latency, memory, throughput) and regression gates.",
    }
    for s in section_signals:
        if not s.present:
            missing.append({"type": "section", "key": s.key, "action": section_actions[s.key]})

    keyword_actions = {
        "numeric_budget": "Add explicit numeric budgets (p95 latency, memory caps, error budgets).",
        "determinism": "Add determinism/replay/rollback/idempotency requirements.",
        "multifile": "Add required multi-file artifact closure list and dependency graph hints.",
        "tests": "Add concrete acceptance-test scenarios and expected outcomes.",
    }
    for s in keyword_signals:
        if not s.present:
            missing.append({"type": "keyword", "key": s.key, "action": keyword_actions[s.key]})

    return missing


def template_for_missing(missing: List[Dict[str, str]]) -> str:
    keys = {m["key"] for m in missing}
    blocks: List[str] = []
    if "goals" in keys:
        blocks.append("## Goals\n- Primary objective:\n- Secondary objective:\n- Explicitly out of scope:")
    if "constraints" in keys:
        blocks.append("## Constraints\n- Language/runtime constraints:\n- Forbidden dependencies/APIs:\n- Determinism/replay requirements:")
    if "environment" in keys:
        blocks.append("## Environment / Projection Targets\n- Target A:\n- Target B:\n- Cross-target invariants:")
    if "performance" in keys or "numeric_budget" in keys:
        blocks.append("## Performance Budgets\n- p95 latency: <N ms>\n- Max memory: <N MB>\n- Throughput floor: <N rps>")
    if "security" in keys:
        blocks.append("## Security\n- Default deny policy:\n- AuthN/AuthZ model:\n- Sensitive data handling:")
    if "observability" in keys:
        blocks.append("## Observability\n- Required metrics:\n- Required logs/traces:\n- Alert thresholds:")
    if "rollout" in keys:
        blocks.append("## Rollout / Migration / Rollback\n- Compatibility window:\n- Migration strategy:\n- Rollback trigger + procedure:")
    if "acceptance" in keys or "tests" in keys:
        blocks.append("## Acceptance Criteria\n- Scenario 1: given/when/then\n- Scenario 2: given/when/then\n- Required artifacts and pass conditions:")
    if "multifile" in keys:
        blocks.append("## Required Artifacts\n- file/path/a\n- file/path/b\n- file/path/c")
    return "\n\n".join(blocks)


def assess(path: Path) -> Dict[str, object]:
    text = path.read_text(encoding="utf-8")
    lines = text.splitlines()
    section_signals = find_section_signals(lines)
    keyword_signals = find_keyword_signals(text)
    scores = score(section_signals, keyword_signals)
    missing = build_missing_actions(section_signals, keyword_signals)

    verdict = "execution_ready" if scores["total"] >= 85 and not missing else "needs_spec_hardening"
    return {
        "file": str(path),
        "verdict": verdict,
        "scores": scores,
        "sections": [{"key": s.key, "present": s.present, "evidence": s.evidence} for s in section_signals],
        "keywords": [{"key": s.key, "present": s.present, "evidence": s.evidence} for s in keyword_signals],
        "missing_actions": missing,
        "recommended_template": template_for_missing(missing),
    }


def main() -> None:
    ap = argparse.ArgumentParser(description="Assess markdown spec execution-readiness and suggest missing constraints.")
    ap.add_argument("--spec", required=True, help="Path to markdown spec")
    ap.add_argument("--out", required=True, help="Output JSON file")
    args = ap.parse_args()

    result = assess(Path(args.spec))
    out = Path(args.out)
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(result, indent=2))


if __name__ == "__main__":
    main()
