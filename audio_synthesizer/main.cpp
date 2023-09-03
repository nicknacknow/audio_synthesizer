#include <iostream>
using namespace std;

#include "noisemaker.h"

#define w(f) (f * 2 * PI)

struct sEnvelopeADSR // Attack, Decay, Sustain & Release Envelope
{
	double dAttackTime;
	double dDecayTime;
	double dReleaseTime;

	double dSustainAmplitude;
	double dStartAmplitude;
	
	double dTriggerPressTime = 0.0; // when key is pressed
	double dTriggerReleaseTime = 0.0; // when key is released, will be set later
	
	bool bNoteOn = false;

	sEnvelopeADSR()
	{
		dAttackTime = 0.1;
		dDecayTime = 0.2;
		dReleaseTime = 0.1;

		dSustainAmplitude = 0.8;
		dStartAmplitude = 1.0;
	}

	// Get the correct amplitude at the requested point in time - dTime is realtime
	double GetAmplitude(double dTime) {
		double dAmplitude = 0.0;

		if (bNoteOn) {
			double dLifeTime = dTime - dTriggerPressTime; // time between current time and time of press

			if (dLifeTime <= dAttackTime) { // lifetime of note is in attack phase - attack is the buildup of a note, so this will be the start of the sound. therefore _approach_ max amplitud
				dAmplitude = (dLifeTime / dAttackTime) * dStartAmplitude; // dLifeTime is smaller than dAttackTime due to if condition, so this will basically be an increasing percentage towards the start amplitude specified.
			}

			if (dLifeTime > dAttackTime && dLifeTime <= (dAttackTime + dDecayTime)) { // next phase is decay - so lifetime of note should be between the end of dAttackTime (start of decay) & end of decay phase (attack time + decay time)
				double dDecayDuration = dLifeTime - dAttackTime; // the current duration of decay
				dAmplitude = (dDecayDuration / dDecayTime) * (dSustainAmplitude - dStartAmplitude) + dStartAmplitude; // basically lerp the amplitude. this would typically decrease the amplitude (as the subtraction is usually negative), so when added to dStartAmplitude, will reduce until decay phase is over.
			}

			if (dLifeTime > (dAttackTime + dDecayTime)) {
				dAmplitude = dSustainAmplitude; // sustain amplitude
			}
		}
		else { // note has been released - so enter release phase
			if (dTriggerReleaseTime != 0)
				dAmplitude = ((dTime - dTriggerReleaseTime) / dReleaseTime) * (0.0 - dSustainAmplitude) + dSustainAmplitude; // slowly reduce amplitude from sustained
		}

		if (dAmplitude <= 0.0001) dAmplitude = 0.0; // should not be negative

		return dAmplitude;
	}

	void NotePressed(double dTimeOn) {
		dTriggerPressTime = dTimeOn;
		bNoteOn = true;
	}

	void NoteReleased(double dTimeOff) {
		dTriggerReleaseTime = dTimeOff;
		bNoteOn = false;
	}
};

sEnvelopeADSR envelope;

enum osc_types {
	sine, square, noise
};

double oscillate(double dFrequency, double dTime, osc_types eType = osc_types::sine) {
	switch (eType) {
	case osc_types::sine:
		return sin(w(dFrequency) * dTime);
	case osc_types::square:
		return sin(w(dFrequency) * dTime) > 0 ? 1 : -1;
	case osc_types::noise:
		return 2 * ((float)rand() / (float)RAND_MAX) - 1;

	default: return 0;
	}
}

// Function used by olcNoiseMaker to generate sound waves
// Returns amplitude (-1.0 to +1.0) as a function of time

int current_note = -1;

double MakeNoise(double dTime)
{

	double dOutput = envelope.GetAmplitude(dTime);
	double test = 0;

	if (current_note == 0) {
		test +=
			(
				+0.1 * oscillate(220, dTime, osc_types::square)
				+ 1 * oscillate(50, dTime, osc_types::sine)
				+ 1 * oscillate(25, dTime, osc_types::sine)
				+ 0.01 * oscillate(500, dTime, osc_types::noise)
				);
	}
	if (current_note == 1) {
		test +=
			(
				+0.1 * oscillate(140, dTime, osc_types::square)
				+ 1 * oscillate(50, dTime, osc_types::sine)
				+ 1 * oscillate(25, dTime, osc_types::sine)
				//+ 0.01 * oscillate(500, dTime, osc_types::noise)
				);
	}

	dOutput *= test;

	return dOutput * 0.2; // master volume
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

	bool prest = false;

	auto function_name = [&](int note) {
			printf("presed\n");
			current_note = note;
			envelope.NotePressed(sound.GetTime());
	};

	while (true) {
		prest = false;
		for (int i = 0; i < 2; i++) {
			if (GetAsyncKeyState((unsigned char)("AS"[i])) & 0x8000) {
				if (current_note == -1) {
					function_name(i);
					current_note = i;
				}
				prest = true;
			}
		}
		if (!prest) {
			if (current_note != -1) {
				printf("relesed\n");
				envelope.NoteReleased(sound.GetTime());
				prest = false;
				current_note = -1;
			}
		}
	}



	std::cin.get();
	return 0;
}


/*
to-do notes:
	* sounds cool if you compress noise - could be done after?
	* low filter pass etc etc
	* in getamplitude function things are done rather linearly - could implement a way to reduce things like oscillator?

*/