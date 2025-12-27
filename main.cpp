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
void loadDataset(int datasetNumber);
void inputCustomFlights(void);
bool validateTimeFormat(const char* timeStr);
void getFlightCode(char* buffer, int flightNum);
void clearInputBuffer(void);
void runAlgorithm(void);

/* UTILITY FUNCTIONS */
void clearInputBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
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

/* OUTPUT FUNCTIONS - FIXED TO MATCH REQUIREMENTS */
void displayResults(void) {
    int unassignedCount = 0;
    
    /* REQUIREMENT i: Display minimum number of gates */
    printf("\n============================================================\n");
    printf("  MINIMUM NUMBER OF GATES REQUIRED: %d\n", numGates);
    printf("============================================================\n\n");
    
    /* REQUIREMENT ii: Show list of flights assigned to specific gates */
    printf("FLIGHT ASSIGNMENTS TO GATES:\n");
    printf("+------------+----------+-----------+-------------------+\n");
    printf("| Flight No. | Arrival  | Departure | Gate Information  |\n");
    printf("+------------+----------+-----------+-------------------+\n");
    
    for (int i = 0; i < numFlights; i++) {
        printf("| %-10s | ", flights[i].flightCode);
        printTime(flights[i].arrivalTime);
        printf("  | ");
        printTime(flights[i].departureTime);
        printf("   | ");
        
        if (!flights[i].isAssigned) {
            printf("UNASSIGNED         |\n");
            unassignedCount++;
        } else {
            printf("Gate %-2d           |\n", flights[i].assignedGate);
        }
    }
    printf("+------------+----------+-----------+-------------------+\n");
    
    printf("\nSUMMARY:\n");
    printf("- Total flights processed: %d\n", numFlights);
    printf("- Flights successfully assigned: %d\n", numFlights - unassignedCount);
    printf("- Flights unassigned: %d\n", unassignedCount);
    printf("- Minimum gates utilized: %d\n\n", numGates);
}

void displayGateUnavailableTimings(void) {
    /* REQUIREMENT iii: List gate number and timings for cleaning */
    printf("\n============================================================\n");
    printf("  GATE UNAVAILABILITY TIMINGS DURING CLEANING AND CHECKS\n");
    printf("  (Each gate unavailable for %d minutes after departure)\n", CLEANING_TIME);
    printf("============================================================\n\n");
    
    bool hasCleaningPeriods = false;
    
    for (int g = 0; g < numGates; g++) {
        if (gates[g].cleaningCount > 0) {
            hasCleaningPeriods = true;
            printf("GATE %d CLEANING PERIODS:\n", g + 1);
            printf("+---------------------+---------------------+\n");
            printf("|    Start Time       |     End Time        |\n");
            printf("+---------------------+---------------------+\n");
            
            for (int c = 0; c < gates[g].cleaningCount; c++) {
                printf("|       ");
                printTime(gates[g].cleaningStartTimes[c]);
                printf("       |       ");
                printTime(gates[g].cleaningEndTimes[c]);
                printf("       |\n");
            }
            printf("+---------------------+---------------------+\n\n");
        }
    }
    
    if (!hasCleaningPeriods) {
        printf("No cleaning periods recorded.\n\n");
    }
}

/* DATASET LOADING - FIXED: Now loads only, doesn't run */
void loadDataset(int datasetNumber) {
    resetData();
    
    switch (datasetNumber) {
        case 1:
            displayHeader("LOADED: Teacher's Sample Dataset (20 Flights)");
            numFlights = 20;
            flights[0] = (Flight){1, "EWA101", 300, 390, -1, false};
            flights[1] = (Flight){2, "EWA102", 330, 420, -1, false};
            flights[2] = (Flight){3, "EWA103", 360, 450, -1, false};
            flights[3] = (Flight){4, "EWA104", 405, 510, -1, false};
            flights[4] = (Flight){5, "EWA105", 420, 540, -1, false};
            flights[5] = (Flight){6, "EWA106", 450, 570, -1, false};
            flights[6] = (Flight){7, "EWA107", 480, 600, -1, false};
            flights[7] = (Flight){8, "EWA108", 540, 630, -1, false};
            flights[8] = (Flight){9, "EWA109", 585, 690, -1, false};
            flights[9] = (Flight){10, "EWA110", 600, 720, -1, false};
            flights[10] = (Flight){11, "EWA111", 660, 750, -1, false};
            flights[11] = (Flight){12, "EWA112", 705, 795, -1, false};
            flights[12] = (Flight){13, "EWA113", 720, 825, -1, false};
            flights[13] = (Flight){14, "EWA114", 780, 870, -1, false};
            flights[14] = (Flight){15, "EWA115", 840, 930, -1, false};
            flights[15] = (Flight){16, "EWA116", 870, 960, -1, false};
            flights[16] = (Flight){17, "EWA117", 900, 1020, -1, false};
            flights[17] = (Flight){18, "EWA118", 960, 1065, -1, false};
            flights[18] = (Flight){19, "EWA119", 1020, 1140, -1, false};
            flights[19] = (Flight){20, "EWA120", 1080, 1200, -1, false};
            break;
            
        case 2:
            displayHeader("LOADED: Dataset 2 - Sequential Flights");
            numFlights = 4;
            flights[0] = (Flight){1, "FL201", 480, 600, -1, false};
            flights[1] = (Flight){2, "FL202", 630, 750, -1, false};
            flights[2] = (Flight){3, "FL203", 780, 900, -1, false};
            flights[3] = (Flight){4, "FL204", 930, 1050, -1, false};
            break;
            
        case 3:
            displayHeader("LOADED: Dataset 3 - Complete Overlap");
            numFlights = 5;
            flights[0] = (Flight){1, "FL301", 840, 960, -1, false};
            flights[1] = (Flight){2, "FL302", 840, 960, -1, false};
            flights[2] = (Flight){3, "FL303", 840, 960, -1, false};
            flights[3] = (Flight){4, "FL304", 840, 960, -1, false};
            flights[4] = (Flight){5, "FL305", 840, 960, -1, false};
            break;
            
        case 4:
            displayHeader("LOADED: Dataset 4 - Cleaning Time Critical");
            numFlights = 4;
            flights[0] = (Flight){1, "FL401", 540, 600, -1, false};
            flights[1] = (Flight){2, "FL402", 615, 675, -1, false};
            flights[2] = (Flight){3, "FL403", 640, 700, -1, false};
            flights[3] = (Flight){4, "FL404", 695, 755, -1, false};
            break;
            
        case 5:
            displayHeader("LOADED: Dataset 5 - Peak Hour Simulation");
            numFlights = 8;
            flights[0] = (Flight){1, "FL501", 1020, 1080, -1, false};
            flights[1] = (Flight){2, "FL502", 1035, 1095, -1, false};
            flights[2] = (Flight){3, "FL503", 1050, 1110, -1, false};
            flights[3] = (Flight){4, "FL504", 1065, 1125, -1, false};
            flights[4] = (Flight){5, "FL505", 1080, 1140, -1, false};
            flights[5] = (Flight){6, "FL506", 1100, 1160, -1, false};
            flights[6] = (Flight){7, "FL507", 1140, 1200, -1, false};
            flights[7] = (Flight){8, "FL508", 1170, 1230, -1, false};
            break;
            
        case 6:
            displayHeader("LOADED: Dataset 6 - Mixed Pattern");
            numFlights = 6;
            flights[0] = (Flight){1, "FL601", 480, 600, -1, false};
            flights[1] = (Flight){2, "FL602", 540, 660, -1, false};
            flights[2] = (Flight){3, "FL603", 630, 750, -1, false};
            flights[3] = (Flight){4, "FL604", 660, 780, -1, false};
            flights[4] = (Flight){5, "FL605", 760, 880, -1, false};
            flights[5] = (Flight){6, "FL606", 800, 920, -1, false};
            break;
    }
    
    printf("\nLoaded %d flights:\n", numFlights);
    printf("+------------+----------+-----------+\n");
    printf("|   Flight   |  Arrival | Departure |\n");
    printf("+------------+----------+-----------+\n");
    for (int i = 0; i < numFlights; i++) {
        printf("| %-10s |   ", flights[i].flightCode);
        printTime(flights[i].arrivalTime);
        printf("  |   ");
        printTime(flights[i].departureTime);
        printf("   |\n");
    }
    printf("+------------+----------+-----------+\n\n");
}

/* CUSTOM INPUT - FIXED: Now inputs only, doesn't run */
void inputCustomFlights(void) {
    char buffer[BUFFER_SIZE];
    char arrivalStr[10], departureStr[10];
    int n;
    
    resetData();
    displayHeader("CUSTOM FLIGHT INPUT");
    
    do {
        printf("\nEnter number of flights (1-%d): ", MAX_FLIGHTS);
        if (scanf("%d", &n) != 1 || n < 1 || n > MAX_FLIGHTS) {
            printf("ERROR: Please enter a number between 1 and %d.\n", MAX_FLIGHTS);
            clearInputBuffer();
            n = 0;
        }
    } while (n < 1 || n > MAX_FLIGHTS);
    
    clearInputBuffer();
    numFlights = n;
    
    printf("\nEnter flight details (HH:MM or HHMM format, 24-hour)\n");
    printf("Examples: 14:30 or 1430, 09:15 or 0915, 05:00 or 0500\n\n");
    
    for (int i = 0; i < numFlights; i++) {
        flights[i].flightNumber = i + 1;
        
        printf("Flight %d - Flight code (e.g., FL001): ", i + 1);
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = 0;
        strcpy(flights[i].flightCode, buffer);
        
        do {
            printf("Flight %s - Arrival time (HH:MM or HHMM): ", flights[i].flightCode);
            fgets(buffer, sizeof(buffer), stdin);
            buffer[strcspn(buffer, "\n")] = 0;
            strcpy(arrivalStr, buffer);
            
            if (!validateTimeFormat(arrivalStr)) {
                printf("ERROR: Invalid format. Use HH:MM or HHMM (00:00-23:59).\n");
                continue;
            }
            flights[i].arrivalTime = timeToMinutes(arrivalStr);
            break;
        } while (1);
        
        do {
            printf("Flight %s - Departure time (HH:MM or HHMM): ", flights[i].flightCode);
            fgets(buffer, sizeof(buffer), stdin);
            buffer[strcspn(buffer, "\n")] = 0;
            strcpy(departureStr, buffer);
            
            if (!validateTimeFormat(departureStr)) {
                printf("ERROR: Invalid format. Use HH:MM or HHMM (00:00-23:59).\n");
                continue;
            }
            
            int depTime = timeToMinutes(departureStr);
            if (depTime <= flights[i].arrivalTime) {
                printf("ERROR: Departure must be after arrival time.\n");
                continue;
            }
            
            flights[i].departureTime = depTime;
            break;
        } while (1);
        
        flights[i].assignedGate = -1;
        flights[i].isAssigned = false;
        printf("\n");
    }
    
    printf("Successfully entered %d flights.\n\n", numFlights);
}

/* RUN ALGORITHM - SEPARATE FUNCTION */
void runAlgorithm(void) {
    if (numFlights == 0) {
        printf("\nERROR: No flights loaded! Please load a dataset or enter flights first.\n\n");
        return;
    }
    
    printf("\n============================================================\n");
    printf("  RUNNING GREEDY ALGORITHM FOR GATE SCHEDULING\n");
    printf("============================================================\n\n");
    
    clock_t start = clock();
    assignFlightsToGates();
    clock_t end = clock();
    
    double timeTaken = ((double)(end - start)) / CLOCKS_PER_SEC * 1000000;
    
    /* DISPLAY ALL OUTPUT AT ONCE (as required) */
    displayResults();
    displayGateUnavailableTimings();
    
    /* Performance info (optional) */
    printf("ALGORITHM PERFORMANCE:\n");
    printf("- Execution time: %.2f microseconds\n", timeTaken);
    printf("- Time complexity: O(n log n) where n = %d flights\n", numFlights);
    printf("- Space complexity: O(n + g) where g = %d gates\n\n", numGates);
}

/* UPDATED MENU - Matches assignment requirements */
void displayMenu(void) {
    printf("    AIRPORT GATE SCHEDULING SYSTEM - GREEDY ALGORITHM\n");
    printf("============================================================\n");
    printf("    1. Load Teacher's Sample Dataset (20 flights)\n");
    printf("    2. Load Dataset 2 - Sequential Flights\n");
    printf("    3. Load Dataset 3 - Complete Overlap\n");
    printf("    4. Load Dataset 4 - Cleaning Time Critical\n");
    printf("    5. Load Dataset 5 - Peak Hour Simulation\n");
    printf("    6. Load Dataset 6 - Mixed Pattern\n");
    printf("    7. Enter Custom Flights\n");
    printf("    8. Run Greedy Algorithm\n");
    printf("    0. Exit\n");
    printf("============================================================\n");
    printf("Enter your choice (0-8): ");
}

/* MAIN FUNCTION - FIXED FLOW */
int main(void) {
    int choice;
    printf("\n");
    printf("============================================================\n");
    printf("                     Simple Guide                           \n");
    printf(" Below, list of datasets, to view && load them, pls input choices (1-6). \n");
    printf(" After loading the dataset, input choice 8 to run Greedy Algorithm. \n");
    printf("                     Thank You. \n");
    printf("============================================================\n");
    
    while (1) {
        displayMenu();
        
        if (scanf("%d", &choice) != 1) {
            printf("\nERROR: Invalid input. Please enter a number 0-8.\n");
            clearInputBuffer();
            continue;
        }
        clearInputBuffer();
        
        if (choice == 0) {
            printf("\nThank you for using the Gate Scheduling System!\n");
            printf("Goodbye!\n\n");
            break;
        }
        
        switch (choice) {
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
                loadDataset(choice);  // Just loads, doesn't run
                break;
            case 7:
                inputCustomFlights();  // Just inputs, doesn't run
                break;
            case 8:
                runAlgorithm();  // Separate run option
                break;
            default:
                printf("\nERROR: Invalid choice. Please enter 0-8.\n");
        }
    }
    
    return 0;
}
