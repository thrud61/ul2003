// Class to drive the UL2003 stepper driver board
// not hugely useful as its for unipolar, 5-wire stepper motors
// most are bipolar 4-wire and would use the TMC22xx class,
// raspberry pi version needs an OS to support the future and atomic
// classes
//
// Author James Wilson
// 1st October 2021
//

#include <iostream>
#include <wiringPi.h>
#include <unistd.h>
#include <future>
#include <atomic>
#include <map>


class Motor {
public:
    int fullsteps = 2;
    int steps = 4096;
    int accel = false;
    int debug = false;
    int speed = 12;   // rpm
    int max_steps = steps*100; // we will wrap around at this point
    int position = 0;
    int target = 0;
    int invert=0;
    bool moving;
    std::map<std::string,int> settings;
    std::future<void> async_m;


private:
    int motor_pins[4] = {17,18,27,22};
    int sequence[8][4] =
                {   // run forwards
                    {1,0,0,1},
                    {1,0,0,0},
                    {1,1,0,0},
                    {0,1,0,0},
                    {0,1,1,0},
                    {0,0,1,0},
                    {0,0,1,1},
                    {0,0,0,1}
                };

    void step(int pos);

public:
    Motor ();
    void halt();
    int move(int target);
    int move(void);
    void async_move(void);
    void nudge(void);
    void async_move(int);
    void wait(void);
    bool is_moving();
};
