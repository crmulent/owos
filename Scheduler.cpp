#include <iostream>
#include <cstdlib>
#include <string>
#include <map>
#include <ctime>
#include <iomanip>
#include <sstream>


class Scheduler{
public:
    enum scheduler_type{
        FCFS,
        RR
    };

    

private:
    scheduler_type scheduler;
    //for now create first come first serve in the same class, next time create a abstracted
};