#pragma once

#include <memory>
#include <vector>

namespace vortex {

class ArchDef;
class Instr;

class Decoder {
 public:
    Decoder(const ArchDef&);

    std::shared_ptr<Instr> decode(uint32_t code) const;
};

}  // namespace vortex