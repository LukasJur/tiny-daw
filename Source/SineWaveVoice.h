#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <cmath>
#include "SineWaveSound.h"

class SineWaveVoice : public juce::SynthesiserVoice
{

public:
    bool canPlaySound (juce::SynthesiserSound* sound) override
    {
        return dynamic_cast<SineWaveSound*> (sound) != nullptr;
    }

    void startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound* sound,
                    int currentPitchWheelPosition) override
    {
        double noteInHertz = juce::MidiMessage::getMidiNoteInHertz (midiNoteNumber);
        double sampleRate = getSampleRate ();

        phaseIncrement = (juce::MathConstants<double>::twoPi * noteInHertz / sampleRate);

        currentPhase = 0.0;
    }

    void stopNote (float velocity, bool allowTailOff) override
    {
        juce::ignoreUnused (velocity, allowTailOff);
        clearCurrentNote ();
    }

    void pitchWheelMoved (int newPitchWheelValue) override
    {
        juce::ignoreUnused (newPitchWheelValue);
    }

    void controllerMoved (int controllerNumber, int newControllerValue) override
    {
        juce::ignoreUnused (controllerNumber, newControllerValue);
    }

    void renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override
    {
        if (!isVoiceActive ()) {
            return;
        }
        for (int sampleIndex = startSample; sampleIndex < startSample + numSamples; sampleIndex++) {
            float sample = sin (currentPhase) * amplitudeCoefficient;

            for (int channelIndex = 0; channelIndex < outputBuffer.getNumChannels (); channelIndex++) {
                outputBuffer.addSample (channelIndex, sampleIndex, sample);
            }

            currentPhase += phaseIncrement;

            if (currentPhase > juce::MathConstants<double>::twoPi) {
                currentPhase -= juce::MathConstants<double>::twoPi;
            }
        }
    }

private:
    const double amplitudeCoefficient = 0.2;
    double phaseIncrement = 0.0;
    double currentPhase = 0.0;
};