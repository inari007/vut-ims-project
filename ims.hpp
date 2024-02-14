/*******************************************\
*           Modelovani a simulace           *
*            Simulace cementarny            *
*               Dobes Zdenek                *
*                 xdobes21                  *
*                 ims.hpp                   *
\*******************************************/

#include <simlib.h>

//  Time in minutes
#define HOUR 60 
#define SEVEN_HOURS 420
#define DAY 1440 
#define WEEK 7200 // 5 days (no weekend)
#define MONTH 30240 // 21 days on average (no weekends and holidays)
#define YEAR 360000 // 250 days (no weekends and holidays)

//  Factory parameters
// in tons
#define PRE_STOCK_CAPACITY 22000
#define HOM_SILO_CAPACITY 6000
#define CLINKER_SILO_CAPACITY 32000
// in tons per hour
#define GRIND_PERFORMANCE 60 
#define ROTARY_KILN_PERFORMANCE 54.166 // in tons per 65min
#define CEMENT_MILL_PERFORMANCE 120
// in tons
#define EXTRACTION_PER_MIN 11.25 // 1350 000 per year
#define GRIND_OUTPUT 48 // GRIND_PERFORMANCE * 0.8

#define NUMBER_OF_GRINDERS 2

//  Experiment 3
//#define ROTARY_KILN_PERFORMANCE 85.764 // in tons per 65min
//  Experiment 4
//#define ROTARY_KILN_PERFORMANCE 157.986 // in tons per 65min
//  Experiemnt 5
//#define ROTARY_KILN_PERFORMANCE 157.986 // in tons per 65min
//#define CEMENT_MILL_PERFORMANCE 220
//#define NUMBER_OF_GRINDERS 3 

//  Machines
Facility extraction("Extraction in process");
Facility grinder[NUMBER_OF_GRINDERS];
Facility rotary_kiln("Calcination in process");
Facility cement_mill("Grinding in process");

//  Storages
Store prehomogenization_stock("Limestone stock", PRE_STOCK_CAPACITY);
Store homogenization_silo("Silo for grinded limestone", HOM_SILO_CAPACITY);
Store clinker_silo("Stock for clinker", CLINKER_SILO_CAPACITY);

//  Queues
Queue extraction_q("Wait until sunrise");
Queue prehom_q("Wait for space in prehomogeniyation stock");
Queue grind_q("Wait for grinder");
Queue silo_q("Wait for space in silo");
Queue clinker_q("Wait for space in clinker silo");
Queue calcination_q("Wait for calcination");
Queue cement_grind_q("Wait for cement grinder");

//  Statistics
bool ShouldBePrinted = false;
Stat Length_of_working_shifts("Length of extraction per day");
Stat Amount_of_addition("Addition in g per kg of cement");
Stat Amount_of_limestone("Grinded limestone in g per kg of cement");
Stat Amount_of_clinker("Clinker in g per kg of cement");
Stat Calcination_time("Time spent calcinating");
unsigned long Exported_material = 0;
unsigned long Extracted_material = 0;
unsigned long Cement_bags = 0;

void PrintStatistics();
unsigned long CheckArgs(int argc, char** argv);

class Production : public Process{
    public:
        void Behavior() override;
};
class Cement_packing : public Process{
    public:
        void Behavior() override;
};
class Cement_grinding : public Process{
    public:
        void Behavior() override;
};
class Calcination : public Process{
    public:
        void Behavior() override;
};
class Grinding : public Process{
    public:
        void Behavior() override;
};
class Extraction : public Process{
    public:
        void Behavior() override;
};
class WorkingShift : public Process{
    public:
        void Behavior() override;
};
class WorkingDay : public Event{
    public:
        void Behavior() override;
};