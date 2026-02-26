#pragma once

// Sprint 259 integration summary:
// - Added adaptive raw retry loop driven by top-gap weighted no-uplift condition.
// - Retry synthesizes expanded top-gap requirements and re-scores candidate output.
// - Summary now includes native_raw_adaptive_retry telemetry with attempted/applied status.
