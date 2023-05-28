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
           // std::cout << sc_time_stamp() << ", value = " << clk->read() << std::endl; // print current clock value
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
        y.write(q4); //write output
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
                R3in.write(0); //equal to y
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
        // Generate input sequence here
        // You can use a loop, a counter, or any other method to generate the desired input values

        // Example: Generate a sequence of input values from 0 to 9
        for (int i = 0; i < 50; i++) {
            float temp = 0;
            if (i > 2) {
                input.write(0);
            }
            wait(); // Wait for the next clock cycle
        }

        // Finish generating inputs and assert reset signal
        input.write(0.0);
        reset.write(true);
        wait();
        reset.write(false);
    }

    SC_CTOR(StimulusGenerator) {
        SC_THREAD(generateInput);  // Use SC_THREAD instead of SC_CTHREAD
        sensitive << clk.pos();
    }
};


int sc_main(int, char* [])
{
    sc_clock clk("clk", 10, SC_NS, 0.5, 0, SC_NS, false);

    CLOCK clock("clock");

    clock.clk(clk);

    DigitalFilter filter("filter");

    //Bind port 3
    sc_signal<bool> reset_signal;
    filter.clk(clk);
    filter.res(reset_signal);

    //Bind port 1
    sc_signal<float> filter_output;
    filter.y(filter_output);

    //Bind port 0
    sc_signal<float> filter_input;
    filter.x(filter_input);

    ResultMonitor result_monitor("result_monitor");
    result_monitor.x(filter.x);
    result_monitor.y(filter.y);
    result_monitor.clk(clk);

    filter_input = 1.0;

    StimulusGenerator stimulus("stimulus");
    stimulus.clk(clk);
    stimulus.input(filter_input);
    stimulus.reset(reset_signal);

    sc_start(500, SC_NS);

    return 0;
}
