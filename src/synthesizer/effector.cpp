#include "effector.h"
namespace mgnr::synthesizer::effector {

effector::~effector() {}

void manager::process(dataBlock* buffer) {
    for (auto& it : eff) {
        it->process(buffer);
    }
}

}  // namespace mgnr::synthesizer::effector