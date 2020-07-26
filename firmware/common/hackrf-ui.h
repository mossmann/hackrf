#ifndef HACKRF_UI_H
#define HACKRF_UI_H

#include <rf_path.h>
#include <stdint.h>

void hackrf_ui_init(void) __attribute__((weak));
void hackrf_ui_setFrequency(uint64_t _freq) __attribute__((weak));
void hackrf_ui_setSampleRate(uint32_t _sample_rate) __attribute__((weak));
void hackrf_ui_setDirection(const rf_path_direction_t _direction) __attribute__((weak));
void hackrf_ui_setFilterBW(uint32_t bw) __attribute__((weak));
void hackrf_ui_setLNAPower(bool _lna_on) __attribute__((weak));
void hackrf_ui_setBBLNAGain(const uint32_t gain_db) __attribute__((weak));
void hackrf_ui_setBBVGAGain(const uint32_t gain_db) __attribute__((weak));
void hackrf_ui_setBBTXVGAGain(const uint32_t gain_db) __attribute__((weak));

#endif
