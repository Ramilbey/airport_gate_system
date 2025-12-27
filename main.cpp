#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

/* CONSTANTS */
#define MAX_FLIGHTS 100
#define MAX_GATES 50
#define CLEANING_TIME 20
#define BUFFER_SIZE 100

/* DATA STRUCTURES */
typedef struct {
    int flightNumber;
    char flightCode[10];
    int arrivalTime;
    int departureTime;
    int assignedGate;
    bool isAssigned;
} Flight;

typedef struct {
    int gateNumber;
    int availableFrom;
    int flightsAssigned;
    int cleaningStartTimes[MAX_FLIGHTS];
    int cleaningEndTimes[MAX_FLIGHTS];
    int cleaningCount;
} Gate;

/* GLOBAL VARIABLES */
Flight flights[MAX_FLIGHTS];
Gate gates[MAX_GATES];
int numFlights = 0;
int numGates = 0;

/* FUNCTION PROTOTYPES */
void displayMenu(void);
void displayHeader(const char* title);
int timeToMinutes(const char* timeStr);
void formatTime(int totalMinutes, char* buffer);
void printTime(int totalMinutes);
int compareFlightsByArrival(const void* a, const void* b);
void initializeGates(void);
void resetData(void);
int findAvailableGate(int arrivalTime);
void assignFlightsToGates(void);
void displayResults(void);
void displayGateUnavailableTimings(void);
void loadDatasetAndRun(int datasetNumber);
void inputCustomFlightsAndRun(void);
void runAlgorithmAndDisplay(void);
bool validateTimeFormat(const char* timeStr);
void getFlightCode(char* buffer, int flightNum);
void clearInputBuffer(void);
void waitForEnter(void);

/* UTILITY FUNCTIONS */
void clearInputBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void waitForEnter(void) {
    printf("\n  Press Enter to continue...");
    clearInputBuffer();
}

int timeToMinutes(const char* timeStr) {
    int hours, minutes;
    if (sscanf(timeStr, "%d:%d", &hours, &minutes) != 2) {
        // Try without colon (0500 format)
        if (strlen(timeStr) == 4) {
            sscanf(timeStr, "%2d%2d", &hours, &minutes);
            return hours * 60 + minutes;
        }
        return -1;
    }
    return hours * 60 + minutes;
}

void formatTime(int totalMinutes, char* buffer) {
    int hours = totalMinutes / 60;
    int minutes = totalMinutes % 60;
    sprintf(buffer, "%02d:%02d", hours, minutes);
}

void printTime(int totalMinutes) {
    char timeStr[10];
    formatTime(totalMinutes, timeStr);
    printf("%s", timeStr);
}

bool validateTimeFormat(const char* timeStr) {
    int hours, minutes;
    
    // Try HH:MM format first
    if (sscanf(timeStr, "%d:%d", &hours, &minutes) == 2) {
        return (hours >= 0 && hours <= 23 && minutes >= 0 && minutes <= 59);
    }
    
    // Try HHMM format (like 0500)
    if (strlen(timeStr) == 4) {
        if (sscanf(timeStr, "%2d%2d", &hours, &minutes) == 2) {
            return (hours >= 0 && hours <= 23 && minutes >= 0 && minutes <= 59);
        }
    }
    
    return false;
}

void getFlightCode(char* buffer, int flightNum) {
    const char* airlines[] = {"AA", "BA", "CX", "EK", "SQ", "MH", "QF", "JL"};
    int airlineIndex = flightNum % 8;
    sprintf(buffer, "%s%03d", airlines[airlineIndex], flightNum);
}

void displayHeader(const char* title) {
    printf("\n");
    for (int i = 0; i < 60; i++) printf("=");
    printf("\n  %s\n", title);
    for (int i = 0; i < 60; i++) printf("=");
    printf("\n");
}

/* INITIALIZATION */
void initializeGates(void) {
    for (int i = 0; i < MAX_GATES; i++) {
        gates[i].gateNumber = i + 1;
        gates[i].availableFrom = 0;
        gates[i].flightsAssigned = 0;
        gates[i].cleaningCount = 0;
    }
    numGates = 0;
}

void resetData(void) {
    numFlights = 0;
    numGates = 0;
    initializeGates();
    
    for (int i = 0; i < MAX_FLIGHTS; i++) {
        flights[i].flightNumber = 0;
        strcpy(flights[i].flightCode, "");
        flights[i].arrivalTime = 0;
        flights[i].departureTime = 0;
        flights[i].assignedGate = -1;
        flights[i].isAssigned = false;
    }
}

/* GREEDY ALGORITHM */
int compareFlightsByArrival(const void* a, const void* b) {
    Flight* flightA = (Flight*)a;
    Flight* flightB = (Flight*)b;
    return flightA->arrivalTime - flightB->arrivalTime;
}

int findAvailableGate(int arrivalTime) {
    int earliestGate = -1;
    int earliestTime = 24 * 60 + 1;
    
    for (int i = 0; i < numGates; i++) {
        if (gates[i].availableFrom <= arrivalTime && 
            gates[i].availableFrom < earliestTime) {
            earliestTime = gates[i].availableFrom;
            earliestGate = i;
        }
    }
    
    if (earliestGate != -1) {
        return earliestGate;
    }
    
    if (numGates < MAX_GATES) {
        return numGates;
    }
    
    return -1;
}

void assignFlightsToGates(void) {
    qsort(flights, numFlights, sizeof(Flight), compareFlightsByArrival);
    
    initializeGates();
    
    for (int i = 0; i < numFlights; i++) {
        int gateIndex = findAvailableGate(flights[i].arrivalTime);
        
        if (gateIndex == -1) {
            flights[i].assignedGate = -1;
            flights[i].isAssigned = false;
        } else {
            flights[i].assignedGate = gateIndex + 1;
            flights[i].isAssigned = true;
            
            gates[gateIndex].availableFrom = flights[i].departureTime + CLEANING_TIME;
            gates[gateIndex].flightsAssigned++;
            
            int cleanIdx = gates[gateIndex].cleaningCount;
            gates[gateIndex].cleaningStartTimes[cleanIdx] = flights[i].departureTime;
            gates[gateIndex].cleaningEndTimes[cleanIdx] = flights[i].departureTime + CLEANING_TIME;
            gates[gateIndex].cleaningCount++;
            
            if (gateIndex >= numGates) {
                numGates = gateIndex + 1;
            }
        }
    }
}

/* OUTPUT FUNCTIONS */
void displayResults(void) {
    int unassignedCount = 0;
    
    displayHeader("GATE ASSIGNMENT RESULTS");
    
    printf("\n  MINIMUM NUMBER OF GATES REQUIRED: %d\n\n", numGates);
    
    printf("  +------------+----------+-----------+-------------+\n");
    printf("  |   Flight   |  Arrival | Departure | Gate Info   |\n");
    printf("  +------------+----------+-----------+-------------+\n");
    
    for (int i = 0; i < numFlights; i++) {
        printf("  | %-10s |   ", flights[i].flightCode);
        printTime(flights[i].arrivalTime);
        printf("  |   ");
        printTime(flights[i].departureTime);
        printf("   |");
        
        if (!flights[i].isAssigned) {
            printf(" UNASSIGNED  |\n");
            unassignedCount++;
        } else {
            printf(" Gate %-3d    |\n", flights[i].assignedGate);
        }
    }
    printf("  +------------+----------+-----------+-------------+\n");
    
    printf("\n  SUMMARY:\n");
    printf("  - Total flights: %d\n", numFlights);
    printf("  - Successfully assigned: %d\n", numFlights - unassignedCount);
    printf("  - Unassigned flights: %d\n", unassignedCount);
    printf("  - Gates utilized: %d\n\n", numGates);
}

void displayGateUnavailableTimings(void) {
    displayHeader("GATE UNAVAILABILITY TIMINGS");
    printf("\n  (Gates unavailable for %d minutes after each departure)\n\n", CLEANING_TIME);
    
    bool hasCleaningPeriods = false;
    
    for (int g = 0; g < numGates; g++) {
        if (gates[g].cleaningCount > 0) {
            hasCleaningPeriods = true;
            printf("  Gate %d:\n", g + 1);
            printf("  +---------------------+---------------------+\n");
            printf("  |    Start Time       |     End Time        |\n");
            printf("  +---------------------+---------------------+\n");
            
            for (int c = 0; c < gates[g].cleaningCount; c++) {
                printf("  |       ");
                printTime(gates[g].cleaningStartTimes[c]);
                printf("       |       ");
                printTime(gates[g].cleaningEndTimes[c]);
                printf("       |\n");
            }
            printf("  +---------------------+---------------------+\n\n");
        }
    }
    
    if (!hasCleaningPeriods) {
        printf("  No cleaning periods recorded.\n");
    }
}

/* DATASET LOADING - Teacher's sample as FIRST option */
void loadDatasetAndRun(int datasetNumber) {
    resetData();
    
    switch (datasetNumber) {
        case 1:
            displayHeader("DATASET 1: Teacher's Sample (20 Flights)");
            numFlights = 20;
            // Given sample dataset - FIRST OPTION
            flights[0] = (Flight){1, "EWA101", 300, 390, -1, false};   // 05:00-06:30
            flights[1] = (Flight){2, "EWA102", 330, 420, -1, false};   // 05:30-07:00
            flights[2] = (Flight){3, "EWA103", 360, 450, -1, false};   // 06:00-07:30
            flights[3] = (Flight){4, "EWA104", 405, 510, -1, false};   // 06:45-08:30
            flights[4] = (Flight){5, "EWA105", 420, 540, -1, false};   // 07:00-09:00
            flights[5] = (Flight){6, "EWA106", 450, 570, -1, false};   // 07:30-09:30
            flights[6] = (Flight){7, "EWA107", 480, 600, -1, false};   // 08:00-10:00
            flights[7] = (Flight){8, "EWA108", 540, 630, -1, false};   // 09:00-10:30
            flights[8] = (Flight){9, "EWA109", 585, 690, -1, false};   // 09:45-11:30
            flights[9] = (Flight){10, "EWA110", 600, 720, -1, false};  // 10:00-12:00
            flights[10] = (Flight){11, "EWA111", 660, 750, -1, false}; // 11:00-12:30
            flights[11] = (Flight){12, "EWA112", 705, 795, -1, false}; // 11:45-13:15
            flights[12] = (Flight){13, "EWA113", 720, 825, -1, false}; // 12:00-13:45
            flights[13] = (Flight){14, "EWA114", 780, 870, -1, false}; // 13:00-14:30
            flights[14] = (Flight){15, "EWA115", 840, 930, -1, false}; // 14:00-15:30
            flights[15] = (Flight){16, "EWA116", 870, 960, -1, false}; // 14:30-16:00
            flights[16] = (Flight){17, "EWA117", 900, 1020, -1, false};// 15:00-17:00
            flights[17] = (Flight){18, "EWA118", 960, 1065, -1, false};// 16:00-17:45
            flights[18] = (Flight){19, "EWA119", 1020, 1140, -1, false};// 17:00-19:00
            flights[19] = (Flight){20, "EWA120", 1080, 1200, -1, false};// 18:00-20:00
            break;
            
        case 2:
            displayHeader("DATASET 2: Optimal Case - 1 Gate Sufficient");
            numFlights = 4;
            flights[0] = (Flight){1, "AA101", 480, 600, -1, false};    // 08:00-10:00
            flights[1] = (Flight){2, "BA102", 630, 750, -1, false};    // 10:30-12:30
            flights[2] = (Flight){3, "CX103", 780, 900, -1, false};    // 13:00-15:00
            flights[3] = (Flight){4, "EK104", 930, 1050, -1, false};   // 15:30-17:30
            break;
            
        case 3:
            displayHeader("DATASET 3: Worst Case - All Overlap");
            numFlights = 5;
            flights[0] = (Flight){1, "AA201", 840, 960, -1, false};    // 14:00-16:00
            flights[1] = (Flight){2, "BA202", 840, 960, -1, false};    // 14:00-16:00
            flights[2] = (Flight){3, "CX203", 840, 960, -1, false};    // 14:00-16:00
            flights[3] = (Flight){4, "EK204", 840, 960, -1, false};    // 14:00-16:00
            flights[4] = (Flight){5, "SQ205", 840, 960, -1, false};    // 14:00-16:00
            break;
            
        case 4:
            displayHeader("DATASET 4: Cleaning Time Critical");
            numFlights = 4;
            flights[0] = (Flight){1, "AA301", 540, 600, -1, false};    // 09:00-10:00
            flights[1] = (Flight){2, "BA302", 615, 675, -1, false};    // 10:15-11:15
            flights[2] = (Flight){3, "CX303", 640, 700, -1, false};    // 10:40-11:40
            flights[3] = (Flight){4, "EK304", 695, 755, -1, false};    // 11:35-12:35
            break;
            
        case 5:
            displayHeader("DATASET 5: Real Peak Hours");
            numFlights = 8;
            flights[0] = (Flight){1, "AA401", 1020, 1080, -1, false};  // 17:00-18:00
            flights[1] = (Flight){2, "BA402", 1035, 1095, -1, false};  // 17:15-18:15
            flights[2] = (Flight){3, "CX403", 1050, 1110, -1, false};  // 17:30-18:30
            flights[3] = (Flight){4, "EK404", 1065, 1125, -1, false};  // 17:45-18:45
            flights[4] = (Flight){5, "SQ405", 1080, 1140, -1, false};  // 18:00-19:00
            flights[5] = (Flight){6, "MH406", 1100, 1160, -1, false};  // 18:20-19:20
            flights[6] = (Flight){7, "QF407", 1140, 1200, -1, false};  // 19:00-20:00
            flights[7] = (Flight){8, "JL408", 1170, 1230, -1, false};  // 19:30-20:30
            break;
            
        case 6:
            displayHeader("DATASET 6: Mixed Pattern");
            numFlights = 6;
            flights[0] = (Flight){1, "AA501", 480, 600, -1, false};    // 08:00-10:00
            flights[1] = (Flight){2, "BA502", 540, 660, -1, false};    // 09:00-11:00
            flights[2] = (Flight){3, "CX503", 630, 750, -1, false};    // 10:30-12:30
            flights[3] = (Flight){4, "EK504", 660, 780, -1, false};    // 11:00-13:00
            flights[4] = (Flight){5, "SQ505", 760, 880, -1, false};    // 12:40-14:40
            flights[5] = (Flight){6, "MH506", 800, 920, -1, false};    // 13:20-15:20
            break;
    }
    
    printf("\n  Loaded %d flights:\n", numFlights);
    printf("  +------------+----------+-----------+\n");
    printf("  |   Flight   |  Arrival | Departure |\n");
    printf("  +------------+----------+-----------+\n");
    for (int i = 0; i < numFlights; i++) {
        printf("  | %-10s |   ", flights[i].flightCode);
        printTime(flights[i].arrivalTime);
        printf("  |   ");
        printTime(flights[i].departureTime);
        printf("   |\n");
    }
    printf("  +------------+----------+-----------+\n");
    
    printf("\n  Running greedy algorithm...\n");
    runAlgorithmAndDisplay();
}

/* CUSTOM INPUT */
void inputCustomFlightsAndRun(void) {
    char buffer[BUFFER_SIZE];
    char arrivalStr[10], departureStr[10];
    int n;
    
    resetData();
    displayHeader("CUSTOM FLIGHT INPUT");
    
    do {
        printf("\n  Enter number of flights (1-%d): ", MAX_FLIGHTS);
        if (scanf("%d", &n) != 1 || n < 1 || n > MAX_FLIGHTS) {
            printf("  ERROR: Please enter a number between 1 and %d.\n", MAX_FLIGHTS);
            clearInputBuffer();
            n = 0;
        }
    } while (n < 1 || n > MAX_FLIGHTS);
    
    clearInputBuffer();
    numFlights = n;
    
    printf("\n  Enter flight details (HH:MM or HHMM format, 24-hour)\n");
    printf("  Examples: 14:30 or 1430, 09:15 or 0915, 05:00 or 0500\n\n");
    
    for (int i = 0; i < numFlights; i++) {
        flights[i].flightNumber = i + 1;
        
        // Get flight code
        printf("  Flight %d - Flight code (e.g., EWA101): ", i + 1);
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = 0;
        strcpy(flights[i].flightCode, buffer);
        
        do {
            printf("  Flight %s - Arrival time (HH:MM or HHMM): ", flights[i].flightCode);
            fgets(buffer, sizeof(buffer), stdin);
            buffer[strcspn(buffer, "\n")] = 0;
            strcpy(arrivalStr, buffer);
            
            if (!validateTimeFormat(arrivalStr)) {
                printf("  ERROR: Invalid format. Use HH:MM or HHMM (00:00-23:59).\n");
                continue;
            }
            flights[i].arrivalTime = timeToMinutes(arrivalStr);
            break;
        } while (1);
        
        do {
            printf("  Flight %s - Departure time (HH:MM or HHMM): ", flights[i].flightCode);
            fgets(buffer, sizeof(buffer), stdin);
            buffer[strcspn(buffer, "\n")] = 0;
            strcpy(departureStr, buffer);
            
            if (!validateTimeFormat(departureStr)) {
                printf("  ERROR: Invalid format. Use HH:MM or HHMM (00:00-23:59).\n");
                continue;
            }
            
            int depTime = timeToMinutes(departureStr);
            if (depTime <= flights[i].arrivalTime) {
                printf("  ERROR: Departure must be after arrival time.\n");
                continue;
            }
            
            flights[i].departureTime = depTime;
            break;
        } while (1);
        
        flights[i].assignedGate = -1;
        flights[i].isAssigned = false;
        printf("\n");
    }
    
    printf("  Successfully entered %d flights.\n", numFlights);
    
    printf("\n  Entered flights:\n");
    printf("  +------------+----------+-----------+\n");
    printf("  |   Flight   |  Arrival | Departure |\n");
    printf("  +------------+----------+-----------+\n");
    for (int i = 0; i < numFlights; i++) {
        printf("  | %-10s |   ", flights[i].flightCode);
        printTime(flights[i].arrivalTime);
        printf("  |   ");
        printTime(flights[i].departureTime);
        printf("   |\n");
    }
    printf("  +------------+----------+-----------+\n");
    
    printf("\n  Running greedy algorithm...\n");
    runAlgorithmAndDisplay();
}

/* RUN ALGORITHM AND DISPLAY */
void runAlgorithmAndDisplay(void) {
    if (numFlights == 0) {
        printf("\n  ERROR: No flights to process!\n");
        waitForEnter();
        return;
    }
    
    clock_t start = clock();
    assignFlightsToGates();
    clock_t end = clock();
    
    double timeTaken = ((double)(end - start)) / CLOCKS_PER_SEC * 1000000;
    
    displayResults();
    displayGateUnavailableTimings();
    
    displayHeader("ALGORITHM PERFORMANCE");
    printf("\n  Execution time: %.2f microseconds\n", timeTaken);
    printf("  Time complexity: O(n log n) for sorting\n");
    printf("                 + O(n * g) for assignment\n");
    printf("  Space complexity: O(n + g)\n");
    printf("  where n = %d flights, g = %d gates\n", numFlights, numGates);
    printf("\n  Greedy optimality: The algorithm always finds the minimum\n");
    printf("  number of gates needed (proven by 'stays ahead' argument).\n");
    
    waitForEnter();
}

/* UPDATED MENU - Teacher's sample as FIRST option */
void displayMenu(void) {
    printf("\n");
    printf("  +==============================================+\n");
    printf("  |   1. The Given Sample Dataset                |\n");
    printf("  |   2. Load Dataset 2 (Optimal: 1 Gate)        |\n");
    printf("  |   3. Load Dataset 3 (Worst: 5 Gates)         |\n");
    printf("  |   4. Load Dataset 4 (Cleaning Critical)      |\n");
    printf("  |   5. Load Dataset 5 (Peak Hour: 4 Gates)     |\n");
    printf("  |   6. Load Dataset 6 (Mixed: 2 Gates)         |\n");
    printf("  |   7. Input Custom Flights                    |\n");
    printf("  |   0. Exit                                    |\n");
    printf("  +==============================================+\n");
    printf("\n  Enter your choice (0-7): ");
}

/* MAIN FUNCTION */
int main(void) {
    int choice;
    
    while (1) {
        displayMenu();
        
        if (scanf("%d", &choice) != 1) {
            printf("\n  ERROR: Invalid input. Please enter a number 0-7.\n");
            clearInputBuffer();
            continue;
        }
        clearInputBuffer();
        
        if (choice == 0) {
            printf("\n  Thank you for using the Gate Scheduling System!\n");
            printf("  Goodbye!\n\n");
            break;
        }
        
        switch (choice) {
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
                loadDatasetAndRun(choice);
                break;
            case 7:
                inputCustomFlightsAndRun();
                break;
            default:
                printf("\n  ERROR: Invalid choice. Please enter 0-7.\n");
        }
    }
    
    return 0;
}
