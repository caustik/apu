//
// File: PythonAudioProcessorEditor
// Desc: Declarations for Python audio processor editor
//

#ifndef PYTHON_AUDIO_PROCESS_EDITOR_H
#define PYTHON_AUDIO_PROCESS_EDITOR_H

#include "apu_python.h"

//
// PythonAudioProcessorEditor
//
// Editor for Python audio processor
//

class PythonAudioProcessorEditor : public AudioProcessorEditor
{
public:
    PythonAudioProcessorEditor(PythonAudioProcessor&, int width, int height);
    ~PythonAudioProcessorEditor();

    void paint(Graphics&) override {}
    void resized() override;

private:
    PythonAudioProcessor& m_processor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PythonAudioProcessorEditor)
};

#endif
