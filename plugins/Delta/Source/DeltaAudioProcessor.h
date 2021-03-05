//
// File: DeltaAudioProcessor.h
// Desc: Manage the delta of synth program and CC changes
//

#ifndef DELTA_AUDIO_PROCESSOR_H
#define DELTA_AUDIO_PROCESSOR_H

#include <JuceHeader.h>

#include <vector>
#include <set>

//
// DeltaAudioProcessor
//
// Main class for DeltaAudioProcessor
//

class DeltaAudioProcessor : public AudioProcessor
{
public:
    DeltaAudioProcessor();
    ~DeltaAudioProcessor();

    // AudioProcessor interface implementation (general)
    const String getName() const override { return JucePlugin_Name; }

    // AudioProcessor interface implementation (playback)
    void prepareToPlay(double sampleRate, int samplesPerBlock) override {}
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
    const String getProgramName(int index) override { return ""; }
    void changeProgramName(int index, const String& newName) override {}

    // AudioProcessor interface implementation (program state)
    void getStateInformation(MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // AudioProcessor interface implementation (editor)
    AudioProcessorEditor* createEditor() override { return nullptr; }
    bool hasEditor() const override { return false; }

private:
    // plugin parameter resources
    juce::AudioProcessorValueTreeState m_vts;
    juce::UndoManager m_undoManager;

    // control change values
    std::vector<int> m_controlChange;
    // pending output parameters
    std::set<int> m_pending;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DeltaAudioProcessor)
};

#endif