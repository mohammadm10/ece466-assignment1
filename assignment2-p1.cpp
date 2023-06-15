// Example: Portion of sc_main.
// Notice the use of "sc_signal_rv".


#include <systemc.h>

#define RDBLK 0
#define RDBYT 1
#define WRBYT 2
#define WRBLK 3

SC_MODULE(mem_ctrl) {

    sc_lv<8> memory[256]; //Total memory

    sc_in_clk clk;
    sc_in<bool> res;
    sc_in<bool> new_comm; //Trigger for a new compand
    sc_out<bool> complete; //Trigger to assert completion of command

    sc_in<sc_uint<2>> comm; //Command itself
    sc_in<sc_uint<8>> addr; //Address to write to
    sc_inout<sc_lv<8>> data; //Data to write to


    void mem_process() {
        //Declare temp variables to be used within mem_process
        sc_uint<2> comm_s;
        sc_uint<8> addr_s;
        sc_lv<8> data_s;
        while (true) {
            if (res.read() == true) complete.write(false);
            else {
                if (new_comm.read() == true) {
                    comm_s = comm.read();
                    switch (comm_s) {
                    case RDBYT:
                        addr_s = addr.read();
                        data_s = memory[addr_s];
                        cout << "@" << sc_time_stamp() << ": RDBYT, address = " << addr_s << ", data = " << data_s << endl;
                        wait();
                        data.write(data_s);
                        break;
                    case WRBYT:
                        addr_s = addr.read();
                        data_s = data.read();
                        cout << "@" << sc_time_stamp() << ": WRBYT, address = " << addr_s << ", data = " << data_s << endl;
                        wait();
                        memory[addr_s] = data_s;
                        break;
                    case RDBLK:
                        addr_s = addr.read();
                        cout << "@" << sc_time_stamp() << ": RDBLK-0, address = " << addr_s << ", data = " << memory[addr_s] << endl;
                        wait();
                        data.write(memory[addr_s]);
                        addr_s++;

                       // addr_s = addr.read();
                        cout << "@" << sc_time_stamp() << ": RDBLK-1, address = " << addr_s << ", data = " << memory[addr_s] << endl;
                        wait();
                        data.write(memory[addr_s]);
                        addr_s++;

                       // addr_s = addr.read();
                        cout << "@" << sc_time_stamp() << ": RDBLK-2, address = " << addr_s << ", data = " << memory[addr_s] << endl;
                        wait();
                        data.write(memory[addr_s]);
                        addr_s++;

                       // addr_s = addr.read();
                        cout << "@" << sc_time_stamp() << ": RDBLK-3, address = " << addr_s << ", data = " << memory[addr_s] << endl;
                        wait();
                        data.write(memory[addr_s]);

                        break;
                    case WRBLK:
                        addr_s = addr.read();
                        data_s = data.read(); // Sample address & data ports
                        cout << "@" << sc_time_stamp() << ": WRBLK-0, address = " << addr_s << ", data = " << data_s << endl;
                        wait();

                        memory[addr_s] = data_s; // After 1 cycle, write into memory space
                        addr_s++;
                        data_s = data.read(); // Increment address & sample data port
                        cout << "@" << sc_time_stamp() << ": WRBLK-1, address = " << addr_s << ", data = " << data_s << endl;
                        wait();

                        memory[addr_s] = data_s; // After 1 cycle, write into memory space
                        addr_s++;
                        data_s = data.read(); // Increment address & sample data port
                        cout << "@" << sc_time_stamp() << ": WRBLK-2, address = " << addr_s << ", data = " << data_s << endl;
                        wait();

                        memory[addr_s] = data_s; // After 1 cycle, write into memory space
                        addr_s++;
                        data_s = data.read(); // Increment address & sample data port
                        cout << "@" << sc_time_stamp() << ": WRBLK-3, address = " << addr_s << ", data = " << data_s << endl;
                        //wait();

                        memory[addr_s] = data_s; // After 1 cycle, write into memory space
                        break;

                    default:
                        cout << "Illegal command: " << comm_s << endl;
                        break;
                    }
                    complete.write(true);
                    while (new_comm.read() == true) {
                        if (res.read() == true)
                            break;
                        wait();
                    }
                    complete.write(false);
                    if (comm_s == RDBYT) // deassert the data line when reading a byte or block
                        data.write("ZZZZZZZZ"); // stop driving
                }
            }
            wait();
        }
    }
    SC_CTOR(mem_ctrl) {
        SC_CTHREAD(mem_process, clk.pos())
    }
};

SC_MODULE(mem_testbench) {
    sc_in_clk clk;
    sc_out<bool> res;

    sc_out<sc_uint<8>> addr; //Address to read/write from/to
    sc_inout<sc_lv<8>> data; //Data to read/write
    sc_out<sc_uint<2>> comm; //command
    sc_out<bool> new_comm;
    sc_in<bool> complete;

    void mem_bench() {
        /*
        * Need to use mem_process to create a test bench to test the following:
        * 1) WRBYT data=00001111 to addr=255
        * 2) WRBYT data=11110000 to addr=0
        * 3) RDBYT from addr=255
        * 4) RDBYT from addr=0
        * 5) WRBLK data=[00000000,00001111,11110000,11111111] to addr=64
        * 6) WRBYT data=10101010 to addr=128
        * 7) RDBLK from addr=64
        * 8) RDBYT from addr=128
        * wait()'s are added to correctly form the trace
        */

        //First initialize our values

        res.write(true);
        data.write("ZZZZZZZZ");
        comm.write(RDBYT);
        wait();
        res.write(false);

        //1): WRBYT data=00001111 to addr=255
        data.write("00001111");
        addr.write(255);
        comm.write(WRBYT);
        new_comm.write(true);
        wait(3); //Needs to run for 3 clock cycles according to waveform trace
        data.write("ZZZZZZZZ");
        new_comm.write(false);

        while (complete.read()) wait(); //wait until complete is false

        ////2): WRBYT data=11110000 to addr=0
        data.write("11110000");
        addr.write(0);
        comm.write(WRBYT);
        new_comm.write(true);
        wait(3);
        data.write("ZZZZZZZZ");
        new_comm.write(false);

        while (complete.read()) wait(); //wait until complete is false

        ////3): RDBYT from addr=255
        addr.write(255);
        comm.write(RDBYT);
        new_comm.write(true);
        wait();

        while (!complete.read()) wait(); //wait until complete is true

        ////now we can read the value
        data.read();
        new_comm.write(false);
        wait(2);

        ////4): RDBYT from addr=0
        addr.write(0);
        comm.write(RDBYT);
        new_comm.write(true);
        wait();

        while (!complete.read()) wait(); //wait until complete is true

        //now we can read the value
        data.read();
        new_comm.write(false);
        wait();

        //5): WRBLK data=[00000000,00001111,11110000,11111111] to addr=64

        sc_lv <8> buffer[4] = {}; //Initilize empty array
        buffer[0] = "00000000";
        buffer[1] = "00001111";
        buffer[2] = "11110000";
        buffer[3] = "11111111";
        wait();
        addr.write(64);
        comm.write(WRBLK);
        new_comm.write(true);
        data.write(buffer[0]);
        wait();
        data.write(buffer[1]);
        wait();
        data.write(buffer[2]);
        wait();
        data.write(buffer[3]);
        wait(2);
        data.write("ZZZZZZZZ");
        new_comm.write(false);

        while (complete.read()) wait(); //wait until complete is false
        wait();

        //6): WRBYT data=10101010 to addr=128
        data.write("10101010");
        addr.write(128);
        comm.write(WRBYT);
        new_comm.write(true);
        wait(3);
        data.write("ZZZZZZZZ");
        new_comm.write(false);

        while (complete.read()) wait(); //wait until complete is false

        //7): RDBLK from addr=64
        //Need to read a whole block this time, not just a byte
        addr.write(64);
        comm.write(RDBLK);
        new_comm.write(true);
        wait(2);
        buffer[0] = data.read();
        wait();
        buffer[1] = data.read();
        wait();
        buffer[2] = data.read();
        wait();
        buffer[3] = data.read();

        while (!complete.read()) wait(); //wait until complete is true

        new_comm.write(false);
        wait(2);

        //8): RDBYT from addr=128
        addr.write(128);
        comm.write(RDBYT);
        new_comm.write(true);
        wait();

        while (!complete.read()) wait(); //wait until complete is true

        //now we can read the value
        data.read();
        new_comm.write(false);
        wait(2);

    }

    SC_CTOR(mem_testbench) {
        SC_CTHREAD(mem_bench, clk.pos())
    }
};

int sc_main(int argc, char* argv[])
{

    sc_clock clk("TestClock", 10, SC_NS, 0.5, 5, SC_NS);
    // Internal signals:
    sc_signal <bool> reset, new_comm, complete;
    sc_signal < sc_uint<2> > comm;
    sc_signal < sc_uint<8> > addr;
    sc_signal_rv <8> data;
    // _rv needed because of multiple drivers

    // Module instances:
    mem_ctrl MC("MC");

    // Interconnects:
    MC.clk(clk);
    MC.res(reset);
    MC.data(data);
    MC.addr(addr);
    MC.comm(comm);
    MC.new_comm(new_comm);
    MC.complete(complete);

    mem_testbench TB("TB");
    TB.clk(clk);
    TB.res(reset);
    TB.data(data);
    TB.addr(addr);
    TB.comm(comm);
    TB.new_comm(new_comm);
    TB.complete(complete);

    // Rest of sc_main 

    //Just need the waveform trace here

    sc_trace_file* trace_file = sc_create_vcd_trace_file("assignment2_part_1_trace"); //Create trace file
  /*  trace_file->set_time_unit(1, SC_NS);*/

    //Set trace fields
    sc_trace(trace_file, clk, "clock");
    sc_trace(trace_file, reset, "reset");
    sc_trace(trace_file, data, "data");
    sc_trace(trace_file, comm, "comm");
    sc_trace(trace_file, new_comm, "new_comm");
    sc_trace(trace_file, addr, "addr");
    sc_trace(trace_file, complete, "complete");

    sc_start(500, SC_NS);  //Run for 12 clock cycles

    sc_close_vcd_trace_file(trace_file);
    return 0;
}
