#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Dict, List


def find_annotation(packet: Dict[str, object], ann_type: str) -> Dict[str, object]:
    for ann in packet.get("annotations", []):
        if isinstance(ann, dict) and ann.get("type") == ann_type:
            return ann
    return {}


def list_str(values: List[object]) -> List[str]:
    out: List[str] = []
    for v in values:
        if isinstance(v, str) and v.strip():
            out.append(v.strip())
    return out


def build_addendum(packet: Dict[str, object]) -> str:
    intent = find_annotation(packet, "intent").get("fields", {}) or {}
    complexity = find_annotation(packet, "complexity").get("fields", {}) or {}
    risk = find_annotation(packet, "risk").get("fields", {}) or {}
    contract = find_annotation(packet, "contract").get("fields", {}) or {}
    tags = find_annotation(packet, "tags").get("fields", {}) or {}

    key_points = list_str(intent.get("key_points", []))
    markers = list_str(complexity.get("markers", []))
    risks = list_str(risk.get("risks", []))
    artifacts = list_str(contract.get("required_artifacts", []))
    constraints = list_str(contract.get("constraints", []))
    budgets = list_str(contract.get("budget_clauses", []))
    sem_tags = list_str(tags.get("tags", []))

    lines: List[str] = []
    lines.append("## Semantic Planning Packet (Auto-Extracted)")
    lines.append("")
    lines.append("This section is generated from semantic annotation extraction and should guide deterministic task decomposition.")
    lines.append("")

    summary = intent.get("summary", "")
    if isinstance(summary, str) and summary:
        lines.append(f"- intent.summary: {summary}")
    level = complexity.get("level", "")
    score = complexity.get("score", "")
    if level:
        lines.append(f"- complexity.level: {level}")
    if isinstance(score, int):
        lines.append(f"- complexity.score: {score}")
    if markers:
        lines.append(f"- complexity.markers: {', '.join(markers)}")
    if risks:
        lines.append(f"- risk.categories: {', '.join(risks)}")
    if sem_tags:
        lines.append(f"- semantic.tags: {', '.join(sem_tags)}")

    if key_points:
        lines.append("")
        lines.append("### Semantic Intent Points")
        for p in key_points[:10]:
            lines.append(f"- {p}")

    if constraints or budgets:
        lines.append("")
        lines.append("### Semantic Constraints")
        for c in constraints[:12]:
            lines.append(f"- constraint: {c}")
        for b in budgets[:12]:
            lines.append(f"- budget: {b}")

    if artifacts:
        lines.append("")
        lines.append("### Semantic Required Artifacts")
        for a in artifacts[:20]:
            lines.append(f"- {a}")

    # Capability requirements are separate annotation entries.
    cap_lines: List[str] = []
    for ann in packet.get("annotations", []):
        if isinstance(ann, dict) and ann.get("type") == "capability_requirement":
            fields = ann.get("fields", {}) or {}
            cap = fields.get("capability")
            reason = fields.get("reason")
            if isinstance(cap, str) and cap:
                if isinstance(reason, str) and reason:
                    cap_lines.append(f"- {cap}: {reason}")
                else:
                    cap_lines.append(f"- {cap}")
    if cap_lines:
        lines.append("")
        lines.append("### Semantic Capability Requirements")
        lines.extend(cap_lines)

    return "\n".join(lines).strip() + "\n"


def main() -> None:
    ap = argparse.ArgumentParser(description="Append semantic packet guidance section to markdown spec.")
    ap.add_argument("--spec", required=True)
    ap.add_argument("--packet", required=True)
    ap.add_argument("--out", required=True)
    args = ap.parse_args()

    spec_path = Path(args.spec)
    packet_path = Path(args.packet)
    out_path = Path(args.out)

    spec_text = spec_path.read_text(encoding="utf-8")
    packet = json.loads(packet_path.read_text(encoding="utf-8"))

    addendum = build_addendum(packet)
    if "## Semantic Planning Packet (Auto-Extracted)" in spec_text:
        merged = spec_text
    else:
        merged = spec_text.rstrip() + "\n\n---\n\n" + addendum

    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_text(merged, encoding="utf-8")

    report = {
        "spec": str(spec_path),
        "packet": str(packet_path),
        "out": str(out_path),
        "added_section": "## Semantic Planning Packet (Auto-Extracted)" in merged,
        "annotation_count": len(packet.get("annotations", [])),
    }
    print(json.dumps(report, indent=2))


if __name__ == "__main__":
    main()
