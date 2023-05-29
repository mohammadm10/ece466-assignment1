// assignment1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <systemc.h>
#include <iostream>

SC_MODULE(CLOCK) {
    sc_port<sc_signal_in_if<bool>> clk; // a port to access clock
    SC_CTOR(CLOCK) {
        SC_THREAD(thread); // register a thread process
        sensitive << clk; // sensitive to clock
        dont_initialize();
    }
    void thread() {
        while (true) {
            wait(); // wait for next clock value change
        }
    }
};

//SC_MODULE to hold SC_METHOD & SC_CTHREAD
SC_MODULE(DigitalFilter) {

    sc_in<float> x; //input
    sc_out<float> y; //output

    sc_in<bool> res; //reset
    sc_in_clk clk; //clock

    //Registers
    sc_signal<float> R0in; //top left
    sc_signal<float> R0out;
    sc_signal<float> R1out; //mid left
    sc_signal<float> R2out; //bottom left
    sc_signal<float> R3in; //top right
    sc_signal<float> R3out;
    sc_signal<float> R4out; //bottom right

    void produce() {
        //read input and registers here
        float xs = x.read();
        float R0s = R0out.read();
        float R1s = R1out.read();
        float R2s = R2out.read();
        float R3s = R3out.read();
        float R4s = R4out.read();

        //equations for each addition
        float q1 = (R1s * 0.5) + (R2s * (-0.1667)); //bottom left "+"
        float q2 = q1 + (R0s * (-0.5)); //middle left "+"
        float q3 = q2 + (xs * 0.1667); //top left "+"
        float q4 = q3 + ((R3s + R4s) * (-0.3333)); //output equation
        R3in.write(q4); //write output to R3
        y.write(q4); //write output y
    }

    void register_updates() {
        //find output y
        while (1) {
            if (res.read()) {
                //reset is 1, reset all registers
                R0in.write(0);
                R0out.write(0);
                R1out.write(0);
                R2out.write(0);
                R3out.write(0);
                R4out.write(0);
            }
            else {
                R0out.write(R0in.read());
                R1out.write(R0out.read());
                R2out.write(R1out.read());
                R3out.write(R3in.read());
                R4out.write(R3out.read());
            }
            wait();
        }
    }


    SC_CTOR(DigitalFilter) { //Constructor
        SC_CTHREAD(register_updates, clk.pos()); //Trigger on positive clock value
        SC_METHOD(produce);
        sensitive << x << R0out << R1out << R2out << R3out << R4out;
    }
};

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

SC_MODULE(StimulusGenerator) {
    sc_out<float> input;
    sc_out<bool> reset;
    sc_in_clk clk;

    void generateInput() {
        wait();
        reset.write(true);
        wait();
        reset.write(false);
        for (int i = 0; i < 50; i++) {
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
        sensitive << clk.pos();
    }
};


int sc_main(int, char* [])
{
    sc_clock clk("clk", 10, SC_NS, 0.5, 0, SC_NS, false);

    sc_trace_file* trace_file = sc_create_vcd_trace_file("C:/466-a1/assignment1");
    trace_file->set_time_unit(1, SC_NS);

    sc_signal<bool> reset_signal;
    sc_signal<float> filter_input;
    sc_signal<float> filter_output;

    sc_trace(trace_file, clk, "clock");
    sc_trace(trace_file, reset_signal, "reset");
    sc_trace(trace_file, filter_input, "x");
    sc_trace(trace_file, filter_output, "y");

    CLOCK clock("clock");
    clock.clk(clk);

    DigitalFilter filter("filter");
    filter.clk(clk);
    filter.res(reset_signal);
    filter.y(filter_output);
    filter.x(filter_input);

    ResultMonitor result_monitor("result_monitor");
    result_monitor.clk(clk);
    result_monitor.x(filter.x);
    result_monitor.y(filter.y);

    StimulusGenerator stimulus("stimulus");
    stimulus.clk(clk);
    stimulus.input(filter_input);
    stimulus.reset(reset_signal);

    sc_start(120, SC_NS);  //Run for 12 clock cycles

    sc_close_vcd_trace_file(trace_file);

    return 0;
}


