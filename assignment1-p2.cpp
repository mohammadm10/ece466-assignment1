#include <systemc.h>
#include <iostream>

//Multiplier module
SC_MODULE(FloatMultiplier) {
	sc_in<float> a;
	sc_in<float> b;
	sc_out<float> product;

	void multiply() {
		float as = a.read();
		float bs = b.read();
		product.write(a.read() * b.read());
	}

	SC_CTOR(FloatMultiplier) {
		SC_METHOD(multiply);
		sensitive << a << b;
	}
};

//Adder module
SC_MODULE(FloatAdder) {
	sc_in<float> a;
	sc_in<float> b;
	sc_out<float> sum;

	void add() {
		float as = a.read();
		float bs = b.read();
		sum.write(as + bs);
	}

	SC_CTOR(FloatAdder) {
		SC_METHOD(add);
		sensitive << a << b;
	}
};

//Clocked register
SC_MODULE(ClkReg) {
	sc_in<float> input;
	sc_out<float> output;
	sc_in<bool> res;
	sc_in<bool> clk;
	float update_register_data = 0;

	void update_register() {
		if (res) {
			update_register_data = 0;
		}
		if (clk.posedge()) {
			float in = input.read();
			update_register_data = in;
		}
		output.write(update_register_data);
	}

	SC_CTOR(ClkReg) {
		SC_METHOD(update_register);
		sensitive << input << res << clk.pos();
	}
};

SC_MODULE(DigitalFilter) {
	sc_in<float> x; //input
	sc_out<float> y; //output

	sc_in<bool> res; //reset
	sc_in_clk clk; //clock

	sc_signal<float> a0, a1, a2, a3, a4; //Coefficients
	sc_signal<float> R0out, R1out, R2out, R3out, R4out; //Signals
	sc_signal<float> A0out, A1out, A2out, A3out, M0out, M1out, M2out, M3out, M4out; //Adders and Multipliers

	FloatAdder ADD0, ADD1, ADD2, ADD3; //Adder instances
	FloatMultiplier MULT0, MULT1, MULT2, MULT3, MULT4; //Multiplier instances
	ClkReg REG0, REG1, REG2, REG3, REG4; //Clock register instances


	SC_CTOR(DigitalFilter) :
		ADD0("adder0"),
		ADD1("adder1"),
		ADD2("adder2"),
		ADD3("adder3"),
		MULT0("multiplier0"),
		MULT1("multiplier1"),
		MULT2("multiplier2"),
		MULT3("multiplier3"),
		MULT4("multiplier4"),
		REG0("register0"),
		REG1("register1"),
		REG2("register2"),
		REG3("register3"),
		REG4("register4") {

		//xs * 0.1667:
		MULT0.a(x);
		MULT0.b(a0);
		MULT0.product(M0out);

		//R0out * -0.5:
		MULT1.a(R0out);
		MULT1.b(a1);
		MULT1.product(M1out);

		//R1out * 0.5:
		MULT2.a(R1out);
		MULT2.b(a2);
		MULT2.product(M2out);

		//R2out * -0.1667:
		MULT3.a(R2out);
		MULT3.b(a3);
		MULT3.product(M3out);

		//R4out * -0.3333:
		MULT4.a(R4out);
		MULT4.b(a4);
		MULT4.product(M4out);

		//Bottom Left "+":
		ADD0.a(M2out);
		ADD0.b(M3out);
		ADD0.sum(A0out);

		//Middle Left "+":
		ADD1.a(A0out);
		ADD1.b(M1out);
		ADD1.sum(A1out);

		//Top Left "+":
		ADD2.a(A1out);
		ADD2.b(M0out);
		ADD2.sum(A2out);

		//Top Right "+":
		ADD3.a(A2out);
		ADD3.b(M4out);
		ADD3.sum(y);

		//Set first signal R0:
		REG0.res(res);
		REG0.clk(clk);
		REG0.input(x);
		REG0.output(R0out);

		//Set second signal R1:
		REG1.res(res);
		REG1.clk(clk);
		REG1.input(R0out);
		REG1.output(R1out);

		//Set third signal R2:
		REG2.res(res);
		REG2.clk(clk);
		REG2.input(R1out);
		REG2.output(R2out);

		//Set fourth signal R3:
		REG3.res(res);
		REG3.clk(clk);
		REG3.input(y);
		REG3.output(R3out);

		//Set fifth signal R4:
		REG4.res(res);
		REG4.clk(clk);
		REG4.input(R3out);
		REG4.output(R4out);

		//Set filter coefficients
		a0.write(0.1667);
		a1.write(-0.5);
		a2.write(0.5);
		a3.write(-0.1667);
		a4.write(-0.3333);
	}
};

//Module for the program monitor
SC_MODULE(ResultMonitor) {

	sc_in<float> x; //input
	sc_in<float> y; //output
	sc_in<bool> clk; //clock

	void monitor() {
		while (1) {
			std::cout << "x = " << x.read() << " | y = " << y.read() << std::endl;
			wait();
		}
	}
	SC_CTOR(ResultMonitor) {
		SC_THREAD(monitor);
		sensitive << clk.pos();
	}
};

//Module to generate test data
SC_MODULE(StimulusGenerator) {
	sc_out<float> input;
	sc_out<bool> reset;
	sc_in_clk clk;

	void generateInput() {
		wait();
		reset.write(true);
		wait();
		reset.write(false);
		for (int i = 0; i < 100; i++) {
			if (i > 0) {
				input.write(0);
			}
			else {
				input.write(1);
			}
			wait(); //wait for the next clock cycle
		}
	}

	SC_CTOR(StimulusGenerator) {
		SC_THREAD(generateInput);
		sensitive << clk.pos(); //trigger only on positive clock
	}
};

int sc_main(int, char* [])
{
	sc_clock clk("clk", 10, SC_NS, 0.5, 0, SC_NS, false);

	sc_trace_file* trace_file = sc_create_vcd_trace_file("assignment1_part 2_trace"); //Create trace file
	trace_file->set_time_unit(1, SC_NS);

	//Trace file contents
	sc_signal<bool> reset_signal;
	sc_signal<float> filter_input;
	sc_signal<float> filter_output;

	sc_trace(trace_file, clk, "clock");
	sc_trace(trace_file, reset_signal, "reset");
	sc_trace(trace_file, filter_input, "x");
	sc_trace(trace_file, filter_output, "y");

	//Digital filter initialization
	DigitalFilter filter("filter");
	filter.clk(clk);
	filter.res(reset_signal);
	filter.y(filter_output);
	filter.x(filter_input);

	//Result monitor initialization
	ResultMonitor result_monitor("result_monitor");
	result_monitor.clk(clk);
	result_monitor.x(filter.x);
	result_monitor.y(filter.y);

	//Stimulus generator initialization
	StimulusGenerator stimulus("stimulus");
	stimulus.clk(clk);
	stimulus.input(filter_input);
	stimulus.reset(reset_signal);

	sc_start(130, SC_NS);  //Run for 12 clock cycles

	sc_close_vcd_trace_file(trace_file);

	return 0;
}