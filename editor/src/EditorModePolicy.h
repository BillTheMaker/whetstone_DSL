#pragma once
#include "BufferManager.h"

inline bool isTextMode(BufferManager::BufferMode mode) {
    return mode == BufferManager::BufferMode::Text;
}

inline bool allowStructuredFeatures(BufferManager::BufferMode mode) {
    return mode == BufferManager::BufferMode::Structured;
}
