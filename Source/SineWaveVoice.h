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
        currentLevel = 1.0;
        releaseDecrement = 0.0;
        isReleasing = false;
    }

    void stopNote (float velocity, bool allowTailOff) override
    {
        if (!allowTailOff) {
            clearCurrentNote ();
            return;
        }
        isReleasing = true;
        double releaseSamples = getSampleRate () * releaseInSeconds;
        releaseDecrement = currentLevel / releaseSamples;

        juce::ignoreUnused (velocity);
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
            double sample = sin (currentPhase) * amplitudeCoefficient * currentLevel;

            for (int channelIndex = 0; channelIndex < outputBuffer.getNumChannels (); channelIndex++) {
                outputBuffer.addSample (channelIndex, sampleIndex, (float) sample);
            }

            currentPhase += phaseIncrement;

            if (currentPhase > juce::MathConstants<double>::twoPi) {
                currentPhase -= juce::MathConstants<double>::twoPi;
            }


            if (isReleasing) {
                currentLevel = std::max (currentLevel - releaseDecrement, 0.0);
                if (currentLevel == 0) {
                    clearCurrentNote();
                    break;
                }
            }
        }
    }

private:
    const double amplitudeCoefficient = 0.2;
    double phaseIncrement = 0.0;
    double currentPhase = 0.0;
    double currentLevel = 0.0;
    bool isReleasing = false;
    double releaseDecrement = 0.0;
    double releaseInSeconds = 0.05;
};