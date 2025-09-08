Chorus F - Fairlight CMI Chorus Emulator Plugin atleast that is the idea, lol. Chorus F is not Fairlight.

Chorus F is an open-source audio plugin that set out to recreate the iconic chorus effect of the Fairlight CMI
, a legendary 1980s sampler and synthesizer. It is in fact one of the earliest digital effects processors in a computer. It had the chorus 
as well as pitch shift and reverb that I am also hoping to start working on. 
This chorus is very grainy and 8 bit audio experience, it almost
has a distortion aspect to it but it felt unique in what sounds it was making. When combined with another chorus like fl studio's
vintage chorus it created a nice character in doubling up the choruses. Because the idea is to process at 8 bit there is a bit of a
bit crush type quality to the sound.

Built with JUCE 8.0.9, its offered as a VST3 , delivering vintage grimmer and depth to synths, guitars, vocals, and more. Ideal for music producers, audio engineers, and plugin developers, Chorus F combines retro 8-bit aesthetics with modern performance for music production and sound design.

Features an attempt at Fairlight architecture:Three-tap delay system with linear interpolation, mimicking the CMI’s chorus
64-entry sine table LFO for true-to-era modulation. Some tuning may need apply.

Internal 32 kHz processing via oversampling for vintage fidelity.

Parameters (0–255, Fairlight Style):Mix: 0 (dry) to 255 (wet), default 128 (50/50)
Depth: 0 to 255, default 40 (moderate modulation)
Center Delay: 0 to 255 (0–40 ms at 32 kHz), default 80 (12 ms)
LFO Rate: 0 to 255 (0–6 Hz), default 4 (0.3 Hz)
Phase Spread: 0 to 255, default 0 (mono-compatible)
Feedback: 0 to 255, default 0 (no feedback)
Gain: 0 to 255, default 255 (full output)

Formats: VST3 for DAWs (Ableton Live, Reaper, FL Studio) and Standalone
Performance: Real-time safe with soft clipping to prevent feedback runaway
GUI: Retro green phosphor interface inspired by the Fairlight CMI colors. 





Prerequisites
To build Chorus F you need:JUCE Framework: Version 8.0.9 Download JUCE here https://juce.com/

C++ Compiler: Visual Studio 2022 Community (Windows) or compatible IDE with C++17 support
Operating System: Windows 11  (tested) 

DAW: Compatible with VST3 (tested in FL Studio)


Copy the VST3 plugin to your DAW’s VST3 folder (e.g., C:\Program Files\Common Files\VST3).


Adjust parameters (mix, depth, center delay, LFO rate, phase spread, feedback, gain) using rotary sliders.

Experiment with settings for grainy synths,  guitars, distorted vocals. It does have a bit of an 80s feel.

 
 

 
JuceLibraryCode/: JUCE module includes 
Development Notes JUCE Version: Built with JUCE 8.0.9 for compatibility

Dependencies: Uses JUCE modules (juce_audio_basics, juce_audio_devices, juce_audio_formats, juce_audio_plugin_client, 
juce_audio_processors, juce_core, juce_dsp, juce_events, juce_graphics, juce_gui_basics, juce_gui_extra)

Fairlight Emulation: Implements a three-tap delay with an 8-bit sine table LFO and 32 kHz oversampling
Limitations: Supports mono/stereo only; no advanced features like preset management (planned for future versions)

ContributingContributions are welcome! To contribute:Fork the repository: github.com/WilliamAshley2019/ChorusF
Create a feature branch: git checkout -b feature/your-feature
Commit changes: git commit -m "Add your feature"
Push to the branch: git push origin feature/your-feature
Open a pull request

Please follow JUCE coding conventions and test changes thoroughly.
LicenseLicensed under the GPLv3  License.
The JUCE framework is subject to GPLv3 or commercial licensing, and the VST3 SDK has its own terms.
Steinberg VST3 SDK. VST is a trademark of Steinberg Media Technologies GmbH, registered in Europe and other countries.

About the Developer
William Ashley is an aspiring hobbyist audio engineer and plugin developer passionate about vintage synths and modern music production.
Connect with me:

Portfolio https://12264447666william.wixsite.com/williamashley

GitHub https://github.com/WilliamAshley2019/

Hopp.bio https://www.hopp.bio/william-ashley

Medium https://medium.com/@12264447666.williamashley

YouTube https://www.youtube.com/@WilliamAshleyOnline


AcknowledgmentsBuilt with the JUCE framework
Inspired by the Fairlight CMI’s iconic chorus effect, the actual .eff files were not modeled but there was an attempt to mirrow
the architecture. Thus this is not an emulation but it is a concept of modeling something using some of the same ideas as the 
Fairlight CMI, perhaps in future copies I can get closer to the real thing. A big thanks to Peter Vogel and Kim Ryrie for pioneering 
computer audio technologies like the Fairlight CMI that has passed down not only some great music but insight into DSP technologies. 
Its not a replacement for the real thing, but who has the real thing. OH right, also check out the actual plugins from companies like 
arturia if you would like a more authentic emulation. This was just a concept build with no attemptto mirror the original just use some of the same framework.
 
 Chorus F Developer: William Ashley
 
