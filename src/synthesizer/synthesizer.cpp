#include "synthesizer.h"
namespace mgnr::synthesizer {

synthesizer::synthesizer() {
}
synthesizer::synthesizer(synthesizer& s)
    : ins(s.ins), eff(s.eff) {}
synthesizer::~synthesizer() {
}

}  // namespace mgnr::synthesizer
