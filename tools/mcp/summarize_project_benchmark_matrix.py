#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
from collections import Counter, defaultdict
from pathlib import Path
from typing import Any, Dict, List


def parse_args() -> argparse.Namespace:
    ap = argparse.ArgumentParser(description="Summarize benchmark matrix JSONL results.")
    ap.add_argument("--results", required=True, help="Path to results.jsonl")
    ap.add_argument("--out", required=True, help="Path to summary.json")
    return ap.parse_args()


def pct(n: int, d: int) -> float:
    if d <= 0:
        return 0.0
    return round((n / d) * 100.0, 2)


def load_rows(path: Path) -> List[Dict[str, Any]]:
    rows: List[Dict[str, Any]] = []
    with path.open("r", encoding="utf-8") as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            rows.append(json.loads(line))
    return rows


def summarize(rows: List[Dict[str, Any]]) -> Dict[str, Any]:
    by_lang: Dict[str, Dict[str, Any]] = defaultdict(lambda: {
        "total": 0,
        "ab_ok": 0,
        "ab_path_a_ready": 0,
        "ab_path_b_ready": 0,
        "ab_path_a_tokens_sum": 0,
        "ab_path_b_tokens_sum": 0,
        "prod_ok": 0,
        "prod_ready": 0,
    })
    by_category: Dict[str, Dict[str, Any]] = defaultdict(lambda: {
        "total": 0,
        "ab_path_a_ready": 0,
        "ab_path_b_ready": 0,
        "prod_ready": 0,
    })
    path_b_failures = Counter()
    false_green_candidates = 0
    ab_prod_divergence_count = 0
    ab_consistency_blocked_count = 0
    projection_contract_failures = 0
    fullstack_contract_failures = 0

    total = len(rows)
    ab_ok = 0
    ab_path_a_ready = 0
    ab_path_b_ready = 0
    prod_ok = 0
    prod_ready = 0
    tok_a_sum = 0
    tok_b_sum = 0

    for r in rows:
        lang = str(r.get("language_exec", "unknown"))
        category = str(r.get("category", "unknown"))
        ab = r.get("ab", {}) or {}
        prod = r.get("production_loop", {}) or {}
        proj = r.get("projection_contract", {}) or {}
        fullstack = r.get("fullstack_contract", {}) or {}

        by_lang[lang]["total"] += 1
        by_category[category]["total"] += 1

        if ab.get("status") == "ok":
            ab_ok += 1
            by_lang[lang]["ab_ok"] += 1

        if bool(ab.get("path_a_ready")):
            ab_path_a_ready += 1
            by_lang[lang]["ab_path_a_ready"] += 1
            by_category[category]["ab_path_a_ready"] += 1
        if bool(ab.get("path_b_ready")):
            ab_path_b_ready += 1
            by_lang[lang]["ab_path_b_ready"] += 1
            by_category[category]["ab_path_b_ready"] += 1

        ta = int(ab.get("path_a_total_tokens", 0) or 0)
        tb = int(ab.get("path_b_total_tokens", 0) or 0)
        tok_a_sum += ta
        tok_b_sum += tb
        by_lang[lang]["ab_path_a_tokens_sum"] += ta
        by_lang[lang]["ab_path_b_tokens_sum"] += tb

        for reason in (ab.get("path_b_failure_reasons") or []):
            path_b_failures[str(reason)] += 1

        if prod.get("status") == "ok":
            prod_ok += 1
            by_lang[lang]["prod_ok"] += 1
        if bool(prod.get("overall_ready")):
            prod_ready += 1
            by_lang[lang]["prod_ready"] += 1
            by_category[category]["prod_ready"] += 1

        prod_ready_flag = bool(prod.get("overall_ready"))
        prod_evidence_flag = bool(prod.get("gate_evidence_complete", False))
        prod_compile_pass = bool(prod.get("compile_pass", False))
        prod_tests_pass = bool(prod.get("tests_pass", False))
        if prod_ready_flag and (not prod_evidence_flag or not prod_compile_pass or not prod_tests_pass):
            false_green_candidates += 1
        if bool(prod.get("ab_divergence", False)):
            ab_prod_divergence_count += 1
        if bool(prod.get("ab_consistency_blocked", False)):
            ab_consistency_blocked_count += 1
        if not bool(proj.get("ok", True)):
            projection_contract_failures += 1
        if not bool(fullstack.get("ok", True)):
            fullstack_contract_failures += 1

    lang_rows: Dict[str, Any] = {}
    for lang, s in sorted(by_lang.items()):
        t = s["total"]
        lang_rows[lang] = {
            "total": t,
            "ab_ok": s["ab_ok"],
            "ab_ok_rate_pct": pct(s["ab_ok"], t),
            "ab_path_a_ready": s["ab_path_a_ready"],
            "ab_path_a_ready_rate_pct": pct(s["ab_path_a_ready"], t),
            "ab_path_b_ready": s["ab_path_b_ready"],
            "ab_path_b_ready_rate_pct": pct(s["ab_path_b_ready"], t),
            "ab_avg_tokens_path_a": round(s["ab_path_a_tokens_sum"] / t, 2) if t else 0.0,
            "ab_avg_tokens_path_b": round(s["ab_path_b_tokens_sum"] / t, 2) if t else 0.0,
            "ab_avg_token_ratio_b_over_a": round((s["ab_path_b_tokens_sum"] / s["ab_path_a_tokens_sum"]), 4)
            if s["ab_path_a_tokens_sum"] > 0
            else None,
            "prod_ok": s["prod_ok"],
            "prod_ok_rate_pct": pct(s["prod_ok"], t),
            "prod_ready": s["prod_ready"],
            "prod_ready_rate_pct": pct(s["prod_ready"], t),
        }

    category_rows: Dict[str, Any] = {}
    for cat, s in sorted(by_category.items()):
        t = s["total"]
        category_rows[cat] = {
            "total": t,
            "ab_path_a_ready": s["ab_path_a_ready"],
            "ab_path_a_ready_rate_pct": pct(s["ab_path_a_ready"], t),
            "ab_path_b_ready": s["ab_path_b_ready"],
            "ab_path_b_ready_rate_pct": pct(s["ab_path_b_ready"], t),
            "prod_ready": s["prod_ready"],
            "prod_ready_rate_pct": pct(s["prod_ready"], t),
        }

    summary = {
        "total_runs": total,
        "ab": {
            "ok_runs": ab_ok,
            "ok_rate_pct": pct(ab_ok, total),
            "path_a_ready": ab_path_a_ready,
            "path_a_ready_rate_pct": pct(ab_path_a_ready, total),
            "path_b_ready": ab_path_b_ready,
            "path_b_ready_rate_pct": pct(ab_path_b_ready, total),
            "avg_tokens_path_a": round(tok_a_sum / total, 2) if total else 0.0,
            "avg_tokens_path_b": round(tok_b_sum / total, 2) if total else 0.0,
            "avg_token_ratio_b_over_a": round((tok_b_sum / tok_a_sum), 4) if tok_a_sum > 0 else None,
            "top_path_b_failure_reasons": path_b_failures.most_common(15),
        },
        "production_loop": {
            "ok_runs": prod_ok,
            "ok_rate_pct": pct(prod_ok, total),
            "ready_runs": prod_ready,
            "ready_rate_pct": pct(prod_ready, total),
            "false_green_candidates": false_green_candidates,
            "ab_prod_divergence_count": ab_prod_divergence_count,
            "ab_consistency_blocked_count": ab_consistency_blocked_count,
        },
        "projection_contract": {
            "invalid_runs": projection_contract_failures,
            "invalid_rate_pct": pct(projection_contract_failures, total),
        },
        "fullstack_contract": {
            "invalid_runs": fullstack_contract_failures,
            "invalid_rate_pct": pct(fullstack_contract_failures, total),
        },
        "by_language": lang_rows,
        "by_category": category_rows,
    }
    return summary


def main() -> None:
    args = parse_args()
    rows = load_rows(Path(args.results))
    summary = summarize(rows)
    out = Path(args.out)
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(json.dumps(summary, indent=2) + "\n", encoding="utf-8")


if __name__ == "__main__":
    main()
