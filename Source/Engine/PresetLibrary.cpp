// Keyverse - factory preset registry.
#include "Presets.h"
#include "Banks/Banks.h"

namespace kv
{
const std::vector<std::string>& categories()
{
    static const std::vector<std::string> c {
        pb::kAcoustic, pb::kElectric, pb::kSynthKeys, pb::kOrgans, pb::kClavs, pb::kMallets,
        pb::kGuitars, pb::kBasses, pb::kPads, pb::kLeads, pb::kSynthBass, pb::kStrBrass,
        pb::kPlucks, pb::kWorld, pb::kLayers, pb::kFx, pb::kInit
    };
    return c;
}

const std::vector<Preset>& factoryPresets()
{
    static const std::vector<Preset> all = []
    {
        pb::Lib lib;
        banks::addAcousticPianos (lib);
        banks::addElectricPianos (lib);
        banks::addSynthKeys (lib);
        banks::addOrgans (lib);
        banks::addClavs (lib);
        banks::addMallets (lib);
        banks::addGuitars (lib);
        banks::addBasses (lib);
        banks::addPads (lib);
        banks::addLeads (lib);
        banks::addSynthBass (lib);
        banks::addStringsBrass (lib);
        banks::addPlucks (lib);
        banks::addWorld (lib);
        banks::addLayers (lib);
        banks::addFx (lib);
        banks::addInit (lib);
        // stable order: by category (browser order), keep insertion order inside a category
        std::vector<Preset> sorted;
        sorted.reserve (lib.v.size());
        for (const auto& c : categories())
            for (const auto& p : lib.v)
                if (p.category == c) sorted.push_back (p);
        return sorted;
    }();
    return all;
}
} // namespace kv
