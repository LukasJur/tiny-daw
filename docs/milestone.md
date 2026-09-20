# Tiny DAW milestones

Quick reference for where the project stands. Each milestone is meant to be small enough to fully understand before moving on.

## Done

1. **Skeleton app** - `AudioAppComponent` + `JUCEApplication`/`DocumentWindow`; opens a window, outputs silence.
2. **One oscillator, one hardcoded note** - sine wave via a phase accumulator in `getNextAudioBlock`.
   - Concepts: audio callback, `prepareToPlay`, phase increment = 2π · frequency / sampleRate, real-time rules (no locks/allocation on the audio thread).
3. **Polyphonic synth (core)** - `Synthesiser` with 8 `SineWaveVoice`s and one `SineWaveSound`, driven by an on-screen `MidiKeyboardComponent` (mouse and computer keyboard) through a shared `MidiKeyboardState`.
   - Files: `SineWaveSound.h`, `SineWaveVoice.h`, `SynthAudioSource.h`, `Main.cpp`.
   - Concepts: voice vs sound, voice stealing, `addSample` (voices sum) vs `setSample` (overwrites), `isVoiceActive()` early return, `static constexpr`, member initialization order.

## In progress

3b. **Note envelope (ADSR)** - remove the click when a key is released.
   - Terms: *envelope* = gain curve (0 to 1) over the life of a note; *attack* = time to rise to the peak; *decay* = time to fall from the peak to the sustain level; *sustain* = a level (not a time) held while the key is down; *release* = time to fade to silence after key-up.
   - The peak is fixed at 1.0. Loudness = `sin(phase) * amplitudeCoefficient * velocity * envelope`.
   - Steps: (a) hand-rolled linear release; honor `allowTailOff` in `stopNote`, and call `clearCurrentNote()` only once the fade reaches 0; (b) add attack; (c) swap in `juce::ADSR`.
   - Also open: use `velocity` in `startNote`.

   Options for the envelope itself

1. Hand-rolled: a double level that steps by a small amount each sample during release. It's the simplest way to understand what is happening.
2. juce::ADSR: JUCE's built-in class. You give it a sample rate and ADSR::Parameters (attack, decay, sustain, release in seconds), call noteOn() and noteOff(), and read getNextSample() per sample. It also reports isActive(), which tells you when the release has finished and you can call clearCurrentNote().

Since you're learning, I'd suggest doing the hand-rolled linear release first so you see the mechanics, and then swapping in juce::ADSR. A release of about 50 ms is enough to kill the click, and around 200–500 ms sounds musical.

## Planned

4. **One effect** - a filter or delay via `juce::dsp`.
5. **Piano roll** - a note data model plus a `Component` that draws and edits it (GUI thread only, no audio yet).
6. **Playback engine** - piano roll notes turned into sample-accurate scheduled MIDI events feeding the synth.
7. **Recording** - capture output audio to a buffer and write it to a WAV file (`AudioFormatWriter`).
8. **Project save/load** - project file format (JUCE `ValueTree` + XML) storing synth params and piano roll notes.

## Ideas / later

- Extra waveforms (saw, square) as additional sounds/voices.
- Real MIDI hardware input (same `noteOn`/`noteOff` path as the on-screen keyboard).
- Parameter sliders (gain, cutoff, envelope times).

## Housekeeping

- Not yet under git: run `git init`, add a `.gitignore` for `build/`, then commit.
- `README.md` milestone list is slightly out of date (milestones 2-3 are done or nearly done).
