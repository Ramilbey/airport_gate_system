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
};

// Gate structure to track availability
struct Gate {
    int gateId;
    int freeTime;    // when gate becomes available (including cleaning)
    vector<pair<int, int>> occupiedTimes; // (start, end) pairs
};

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

// Greedy gate assignment algorithm
vector<Gate> assignGates(vector<Flight>& flights, int cleaningTime = 20) {
    // Sort flights by arrival time
    sort(flights.begin(), flights.end(),
         [](const Flight& a, const Flight& b) {
             return a.arrival < b.arrival;
         });

    // Min-heap (priority queue) storing (freeTime, gateIndex)
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> gateQueue;

    vector<Gate> gates;
    int gateCounter = 1;

    for (auto& flight : flights) {
        bool assigned = false;

        // Try to assign to existing gate
        if (!gateQueue.empty()) {
            auto earliest = gateQueue.top();
            if (earliest.first <= flight.arrival) {
                // Assign to this gate
                gateQueue.pop();
                int gateIdx = earliest.second;
                gates[gateIdx].occupiedTimes.push_back({flight.arrival, flight.departure});
                gates[gateIdx].freeTime = flight.departure + cleaningTime;
                gateQueue.push({gates[gateIdx].freeTime, gateIdx});
                flight.gate = gates[gateIdx].gateId;
                assigned = true;
            }
        }

        // If no gate available, create new gate
        if (!assigned) {
            Gate newGate;
            newGate.gateId = gateCounter++;
            newGate.occupiedTimes.push_back({flight.arrival, flight.departure});
            newGate.freeTime = flight.departure + cleaningTime;
            gates.push_back(newGate);
            gateQueue.push({newGate.freeTime, gates.size() - 1});
            flight.gate = newGate.gateId;
        }
    }

    return gates;
}

// Display results
void displayResults(const vector<Flight>& flights, const vector<Gate>& gates) {
    cout << "\n========== GATE ASSIGNMENT RESULTS ==========\n";

    // Minimum gates needed
    cout << "Minimum gates required: " << gates.size() << endl;

    // Flight assignments
    cout << "\n--- Flight Assignments ---\n";
    cout << left << setw(10) << "Flight"
         << setw(12) << "Arrival"
         << setw(12) << "Departure"
         << setw(10) << "Gate" << endl;
    cout << string(44, '-') << endl;

    for (const auto& flight : flights) {
        cout << left << setw(10) << flight.flightNo
             << setw(12) << minutesToTime(flight.arrival)
             << setw(12) << minutesToTime(flight.departure)
             << setw(10) << (flight.gate > 0 ? "Gate " + to_string(flight.gate) : "UNASSIGNED")
             << endl;
    }

    // Gate unavailable times (cleaning)
    cout << "\n--- Gate Cleaning/Unavailable Times ---\n";
    for (const auto& gate : gates) {
        cout << "\nGate " << gate.gateId << " cleaning periods:\n";
        for (const auto& occupied : gate.occupiedTimes) {
            int cleanStart = occupied.second;
            int cleanEnd = occupied.second + 20; // 20 min cleaning
            cout << "  " << minutesToTime(cleanStart) << " - "
                 << minutesToTime(cleanEnd) << " (after flight departure)\n";
        }
    }

    // Gate utilization summary
    cout << "\n--- Gate Utilization Summary ---\n";
    for (const auto& gate : gates) {
        cout << "Gate " << gate.gateId << ": "
             << gate.occupiedTimes.size() << " flights assigned\n";
    }
}

// Datasets
vector<vector<Flight>> createDatasets() {
    vector<vector<Flight>> datasets(5);

    // Dataset 1: Provided dataset (20 flights)
    datasets[0] = {
        {"EWA101", 500, 630}, {"EWA102", 530, 700}, {"EWA103", 600, 730},
        {"EWA104", 645, 830}, {"EWA105", 700, 900}, {"EWA106", 730, 930},
        {"EWA107", 800, 1000}, {"EWA108", 900, 1030}, {"EWA109", 945, 1130},
        {"EWA110", 1000, 1200}, {"EWA111", 1100, 1230}, {"EWA112", 1145, 1315},
        {"EWA113", 1200, 1345}, {"EWA114", 1300, 1430}, {"EWA115", 1400, 1530},
        {"EWA116", 1430, 1600}, {"EWA117", 1500, 1700}, {"EWA118", 1600, 1745},
        {"EWA119", 1700, 1900}, {"EWA120", 1800, 2000}
    };

    // Dataset 2: All non-overlapping (3 flights, needs 1 gate)
    datasets[1] = {
        {"EWB201", 800, 930},   // 08:00-09:30
        {"EWB202", 1000, 1130}, // 10:00-11:30 (30 min after cleaning)
        {"EWB203", 1200, 1330}  // 12:00-13:30
    };

    // Dataset 3: All overlapping (4 flights, needs 4 gates)
    datasets[2] = {
        {"EWC301", 900, 1030},  // 09:00-10:30
        {"EWC302", 915, 1045},  // 09:15-10:45
        {"EWC303", 930, 1100},  // 09:30-11:00
        {"EWC304", 945, 1115}   // 09:45-11:15
    };

    // Dataset 4: Mixed with cleaning reuse (5 flights, needs 2 gates)
    datasets[3] = {
        {"EWD401", 800, 900},   // 08:00-09:00 (Gate 1)
        {"EWD402", 920, 1020},  // 09:20-10:20 (Gate 1 after cleaning)
        {"EWD403", 800, 1000},  // 08:00-10:00 (Gate 2)
        {"EWD404", 1020, 1120}, // 10:20-11:20 (Gate 2 after cleaning)
        {"EWD405", 1140, 1240}  // 11:40-12:40 (Gate 1 after cleaning)
    };

    // Dataset 5: Realistic large schedule (15 flights)
    datasets[4] = {
        {"EWE501", 600, 720},   {"EWE502", 630, 750},   {"EWE503", 700, 820},
        {"EWE504", 730, 850},   {"EWE505", 800, 920},   {"EWE506", 830, 950},
        {"EWE507", 900, 1020},  {"EWE508", 930, 1050},  {"EWE509", 1000, 1120},
        {"EWE510", 1030, 1150}, {"EWE511", 1100, 1220}, {"EWE512", 1130, 1250},
        {"EWE513", 1200, 1320}, {"EWE514", 1230, 1350}, {"EWE515", 1300, 1420}
    };

    return datasets;
}

int main() {
    cout << "=== AIRPORT GATE SCHEDULING SYSTEM ===\n";
    cout << "Using Greedy Algorithm with 20-minute cleaning time\n\n";

    vector<vector<Flight>> datasets = createDatasets();

    int choice;
    cout << "Choose input method:\n";
    cout << "1. Use predefined dataset (1-5)\n";
    cout << "2. Enter flights manually\n";
    cout << "Choice: ";
    cin >> choice;

    vector<Flight> flights;

    if (choice == 1) {
        int datasetChoice;
        cout << "\nSelect dataset (1-5):\n";
        cout << "1. Provided dataset (20 flights)\n";
        cout << "2. Non-overlapping flights (3 flights)\n";
        cout << "3. All overlapping (4 flights)\n";
        cout << "4. Mixed with cleaning reuse (5 flights)\n";
        cout << "5. Realistic large schedule (15 flights)\n";
        cout << "Choice: ";
        cin >> datasetChoice;

        if (datasetChoice >= 1 && datasetChoice <= 5) {
            flights = datasets[datasetChoice - 1];
        } else {
            cout << "Invalid choice. Using dataset 1.\n";
            flights = datasets[0];
        }
    } else {
        // Manual input
        int n;
        cout << "\nEnter number of flights: ";
        cin >> n;

        for (int i = 0; i < n; i++) {
            Flight f;
            string arrivalStr, departureStr;

            cout << "\nFlight " << (i + 1) << ":\n";
            cout << "Flight number: ";
            cin >> f.flightNo;
            cout << "Arrival time (HHMM): ";
            cin >> arrivalStr;
            cout << "Departure time (HHMM): ";
            cin >> departureStr;

            f.arrival = timeToMinutes(arrivalStr);
            f.departure = timeToMinutes(departureStr);
            flights.push_back(f);
        }
    }

    // Run greedy algorithm
    vector<Gate> gates = assignGates(flights);

    // Display results
    displayResults(flights, gates);

    return 0;
}

