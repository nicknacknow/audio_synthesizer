#include "synth.h"

#include <api/fftw3.h>


std::vector<synth::note> vecNotes;
std::mutex muxNotes;

synth::instrument_harmonica instHarm;
synth::instrument_synth1 instSynth1;
synth::instrument_synth2 instSynth2;
synth::instrument_synth3 instSynth3;

synth::instrument_ethereal_pad instEtherealPad;
synth::instrument_celestial_pad instCelestialPad;
synth::instrument_classic_piano instClassicPiano;
synth::instrument_epic_choir instEpicChoir;

synth::instrument_analog_pad instAnalogPad;

// thx olc :)
typedef bool(*lambda)(synth::note const& item);
template<class T>
void safe_remove(T& v, lambda f)
{
	auto n = v.begin();
	while (n != v.end())
		if (!f(*n))
			n = v.erase(n);
		else
			++n;
}

// Filter coefficients (customize for your desired cutoff frequency)
/*const std::vector<double> filterCoeff = {
	-0.1, -0.2, 0.8, -0.2, -0.1, // First high-pass filter
	0.2, 0.4, -1.6, 0.4, 0.2    // Second high-pass filter
};*/

// Function used by olcNoiseMaker to generate sound waves
// Returns amplitude (-1.0 to +1.0) as a function of time
	double cutoffFreq = 100.0;  // Adjust this cutoff frequency as needed
double MakeNoise(int nChannel, double dTime)
{
	std::unique_lock<mutex> lm(muxNotes);
	double dMixedOutput = 0.0;

	// Filter parameters
	double RC = 1.0 / (2.0 * PI * cutoffFreq);
	double dt = 1.0 / 44100.0;  // Sample rate
	double alpha = RC / (RC + dt);

	static double prevSample = 0.0;

	for (auto& n : vecNotes) {
		bool bNoteFinished = false;
		double dSound = 0;
		if (n.id == 0)
			dSound = instSynth1.sound(dTime, n, bNoteFinished);
		if (n.id == 1)
			dSound = instAnalogPad.sound(dTime, n, bNoteFinished);
		if (n.id == 2)
			dSound = instEtherealPad.sound(dTime, n, bNoteFinished);
		if (n.id == 3)
			dSound = instCelestialPad.sound(dTime, n, bNoteFinished);
		if (n.id == 4)
			dSound = instEpicChoir.sound(dTime, n, bNoteFinished);


		// Apply the high-pass filter
		dSound -= alpha * (dSound - prevSample);
		prevSample = dSound;


		dMixedOutput += dSound;

		if (bNoteFinished && n.released > n.pressed)
			n.active = false;
	}

	// wow ! modern c++ overload!! !!
	safe_remove<std::vector<synth::note>>(vecNotes, [](synth::note const& item) {return item.active; });

	return dMixedOutput * 0.2;
}


int main() {
	// Get all sound hardware
	vector<wstring> devices = olcNoiseMaker<short>::Enumerate();

	// Display findings
	for (auto d : devices) wcout << "Found Output Device: " << d << endl;
	wcout << "Using Device: " << devices[0] << endl;

	// Create sound machine!!
	olcNoiseMaker<short> sound(devices[0], 44100, 1, 8, 512);

	// Link noise function with sound machine
	sound.SetUserFunction(MakeNoise);

	while (true) {
		if (GetAsyncKeyState(VK_UP) & 1) cutoffFreq += 10;
		if (GetAsyncKeyState(VK_DOWN) & 1) cutoffFreq -= 10;

		for (int i = 0; i < 5; i++) {
			short nKeyState = GetAsyncKeyState((unsigned char)("ASDFE"[i]));
			double dTimeNow = sound.GetTime();

			muxNotes.lock();
			auto noteFound = find_if(vecNotes.begin(), vecNotes.end(), [&i](synth::note const& item) { return item.id == i; });
			if (noteFound == vecNotes.end()) { // note not found in vector
				if (nKeyState & 0x8000) {
					// create note
					synth::note n;
					n.id = i;
					n.pressed = dTimeNow;
					n.channel = 1;
					n.active = true;

					// add note to vector
					vecNotes.emplace_back(n);
				}
			}
			else { // note is in vector
				if (nKeyState & 0x8000) { // key is being held
					if (noteFound->released > noteFound->pressed) { // key has been pressed again during release phase
						noteFound->pressed = dTimeNow;
						noteFound->active = true;
					}
				}
				else { // key has been released, so switch off
					if (noteFound->released < noteFound->pressed)
						noteFound->released = dTimeNow;
				}
			}
			muxNotes.unlock();
		}

		wcout << "\rNotes: " << vecNotes.size() << "          cut off frequency: " << cutoffFreq << "    ";
	}



	std::cin.get();
	return 0;
}


/*
to-do notes:
	* sounds cool if you compress noise - could be done after?
	* low filter pass etc etc
	* in getamplitude function things are done rather linearly - could implement a way to reduce things like oscillator?



	* make an echo thingy... should be simple .... i meant fade out here but echo would be cool too


	* create struct / class where when key is held down a filter is applied to the instruments that are 'attached' to it
		*	e.g. this can help with a fade-in 


	!! for instruments, add a fixed duration for some. olc did this in part 4 so i can see how he did it there
*/