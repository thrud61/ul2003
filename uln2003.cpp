// Class to drive the UL2003 stepper driver board
// not hugely useful as its for unipolar, 5-wire stepper motors
// most are bipolar 4-wire and would use the TMC22xx class,
// raspberry pi version needs an OS to support the future and atomic
// classes
//
// Author James Wilson
// 1st October 2021
//


#include "uln2003.h"

//#define DEBUG 1

#ifdef DEBUG
#define debug(a) printf("%s\n",a) 
#define debugb(a,b) printf(a,b) 
#else
#define debug(a)
#define debugb(a,b)
#endif

#define sign(a) (a<0?-1:1)

Motor::Motor()
{
    // setup wiringPi to GPIO mode
    wiringPiSetupGpio();

    settings["D"] = 2;

    moving = false;

    // set them to output
    for (int pin : motor_pins)
    {
        pinMode(pin, OUTPUT);
    }

    // gpio 23 for end stop
    pinMode(23, INPUT);
    pullUpDnControl (23, PUD_UP);

    // set non-energised state
    halt();
}

bool Motor::is_moving()
{
    return moving;
}

void Motor::halt()
{
debug("Halt");
    moving = false;
    usleep(100000); // wait 100ms to give the move time to notice if its async
    for (int pin : motor_pins)
    {
        digitalWrite(pin, 0);
    }
}

// take a single step
void Motor::step(int pos)
{
    int seq = pos%8;
    int i;

debugb("step (invert %d)",invert);
    // check the end stop
    if (!digitalRead(23))
    {
debug("End Stop");
      
//        position = 0;
        halt();
    } 

    for (i=0;i<4;i++)
    {
        digitalWrite(motor_pins[i], sequence[seq][i]);
    }
}

void Motor::async_move(int new_pos)
{
    moving = true;
    target = new_pos;
    return async_move();
}
void Motor::async_move(void)
{
    async_m = std::async(std::launch::async, [&](){move();});
}

void Motor::wait(void)
{
     async_m.wait();
}

int Motor::move(int new_pos)
{
    target = new_pos;
    return move();
}

void Motor::nudge(void)
{
    int direction;
    int delay;
debug("Nudge");

    // do some stuff with the motor
    delay = (1000000/((speed*(steps))/60));

    if (position > target)
        direction = -1;
    else
        direction = 1;
    
    step(position);

    position += direction;
    position = position%max_steps;
            
    delayMicroseconds(delay);
    halt();
}

// move to a specific position
int Motor::move(void)
{
    int direction;
    int slow=0;
    int delay;
debug("Move");

    // do some stuff with the motor
    delay = (1000000/((speed*(steps/fullsteps))/60));

//    target = target < 0 ? 0 : target; // make sure we don't go negative
    delay = delay > 800*fullsteps ? delay : 800*fullsteps; // limit to about 15 rmp, max for 28BYJ-48, change for other steppers

    if (accel)
        slow = (steps/10)*100;

    if (position > target)
        direction = -fullsteps;
    else
        direction = fullsteps;//

    // someone can stop this with moving set to false
    while ((target != position) && moving)
    {
	    step(position);
        
        if (moving)
        {
            position += direction;
            position = position%max_steps;

            if (accel)
            {
                if (abs(target - position) < steps/10)
                {
                    slow += 100*fullsteps;
                }
                else
                if (slow)
                {
                    slow -= 100;
                    if (slow < 0)
                        slow = 0;
                }
            }

            // only wait if we are not stopped
            delayMicroseconds(slow+delay);
        }
    }
    halt();

    return position;
}
