#pragma once

// Sprint 240 integration summary:
// - Added native decomposition retry path with decomposition-min-task policy prompt.
// - Retry is applied only when it improves native task count.
// - Pipeline now emits native decomposition retry telemetry.
