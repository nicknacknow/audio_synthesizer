#pragma once
#include "noisemaker.h"

#define w(f) (f * 2 * PI)

namespace synth {
	struct note {
		int id = -1; // position in scale
		double pressed = 0.0; // time note was pressed
		double released = 0.0; // time note was released
		bool active = false;
		int channel = -1;
	};

	enum osc_types {
		sine, square, triangle, saw, noise
	};

	double oscillate(double dFrequency, double dTime, osc_types eType = osc_types::sine, double dLFOFrequency = 0.0, double dLFOAmplitude = 0.0) { // LFO = low frequency oscillator
		double base_frequency = w(dFrequency) * dTime
			+ dLFOAmplitude * dFrequency * sin(w(dLFOFrequency) * dTime);
		
		switch (eType) {
		case osc_types::sine:
			return sin(base_frequency);
		case osc_types::square:
			return sin(base_frequency) > 0 ? 1 : -1;
		case osc_types::triangle:
			return asin(sin(base_frequency)) * (2.0 / PI);
		case osc_types::saw:
		{
			double dOutput = 0;
			for (double n = 1; n < 50; n++)
				dOutput += sin(n * base_frequency) / n;
			return dOutput * (2.0 / PI);
		}
		case osc_types::noise:
			return 2 * ((float)rand() / (float)RAND_MAX) - 1;

		default: return 0;
		}
	}

	struct envelope {
		virtual double amplitude(double dTime, double dTimePressed, double dTimeReleased) = 0;
	};

	struct envelope_adsr : public envelope {
		double dAttackTime;
		double dDecayTime;
		double dReleaseTime;
		double dSustainAmplitude;
		double dStartAmplitude;

		envelope_adsr()
		{
			dAttackTime = 0.1;
			dDecayTime = 0.2;
			dReleaseTime = 0.1;

			dSustainAmplitude = 0.8;
			dStartAmplitude = 1.0;
		}

		virtual double amplitude(double dTime, double dTimePressed, double dTimeReleased) {
			double dAmplitude = 0.0;

			if (dTimePressed > dTimeReleased) { // note is on
				double dDuration = dTime - dTimePressed; // duration of note so far

				if (dDuration <= dAttackTime)
					dAmplitude = (dDuration / dAttackTime) * dStartAmplitude;

				if (dDuration > dAttackTime && dDuration <= (dAttackTime + dDecayTime))
					dAmplitude = ((dDuration - dAttackTime) / dDecayTime) * (dSustainAmplitude - dStartAmplitude) + dStartAmplitude;

				if (dDuration > (dAttackTime + dDecayTime))
					dAmplitude = dSustainAmplitude;
			}
			else { // note is off
				double dDuration = dTimeReleased - dTimePressed;
				double dReleaseAmplitude = 0.0;

				// pick up from where we left off
				if (dDuration <= dAttackTime)
					dReleaseAmplitude = (dDuration / dAttackTime) * dStartAmplitude;

				if (dDuration > dAttackTime && dDuration <= (dAttackTime + dDecayTime))
					dReleaseAmplitude = ((dDuration - dAttackTime) / dDecayTime) * (dSustainAmplitude - dStartAmplitude) + dStartAmplitude;

				if (dDuration > (dAttackTime + dDecayTime))
					dReleaseAmplitude = dSustainAmplitude;

				dAmplitude = ((dTime - dTimeReleased) / dReleaseTime) * (-dReleaseAmplitude) + dReleaseAmplitude;
			}

			if (dAmplitude <= 0.0001) dAmplitude = 0.0; // should not be negative

			return dAmplitude;
		}
	};

	double env(double dTime, envelope& env, double dTimePressed, double dTimeReleased) {
		return env.amplitude(dTime, dTimePressed, dTimeReleased);
	}

	struct instrument_base {
		double dVolume;
		synth::envelope_adsr env;
		virtual double sound(double dTime, synth::note n, bool& bNoteFinished) = 0;
	};

	struct instrument_harmonica : public instrument_base {
		instrument_harmonica()
		{
			env.dAttackTime = 0.05;
			env.dDecayTime = 1.0;
			env.dSustainAmplitude = 0.95;
			env.dReleaseTime = 0.1;

			dVolume = 1.0;
		}

		virtual double sound(const double dTime, synth::note n, bool& bNoteFinished)
		{
			double dAmplitude = synth::env(dTime, env, n.pressed, n.released);
			if (dAmplitude <= 0.0) bNoteFinished = true;

			double dSound =
				+ 0.1 * oscillate(220, n.pressed - dTime, osc_types::square);

			return dAmplitude * dSound * dVolume;
		}
	};

	struct instrument_synth1 : public instrument_base {
		instrument_synth1()
		{
			env.dAttackTime = 0.1;
			env.dDecayTime = 0.2;
			env.dSustainAmplitude = 0.8;
			env.dReleaseTime = 0.1;

			dVolume = 0.8;
		}

		virtual double sound(const double dTime, synth::note n, bool& bNoteFinished)
		{
			double dAmplitude = synth::env(dTime, env, n.pressed, n.released);
			if (dAmplitude <= 0.0) bNoteFinished = true;

			double dSound = (
				+0.1 * oscillate(220, dTime, osc_types::square)
				+ 1 * oscillate(50, dTime, osc_types::sine)
				+ 1 * oscillate(25, dTime, osc_types::sine)
				+ 0.01 * oscillate(500, dTime, osc_types::noise)
				);

			return dAmplitude * dSound * dVolume;
		}
	};

	struct instrument_synth2 : public instrument_base {
		instrument_synth2()
		{
			env.dAttackTime = 0.1;
			env.dDecayTime = 0.2;
			env.dSustainAmplitude = 0.8;
			env.dReleaseTime = 0.1;

			dVolume = 0.8;
		}

		virtual double sound(const double dTime, synth::note n, bool& bNoteFinished)
		{
			double dAmplitude = synth::env(dTime, env, n.pressed, n.released);
			if (dAmplitude <= 0.0) bNoteFinished = true;

			double dSound = (
				+ 0.1 * oscillate(140, dTime, osc_types::square)
				+ 1 * oscillate(50, dTime, osc_types::sine)
				+ 1 * oscillate(25, dTime, osc_types::sine)
				+ 0.01 * oscillate(500, dTime, osc_types::noise)
				);

			return dAmplitude * dSound * dVolume;
		}
	};

	struct instrument_synth3 : public instrument_base {
		instrument_synth3()
		{
			env.dAttackTime = 0.1;
			env.dDecayTime = 0.2;
			env.dSustainAmplitude = 0.8;
			env.dReleaseTime = 0.1;

			dVolume = 0.8;
		}

		virtual double sound(const double dTime, synth::note n, bool& bNoteFinished)
		{
			/*double dAmplitude = synth::env(dTime, env, n.pressed, n.released);
			if (dAmplitude <= 0.0) bNoteFinished = true;

			double dSound = (
				+ 0.6 * oscillate(550, dTime, osc_types::triangle)
				+ 0.3 * oscillate(200, dTime, osc_types::sine)
				+ .2 * oscillate(50, dTime, osc_types::square)
				+ .2 * oscillate(25, dTime, osc_types::square)
				+ 0.01 * oscillate(500, dTime, osc_types::noise)
				);

			return dAmplitude * dSound * dVolume;*/

			double dAmplitude = synth::env(dTime, env, n.pressed, n.released);
			if (dAmplitude <= 0.0) bNoteFinished = true;

			// Generate the raw sound
			double dSound =
				+0.1 * oscillate(220, n.pressed - dTime, osc_types::square);


			// Adjust amplitude and volume
			dSound *= dAmplitude * dVolume;

			return dSound;
		}
	};

	struct instrument_ethereal_pad : public instrument_base {
		instrument_ethereal_pad() {
			env.dAttackTime = 2.0;        // Slow attack for a gradual onset
			env.dDecayTime = 4.0;         // A slow decay for a fading effect
			env.dSustainAmplitude = 0.5; // Sustain at a moderate level
			env.dReleaseTime = 5.0;       // Slow release for a smooth fade-out
			dVolume = 0.5;                // Adjust the volume to your liking
		}

		virtual double sound(const double dTime, synth::note n, bool& bNoteFinished) {
			double dAmplitude = synth::env(dTime, env, n.pressed, n.released);
			if (dAmplitude <= 0.0) bNoteFinished = true;

			// Generate the raw sound with a combination of sine and triangle waves
			double dSound =
				+0.5 * oscillate(220, dTime, osc_types::sine)
				+ 0.3 * oscillate(330, dTime, osc_types::sine)
				+ 0.2 * oscillate(440, dTime, osc_types::triangle);

			// Apply amplitude and volume
			dSound *= dAmplitude * dVolume;

			return dSound;
		}
	};

	struct instrument_celestial_pad : public instrument_base {
		instrument_celestial_pad() {
			env.dAttackTime = 3.0;         // Slow attack for gradual onset
			env.dDecayTime = 6.0;          // Slow decay for a fading effect
			env.dSustainAmplitude = 0.6;  // Sustain at a moderate level
			env.dReleaseTime = 8.0;        // Slow release for a smooth fade-out
			dVolume = 0.5;                 // Adjust the volume to your liking
		}

		virtual double sound(const double dTime, synth::note n, bool& bNoteFinished) {
			double dAmplitude = synth::env(dTime, env, n.pressed, n.released);
			if (dAmplitude <= 0.0) bNoteFinished = true;

			// Generate the celestial pad sound with a combination of sine and triangle waves
			double dSound =
				+0.4 * oscillate(150, dTime, osc_types::sine)
				+ 0.3 * oscillate(220, dTime, osc_types::sine)
				+ 0.2 * oscillate(330, dTime, osc_types::triangle);

			// Apply amplitude and volume
			dSound *= dAmplitude * dVolume;

			return dSound;
		}
	};

	struct instrument_classic_piano : public instrument_base {
		instrument_classic_piano() {
			env.dAttackTime = 0.1;       // Quick attack for a piano-like response
			env.dDecayTime = 0.4;        // Moderate decay for a natural piano sound
			env.dSustainAmplitude = 0.6; // Sustain at a moderate level
			env.dReleaseTime = 0.3;      // Medium release for smooth note endings
			dVolume = 0.8;               // Adjust the volume to your liking
		}

		virtual double sound(const double dTime, synth::note n, bool& bNoteFinished) {
			double dAmplitude = synth::env(dTime, env, n.pressed, n.released);
			if (dAmplitude <= 0.0) bNoteFinished = true;

			// Generate a classic piano sound with a combination of sine waves
			double dSound =
				+0.8 * oscillate(440, dTime, osc_types::sine)
				+ 0.2 * oscillate(880, dTime, osc_types::sine);

			// Apply amplitude and volume
			dSound *= dAmplitude * dVolume;

			return dSound;
		}
	};

	struct instrument_epic_choir : public instrument_base {
		instrument_epic_choir() {
			env.dAttackTime = 2.0;        // Slow attack for a grand choir entrance
			env.dDecayTime = 3.0;         // Longer decay for expressive dynamics
			env.dSustainAmplitude = 0.6; // Sustain at a moderate level
			env.dReleaseTime = 2.0;       // Medium release for smooth note endings
			dVolume = 0.7;                // Adjust the volume to your liking
		}

		virtual double sound(const double dTime, synth::note n, bool& bNoteFinished) {
			double dAmplitude = synth::env(dTime, env, n.pressed, n.released);
			if (dAmplitude <= 0.0) bNoteFinished = true;

			// Generate an epic choir-like sound with a combination of sine waves
			double dSound =
				+0.7 * oscillate(220, dTime, osc_types::sine)
				+ 0.3 * oscillate(330, dTime, osc_types::sine)
				+ 0.2 * oscillate(440, dTime, osc_types::saw)  // Add a sawtooth wave for a vibrant texture
				+ 0.1 * oscillate(550, dTime, osc_types::triangle);  // Add a triangle wave for variation

			// Apply amplitude and volume
			dSound *= dAmplitude * dVolume;

			return dSound;
		}
	};

	struct instrument_analog_pad : public instrument_base {
		double dVolume;

		instrument_analog_pad() {
			dVolume = 0.6;  // Adjust the volume to your liking
			env.dAttackTime = 2.0; // Adjust the attack time (in seconds)
			env.dDecayTime = 3.0;  // Adjust the decay time (in seconds)
			env.dSustainAmplitude = 0.6;
			env.dReleaseTime = 2.0; // Adjust the release time (in seconds)
		}

		virtual double sound(const double dTime, synth::note n, bool& bNoteFinished) {
			double dAmplitude = synth::env(dTime, env, n.pressed, n.released);
			if (dAmplitude <= 0.0) bNoteFinished = true;

			// Create an evolving pad sound using multiple oscillators and modulation
			double dSound = (
				+0.4 * oscillate(220, dTime, osc_types::sine)
				+ 0.3 * oscillate(440, dTime, osc_types::sine)
				+ 0.2 * oscillate(880, dTime, osc_types::sine)
				+ 0.1 * oscillate(1760, dTime, osc_types::sine)
				);

			// Apply amplitude and volume
			dSound *= dAmplitude * dVolume;

			return dSound;
		}
	};

}