# Tiny DAW

A learning project: build a minimal digital audio workstation from scratch in C++ with [JUCE](https://juce.com/), to actually understand how real-time audio, MIDI, and a DAW's moving parts fit together — not just use one.

Target feature set:
- Open / save project files
- One synth with basic effects
- Piano roll
- Record / apply effects

## Development milestones

Roughly in build order — each one is meant to be small enough to fully understand before moving to the next.

1. **Skeleton app** — a JUCE `AudioAppComponent` + `JUCEApplication`/`DocumentWindow` that opens a window and outputs silence. ✅ done
2. **One oscillator, one hardcoded note** — a sine wave at a fixed pitch, written directly in `getNextAudioBlock`, to learn the audio callback and a phase-accumulator oscillator. 🚧 in progress
3. **Polyphonic synth** — JUCE's `Synthesiser` / `SynthesiserVoice` / `SynthesiserSound`, driven by real MIDI input.
4. **One effect** — a filter or delay via `juce::dsp`.
5. **Piano roll** — a note data model plus a `Component` that draws/edits it (GUI-thread only, no audio yet).
6. **Playback engine** — piano roll notes turned into sample-accurate scheduled MIDI events feeding the synth.
7. **Recording** — capture output audio to a buffer and write it to a WAV file (`AudioFormatWriter`).
8. **Project save/load** — a project file format (JUCE `ValueTree` + XML) storing synth params and piano roll notes.

See [`docs/cpp-crash-course.md`](docs/cpp-crash-course.md) for the C++ concepts this project has surfaced so far (value semantics, RAII, smart pointers, etc.), written for a Java/C background.

## Prerequisites

- **Visual Studio Build Tools** (or full Visual Studio) with the **"Desktop development with C++"** workload — provides the MSVC compiler and a CMake + Ninja install bundled under the VS install path.
- **Git** — required at CMake *configure* time, since `CMakeLists.txt` uses `FetchContent` to clone JUCE straight from GitHub (no manual JUCE install needed).

This repo doesn't assume `cmake` is on your `PATH` — `build.ps1` locates it itself (see below), and CMake's own generator step locates your MSVC toolset itself. Neither this README nor the scripts hardcode a machine-specific install path.

## Configure (first time only, or after editing `CMakeLists.txt`)

From the `tiny-daw` folder, using whichever `cmake` you have (on `PATH`, or via a **"Developer PowerShell for VS"** shortcut from the Start menu, which puts VS's tools on `PATH` for that session):

```powershell
cmake -S . -B build -A x64
```

This clones JUCE (via `FetchContent`) and generates a build for your default Visual Studio version. If CMake doesn't have write access to your default generator (e.g. multiple VS versions installed) list the available ones with `cmake --help` and pass one explicitly with `-G "Visual Studio 17 2022"` (or whichever matches what's installed).

The first configure takes a few minutes; only needed again if `CMakeLists.txt` changes.

## Build & run (day-to-day)

```powershell
.\build.ps1        # build only
.\build.ps1 -Run   # build, then launch if the build succeeded
```

`build.ps1` finds `cmake` itself — first on `PATH`, then by asking `vswhere.exe` (Microsoft's own tool for locating Visual Studio installs) where VS lives, rather than hardcoding a version/edition-specific path that would only work on one machine.

Or run the built exe manually:

```powershell
& "build\TinyDAW_artefacts\Debug\Tiny DAW.exe"
```

A window titled "Tiny DAW" should open; closing it quits the app.
