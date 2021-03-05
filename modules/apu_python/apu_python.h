/*******************************************************************************

 BEGIN_JUCE_MODULE_DECLARATION

  ID:                 apu_python
  vendor:             caustik
  version:            0.0.1
  name:               Python module
  description:        Allows Python to be used with JUCE for prototyping
  website:            http://www.caustik.com/
  license:            Commercial

  dependencies:       juce_gui_extra
  windowsLibs:        python36

 END_JUCE_MODULE_DECLARATION

*******************************************************************************/

#ifndef APU_PYTHON_H
#define APU_PYTHON_H

#include <JuceHeader.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_processors/juce_audio_processors.h>
using namespace juce;

#pragma warning(disable : 4100)
#include "PythonExecutor.h"
#include "PythonCodeTokeniserFunctions.h"
#include "PythonCodeTokeniser.h"
#include "PythonEditor.h"
#include "PythonAudioProcessor.h"
#include "PythonAudioProcessorEditor.h"

#endif /* APU_PYTHON_H */
