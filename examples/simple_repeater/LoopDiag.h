#pragma once
#include <stdint.h>

// ESP32 loop-hang diagnostics: a 60 s task watchdog on the loop task (a wedged
// loop reboots itself instead of needing a power cycle) plus a phase breadcrumb
// kept in RTC-noinit memory, so after a watchdog rescue the 'diag' CLI command
// can say which part of the loop was stuck. nRF52 boards already have a 90 s
// hardware watchdog (see main.cpp); the macros compile out there.
// Ported from the companion's LoopDiag (companion-presence 252944e7).
#if defined(ESP32)
namespace loop_diag {
  enum Phase : uint8_t { IDLE = 0, CLI, BUTTON, MESH, ADVERT, BEACON, READVERT,
                         SENSORS, UI, CLOCK, SLEEP, HIBERNATE };
  void begin();              // classify the reset + arm the watchdog (end of setup)
  void mark(Phase p);        // record the part of the loop about to run (cheap store)
  void feed();               // pet the watchdog, once per healthy loop pass
  // "<reset>.<hung-phase>.<rescues> last=<phase>", e.g. "taskwdt.beacon.2 last=beacon".
  // last = where the previous run was when it ended, whatever ended it -- so a
  // node that went into hold-to-hibernate and was then reset reads "last=hib".
  const char* summary();
}
  #define LOOP_DIAG_BEGIN()  loop_diag::begin()
  #define LOOP_DIAG_MARK(p)  loop_diag::mark(loop_diag::p)
  #define LOOP_DIAG_FEED()   loop_diag::feed()
#else
  #define LOOP_DIAG_BEGIN()
  #define LOOP_DIAG_MARK(p)
  #define LOOP_DIAG_FEED()
#endif
