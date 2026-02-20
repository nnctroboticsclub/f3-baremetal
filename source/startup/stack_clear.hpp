#pragma once

namespace arm {

extern "C" char _sstack;
extern "C" char _estack;

void ClearStack() {
  for (char* p = &_sstack; p < &_estack && &_sstack < p; p++) {
    *p = 0;
  }
}

};  // namespace arm