#include <juce_audio_utils/juce_audio_utils.h>
#include "SynthAudioSource.h"

//==============================================================================
// MainComponent is where the audio callback lives. AudioAppComponent gives us
// three methods the audio device calls on its own thread:
//   prepareToPlay   - called once before playback starts, and again if the
//                      sample rate or block size changes
//   getNextAudioBlock - called repeatedly, must fill the output buffer;
//                        this is the real-time audio thread, so no locks,
//                        no allocation, no logging in here
//   releaseResources  - called once when playback stops
class MainComponent : public juce::AudioAppComponent
{
public:
    MainComponent()
        : synthAudioSource (keyboardState),
          keyboardComponent (keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
    {
        addAndMakeVisible (keyboardComponent);
        setSize (600, 400);
        setAudioChannels (0, 2);
    }

    ~MainComponent() override
    {
        // Every AudioAppComponent subclass must call this in its destructor,
        // before its own members are torn down, so the audio thread can't
        // call back into a half-destroyed object.
        shutdownAudio();
    }

    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override
    {
        synthAudioSource.prepareToPlay (samplesPerBlockExpected, sampleRate);
    }

    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override
    {
        synthAudioSource.getNextAudioBlock (bufferToFill);
    }

    void releaseResources() override
    {
        synthAudioSource.releaseResources();
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    }

    void resized() override
    {
        keyboardComponent.setBounds (getLocalBounds().removeFromBottom (120));
    }

private:
    // Declaration order matters: members initialize top to bottom, and both
    // of the following hold a reference to keyboardState.
    juce::MidiKeyboardState keyboardState;
    SynthAudioSource synthAudioSource;
    juce::MidiKeyboardComponent keyboardComponent;
};

//==============================================================================
// TinyDAWApplication owns the OS-level application lifecycle: it creates the
// window on launch and tears everything down on quit. This is separate from
// MainComponent because the app lifecycle (one instance, one window) is a
// different concern from what's drawn/played inside that window.
class TinyDAWApplication : public juce::JUCEApplication
{
public:
    TinyDAWApplication() = default;

    const juce::String getApplicationName() override       { return JUCE_APPLICATION_NAME_STRING; }
    const juce::String getApplicationVersion() override    { return JUCE_APPLICATION_VERSION_STRING; }
    bool moreThanOneInstanceAllowed() override              { return true; }

    void initialise (const juce::String& commandLine) override
    {
        juce::ignoreUnused (commandLine);
        mainWindow.reset (new MainWindow (getApplicationName()));
    }

    void shutdown() override
    {
        mainWindow = nullptr;
    }

    void systemRequestedQuit() override
    {
        quit();
    }

    //==============================================================================
    // A DocumentWindow is the native OS window (title bar, close button, etc.)
    // that hosts our MainComponent.
    class MainWindow : public juce::DocumentWindow
    {
    public:
        explicit MainWindow (juce::String name)
            : DocumentWindow (name,
                               juce::Desktop::getInstance().getDefaultLookAndFeel()
                                   .findColour (juce::ResizableWindow::backgroundColourId),
                               DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar (true);
            setContentOwned (new MainComponent(), true);
            centreWithSize (getWidth(), getHeight());
            setVisible (true);
        }

        void closeButtonPressed() override
        {
            JUCEApplication::getInstance()->systemRequestedQuit();
        }
    };

private:
    std::unique_ptr<MainWindow> mainWindow;
};

//==============================================================================
// Expands to a main() that constructs TinyDAWApplication and runs the
// message loop.
START_JUCE_APPLICATION (TinyDAWApplication)
