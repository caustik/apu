//
// File: PythonAudioProcessor.h
// Desc: Process audio using Python script
//

#ifndef PYTHON_AUDIO_PROCESSOR_H
#define PYTHON_AUDIO_PROCESSOR_H

#include "apu_python.h"

#include <vector>
#include <tuple>
#include <set>

//
// PythonAudioProcessor
//
// Main class for audio processor which uses Python script to manipulate audio
//

class PythonAudioProcessor : public AudioProcessor, public PythonExecutor, public FilenameComponentListener
{
public:
    PythonAudioProcessor();
    ~PythonAudioProcessor();

    void setEditorSize(int width, int height) { m_editorWidth = width; m_editorHeight = height; }

    // PythonExecutor interface
    void execute(const char* filename, const char* script) override;

    // FilenameComponentListener interace implementation
    void filenameComponentChanged(FilenameComponent* filenameComponent) override;

    // AudioProcessor interface implementation (general)
    const String getName() const override { return JucePlugin_Name; }

    // AudioProcessor interface implementation (playback)
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processBlock(AudioBuffer<float>&, MidiBuffer&) override;

    // AudioProcessor interface implementation (midi, general)
    bool acceptsMidi() const override { return JucePlugin_WantsMidiInput; }
    bool producesMidi() const override { return JucePlugin_ProducesMidiOutput; }
    bool isMidiEffect() const override { return JucePlugin_IsMidiEffect; }
    double getTailLengthSeconds() const override { return 0.0; }

    // AudioProcessor interface implementation (midi, programs)
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int index) override {}
    const String getProgramName(int index) override { return m_program.c_str(); }
    void changeProgramName(int index, const String& newName) override {}

    // AudioProcessor interface implementation (program state)
    void getStateInformation(MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // AudioProcessor interface implementation (editor)
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    PythonEditor& getPythonEditor() { return m_pythonEditor; }

private:
    // editor resources
    PythonEditor m_pythonEditor;
    std::string m_filename;
    std::string m_program;
    int m_editorWidth = 1024;
    int m_editorHeight = 768;
    std::string m_globals;

    // plugin parameter resources
    juce::AudioProcessorValueTreeState m_vts;
    juce::UndoManager m_undoManager;

    // mapping from MIDI output index to MIDI output event (cc, prefix, suffix)
    std::vector<std::tuple<std::string, std::string>> m_outputs;

    // previous MIDI outputs, used to implement CC pickup for devices which don't (reliably) support it
    std::map<int, int> m_PrevMidiOutputs;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PythonAudioProcessor)
};

#endif