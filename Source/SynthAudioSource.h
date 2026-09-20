#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include "SineWaveSound.h"
#include "SineWaveVoice.h"

// Owns the Synthesiser and turns keyboard/MIDI events into audio. Lives on the
// audio thread side: getNextAudioBlock is called by the device callback.
class SynthAudioSource : public juce::AudioSource
{
public:
    explicit SynthAudioSource (juce::MidiKeyboardState& state) : keyboardState (state)
    {
        for (int i = 0; i < numVoices; i++) {
            synth.addVoice (new SineWaveVoice());
        }
        synth.addSound (new SineWaveSound());
    }

    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override
    {
        juce::ignoreUnused (samplesPerBlockExpected);
        synth.setCurrentPlaybackSampleRate (sampleRate);
    }

    void releaseResources() override
    {
    }

    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override
    {
        bufferToFill.clearActiveBufferRegion();

        juce::MidiBuffer incomingMidi;
        keyboardState.processNextMidiBuffer (incomingMidi, bufferToFill.startSample, bufferToFill.numSamples, true);

        synth.renderNextBlock (*bufferToFill.buffer, incomingMidi, bufferToFill.startSample, bufferToFill.numSamples);
    }

private:
    static constexpr int numVoices = 8;

    juce::MidiKeyboardState& keyboardState;
    juce::Synthesiser synth;
};
