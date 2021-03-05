//
// File: PySynthAudioProcessor
// Desc: Process audio using Python script
//

#include "PySynthAudioProcessor.h"

AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new PySynthAudioProcessor(); }
