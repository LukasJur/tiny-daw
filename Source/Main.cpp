#define _USE_MATH_DEFINES

#include <juce_audio_utils/juce_audio_utils.h>
#include <cmath>

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
    {
        setSize (600, 400);

        // Ask for 0 input channels, 2 output channels (stereo).
        // This also triggers an OS microphone-permission prompt on some
        // platforms if input channels are requested, which we don't need yet.
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
        phaseIncrement = (juce::MathConstants<double>::twoPi * frequencyInHz / sampleRate);
    }

    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override
    {
       for (int sampleIndex = bufferToFill.startSample; sampleIndex < bufferToFill.startSample + bufferToFill.numSamples; sampleIndex++) {
        float sample = sin(currentPhase) * amplitudeCoefficient;
        
        for (int channelIndex = 0; channelIndex < bufferToFill.buffer->getNumChannels(); channelIndex++) {
            bufferToFill.buffer->setSample(channelIndex, sampleIndex, sample);
        } 
        
        currentPhase += phaseIncrement;
        if (currentPhase > juce::MathConstants<double>::twoPi) {
            currentPhase -= (juce::MathConstants<double>::twoPi);
        }
       } 
    }

    void releaseResources() override
    {
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    }

    void resized() override
    {
    }
private:
    const float frequencyInHz = 440.0; 
    const float amplitudeCoefficient = 0.2;
    float phaseIncrement = 0.0;
    float currentPhase = 0.0;  
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
