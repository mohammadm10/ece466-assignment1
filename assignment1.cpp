// assignment1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <systemc.h>

SC_MODULE(CLOCK) {
    sc_port<sc_signal_in_if<bool>> clk; // a port to access clock
    SC_CTOR(CLOCK) {
        SC_THREAD(thread); // register a thread process
        sensitive << clk; // sensitive to clock
        dont_initialize();
    }
    void thread() {
        while (true) {
            std::cout << sc_time_stamp() << ", value = " << clk->read() << std::endl; // print current clock value
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
                R0in = 0;
                R0out = 0;
                R1out = 0;
                R2out = 0;
                R3in = 0; //equal to y
                R3out = 0;
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


int sc_main(int, char* [])
{
    sc_clock clk("clk", 10, SC_NS, 0.5, 0, SC_NS, false);

    CLOCK clock("clock");

    clock.clk(clk);

    sc_start(500, SC_NS);

    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
