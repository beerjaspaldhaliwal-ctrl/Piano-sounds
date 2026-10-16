// Keyverse - factory preset library interface.
#pragma once

#include "Params.h"
#include <string>
#include <vector>

namespace kv
{
struct Preset
{
    std::string name;
    std::string category;      // top-level browser category
    std::string subcategory;   // instrument type
    std::string tags;          // free text, used by search
    SoundData data;
};

// Ordered top-level categories (pianos & synth keys first).
const std::vector<std::string>& categories();

// All factory presets, grouped by category in browser order.
const std::vector<Preset>& factoryPresets();

} // namespace kv
