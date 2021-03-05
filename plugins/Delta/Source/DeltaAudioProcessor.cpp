//
// File: DeltaAudioProcessor
// Desc: Manage the delta of synth program and CC changes
//

#include "DeltaAudioProcessor.h"

#if defined(WIN32) && defined(_DEBUG)
#include <windows.h>
#endif

static const int controlChangeCount = 128;

DeltaAudioProcessor::DeltaAudioProcessor() : m_vts(*this, &m_undoManager)
{
#if defined(WIN32) && defined(_DEBUG)
    if (AllocConsole()) {
        SetConsoleTitleW(L"APU Debug Console");
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED);
        freopen("CONOUT$", "wt", stdout);
        printf(" APU Debug Console\n");
    }
#endif

    // control change parameters
    m_controlChange.resize(controlChangeCount);

    m_vts.state = ValueTree(Identifier(JucePlugin_Name));
}

DeltaAudioProcessor::~DeltaAudioProcessor() {}

void DeltaAudioProcessor::processBlock(AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;

    // clean audio output
    buffer.clear();

    // prepare midi input
    MidiBuffer processedMidi;
    for (auto metaData : midiMessages) {
        const MidiMessage message = metaData.getMessage();
        const juce::uint8* messageData = message.getRawData();

        // add control changes to pending
        if (message.isController()) {
            const juce::uint8 cc = messageData[1];
            const juce::uint8 value = messageData[2];
            m_pending.insert(cc);
            m_controlChange[cc] = value;
        }
        // flush pending on program change
        else if (message.isProgramChange()) {
            m_pending.clear();
        }

        // pass through all MIDI events
        processedMidi.addEvent(message, midiMessages.getLastEventTime());
    }

    // send any pending outputs at the start of playback
    AudioPlayHead::CurrentPositionInfo position;
    AudioPlayHead* audioPlayHead = getPlayHead();
    if (playHead && audioPlayHead->getCurrentPosition(position) && position.isPlaying && position.timeInSamples == 0) {
        for (int idx : m_pending) {
            MidiMessage message = MidiMessage::controllerEvent(1, idx, m_controlChange[idx]);
            processedMidi.addEvent(message, (int)midiMessages.getLastEventTime());
        }
    }

    // replace MIDI output to our modified MIDI
    midiMessages.swapWith(processedMidi);
}

void DeltaAudioProcessor::getStateInformation(MemoryBlock& destData)
{
    // get parameter state
    auto state = m_vts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
};

void DeltaAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // set parameter state
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr) {
        if (xmlState->hasTagName(m_vts.state.getType()))
            m_vts.replaceState(juce::ValueTree::fromXml(*xmlState));
    }
}

AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new DeltaAudioProcessor(); }
