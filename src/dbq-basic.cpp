#include "plugin.hpp"

struct DbqFilterState {
	float x1 = 0.f, x2 = 0.f, y1 = 0.f, y2 = 0.f;
};

struct Dbq_basic : Module {
	enum ParamId {
		A1_PARAM,
		A2_PARAM,
		B1_PARAM,
		B2_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		AUDIOIN_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		AUDIOOUT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};


	Dbq_basic() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(A1_PARAM, -2.f, 2.f, 1.f, "a1");
		configParam(A2_PARAM, -2.f, 2.f, 0.f, "a2");
		configParam(B1_PARAM, -2.f, 2.f, 0.f, "b1");
		configParam(B2_PARAM, -1.f, 1.f, 0.f, "b2");
		configInput(AUDIOIN_INPUT, "Audio");
		configOutput(AUDIOOUT_OUTPUT, "Filtered Audio");
	}

	void process(const ProcessArgs& args) override {
		int channels = std::max(1, inputs[AUDIOIN_INPUT].getChannels());
		for (int chan = 0; chan < channels; chan++) {
			DbqFilterState &fs = filterStates[chan];
			float input = getInput(AUDIOIN_INPUT).getPolyVoltage(chan);
			float a1 = getParam(A1_PARAM).getValue();
			float a2 = getParam(A2_PARAM).getValue();
			float b1 = getParam(B1_PARAM).getValue();
			float b2 = getParam(B2_PARAM).getValue();

			// Compute output sample
			float output = /* a0 * */ input + a1 * fs.x1 + a2 * fs.x2 - b1 * fs.y1 - b2 * fs.y2;
			output = clamp(output, -11.7f, 11.7f);

			// Shift state variables if filter is still stable
			if (isfinite(output)) {
				fs.x2 = fs.x1;
				fs.x1 = input;
				fs.y2 = fs.y1;
				fs.y1 = output;
			} else {
				// TODO blink unstable LED
				output = 0.f;
				fs.x2 = 0.f;
				fs.x1 = 0.f;
				fs.y2 = 0.f;
				fs.y1 = 0.f;
			}

			getOutput(AUDIOOUT_OUTPUT).setVoltage(output, chan);
		}

		getOutput(AUDIOOUT_OUTPUT).setChannels(channels);
	}

	private:
	DbqFilterState filterStates[PORT_MAX_CHANNELS];
};

struct Dbq_basicWidget : ModuleWidget {
	Dbq_basicWidget(Dbq_basic* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/dbq-basic.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(12.458, 22.28)), module, Dbq_basic::A1_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(12.458, 38.985)), module, Dbq_basic::A2_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(12.458, 59.167)), module, Dbq_basic::B1_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(12.458, 75.872)), module, Dbq_basic::B2_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(12.555, 96.974)), module, Dbq_basic::AUDIOIN_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(12.676, 113.717)), module, Dbq_basic::AUDIOOUT_OUTPUT));
	}
};


Model* modelDbq_basic = createModel<Dbq_basic, Dbq_basicWidget>("dbq-basic");