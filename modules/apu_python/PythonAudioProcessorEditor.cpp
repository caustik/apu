//
// File: PythonAudioProcessorEditor
// Desc: Definitions for Python audio processor editor
//

#include "PythonAudioProcessor.h"
#include "PythonAudioProcessorEditor.h"

PythonAudioProcessorEditor::PythonAudioProcessorEditor(PythonAudioProcessor& processor, int width, int height)
  : AudioProcessorEditor(&processor), m_processor(processor)
{
    AudioProcessorEditor::setResizable(true, true);
    Component::addAndMakeVisible(processor.getPythonEditor());
    Component::setSize(width, height);
}

PythonAudioProcessorEditor::~PythonAudioProcessorEditor() {}

void PythonAudioProcessorEditor::resized()
{
    // set the size of the grid to fill the whole window.
    m_processor.getPythonEditor().setBounds(getLocalBounds());

    m_processor.setEditorSize(Component::getWidth(), Component::getHeight());
}
