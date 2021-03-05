//
// File: PySynthAudioProcessor.h
// Desc: Process audio using Python script
//

#ifndef PYSYNTH_AUDIO_PROCESSOR_H
#define PYSYNTH_AUDIO_PROCESSOR_H

#include <JuceHeader.h>

#include <apu_python/PythonAudioProcessor.h>

//
// PySynthAudioProcessor
//
// Main class for audio processor which uses Python script to manipulate audio
//

class PySynthAudioProcessor : public PythonAudioProcessor
{
public:
    PySynthAudioProcessor() : PythonAudioProcessor() {}

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PySynthAudioProcessor)
};

#endif