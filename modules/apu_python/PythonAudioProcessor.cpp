//
// File: PythonAudioProcessor
// Desc: Process audio using Python script
//

#include "PythonAudioProcessor.h"
#include "PythonAudioProcessorEditor.h"

static py::list createEvent(MidiMessage message)
{
    py::bytes prefix((const char*)message.getRawData(), message.getRawDataSize() - 1);
    py::bytes suffix;
    return py::list(py::make_tuple(prefix, suffix));
}

static py::list controllerEvent(int cc)
{
    MidiMessage message = MidiMessage::controllerEvent(1, cc, 0);
    return createEvent(message);
}

static py::list noteEvent(int note, int on)
{
    MidiMessage message = on ? MidiMessage::noteOn(1, note, (uint8_t)0) : MidiMessage::noteOff(1, note, (uint8_t)0);
    return createEvent(message);
}

static py::list programChangeEvent()
{
    MidiMessage message = MidiMessage::programChange(1, 0);
    return createEvent(message);
}

PYBIND11_EMBEDDED_MODULE(apu, module)
{
    module.def(
        "print",
        [](py::args args, py::kwargs kwargs) {
            try {
                for (py::size_t i = 0; i < args.size(); ++i) {
                    std::string str = py::str(args[i]);
                    printf(i == 0 ? "%s" : " %s", str.c_str());
                }
                printf("\n");
            }
            catch (...) {
            }
        },
        "print a string");
    module.def("controllerEvent", controllerEvent);
    module.def("noteEvent", noteEvent);
    module.def("programChangeEvent", programChangeEvent);
    module.attr("__dict__")["globals"] = py::dict();
}

PythonAudioProcessor::PythonAudioProcessor() : m_pythonEditor(*this, this, ""), m_vts(*this, &m_undoManager)
{
    m_vts.state = ValueTree(Identifier(JucePlugin_Name));
}

PythonAudioProcessor::~PythonAudioProcessor() {}

void PythonAudioProcessor::execute(const char* filename, const char* script)
{
    PythonExecutor::execute(filename, script);

    PythonExecutor::lock();

    // initialize output tuples
    try {
        m_outputs.clear();

        // request midi outputs from the script
        py::dict dict = PythonExecutor::getModule().attr("__dict__");
        if (dict.contains("getMidiOutputs")) {
            py::list midiOutputs = dict["getMidiOutputs"]();
            for (auto& midiOutput : midiOutputs) {
                py::tuple tuple = midiOutput.cast<py::tuple>();
                std::string prefix = tuple[0].cast<std::string>();
                std::string suffix = tuple[1].cast<std::string>();
                m_outputs.emplace_back(std::make_tuple(prefix, suffix));
            }
        }

        // load globals dictionary
        py::exec("def setGlobals(data):\n    import json\n    apu.globals.update(json.loads(data), **apu.globals)", dict, dict);
        dict["setGlobals"](m_globals == "" ? "{}" : m_globals);
    }
    catch (...) {
    }

    PythonExecutor::unlock();
}

void PythonAudioProcessor::filenameComponentChanged(FilenameComponent* filenameComponent)
{
    m_filename = filenameComponent->getCurrentFileText().toStdString();
    auto offset = m_filename.find_last_of("\\/");
    m_program = (offset != std::string::npos) ? &m_filename[offset + 1] : m_filename;
    updateHostDisplay();
}

void PythonAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // bind current samplerate so its accessible to processing functions
    PythonExecutor::lock();
    PythonExecutor::bind("sample_rate", py::int_((int)getSampleRate()));
    PythonExecutor::unlock();
}

void PythonAudioProcessor::processBlock(AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;

    PythonExecutor::lock();

    auto totalNumInputChanenls = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    try {
        MidiBuffer processedMidi;

        py::list audioInputs;
        py::list audioOutputs;

        py::dict midiControlInputs;
        py::dict midiNoteOnInputs;
        py::dict midiNoteOffInputs;
        py::dict midiProgramChangeInputs;
        py::dict midiOutputs;

        // allow script to process any audio and/or midi messages
        const py::dict dict = PythonExecutor::getModule().attr("__dict__");

        // check for existance of processing funtions
        const bool processAudio = dict.contains("processAudio");
        const bool processMidiControls = dict.contains("processMidiControls");
        const bool processMidiNotes = dict.contains("processMidiNotes");
        const bool processProgramChanges = dict.contains("processProgramChanges");

        // prepare audio input
        for (auto i = 0; i < totalNumInputChanenls; ++i) {
            py::str dummy; // prevent pybind11 from copying (jackie chan wtf meme)
            audioInputs.append(py::array(buffer.getNumSamples(), buffer.getReadPointer(i), dummy));
        }

        // prepare audio output
        if (processAudio) {
            for (auto i = 0; i < totalNumOutputChannels; ++i) {
                py::str dummy; // prevent pybind11 from copying (jackie chan wtf meme)
                audioOutputs.append(py::array(buffer.getNumSamples(), buffer.getWritePointer(i), dummy));
            }
        }
        else {
            // @todo: pass-through audio (copy data)
        }

        // prepare midi input
        for (auto metaData : midiMessages) {
            auto message = metaData.getMessage();
            const int messageSize = message.getRawDataSize();
            const int channel = 1 - 1;
            const juce::uint8* messageData = message.getRawData();
            // filter MIDI cc input through Python
            if (processMidiControls && message.isController()) {
                const juce::uint8* payload = &messageData[1];
                midiControlInputs[py::int_(payload[0])] = payload[1];
            }
            // filter MIDI note on through Python
            else if (processMidiNotes && message.isNoteOn()) {
                const juce::uint8* payload = &messageData[1];
                midiNoteOnInputs[py::int_(payload[0])] = payload[1];
            }
            // filter MIDI note off through Python
            else if (processMidiNotes && !message.isNoteOn()) {
                const juce::uint8* payload = &messageData[1];
                midiNoteOffInputs[py::int_(payload[0])] = payload[1];
            }
            else if (processProgramChanges && message.isProgramChange()) {
                const juce::uint8* payload = &messageData[1];
                midiProgramChangeInputs[py::int_(payload[0])] = payload[1];
            }
            // pass through all unhandled MIDI events
            else {
                processedMidi.addEvent(message, midiMessages.getLastEventTime());
            }
        }

        // optional audio processing
        if (dict.contains("processAudio"))
            dict["processAudio"](audioInputs, audioOutputs);

        // optional midi control change processing
        if (processMidiControls && !midiControlInputs.empty())
            dict["processMidiControls"](midiControlInputs, midiOutputs);

        // optional midi note processing
        if (processMidiNotes && !midiNoteOnInputs.empty())
            dict["processMidiNotes"](midiNoteOnInputs, true, midiOutputs);
        if (processMidiNotes && !midiNoteOffInputs.empty())
            dict["processMidiNotes"](midiNoteOffInputs, false, midiOutputs);

        // optional program change processing
        if (processProgramChanges && !midiProgramChangeInputs.empty())
            dict["processProgramChanges"](midiProgramChangeInputs, midiOutputs);

        // process midi output
        for (auto item : midiOutputs) {
            // parse index/value
            const auto outputIdx = item.first.cast<juce::uint8>();
            const auto outputValue = item.second.cast<juce::uint8>();
            // skip invalid outputs
            if (outputIdx >= m_outputs.size())
                continue;
            // reconstruct the midi message for this output
            const std::tuple<std::string, std::string> tuple = m_outputs[outputIdx];
            const std::string rawMessage = std::get<0>(tuple) + (char)outputValue + std::get<1>(tuple);
            const MidiMessage message(rawMessage.c_str(), (int)rawMessage.length());
            // skip cc values which haven't changed (midi cc pickup)
            if (message.isController() && dict.contains("processMidiControls")) {
                const auto cc = message.getControllerNumber();
                const auto value = message.getControllerValue();
                if (m_PrevMidiOutputs.find(cc) != m_PrevMidiOutputs.end() && value == m_PrevMidiOutputs[cc])
                    continue;
                // update previous midi outputs state
                m_PrevMidiOutputs[cc] = value;
            }
            // send the output event!
            processedMidi.addEvent(message, (int)midiMessages.getLastEventTime());
        }

        midiMessages.swapWith(processedMidi);
    }
    catch (...) {
    }

    PythonExecutor::unlock();
}

void PythonAudioProcessor::getStateInformation(MemoryBlock& destData)
{
    // update parameter state
    m_vts.state.setProperty("m_filename", m_filename.c_str(), &m_undoManager);
    m_vts.state.setProperty("editorWidth", m_editorWidth, &m_undoManager);
    m_vts.state.setProperty("editorHeight", m_editorHeight, &m_undoManager);

    // save globals dictionary
    PythonExecutor::lock();
    try {
        py::dict dict = PythonExecutor::getModule().attr("__dict__");
        py::exec("def getGlobals():\n    import json\n    return json.dumps(apu.globals)", dict, dict);
        std::string json = py::cast<std::string>(dict["getGlobals"]());
        m_vts.state.setProperty("globals", json.c_str(), &m_undoManager);
    }
    catch(...) {

    }
    PythonExecutor::unlock();

    // get parameter state
    auto state = m_vts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
};

void PythonAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // set parameter state
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr) {
        if (xmlState->hasTagName(m_vts.state.getType()))
            m_vts.replaceState(juce::ValueTree::fromXml(*xmlState));
    }

    // update globals from parameter state
    m_globals = m_vts.state.getProperty("globals").toString().toStdString();

    // update filename from parameter state
    m_filename = m_vts.state.getProperty("m_filename").toString().toStdString();
    m_pythonEditor.setFilename(m_filename.c_str());

    // update editor position from parameter state
    m_editorWidth = m_vts.state.getProperty("editorWidth");
    m_editorWidth = m_editorWidth ? m_editorWidth : 1024;
    m_editorHeight = m_vts.state.getProperty("editorHeight");
    m_editorHeight = m_editorHeight ? m_editorHeight : 768;
}

AudioProcessorEditor* PythonAudioProcessor::createEditor() { return new PythonAudioProcessorEditor(*this, m_editorWidth, m_editorHeight); }
