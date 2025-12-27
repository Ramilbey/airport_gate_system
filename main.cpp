/*
 * ============================================================================
 * FILE: gate_scheduling.c
 * AUTHOR: [Your Name] - [Your Student ID]
 * COURSE: Algorithm Design and Analysis
 * ASSIGNMENT: Assignment 2 - Greedy Algorithm (Airport Gate Scheduling)
 * DATE: December 2025
 * 
 * DESCRIPTION:
 * This program solves the Airport Gate Scheduling problem using a Greedy
 * Algorithm approach. Given a list of flights with arrival and departure
 * times, it determines the minimum number of gates required and assigns
 * each flight to a specific gate while respecting the 20-minute cleaning
 * time constraint between consecutive flights at the same gate.
 * 
 * ALGORITHM:
 * The greedy approach sorts flights by arrival time and assigns each flight
 * to the first available gate. This is similar to the Interval Scheduling
 * Problem (Kleinberg & Tardos, 2006, Chapter 4).
 * 
 * REFERENCES:
 * [1] Kleinberg, J., & Tardos, E. (2006). Algorithm Design. Pearson Education.
 * [2] Cormen, T. H., Leiserson, C. E., Rivest, R. L., & Stein, C. (2009). 
 *     Introduction to Algorithms (3rd ed.). MIT Press.
 * [3] GeeksforGeeks. (2023). Minimum Number of Platforms Required for a 
 *     Railway/Bus Station. Retrieved from https://www.geeksforgeeks.org/
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

/* ============================================================================
 * CONSTANTS AND MACROS
 * ============================================================================ */
#define MAX_FLIGHTS 100      /* Maximum number of flights supported */
#define MAX_GATES 50         /* Maximum number of gates */
#define CLEANING_TIME 20     /* 20 minutes cleaning time between flights */
#define TIME_FORMAT 1440     /* Minutes in a day (24 * 60) */

/* ============================================================================
 * DATA STRUCTURES
 * Flight structure stores all information about a single flight.
 * Reference: Standard struct design as per Cormen et al. (2009), Chapter 10.
 * ============================================================================ */
typedef struct {
    int flightNumber;        /* Unique flight identifier (FL001, FL002, etc.) */
    int arrivalTime;         /* Arrival time in minutes from midnight */
    int departureTime;       /* Departure time in minutes from midnight */
    int assignedGate;        /* Gate number assigned (-1 if unassigned) */
} Flight;

/* Gate structure tracks availability of each gate */
typedef struct {
    int gateNumber;          /* Gate identifier (1, 2, 3, ...) */
    int availableFrom;       /* Time when gate becomes available (after cleaning) */
    int flightsAssigned;     /* Count of flights assigned to this gate */
} Gate;

/* ============================================================================
 * GLOBAL VARIABLES
 * ============================================================================ */
Flight flights[MAX_FLIGHTS];     /* Array to store all flights */
Gate gates[MAX_GATES];           /* Array to store all gates */
int numFlights = 0;              /* Current number of flights */
int numGates = 0;                /* Number of gates used */

/* ============================================================================
 * FUNCTION PROTOTYPES
 * ============================================================================ */
void displayMenu(void);
void displayHeader(const char* title);
int timeToMinutes(int hours, int minutes);
void minutesToTime(int totalMinutes, int* hours, int* minutes);
void printTime(int totalMinutes);
int compareFlightsByArrival(const void* a, const void* b);
void initializeGates(void);
void resetData(void);
int findAvailableGate(int arrivalTime);
void assignFlightsToGates(void);
void displayResults(void);
void displayCleaningSchedule(void);
void loadDataset(int datasetNumber);
void inputCustomFlights(void);
void runProgram(void);

/* ============================================================================
 * UTILITY FUNCTIONS
 * These helper functions handle time conversion and display formatting.
 * ============================================================================ */

/**
 * Converts hours and minutes to total minutes from midnight.
 * Example: 14:30 -> 870 minutes
 * 
 * @param hours   Hour component (0-23)
 * @param minutes Minute component (0-59)
 * @return        Total minutes from midnight
 */
int timeToMinutes(int hours, int minutes) {
    return hours * 60 + minutes;
}

/**
 * Converts total minutes back to hours and minutes.
 * Example: 870 minutes -> 14:30
 * 
 * @param totalMinutes Total minutes from midnight
 * @param hours        Pointer to store hour component
 * @param minutes      Pointer to store minute component
 */
void minutesToTime(int totalMinutes, int* hours, int* minutes) {
    *hours = totalMinutes / 60;
    *minutes = totalMinutes % 60;
}

/**
 * Prints time in HH:MM format.
 * 
 * @param totalMinutes Time in minutes from midnight
 */
void printTime(int totalMinutes) {
    int hours, minutes;
    minutesToTime(totalMinutes, &hours, &minutes);
    printf("%02d:%02d", hours, minutes);
}

/**
 * Displays a formatted header for sections.
 * 
 * @param title The title text to display
 */
void displayHeader(const char* title) {
    printf("\n");
    printf("============================================================\n");
    printf(" %s\n", title);
    printf("============================================================\n");
}

/* ============================================================================
 * SORTING FUNCTION
 * Comparison function for qsort to sort flights by arrival time.
 * Reference: qsort usage as per Cormen et al. (2009), Chapter 7.
 * ============================================================================ */

/**
 * Comparison function for sorting flights by arrival time (ascending).
 * Used with qsort() for efficient O(n log n) sorting.
 * 
 * @param a Pointer to first flight
 * @param b Pointer to second flight
 * @return  Negative if a < b, positive if a > b, zero if equal
 */
int compareFlightsByArrival(const void* a, const void* b) {
    Flight* flightA = (Flight*)a;
    Flight* flightB = (Flight*)b;
    return flightA->arrivalTime - flightB->arrivalTime;
}

/* ============================================================================
 * INITIALIZATION FUNCTIONS
 * ============================================================================ */

/**
 * Initializes all gates to available state (available from time 0).
 */
void initializeGates(void) {
    for (int i = 0; i < MAX_GATES; i++) {
        gates[i].gateNumber = i + 1;
        gates[i].availableFrom = 0;
        gates[i].flightsAssigned = 0;
    }
    numGates = 0;
}

/**
 * Resets all data for a new run.
 */
void resetData(void) {
    numFlights = 0;
    numGates = 0;
    initializeGates();
    
    for (int i = 0; i < MAX_FLIGHTS; i++) {
        flights[i].flightNumber = 0;
        flights[i].arrivalTime = 0;
        flights[i].departureTime = 0;
        flights[i].assignedGate = -1;
    }
}

/* ============================================================================
 * CORE GREEDY ALGORITHM
 * This is the main algorithm that finds the minimum number of gates needed.
 * 
 * GREEDY STRATEGY:
 * 1. Sort flights by arrival time
 * 2. For each flight, find a gate that is available (including cleaning time)
 * 3. If no existing gate is available, open a new gate
 * 4. Assign the flight to the selected gate
 * 
 * TIME COMPLEXITY: O(n * g) where n = flights, g = gates
 * Can be optimized to O(n log n) using a min-heap for gate availability.
 * Reference: Similar to Interval Partitioning, Kleinberg & Tardos (2006), Ch 4.
 * ============================================================================ */

/**
 * Finds the first available gate for a flight arriving at given time.
 * A gate is available if: gateAvailableFrom + CLEANING_TIME <= arrivalTime
 * 
 * @param arrivalTime The arrival time of the flight
 * @return            Gate index if found, -1 if no gate available
 */
int findAvailableGate(int arrivalTime) {
    /* 
     * GREEDY CHOICE: Select the first gate that becomes available
     * before the flight's arrival time. This greedy choice leads to
     * an optimal solution (proof in report).
     * Reference: Kleinberg & Tardos (2006), Theorem 4.7
     */
    for (int i = 0; i < numGates; i++) {
        /* Check if gate is available (considering cleaning time) */
        if (gates[i].availableFrom <= arrivalTime) {
            return i;  /* Return first available gate */
        }
    }
    
    /* No existing gate available - need to open a new gate */
    if (numGates < MAX_GATES) {
        return numGates;  /* Return index for new gate */
    }
    
    return -1;  /* No gates available (max capacity reached) */
}

/**
 * Main greedy algorithm to assign all flights to gates.
 * This function implements the core logic of the solution.
 */
void assignFlightsToGates(void) {
    /* 
     * STEP 1: Sort flights by arrival time (O(n log n))
     * Reference: QuickSort implementation in C standard library
     */
    qsort(flights, numFlights, sizeof(Flight), compareFlightsByArrival);
    
    /* Initialize gates */
    initializeGates();
    
    /* 
     * STEP 2: Process each flight in order of arrival time
     * This is the greedy iteration - O(n * g) total
     */
    for (int i = 0; i < numFlights; i++) {
        int gateIndex = findAvailableGate(flights[i].arrivalTime);
        
        if (gateIndex == -1) {
            /* No gate available - flight is UNASSIGNED */
            flights[i].assignedGate = -1;
            printf("  WARNING: Flight FL%03d cannot be assigned (no gates available)\n",
                   flights[i].flightNumber);
        } else {
            /* Assign flight to gate */
            flights[i].assignedGate = gateIndex + 1;  /* Gate numbers start from 1 */
            
            /* 
             * Update gate availability: departure time + cleaning time
             * This ensures the 20-minute gap between flights
             */
            gates[gateIndex].availableFrom = flights[i].departureTime + CLEANING_TIME;
            gates[gateIndex].flightsAssigned++;
            
            /* Track if this is a new gate */
            if (gateIndex >= numGates) {
                numGates = gateIndex + 1;
            }
        }
    }
}

/* ============================================================================
 * OUTPUT FUNCTIONS
 * These functions display the results in a well-formatted manner.
 * ============================================================================ */

/**
 * Displays the complete results including minimum gates and flight assignments.
 */
void displayResults(void) {
    int unassignedCount = 0;
    
    displayHeader("RESULTS");
    
    /* Display minimum gates needed */
    printf("\n  MINIMUM NUMBER OF GATES NEEDED: %d\n", numGates);
    printf("\n");
    
    /* Display flight assignments table */
    printf("  +-----------+----------+-----------+------------+\n");
    printf("  |  Flight   |  Arrival | Departure |    Gate    |\n");
    printf("  +-----------+----------+-----------+------------+\n");
    
    /* Sort back by flight number for display */
    for (int i = 0; i < numFlights; i++) {
        printf("  |  FL%03d    |   ", flights[i].flightNumber);
        printTime(flights[i].arrivalTime);
        printf("  |   ");
        printTime(flights[i].departureTime);
        printf("   |");
        
        if (flights[i].assignedGate == -1) {
            printf(" UNASSIGNED |\n");
            unassignedCount++;
        } else {
            printf("   Gate %-3d |\n", flights[i].assignedGate);
        }
    }
    
    printf("  +-----------+----------+-----------+------------+\n");
    
    /* Summary */
    printf("\n  SUMMARY:\n");
    printf("  - Total flights processed: %d\n", numFlights);
    printf("  - Flights successfully assigned: %d\n", numFlights - unassignedCount);
    printf("  - Flights unassigned: %d\n", unassignedCount);
    printf("  - Gates utilized: %d\n", numGates);
}

/**
 * Displays the cleaning schedule for each gate.
 */
void displayCleaningSchedule(void) {
    displayHeader("GATE CLEANING SCHEDULE");
    
    printf("\n  (Cleaning time: %d minutes after each departure)\n\n", CLEANING_TIME);
    printf("  +--------+----------+----------+-----------------+\n");
    printf("  |  Gate  |  Start   |   End    |     Status      |\n");
    printf("  +--------+----------+----------+-----------------+\n");
    
    /* For each gate, show cleaning periods */
    for (int g = 0; g < numGates; g++) {
        /* Find all flights assigned to this gate */
        for (int i = 0; i < numFlights; i++) {
            if (flights[i].assignedGate == g + 1) {
                int cleanStart = flights[i].departureTime;
                int cleanEnd = flights[i].departureTime + CLEANING_TIME;
                
                printf("  | Gate %d |   ", g + 1);
                printTime(cleanStart);
                printf("  |   ");
                printTime(cleanEnd);
                printf("   | Cleaning/Checks |\n");
            }
        }
    }
    
    printf("  +--------+----------+----------+-----------------+\n");
}

/* ============================================================================
 * DATASET FUNCTIONS
 * Pre-defined datasets for testing the algorithm's correctness.
 * Each dataset is designed to test different scenarios.
 * ============================================================================ */

/**
 * Loads one of the pre-defined datasets.
 * 
 * @param datasetNumber Dataset to load (1-5)
 */
void loadDataset(int datasetNumber) {
    resetData();
    
    switch (datasetNumber) {
        case 1:
            /* 
             * DATASET 1: Simple non-overlapping flights
             * Expected: 1 gate (all flights are sequential)
             * Purpose: Tests basic greedy assignment
             */
            displayHeader("DATASET 1: Sequential Flights (Expected: 1 Gate)");
            numFlights = 4;
            flights[0] = (Flight){1, timeToMinutes(6, 0), timeToMinutes(7, 0), -1};
            flights[1] = (Flight){2, timeToMinutes(7, 30), timeToMinutes(8, 30), -1};
            flights[2] = (Flight){3, timeToMinutes(9, 0), timeToMinutes(10, 0), -1};
            flights[3] = (Flight){4, timeToMinutes(10, 30), timeToMinutes(11, 30), -1};
            break;
            
        case 2:
            /* 
             * DATASET 2: All flights overlap at same time
             * Expected: 5 gates (all flights need separate gates)
             * Purpose: Tests maximum gate requirement scenario
             */
            displayHeader("DATASET 2: All Overlapping (Expected: 5 Gates)");
            numFlights = 5;
            flights[0] = (Flight){1, timeToMinutes(10, 0), timeToMinutes(12, 0), -1};
            flights[1] = (Flight){2, timeToMinutes(10, 0), timeToMinutes(12, 0), -1};
            flights[2] = (Flight){3, timeToMinutes(10, 0), timeToMinutes(12, 0), -1};
            flights[3] = (Flight){4, timeToMinutes(10, 0), timeToMinutes(12, 0), -1};
            flights[4] = (Flight){5, timeToMinutes(10, 0), timeToMinutes(12, 0), -1};
            break;
            
        case 3:
            /* 
             * DATASET 3: Cleaning time constraint test
             * Expected: 2 gates (cleaning time prevents reuse)
             * Purpose: Tests 20-minute cleaning window
             */
            displayHeader("DATASET 3: Cleaning Time Test (Expected: 2 Gates)");
            numFlights = 4;
            /* Flight 2 arrives 15 min after Flight 1 departs - needs new gate */
            flights[0] = (Flight){1, timeToMinutes(8, 0), timeToMinutes(9, 0), -1};
            flights[1] = (Flight){2, timeToMinutes(9, 15), timeToMinutes(10, 15), -1};  /* Only 15 min gap */
            flights[2] = (Flight){3, timeToMinutes(9, 30), timeToMinutes(10, 30), -1};  /* 30 min gap - OK */
            flights[3] = (Flight){4, timeToMinutes(10, 45), timeToMinutes(11, 45), -1}; /* Can reuse Gate 1 */
            break;
            
        case 4:
            /* 
             * DATASET 4: Peak hour simulation
             * Expected: 3 gates (realistic scenario)
             * Purpose: Tests algorithm under realistic conditions
             */
            displayHeader("DATASET 4: Peak Hour Simulation (Expected: 3 Gates)");
            numFlights = 8;
            flights[0] = (Flight){1, timeToMinutes(8, 0), timeToMinutes(9, 30), -1};
            flights[1] = (Flight){2, timeToMinutes(8, 15), timeToMinutes(10, 0), -1};
            flights[2] = (Flight){3, timeToMinutes(8, 30), timeToMinutes(9, 45), -1};
            flights[3] = (Flight){4, timeToMinutes(10, 0), timeToMinutes(11, 30), -1};
            flights[4] = (Flight){5, timeToMinutes(10, 15), timeToMinutes(11, 45), -1};
            flights[5] = (Flight){6, timeToMinutes(10, 30), timeToMinutes(12, 0), -1};
            flights[6] = (Flight){7, timeToMinutes(12, 0), timeToMinutes(13, 30), -1};
            flights[7] = (Flight){8, timeToMinutes(12, 30), timeToMinutes(14, 0), -1};
            break;
            
        case 5:
            /* 
             * DATASET 5: Edge case - exact timing
             * Expected: 2 gates (exact 20-min gaps)
             * Purpose: Tests boundary condition of cleaning time
             */
            displayHeader("DATASET 5: Exact Cleaning Time Boundary (Expected: 2 Gates)");
            numFlights = 4;
            /* Each flight departs exactly 20 min before next arrives */
            flights[0] = (Flight){1, timeToMinutes(8, 0), timeToMinutes(9, 0), -1};
            flights[1] = (Flight){2, timeToMinutes(9, 20), timeToMinutes(10, 20), -1};  /* Exactly 20 min - OK */
            flights[2] = (Flight){3, timeToMinutes(9, 10), timeToMinutes(10, 10), -1};  /* 10 min - needs Gate 2 */
            flights[3] = (Flight){4, timeToMinutes(10, 40), timeToMinutes(11, 40), -1}; /* Can reuse Gate 1 */
            break;
            
        default:
            printf("  Invalid dataset number!\n");
            return;
    }
    
    printf("\n  Dataset loaded successfully with %d flights.\n", numFlights);
    printf("\n  Flight Schedule:\n");
    printf("  +----------+----------+-----------+\n");
    printf("  |  Flight  |  Arrival | Departure |\n");
    printf("  +----------+----------+-----------+\n");
    
    for (int i = 0; i < numFlights; i++) {
        printf("  |  FL%03d   |   ", flights[i].flightNumber);
        printTime(flights[i].arrivalTime);
        printf("  |   ");
        printTime(flights[i].departureTime);
        printf("   |\n");
    }
    printf("  +----------+----------+-----------+\n");
}

/**
 * Allows user to input custom flight data.
 */
void inputCustomFlights(void) {
    int n, arrHour, arrMin, depHour, depMin;
    
    resetData();
    displayHeader("CUSTOM FLIGHT INPUT");
    
    /* Input validation for number of flights */
    do {
        printf("\n  Enter number of flights (1-%d): ", MAX_FLIGHTS);
        if (scanf("%d", &n) != 1 || n < 1 || n > MAX_FLIGHTS) {
            printf("  ERROR: Please enter a valid number between 1 and %d.\n", MAX_FLIGHTS);
            while (getchar() != '\n');  /* Clear input buffer */
            n = 0;
        }
    } while (n < 1 || n > MAX_FLIGHTS);
    
    numFlights = n;
    
    printf("\n  Enter flight details (24-hour format HH MM):\n");
    printf("  Example: For 14:30, enter: 14 30\n\n");
    
    for (int i = 0; i < numFlights; i++) {
        flights[i].flightNumber = i + 1;
        
        /* Input arrival time with validation */
        do {
            printf("  Flight FL%03d - Arrival time (HH MM): ", i + 1);
            if (scanf("%d %d", &arrHour, &arrMin) != 2 || 
                arrHour < 0 || arrHour > 23 || arrMin < 0 || arrMin > 59) {
                printf("  ERROR: Invalid time. Use format HH MM (e.g., 14 30).\n");
                while (getchar() != '\n');
                arrHour = -1;
            }
        } while (arrHour < 0);
        
        flights[i].arrivalTime = timeToMinutes(arrHour, arrMin);
        
        /* Input departure time with validation */
        do {
            printf("  Flight FL%03d - Departure time (HH MM): ", i + 1);
            if (scanf("%d %d", &depHour, &depMin) != 2 || 
                depHour < 0 || depHour > 23 || depMin < 0 || depMin > 59) {
                printf("  ERROR: Invalid time. Use format HH MM (e.g., 16 45).\n");
                while (getchar() != '\n');
                depHour = -1;
            } else if (timeToMinutes(depHour, depMin) <= flights[i].arrivalTime) {
                printf("  ERROR: Departure must be after arrival time.\n");
                depHour = -1;
            }
        } while (depHour < 0);
        
        flights[i].departureTime = timeToMinutes(depHour, depMin);
        flights[i].assignedGate = -1;
        
        printf("\n");
    }
    
    printf("  Successfully entered %d flights.\n", numFlights);
}

/* ============================================================================
 * MENU AND MAIN PROGRAM
 * ============================================================================ */

/**
 * Displays the main menu options.
 */
void displayMenu(void) {
    printf("\n");
    printf("  +-----------------------------------------------+\n");
    printf("  |       AIRPORT GATE SCHEDULING SYSTEM         |\n");
    printf("  |         (Greedy Algorithm Solution)          |\n");
    printf("  +-----------------------------------------------+\n");
    printf("  |  1. Load Dataset 1 (Sequential)              |\n");
    printf("  |  2. Load Dataset 2 (All Overlapping)         |\n");
    printf("  |  3. Load Dataset 3 (Cleaning Time Test)      |\n");
    printf("  |  4. Load Dataset 4 (Peak Hour)               |\n");
    printf("  |  5. Load Dataset 5 (Boundary Test)           |\n");
    printf("  |  6. Input Custom Flights                     |\n");
    printf("  |  7. Run Algorithm on Loaded Data             |\n");
    printf("  |  0. Exit Program                             |\n");
    printf("  +-----------------------------------------------+\n");
    printf("  Enter your choice: ");
}

/**
 * Runs the algorithm and displays all results.
 */
void runProgram(void) {
    if (numFlights == 0) {
        printf("\n  ERROR: No flights loaded! Please load a dataset first.\n");
        return;
    }
    
    printf("\n  Processing %d flights...\n", numFlights);
    
    /* Execute the greedy algorithm */
    clock_t start = clock();
    assignFlightsToGates();
    clock_t end = clock();
    
    double timeTaken = ((double)(end - start)) / CLOCKS_PER_SEC * 1000;
    
    /* Display all results */
    displayResults();
    displayCleaningSchedule();
    
    /* Display execution time */
    displayHeader("ALGORITHM PERFORMANCE");
    printf("\n  Execution time: %.4f milliseconds\n", timeTaken);
    printf("  Time complexity: O(n log n) for sorting + O(n * g) for assignment\n");
    printf("  Space complexity: O(n + g) for storing flights and gates\n");
    printf("  where n = %d flights, g = %d gates\n", numFlights, numGates);
}

/**
 * Main function - Entry point of the program.
 * Implements a user-friendly menu loop that allows multiple runs.
 * 
 * Reference: Menu-driven program structure as recommended by 
 *            Kernighan & Ritchie (1988), The C Programming Language.
 */
int main(void) {
    int choice;
    bool running = true;
    
    /* Display welcome message */
    printf("\n");
    printf("  ****************************************************\n");
    printf("  *                                                  *\n");
    printf("  *    EASTWEST INTERNATIONAL AIRPORT               *\n");
    printf("  *    Gate Scheduling Optimization System          *\n");
    printf("  *                                                  *\n");
    printf("  *    Algorithm: Greedy (Interval Partitioning)    *\n");
    printf("  *    Constraint: 20-minute cleaning between uses  *\n");
    printf("  *                                                  *\n");
    printf("  ****************************************************\n");
    
    /* Main menu loop - allows multiple runs without restarting */
    while (running) {
        displayMenu();
        
        if (scanf("%d", &choice) != 1) {
            printf("\n  ERROR: Invalid input. Please enter a number.\n");
            while (getchar() != '\n');  /* Clear input buffer */
            continue;
        }
        
        switch (choice) {
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
                loadDataset(choice);
                break;
                
            case 6:
                inputCustomFlights();
                break;
                
            case 7:
                runProgram();
                break;
                
            case 0:
                printf("\n  Thank you for using the Gate Scheduling System!\n");
                printf("  Goodbye.\n\n");
                running = false;
                break;
                
            default:
                printf("\n  ERROR: Invalid choice. Please select 0-7.\n");
        }
    }
    
    return 0;
}
