#ifndef _SCOPE_WAVEFORM_DATA_H
#define _SCOPE_WAVEFORM_DATA_H

const int FAKE_SCOPE_DATA_ARRAY_SIZE = 792;

void start_scope_waveform_thread();
void stop_scope_waveform_thread();

extern bool g_changing_scale;

#endif // _SCOPE_WAVEFORM_DATA_H
