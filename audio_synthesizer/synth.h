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

	double oscillate(double dFrequency, double dTime, osc_types eType = osc_types::sine) {
		switch (eType) {
		case osc_types::sine:
			return sin(w(dFrequency) * dTime);
		case osc_types::square:
			return sin(w(dFrequency) * dTime) > 0 ? 1 : -1;
		case osc_types::triangle:
			return asin(sin(w(dFrequency) * dTime)) * (2.0 / PI);
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

			if (dAmplitude <= 0.000)

			return dAmplitude;
		}
	};
}