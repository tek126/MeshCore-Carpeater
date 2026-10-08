#if defined(ESP32)
#include "LoopDiag.h"
#include <Arduino.h>
#include <esp_task_wdt.h>
#include <esp_attr.h>
#include <esp_system.h>

// RTC-noinit survives every reset except full power removal, which is exactly
// the lifetime we want: the phase written just before a watchdog reset is still
// readable after the reboot.
RTC_NOINIT_ATTR static uint32_t diag_magic;
RTC_NOINIT_ATTR static uint8_t  diag_cur_phase;
RTC_NOINIT_ATTR static uint8_t  diag_hung_phase;
RTC_NOINIT_ATTR static uint16_t diag_rescues;

static esp_reset_reason_t boot_reason;
static uint8_t last_phase = 0xFF;   // where the previous run was when it ended
static const uint32_t DIAG_MAGIC = 0x52504447;   // 'RPDG'

static const char* phase_name(uint8_t p) {
  switch (p) {
    case loop_diag::IDLE:     return "idle";
    case loop_diag::CLI:      return "cli";
    case loop_diag::BUTTON:   return "btn";
    case loop_diag::MESH:     return "mesh";
    case loop_diag::ADVERT:   return "advert";
    case loop_diag::BEACON:   return "beacon";
    case loop_diag::READVERT: return "readvert";
    case loop_diag::SENSORS:  return "sens";
    case loop_diag::UI:       return "ui";
    case loop_diag::CLOCK:    return "clk";
    case loop_diag::SLEEP:    return "sleep";
    case loop_diag::HIBERNATE: return "hib";
    case loop_diag::ACLSAVE:  return "aclsave";
    case loop_diag::PRESENCE: return "pres";
    case loop_diag::INTERFACES: return "intf";
  }
  return "unk";
}

static const char* reason_name(esp_reset_reason_t r) {
  switch (r) {
    case ESP_RST_POWERON:   return "pwron";
    case ESP_RST_EXT:       return "ext";
    case ESP_RST_SW:        return "sw";
    case ESP_RST_PANIC:     return "panic";
    case ESP_RST_INT_WDT:   return "intwdt";
    case ESP_RST_TASK_WDT:  return "taskwdt";
    case ESP_RST_WDT:       return "wdt";
    case ESP_RST_DEEPSLEEP: return "sleep";
    case ESP_RST_BROWNOUT:  return "brownout";
    default:                return "other";
  }
}

void loop_diag::begin() {
  boot_reason = esp_reset_reason();
  if (diag_magic != DIAG_MAGIC) {   // cold power-on: RTC memory is garbage
    diag_magic = DIAG_MAGIC;
    diag_cur_phase = IDLE;
    diag_hung_phase = 0xFF;
    diag_rescues = 0;
  } else {
    last_phase = diag_cur_phase;
  }
  if (last_phase != 0xFF && (boot_reason == ESP_RST_TASK_WDT || boot_reason == ESP_RST_INT_WDT
          || boot_reason == ESP_RST_WDT || boot_reason == ESP_RST_PANIC)) {
    diag_hung_phase = last_phase;
    diag_rescues++;
  }
  Serial.printf("loop_diag: %s\n", summary());
  esp_task_wdt_init(60, true);   // panic (-> reboot) if the loop stalls 60 s
  esp_task_wdt_add(NULL);        // watch the task running setup()/loop()
}

void loop_diag::mark(Phase p) { diag_cur_phase = p; }
void loop_diag::feed() { esp_task_wdt_reset(); }

const char* loop_diag::summary() {
  static char buf[48];
  snprintf(buf, sizeof(buf), "%s.%s.%u last=%s", reason_name(boot_reason),
           diag_hung_phase == 0xFF ? "none" : phase_name(diag_hung_phase),
           (unsigned)diag_rescues, last_phase == 0xFF ? "none" : phase_name(last_phase));
  return buf;
}
#endif
