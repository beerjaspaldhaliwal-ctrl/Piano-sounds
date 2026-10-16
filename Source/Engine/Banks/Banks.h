// Keyverse - factory preset banks (one translation unit per category).
#pragma once

#include "../Instruments.h"

namespace kv::banks
{
void addAcousticPianos (pb::Lib&);
void addElectricPianos (pb::Lib&);
void addSynthKeys (pb::Lib&);
void addOrgans (pb::Lib&);
void addClavs (pb::Lib&);
void addMallets (pb::Lib&);
void addGuitars (pb::Lib&);
void addBasses (pb::Lib&);
void addPads (pb::Lib&);
void addLeads (pb::Lib&);
void addSynthBass (pb::Lib&);
void addStringsBrass (pb::Lib&);
void addPlucks (pb::Lib&);
void addWorld (pb::Lib&);
void addLayers (pb::Lib&);
void addFx (pb::Lib&);
void addInit (pb::Lib&);
} // namespace kv::banks
