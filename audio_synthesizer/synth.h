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
		sine, square, triangle, noise
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
}