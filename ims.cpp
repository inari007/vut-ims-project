/*******************************************\
*           Modelovani a simulace           *
*            Simulace cementarny            *
*               Dobes Zdenek                *
*                 xdobes21                  *
*                 ims.cpp                   *
\*******************************************/

#include <simlib.h>
#include <string.h>
#include <ctype.h>

#include "ims.hpp"

//  Production process
void Production::Behavior(){

    (new Grinding)->Activate();
    grind_q.Insert(this);
    Passivate();

    (new Calcination)->Activate();
    calcination_q.Insert(this);
    Passivate();

    (new Cement_grinding)->Activate();
    cement_grind_q.Insert(this);
    Passivate();

    (new Cement_packing)->Activate();
};

// Simulates ending process of packing cement
void Cement_packing::Behavior(){
    Wait(50); // 1 minute preparing, 50 minutes packing -> 4 800 packages, 25kg per each
    Cement_bags = Cement_bags + CEMENT_MILL_PERFORMANCE*40; // STAT
};

// Simulates grind clinker with limestone and addition
void Cement_grinding::Behavior(){

    if(CLINKER_SILO_CAPACITY - clinker_silo.Free() > CEMENT_MILL_PERFORMANCE * 0.94 && PRE_STOCK_CAPACITY -  prehomogenization_stock.Free() > CEMENT_MILL_PERFORMANCE * 0.20){ 
        Seize(cement_mill);                                                                                                
        double addition = Random() * CEMENT_MILL_PERFORMANCE/20;
        double limestone = (Random() * ((CEMENT_MILL_PERFORMANCE/100 * 20 - 7.2) - addition)) + (CEMENT_MILL_PERFORMANCE/100 * 6);   // 7.2 = CEMENT_MILL_PERFORMANCE/100 * 6
                                                                                                                                     // 16.8 = CEMENT_MILL_PERFORMANCE/100 * 20 - 7.2
        Amount_of_addition(addition *25/3); // STAT
        Amount_of_limestone(limestone*25/3); // STAT
        Amount_of_clinker((CEMENT_MILL_PERFORMANCE-addition-limestone)*25/3); // STAT
        if(PRE_STOCK_CAPACITY -  prehomogenization_stock.Free() < CEMENT_MILL_PERFORMANCE * 0.20){
            prehom_q.Insert(this);
            Passivate();
        }
        Leave(prehomogenization_stock, limestone);
        if(CLINKER_SILO_CAPACITY - clinker_silo.Free() <= CEMENT_MILL_PERFORMANCE * 0.94){
            clinker_q.Insert(this);
            Passivate();
        }
        Leave(clinker_silo, CEMENT_MILL_PERFORMANCE - limestone - addition);

        Wait(HOUR);
        Release(cement_mill);
        if(!cement_grind_q.Empty()){
            cement_grind_q.GetFirst()->Activate();
        }
        if(calcination_q.Empty()){
            (new Cement_grinding)->Activate();
        }
        else{
            calcination_q.GetFirst()->Activate();
        }
    }
};

// Simulates limestone turning into clinker
void Calcination::Behavior(){
    if(HOM_SILO_CAPACITY - homogenization_silo.Free() > ROTARY_KILN_PERFORMANCE){ // if there is enought limestone to calcinate
        Seize(rotary_kiln);
        if(HOM_SILO_CAPACITY - homogenization_silo.Free() < ROTARY_KILN_PERFORMANCE){
            silo_q.Insert(this);
            Passivate();
        }
        Leave(homogenization_silo, ROTARY_KILN_PERFORMANCE);
        if(!silo_q.Empty()){ // makes space in the silo
            silo_q.GetFirst()->Activate();
        }
        double calc = Random()*10 + HOUR;
        Wait(calc);
        Enter(clinker_silo, ROTARY_KILN_PERFORMANCE);
        if(CLINKER_SILO_CAPACITY - clinker_silo.Free() > CEMENT_MILL_PERFORMANCE * 0.94){
            if(!clinker_q.Empty()){
                clinker_q.GetFirst()->Activate();
            }
        }
        Release(rotary_kiln);
        Calcination_time(calc); // STAT
        if(!calcination_q.Empty()){
            calcination_q.GetFirst()->Activate();
        }
        if(grind_q.Empty()){
            (new Calcination)->Activate();
        }
        else{
            grind_q.GetFirst()->Activate();
        }
    }
};

// Simulates limestone being grinded
void Grinding::Behavior(){
    if(PRE_STOCK_CAPACITY - prehomogenization_stock.Free() > GRIND_PERFORMANCE){ // if there is enought limestone to grind
        for(int x = 0; x < NUMBER_OF_GRINDERS; x++){
            if(!grinder[x].Busy()){
                Seize(grinder[x]);
                Leave(prehomogenization_stock, GRIND_PERFORMANCE);
                Wait(HOUR);
                if(homogenization_silo.Free() < GRIND_OUTPUT){ // stays in the grinded until space is avaiable
                    silo_q.Insert(this);
                    Passivate();
                }
                Enter(homogenization_silo, GRIND_OUTPUT);
                if(!silo_q.Empty()){
                    if(HOM_SILO_CAPACITY - homogenization_silo.Free() > ROTARY_KILN_PERFORMANCE){
                        silo_q.GetFirst()->Activate();
                    }
                }
                Release(grinder[x]);
                if(!grind_q.Empty()){
                    grind_q.GetFirst()->Activate();
                }
                (new Production)->Activate();
                return;
            }
        }
    }
    else{
        (new Production)->Activate(Time + 1);
    }
};

// Simulates extraction process
void Extraction::Behavior(){
    if(extraction.Busy() && prehomogenization_stock.Free() > EXTRACTION_PER_MIN){ // if is a shift and stock isn't full
        Enter(prehomogenization_stock, EXTRACTION_PER_MIN);
        Extracted_material = Extracted_material + EXTRACTION_PER_MIN; // STAT
        if(!prehom_q.Empty()){
            if(PRE_STOCK_CAPACITY -  prehomogenization_stock.Free() > CEMENT_MILL_PERFORMANCE * 0.20){
                prehom_q.GetFirst()->Activate();
            }
        }
    }
    else if(prehomogenization_stock.Free() > EXTRACTION_PER_MIN){ // if shift is over
        extraction_q.Insert(this);
        Passivate();
    }
    else{   // if stock is full
        Exported_material = Exported_material + EXTRACTION_PER_MIN; // STAT
    }
    (new Extraction)->Activate(Time + 1);
};

// Simulates a 7-8h work shift 
void WorkingShift::Behavior(){
    Seize(extraction); // enables material being extracted
    if(!extraction_q.Empty()){ // puts Extraction out of sleep
        extraction_q.GetFirst()->Activate();
    }
    double length_of_shift = Random()*60 + SEVEN_HOURS;
    Wait(length_of_shift);
    Release(extraction); // unenables material being extracted after 7-8h
    Length_of_working_shifts(length_of_shift / HOUR); // STAT
    
};

// Simulates a working day
void WorkingDay::Behavior(){
    (new WorkingShift)->Activate();
    (new WorkingDay)->Activate(Time + DAY);
};

//  Parse arguments
unsigned long CheckArgs(int argc, char** argv){
    unsigned long PRODUCTION_TIME = WEEK; // default time of simulation
    for(int i = 1; i < argc; i++){
        if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "-H") == 0){
            printf("This program can take up to 2 optional arguments.\n\n");
            printf("First one is number of working days. To represent standart week the number would be 5.\n");
            printf("You can use 'year', 'month', 'week' or 'day' as well. These do not contain weekends and holidays.\n");
            printf("Simulation takes 5 working days by default if this argument is not included.\n\n");
            printf("Second one represents if statistics should be printed.\n");
            printf("You can use 'yes' or 'no'. No is set by default.\n");
            printf("Examples: \n");
            printf("./ims day \n");
            printf("./ims 365 \n");
            printf("./ims no \n");
            printf("./ims month no \n");
            exit(0);
        }
        else if(strcmp(argv[i], "year") == 0 || strcmp(argv[i], "YEAR") == 0){
            PRODUCTION_TIME = YEAR;
        }
        else if(strcmp(argv[i], "month") == 0 || strcmp(argv[i], "MONTH") == 0){
            PRODUCTION_TIME = MONTH;
        }
        else if(strcmp(argv[i], "week") == 0 || strcmp(argv[i], "WEEK") == 0){
            PRODUCTION_TIME = WEEK;
        }
        else if(strcmp(argv[i], "day") == 0 || strcmp(argv[i], "DAY") == 0){
            PRODUCTION_TIME = DAY;
        }
        else if(strcmp(argv[i], "yes") == 0 || strcmp(argv[i], "YES") == 0){
            ShouldBePrinted = true;
        }
        else if(strcmp(argv[i], "no") == 0 || strcmp(argv[i], "NO") == 0){
            ShouldBePrinted = false;
        }
        else if(atoi(argv[i]) > 0){
            PRODUCTION_TIME = atoi(argv[i]) * DAY;
        }
        else{
            return 0;
        }
    }
    return PRODUCTION_TIME;
}

//  Print results of simulation
void PrintStatistics(){
    if(ShouldBePrinted){
        printf("\n");
        Length_of_working_shifts.Output();
        printf("\n");
        Calcination_time.Output();
        printf("\n");
        Amount_of_addition.Output();
        printf("\n");
        Amount_of_limestone.Output();
        printf("\n");
        Amount_of_clinker.Output();
        printf("\n+\n");
        printf("| The amount of limestone extracted is %lut,\n", Extracted_material + Exported_material);
        printf("| where %lut were used for factory purposes\n", Extracted_material);
        printf("| and %lut were exported away.", Exported_material);
        printf("\n+\n\n");
    }
    printf("+\n");
    printf("| Number of cement bags produced is %lu.\n", Cement_bags);
    printf("| Total production of cement is %lukg.", Cement_bags*25);
    printf("\n+\n");
}

int main(int argc, char** argv){
    // Argument parsing
    unsigned long PRODUCTION_TIME = CheckArgs(argc, argv);
    if(PRODUCTION_TIME == 0){
        fprintf(stderr, "Chybný tvar argumentu. Pro nápovědu zadejte argument -h.\n");
        return 1;
    }

    // Simulation run
    Init(0, PRODUCTION_TIME);
    for(int i = 0; i < NUMBER_OF_GRINDERS; i++){ // start all of the grinders
        (new Production)->Activate();
    }
    (new Extraction)->Activate();
    (new WorkingDay)->Activate();
    Run();

    // Print results
    PrintStatistics();
    return 0;
}