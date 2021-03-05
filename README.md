# APU VST Plugins

This repository houses some VST plugins designed to facilitate control of external synths via LaunchKey and other MIDI devices with knobs/slider/buttons. The purpose is to allow a production workflow where you can switch between external synths and manipulate their controls without touching the DAW. To accomplish this, there are two plugins as described in the sections below. Note that this project has thus far been for my own personal usage, so it is not packaged up in a form which is easy for others to use. It relies upon a relatively complex Ableton Live template to perform the routing necessary to expose all of the features. I'm sharing the source code in help others who may wish to embed Python into VST(s), or who may wish to help extend this project into something more complete.

These plugins are developed using the [JUCE](https://juce.com/) platform which greatly simplifies plugin for C/C++. 

# PySynth

This VST plugin embeds a Python interpretter which is used to allow arbitrary Python to filter/translate MIDI CC / program change events. The purpose is to be able to take incoming knobs/sliders/buttons from one external controller and map them to the CC messages supported by an external synthesizer. It's possible to continuously make changes to the Python scripts while the DAW is running, which makes development pretty rapid paced.

# Delta

This VST plugin captures the delta in state from the time it last witnessed a program change message. It keeps track of the last CC sent and will send these in batch at the start of playback. The purpose is to allow external synth programs to be modified on the fly and then those modifications recalled later without any special extra effort.