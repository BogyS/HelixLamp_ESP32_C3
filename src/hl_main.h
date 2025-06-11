#pragma once

void hl_Click();
void hl_DoubleClick();
void hl_MultiClick();
void hl_LongPressStart();

void hl_setup();
void hl_loop();

uint8_t hl_NoLists();
String hl_ListName(uint8_t idx);
uint8_t hl_ListMax(uint8_t idx);
String hl_EffectName(uint8_t l_idx, uint8_t e_idx);

void hl_Get(uint8_t& l_idx, uint8_t& e_idx, bool& isRandom);
void hl_Set(uint8_t l_idx, uint8_t e_idx, bool isRandom);

void hl_Sleep();
