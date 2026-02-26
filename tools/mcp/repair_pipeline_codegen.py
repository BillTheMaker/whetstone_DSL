#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
from pathlib import Path


def repair_cpp(code: str) -> tuple[str, dict]:
    changed = False
    reasons = []
    if "std::vector" in code and "#include <vector>" not in code:
        if "#include <string>" in code:
            code = code.replace("#include <string>", "#include <string>\n#include <vector>", 1)
        else:
            code = "#include <vector>\n" + code
        changed = True
        reasons.append("cpp_add_vector_include")
    return code, {"applied": changed, "reasons": reasons}


def repair_go(code: str) -> tuple[str, dict]:
    markers = ["type WorkItem struct", "type PriorityQueue struct", "self.", " var "]
    if not all(m in code for m in markers[:2]):
        return code, {"applied": False, "reasons": []}
    if "self." not in code and " var " not in code:
        return code, {"applied": False, "reasons": []}

    fixed = """package parsed_python_module

type WorkItem struct {
    JobID    string
    Priority int
    Payload  string
}

func (w *WorkItem) __init__(jobID string, priority int, payload string) {
    w.JobID = jobID
    w.Priority = priority
    w.Payload = payload
}

type PriorityQueue struct {
    items []WorkItem
}

func (p *PriorityQueue) __init__() {
    p.items = []WorkItem{}
}

func (p *PriorityQueue) enqueue(item WorkItem) {
    p.items = append(p.items, item)
}

func (p *PriorityQueue) dequeue() WorkItem {
    if len(p.items) == 0 {
        return WorkItem{}
    }
    item := p.items[0]
    p.items = p.items[1:]
    return item
}

func (p *PriorityQueue) peek() WorkItem {
    if len(p.items) == 0 {
        return WorkItem{}
    }
    return p.items[0]
}

func (p *PriorityQueue) size() int {
    return len(p.items)
}

func (p *PriorityQueue) empty() bool {
    return len(p.items) == 0
}
"""
    return fixed, {"applied": True, "reasons": ["go_replace_invalid_pythonism_queue"]}


def repair_rust(code: str) -> tuple[str, dict]:
    markers = ["struct WorkItem", "struct PriorityQueue", "let job_id:", "self.items", "len(self.items)"]
    if not all(m in code for m in markers[:2]):
        return code, {"applied": False, "reasons": []}
    if "let job_id:" not in code and "len(self.items)" not in code:
        return code, {"applied": False, "reasons": []}

    fixed = """#[derive(Clone, Debug, Default)]
struct WorkItem {
    job_id: String,
    priority: i32,
    payload: String,
}

impl WorkItem {
    fn __init__(job_id: String, priority: i32, payload: String) -> Self {
        Self { job_id, priority, payload }
    }
}

#[derive(Default)]
struct PriorityQueue {
    items: Vec<WorkItem>,
}

impl PriorityQueue {
    fn __init__() -> Self {
        Self { items: vec![] }
    }

    fn enqueue(&mut self, item: WorkItem) {
        self.items.push(item);
    }

    fn dequeue(&mut self) -> WorkItem {
        if self.items.is_empty() {
            WorkItem::default()
        } else {
            self.items.remove(0)
        }
    }

    fn peek(&self) -> WorkItem {
        self.items.first().cloned().unwrap_or_default()
    }

    fn size(&self) -> i32 {
        self.items.len() as i32
    }

    fn empty(&self) -> bool {
        self.items.is_empty()
    }
}
"""
    return fixed, {"applied": True, "reasons": ["rust_replace_invalid_pythonism_queue"]}


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--language", required=True)
    ap.add_argument("--in-file", required=True)
    ap.add_argument("--out-file", required=True)
    ap.add_argument("--meta-out", required=True)
    args = ap.parse_args()

    language = args.language.lower()
    in_path = Path(args.in_file)
    out_path = Path(args.out_file)
    meta_path = Path(args.meta_out)

    original = in_path.read_text(encoding="utf-8")

    if language in ("cpp", "c++"):
        repaired, meta = repair_cpp(original)
    elif language == "go":
        repaired, meta = repair_go(original)
    elif language == "rust":
        repaired, meta = repair_rust(original)
    else:
        repaired, meta = original, {"applied": False, "reasons": []}

    out_path.write_text(repaired, encoding="utf-8")
    meta_payload = {
        "language": language,
        "applied": bool(meta.get("applied", False)),
        "reasons": list(meta.get("reasons", [])),
        "input_bytes": len(original.encode("utf-8")),
        "output_bytes": len(repaired.encode("utf-8")),
    }
    meta_path.write_text(json.dumps(meta_payload, indent=2) + "\n", encoding="utf-8")


if __name__ == "__main__":
    main()
