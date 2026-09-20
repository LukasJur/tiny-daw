#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

// Describes which notes/channels this sound is valid for. The Synthesiser
// consults this before assigning a voice to a note; it holds no audio state
// itself.
class SineWaveSound : public juce::SynthesiserSound
{
public:
    SineWaveSound() = default;

    bool appliesToNote (int midiNoteNumber) override {
        return true;
    }
    bool appliesToChannel (int midiChannel) override {
        return true;
    }
};
