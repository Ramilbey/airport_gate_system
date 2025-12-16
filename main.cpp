#include <iostream>
#include <vector>
#include <algorithm>
#include <queue>
#include <iomanip>
#include <sstream>

using namespace std;

// Flight structure
struct Flight {
    string flightNo;
    int arrival;     // minutes from midnight
    int departure;   // minutes from midnight
    int gate;        // assigned gate number (0 = unassigned)

    Flight(string fn, int arr, int dep) : flightNo(fn), arrival(arr), departure(dep), gate(0) {}
};

// Gate structure to track availability
struct Gate {
    int gateId;
    int freeTime;    // when gate becomes available (including cleaning)
    vector<pair<int, int>> occupiedTimes; // (start, end) pairs

    Gate(int id) : gateId(id), freeTime(0) {}
};

// Validate time format HHMM
bool isValidTimeFormat(const string& timeStr) {
    if (timeStr.length() != 4) return false;
    for (char c : timeStr) {
        if (!isdigit(c)) return false;
    }
    int hours = stoi(timeStr.substr(0, 2));
    int minutes = stoi(timeStr.substr(2, 2));
    return (hours >= 0 && hours <= 23 && minutes >= 0 && minutes <= 59);
}

// Convert HHMM to minutes
int timeToMinutes(string timeStr) {
    int hours = stoi(timeStr.substr(0, 2));
    int minutes = stoi(timeStr.substr(2, 2));
    return hours * 60 + minutes;
}

// Convert minutes to HHMM
string minutesToTime(int minutes) {
    int hours = minutes / 60;
    int mins = minutes % 60;
    stringstream ss;
    ss << setw(2) << setfill('0') << hours
       << setw(2) << setfill('0') << mins;
    return ss.str();
}

// Greedy gate assignment algorithm with optional gate limit
vector<Gate> assignGates(vector<Flight>& flights, int cleaningTime = 20, int maxGates = -1) {
    // Sort flights by arrival time
    sort(flights.begin(), flights.end(),
         [](const Flight& a, const Flight& b) {
             return a.arrival < b.arrival;
         });

    // Min-heap (priority queue) storing (freeTime, gateIndex)
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> gateQueue;

    vector<Gate> gates;
    int gateCounter = 1;

    for (size_t i = 0; i < flights.size(); i++) {
        bool assigned = false;

        // Try to assign to existing gate
        if (!gateQueue.empty()) {
            auto earliest = gateQueue.top();
            if (earliest.first <= flights[i].arrival) {
                // Assign to this gate
                gateQueue.pop();
                int gateIdx = earliest.second;
                gates[gateIdx].occupiedTimes.push_back(make_pair(flights[i].arrival, flights[i].departure));
                gates[gateIdx].freeTime = flights[i].departure + cleaningTime;
                gateQueue.push(make_pair(gates[gateIdx].freeTime, gateIdx));
                flights[i].gate = gates[gateIdx].gateId;
                assigned = true;
            }
        }

        // If no gate available, create new gate (if limit not reached)
        if (!assigned) {
            // Check if we can create a new gate
            if (maxGates == -1 || gateCounter <= maxGates) {
                Gate newGate(gateCounter++);
                newGate.occupiedTimes.push_back(make_pair(flights[i].arrival, flights[i].departure));
                newGate.freeTime = flights[i].departure + cleaningTime;
                gates.push_back(newGate);
                gateQueue.push(make_pair(newGate.freeTime, gates.size() - 1));
                flights[i].gate = newGate.gateId;
            } else {
                // Gate limit reached - flight cannot be assigned
                flights[i].gate = 0; // UNASSIGNED
            }
        }
    }

    return gates;
}

// Display results with detailed unavailable periods
void displayResults(const vector<Flight>& flights, const vector<Gate>& gates, int cleaningTime = 20) {
    cout << "\n========== GATE ASSIGNMENT RESULTS ==========\n";

    // Count unassigned flights
    int unassignedCount = 0;
    for (size_t i = 0; i < flights.size(); i++) {
        if (flights[i].gate == 0) unassignedCount++;
    }

    // Minimum gates needed vs used
    cout << "Gates used: " << gates.size() << endl;
    cout << "Flights successfully assigned: " << (flights.size() - unassignedCount) << endl;
    if (unassignedCount > 0) {
        cout << "Flights UNASSIGNED: " << unassignedCount << " (need to delay/divert)\n";
    }

    // Flight assignments
    cout << "\n--- Flight Assignments ---\n";
    cout << left << setw(10) << "Flight"
         << setw(12) << "Arrival"
         << setw(12) << "Departure"
         << setw(10) << "Gate" << endl;
    cout << string(44, '-') << endl;

    for (size_t i = 0; i < flights.size(); i++) {
        cout << left << setw(10) << flights[i].flightNo
             << setw(12) << minutesToTime(flights[i].arrival)
             << setw(12) << minutesToTime(flights[i].departure);

        if (flights[i].gate > 0) {
            cout << setw(10) << ("Gate " + to_string(flights[i].gate));
        } else {
            cout << setw(10) << "**UNASSIGNED**";
        }
        cout << endl;
    }

    // Gate unavailable periods (CRITICAL: shows occupancy + cleaning)
    cout << "\n--- Gate Unavailable Times (Occupancy + Cleaning) ---\n";
    for (size_t i = 0; i < gates.size(); i++) {
        cout << "\nGate " << gates[i].gateId << " unavailable periods:\n";

        for (size_t j = 0; j < gates[i].occupiedTimes.size(); j++) {
            int flightStart = gates[i].occupiedTimes[j].first;
            int flightEnd = gates[i].occupiedTimes[j].second;
            int cleanEnd = flightEnd + cleaningTime;

            // Find which flight is using this gate at this time
            string flightNo = "Unknown";
            for (size_t k = 0; k < flights.size(); k++) {
                if (flights[k].gate == gates[i].gateId &&
                    flights[k].arrival == flightStart &&
                    flights[k].departure == flightEnd) {
                    flightNo = flights[k].flightNo;
                    break;
                }
            }

            // Show flight occupancy period
            cout << "  " << minutesToTime(flightStart) << " - "
                 << minutesToTime(flightEnd) << " (Flight " << flightNo << " - OCCUPIED)\n";

            // Show cleaning period
            cout << "  " << minutesToTime(flightEnd) << " - "
                 << minutesToTime(cleanEnd) << " (CLEANING - Unavailable)\n";
        }

        // Show total unavailable time for this gate
        if (!gates[i].occupiedTimes.empty()) {
            int firstStart = gates[i].occupiedTimes[0].first;
            int lastEnd = gates[i].occupiedTimes[gates[i].occupiedTimes.size()-1].second + cleaningTime;
            int totalMinutes = 0;
            for (size_t j = 0; j < gates[i].occupiedTimes.size(); j++) {
                totalMinutes += gates[i].occupiedTimes[j].second - gates[i].occupiedTimes[j].first + cleaningTime;
            }
            cout << "  Total unavailable time: " << totalMinutes << " minutes\n";
        }
    }

    // Gate utilization summary
    cout << "\n--- Gate Utilization Summary ---\n";
    for (size_t i = 0; i < gates.size(); i++) {
        cout << "Gate " << gates[i].gateId << ": "
             << gates[i].occupiedTimes.size() << " flights assigned\n";
    }

    // Recommendations for unassigned flights
    if (unassignedCount > 0) {
        cout << "\n--- UNASSIGNED Flights Recommendations ---\n";
        cout << "The following flights could not be assigned due to gate capacity:\n";
        for (size_t i = 0; i < flights.size(); i++) {
            if (flights[i].gate == 0) {
                cout << "  - " << flights[i].flightNo
                     << " (Arrival: " << minutesToTime(flights[i].arrival) << ")\n";
                cout << "    Action: Delay arrival or divert to another terminal\n";
            }
        }
    }
}

// Create datasets - CodeBlocks compatible
vector<vector<Flight>> createDatasets() {
    vector<vector<Flight>> datasets;

    // Dataset 1: Provided dataset (20 flights)
    vector<Flight> dataset1;
    dataset1.push_back(Flight("EWA101", 500, 630));
    dataset1.push_back(Flight("EWA102", 530, 700));
    dataset1.push_back(Flight("EWA103", 600, 730));
    dataset1.push_back(Flight("EWA104", 645, 830));
    dataset1.push_back(Flight("EWA105", 700, 900));
    dataset1.push_back(Flight("EWA106", 730, 930));
    dataset1.push_back(Flight("EWA107", 800, 1000));
    dataset1.push_back(Flight("EWA108", 900, 1030));
    dataset1.push_back(Flight("EWA109", 945, 1130));
    dataset1.push_back(Flight("EWA110", 1000, 1200));
    dataset1.push_back(Flight("EWA111", 1100, 1230));
    dataset1.push_back(Flight("EWA112", 1145, 1315));
    dataset1.push_back(Flight("EWA113", 1200, 1345));
    dataset1.push_back(Flight("EWA114", 1300, 1430));
    dataset1.push_back(Flight("EWA115", 1400, 1530));
    dataset1.push_back(Flight("EWA116", 1430, 1600));
    dataset1.push_back(Flight("EWA117", 1500, 1700));
    dataset1.push_back(Flight("EWA118", 1600, 1745));
    dataset1.push_back(Flight("EWA119", 1700, 1900));
    dataset1.push_back(Flight("EWA120", 1800, 2000));
    datasets.push_back(dataset1);

    // Dataset 2: All non-overlapping (3 flights, needs 1 gate)
    vector<Flight> dataset2;
    dataset2.push_back(Flight("EWB201", 800, 930));
    dataset2.push_back(Flight("EWB202", 1000, 1130));
    dataset2.push_back(Flight("EWB203", 1200, 1330));
    datasets.push_back(dataset2);

    // Dataset 3: All overlapping (4 flights, needs 4 gates)
    vector<Flight> dataset3;
    dataset3.push_back(Flight("EWC301", 900, 1030));
    dataset3.push_back(Flight("EWC302", 915, 1045));
    dataset3.push_back(Flight("EWC303", 930, 1100));
    dataset3.push_back(Flight("EWC304", 945, 1115));
    datasets.push_back(dataset3);

    // Dataset 4: Mixed with cleaning reuse (5 flights, needs 2 gates)
    vector<Flight> dataset4;
    dataset4.push_back(Flight("EWD401", 800, 900));
    dataset4.push_back(Flight("EWD402", 920, 1020));
    dataset4.push_back(Flight("EWD403", 800, 1000));
    dataset4.push_back(Flight("EWD404", 1020, 1120));
    dataset4.push_back(Flight("EWD405", 1140, 1240));
    datasets.push_back(dataset4);

    // Dataset 5: Realistic large schedule (15 flights)
    vector<Flight> dataset5;
    dataset5.push_back(Flight("EWE501", 600, 720));
    dataset5.push_back(Flight("EWE502", 630, 750));
    dataset5.push_back(Flight("EWE503", 700, 820));
    dataset5.push_back(Flight("EWE504", 730, 850));
    dataset5.push_back(Flight("EWE505", 800, 920));
    dataset5.push_back(Flight("EWE506", 830, 950));
    dataset5.push_back(Flight("EWE507", 900, 1020));
    dataset5.push_back(Flight("EWE508", 930, 1050));
    dataset5.push_back(Flight("EWE509", 1000, 1120));
    dataset5.push_back(Flight("EWE510", 1030, 1150));
    dataset5.push_back(Flight("EWE511", 1100, 1220));
    dataset5.push_back(Flight("EWE512", 1130, 1250));
    dataset5.push_back(Flight("EWE513", 1200, 1320));
    dataset5.push_back(Flight("EWE514", 1230, 1350));
    dataset5.push_back(Flight("EWE515", 1300, 1420));
    datasets.push_back(dataset5);

    // Dataset 6: Limited gates scenario (10 flights, only 3 gates available)
    // This will show UNASSIGNED flights when capacity is exceeded
    vector<Flight> dataset6;
    dataset6.push_back(Flight("EWF601", 800, 900));   // Gate 1
    dataset6.push_back(Flight("EWF602", 800, 930));   // Gate 2
    dataset6.push_back(Flight("EWF603", 800, 1000));  // Gate 3
    dataset6.push_back(Flight("EWF604", 830, 930));   // UNASSIGNED (all gates busy)
    dataset6.push_back(Flight("EWF605", 850, 950));   // UNASSIGNED (all gates busy)
    dataset6.push_back(Flight("EWF606", 920, 1020));  // Gate 1 (after cleaning)
    dataset6.push_back(Flight("EWF607", 950, 1050));  // Gate 2 (after cleaning)
    dataset6.push_back(Flight("EWF608", 1020, 1120)); // Gate 3 (after cleaning)
    dataset6.push_back(Flight("EWF609", 1040, 1140)); // Gate 1 (after cleaning)
    dataset6.push_back(Flight("EWF610", 1070, 1170)); // Gate 2 (after cleaning)
    datasets.push_back(dataset6);

    return datasets;
}

int main() {
    cout << "=== AIRPORT GATE SCHEDULING SYSTEM ===\n";
    cout << "Using Greedy Algorithm with 20-minute cleaning time\n\n";

    vector<vector<Flight>> datasets = createDatasets();

    int choice;
    bool validChoice = false;

    // Validate main menu choice
    while (!validChoice) {
        cout << "Choose input method:\n";
        cout << "1. Use predefined dataset (1-6)\n";
        cout << "2. Enter flights manually\n";
        cout << "Choice: ";

        if (!(cin >> choice)) {
            cout << "Error: Invalid input. Please enter 1 or 2.\n\n";
            cin.clear();
            cin.ignore(10000, '\n');
            continue;
        }

        if (choice == 1 || choice == 2) {
            validChoice = true;
        } else {
            cout << "Error: Invalid choice. Please enter 1 or 2.\n\n";
        }
    }

    vector<Flight> flights;
    int maxGates = -1; // -1 means unlimited gates

    if (choice == 1) {
        int datasetChoice;
        validChoice = false;

        // Validate dataset choice
        while (!validChoice) {
            cout << "\nSelect dataset (1-6):\n";
            cout << "1. Provided dataset (20 flights)\n";
            cout << "2. Non-overlapping flights (3 flights, optimal: 1 gate)\n";
            cout << "3. All overlapping (4 flights, optimal: 4 gates)\n";
            cout << "4. Mixed with cleaning reuse (5 flights, optimal: 2 gates)\n";
            cout << "5. Realistic large schedule (15 flights)\n";
            cout << "6. Limited gates scenario (10 flights, 3 gates max - shows UNASSIGNED)\n";
            cout << "Choice: ";

            if (!(cin >> datasetChoice)) {
                cout << "Error: Invalid input. Please enter a number between 1-6.\n";
                cin.clear();
                cin.ignore(10000, '\n');
                continue;
            }

            if (datasetChoice >= 1 && datasetChoice <= 6) {
                flights = datasets[datasetChoice - 1];

                // Dataset 6 has limited gates
                if (datasetChoice == 6) {
                    maxGates = 3;
                    cout << "\n*** This dataset simulates LIMITED GATE CAPACITY (3 gates) ***\n";
                    cout << "*** Some flights will be UNASSIGNED ***\n";
                }

                validChoice = true;
            } else {
                cout << "Error: Invalid choice. Please enter a number between 1-6.\n\n";
            }
        }
    } else {
        // Manual input with validation
        int n;

        // Validate number of flights
        while (true) {
            cout << "\nEnter number of flights (1-100): ";

            if (!(cin >> n)) {
                cout << "Error: Invalid input. Please enter a valid number.\n";
                cin.clear();
                cin.ignore(10000, '\n');
                continue;
            }

            if (n < 1 || n > 100) {
                cout << "Error: Number of flights must be between 1 and 100.\n";
                continue;
            }

            break;
        }

        // Ask if user wants to limit gates
        char limitGates;
        cout << "\nDo you want to limit the number of available gates? (y/n): ";
        cin >> limitGates;

        if (limitGates == 'y' || limitGates == 'Y') {
            while (true) {
                cout << "Enter maximum number of gates (1-20): ";

                if (!(cin >> maxGates)) {
                    cout << "Error: Invalid input.\n";
                    cin.clear();
                    cin.ignore(10000, '\n');
                    continue;
                }

                if (maxGates < 1 || maxGates > 20) {
                    cout << "Error: Gates must be between 1 and 20.\n";
                    continue;
                }

                cout << "\n*** GATE LIMIT SET: " << maxGates << " gates ***\n";
                cout << "*** Flights may be UNASSIGNED if capacity exceeded ***\n";
                break;
            }
        }

        for (int i = 0; i < n; i++) {
            string flightNo, arrivalStr, departureStr;
            int arrival, departure;
            bool validFlight = false;

            cout << "\n--- Flight " << (i + 1) << " ---\n";

            // Get flight number
            cout << "Flight number: ";
            cin >> flightNo;

            // Validate flight number not empty
            if (flightNo.empty()) {
                cout << "Error: Flight number cannot be empty.\n";
                i--;
                continue;
            }

            // Validate arrival time
            while (true) {
                cout << "Arrival time (HHMM, e.g., 0830): ";
                cin >> arrivalStr;

                if (!isValidTimeFormat(arrivalStr)) {
                    cout << "Error: Invalid time format. Use HHMM (e.g., 0830 for 8:30 AM).\n";
                    cout << "       Hours: 00-23, Minutes: 00-59\n";
                    continue;
                }

                arrival = timeToMinutes(arrivalStr);
                break;
            }

            // Validate departure time
            while (true) {
                cout << "Departure time (HHMM, e.g., 1015): ";
                cin >> departureStr;

                if (!isValidTimeFormat(departureStr)) {
                    cout << "Error: Invalid time format. Use HHMM (e.g., 1015 for 10:15 AM).\n";
                    cout << "       Hours: 00-23, Minutes: 00-59\n";
                    continue;
                }

                departure = timeToMinutes(departureStr);

                // Check departure is after arrival
                if (departure <= arrival) {
                    cout << "Error: Departure time must be after arrival time.\n";
                    cout << "       Arrival: " << arrivalStr << ", Departure: " << departureStr << "\n";
                    continue;
                }

                // Check reasonable flight duration (at least 30 minutes, max 12 hours)
                int duration = departure - arrival;
                if (duration < 30) {
                    cout << "Warning: Very short turnaround time (" << duration << " minutes).\n";
                    cout << "         Minimum recommended: 30 minutes. Continue? (y/n): ";
                    char confirm;
                    cin >> confirm;
                    if (confirm != 'y' && confirm != 'Y') {
                        continue;
                    }
                }

                if (duration > 720) {
                    cout << "Warning: Very long duration (" << duration/60 << " hours).\n";
                    cout << "         Maximum recommended: 12 hours. Continue? (y/n): ";
                    char confirm;
                    cin >> confirm;
                    if (confirm != 'y' && confirm != 'Y') {
                        continue;
                    }
                }

                break;
            }

            flights.push_back(Flight(flightNo, arrival, departure));
            cout << "Flight " << flightNo << " added successfully.\n";
        }

        cout << "\nTotal flights entered: " << flights.size() << "\n";
    }

    // Run greedy algorithm
    if (flights.empty()) {
        cout << "\nError: No flights to schedule.\n";
        return 1;
    }

    cout << "\nProcessing " << flights.size() << " flights...\n";
    if (maxGates != -1) {
        cout << "Gate capacity limit: " << maxGates << " gates\n";
    }

    vector<Gate> gates = assignGates(flights, 20, maxGates);

    // Display results
    displayResults(flights, gates, 20);

    return 0;
}