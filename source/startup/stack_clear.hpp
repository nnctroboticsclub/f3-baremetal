#pragma once

namespace arm {

extern "C" char _sstack;
extern "C" char _estack;

void ClearStack() {
  for (char* p = &_sstack; p < &_estack; p++) {
    *p = 0;
  }
}

};  // namespace arm