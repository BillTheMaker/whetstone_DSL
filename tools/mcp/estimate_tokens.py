#!/usr/bin/env python3
"""Deterministic token estimator for local artifacts.

If tiktoken is available, use cl100k_base for a closer estimate.
Otherwise, fall back to ceil(chars/4).
"""

from __future__ import annotations

import argparse
import json
import math
from pathlib import Path


def estimate_tokens(text: str) -> dict:
    chars = len(text)
    words = len(text.split())
    approx = int(math.ceil(chars / 4.0))

    exact = None
    encoding = "chars_div_4"
    try:
        import tiktoken  # type: ignore

        enc = tiktoken.get_encoding("cl100k_base")
        exact = len(enc.encode(text))
        encoding = "cl100k_base"
    except Exception:
        pass

    return {
        "chars": chars,
        "words": words,
        "approx_tokens": approx,
        "tokenizer": encoding,
        "tokens": exact if exact is not None else approx,
    }


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--file", default="")
    ap.add_argument("--text", default="")
    args = ap.parse_args()

    if args.file:
        text = Path(args.file).read_text()
    else:
        text = args.text

    print(json.dumps(estimate_tokens(text), indent=2, sort_keys=True))


if __name__ == "__main__":
    main()
