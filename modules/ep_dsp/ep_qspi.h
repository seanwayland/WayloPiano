#pragma once
// On real Daisy hardware this places the wavetable in external QSPI NOR
// flash (memory-mapped, XIP read) via a GCC-only section attribute pointing
// at a linker-script-defined region. This is a separate, desktop-only copy
// used by the JUCE plugin build (the real Daisy firmware in ../../daisy_ep
// keeps its own hardware copy) - there's no QSPI region here, and no
// desktop linker (MSVC, ld64, GNU ld with a normal desktop link script)
// understands that section name, so this is always a no-op here.
#define DSY_QSPI_DATA
