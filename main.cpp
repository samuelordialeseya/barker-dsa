#include <algorithm>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

// Struct to hold incident details
struct Incident {
  std::string type;
  std::string desc;
};

// Struct to hold driver profile data
// Similar to the python dictionary for a driver
struct Driver {
  string name;
  string standing;
  vector<Incident> incidents;
  string resolved;
};

// Struct to represent a driver in the dispatch queue
struct QueueItem {
  string body_num;
  string status; // "waiting" or "dispatched"
  string ride_type;
  double fare;
};

// Global variables to store our data (these replace the Python
// dictionaries/lists)
map<string, Driver> drivers_db;
vector<QueueItem> shift_line;

// File paths pointing to the parent folder so it shares data with the Python
// version
const string DRIVERS_FILE = "../drivers.csv";
const string EARNINGS_FILE = "../daily_earnings.csv";

// Helper function to split strings by a delimiter
// Used for parsing CSV rows and incident strings
vector<string> split(const string &str, const string &delimiter) {
  vector<string> tokens;
  size_t prev = 0, pos = 0;
  do {
    pos = str.find(delimiter, prev);
    if (pos == string::npos)
      pos = str.length();
    string token = str.substr(prev, pos - prev);
    tokens.push_back(token); // keep empty tokens for CSV parsing
    prev = pos + delimiter.length();
  } while (pos < str.length() && prev <= str.length());
  return tokens;
}

// Helper function to remove leading/trailing whitespace
string trim(const string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (string::npos == first) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

// Helper to sanitize input (remove characters that break CSV)
string sanitize(string str) {
    str.erase(remove(str.begin(), str.end(), ','), str.end());
    str.erase(remove(str.begin(), str.end(), '|'), str.end());
    return trim(str);
}

// Helper to get current time in YYYY-MM-DD HH:MM format
string get_current_time() {
  time_t now = time(0);
  tm *ltm = localtime(&now);
  char buffer[20];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M", ltm);
  return string(buffer);
}

// Helper to get current date in YYYY-MM-DD format
string get_current_date() {
  time_t now = time(0);
  tm *ltm = localtime(&now);
  char buffer[20];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d", ltm);
  return string(buffer);
}

// --- Data Loading & Saving ---

// Load drivers from CSV into memory (Equivalent to load_drivers in Python)
void load_drivers() {
  ifstream file(DRIVERS_FILE);
  if (!file.is_open())
    return;

  string line;
  while (getline(file, line)) {
    vector<string> row = split(line, ",");
    if (row.size() >= 2) {
      string body_num = row[0];
      Driver d;
      d.name = row[1];
      d.standing = (row.size() > 2 && !row[2].empty()) ? row[2] : "Good";

      // Parse incidents from format "type1|desc1||type2|desc2"
      string incidents_str = (row.size() > 3) ? row[3] : "";
      if (incidents_str != "" && incidents_str != "None") {
        vector<string> incident_pairs = split(incidents_str, "||");
        for (const string &pair : incident_pairs) {
          size_t pipe_pos = pair.find("|");
          if (pipe_pos != string::npos) {
            Incident inc;
            inc.type = pair.substr(0, pipe_pos);
            inc.desc = pair.substr(pipe_pos + 1);
            d.incidents.push_back(inc);
          }
        }
      }
      d.resolved = (row.size() > 4 && !row[4].empty()) ? row[4] : "Yes";
      drivers_db[body_num] = d;
    }
  }
  file.close();
}

// Save memory database to CSV (Equivalent to save_drivers in Python)
void save_drivers() {
  ofstream file(DRIVERS_FILE);
  for (auto const &[body_num, data] : drivers_db) {
    string incidents_str = "None";
    if (!data.incidents.empty()) {
      incidents_str = "";
      for (size_t i = 0; i < data.incidents.size(); ++i) {
        incidents_str += data.incidents[i].type + "|" + data.incidents[i].desc;
        if (i < data.incidents.size() - 1)
          incidents_str += "||";
      }
    }
    file << body_num << "," << data.name << "," << data.standing << ","
         << incidents_str << "," << data.resolved << "\n";
  }
  file.close();
}

// --- Dashboard Functions ---

// Add driver to waiting queue
void check_in_driver() {
  cout << "Enter Body Number to Check In: ";
  string body_val;
  getline(cin, body_val);
  body_val = trim(body_val);

  if (body_val.empty()) {
    cout << "[!] Please enter a Body Number.\n";
    return;
  }

  if (drivers_db.find(body_val) == drivers_db.end()) {
    cout << "[!] Body Number " << body_val << " is not registered.\n";
    return;
  }

  for (const auto &driver : shift_line) {
    if (driver.body_num == body_val) {
      cout << "[!] Driver " << body_val << " is already in the queue.\n";
      return;
    }
  }

  QueueItem new_item = {body_val, "waiting", "", 0.0};
  shift_line.push_back(new_item);
  cout << "[SUCCESS] Driver " << drivers_db[body_val].name
       << " added to queue as WAITING.\n";
}

// Display the current queue
void view_queue() {
  cout << "\n--- Current Queue ---\n";
  if (shift_line.empty()) {
    cout << "Queue is empty.\n";
    return;
  }

  for (size_t i = 0; i < shift_line.size(); ++i) {
    const auto &driver = shift_line[i];
    string name = "Unknown";
    if (drivers_db.find(driver.body_num) != drivers_db.end()) {
        name = drivers_db[driver.body_num].name;
    }
    cout << i + 1 << ". [" << driver.body_num << "] " << name << " - ";

    if (driver.status == "dispatched") {
      cout << "DISPATCHED (" << driver.ride_type << ", P" << driver.fare
           << ")\n";
    } else {
      cout << "WAITING\n";
    }
  }
}

// Remove driver from the queue manually
void remove_from_queue() {
  view_queue();
  if (shift_line.empty())
    return;

  cout << "Enter the line number to remove (0 to cancel): ";
  string input;
  getline(cin, input);
  try {
    int index = stoi(input) - 1;
    if (index >= 0 && index < shift_line.size()) {
      string body_num = trim(shift_line[index].body_num);
      shift_line.erase(shift_line.begin() +
                       index); // Removes element from vector
      cout << "[SUCCESS] Driver " << body_num << " removed from queue.\n";
    } else if (index != -1) {
      cout << "[!] Invalid line number.\n";
    }
  } catch (...) {
    cout << "[!] Invalid input.\n";
  }
}

// Move dispatched driver back to the end of the line
void complete_ride() {
  view_queue();
  if (shift_line.empty())
    return;

  cout << "Enter the line number of the driver returning (0 to cancel): ";
  string input;
  getline(cin, input);
  try {
    int index = stoi(input) - 1;
    if (index >= 0 && index < shift_line.size()) {
      if (shift_line[index].status != "dispatched") {
        cout << "[!] Only dispatched drivers can complete rides.\n";
        return;
      }

      QueueItem returning_driver = shift_line[index];
      shift_line.erase(shift_line.begin() + index); // Remove from current pos

      // Reset status and add to back of queue
      returning_driver.status = "waiting";
      returning_driver.ride_type = "";
      returning_driver.fare = 0.0;

      shift_line.push_back(returning_driver);
      cout << "[SUCCESS] Driver " << returning_driver.body_num
           << " arrived and returned to end of queue.\n";
    } else if (index != -1) {
      cout << "[!] Invalid line number.\n";
    }
  } catch (...) {
    cout << "[!] Invalid input.\n";
  }
}

// Log a finished trip to the CSV
void log_earnings(string body_num, string ride_type, double fare) {
  ofstream file(EARNINGS_FILE, ios::app); // Open in append mode
  if (file.is_open()) {
    file << get_current_time() << "," << body_num << "," << ride_type << ","
         << fare << "\n";
    file.close();
  }
}

// Send the first waiting driver on a ride
void dispatch_driver() {
  if (shift_line.empty()) {
    cout << "[!] The line is empty.\n";
    return;
  }

  // Find the first waiting driver
  int waiting_index = -1;
  for (size_t i = 0; i < shift_line.size(); ++i) {
    if (shift_line[i].status == "waiting") {
      waiting_index = i;
      break;
    }
  }

  if (waiting_index == -1) {
    cout << "[!] No waiting drivers to dispatch.\n";
    return;
  }

  cout << "Select Ride Type:\n1. Regular (P40)\n2. Special (Custom "
          "Fare)\nChoice: ";
  string choice;
  getline(cin, choice);

  string ride_type = "";
  double fare = 0.0;

  if (choice == "1") {
    ride_type = "Regular";
    fare = 40.0;
  } else if (choice == "2") {
    ride_type = "Special";
    cout << "Enter Special Fare (P): ";
    string fare_str;
    getline(cin, fare_str);
    try {
      fare = stod(fare_str);
    } catch (...) {
      cout << "[!] Invalid fare amount.\n";
      return;
    }
  } else {
    cout << "[!] Invalid choice.\n";
    return;
  }

  // Update the queue state
  shift_line[waiting_index].status = "dispatched";
  shift_line[waiting_index].ride_type = ride_type;
  shift_line[waiting_index].fare = fare;

  log_earnings(shift_line[waiting_index].body_num, ride_type, fare);

  cout << "[SUCCESS] Driver " << shift_line[waiting_index].body_num
       << " dispatched!\n";
  cout << "Total Fare Collected: P" << fare << "\n";
}

// Recursive function to calculate total earnings
// Iterative function to calculate total earnings (prevents stack overflow)
double calculate_total_earnings(const vector<double> &fares) {
  double total = 0;
  for (double fare : fares) {
    total += fare;
  }
  return total;
}

// Display daily earnings
void show_daily_total() {
  ifstream file(EARNINGS_FILE);
  if (!file.is_open()) {
    cout << "\nDaily Report: No dispatch records yet for today. Total: P0\n";
    return;
  }

  string today = get_current_date();
  vector<double> today_fares;
  string line;

  while (getline(file, line)) {
    vector<string> row = split(line, ",");
    if (row.size() >= 4) {
      string timestamp = row[0];
      if (timestamp.length() >= 10) {
        string date_part = timestamp.substr(0, 10); // Extract YYYY-MM-DD
        if (date_part == today) {
          try {
            today_fares.push_back(stod(row[3])); // Convert string to double
          } catch (...) {}
        }
      }
    }
  }
  file.close();

  if (today_fares.empty()) {
    cout << "\nDaily Report: No dispatch records yet for today. Total: P0\n";
  } else {
    double total = calculate_total_earnings(today_fares);
    cout << "\n--- Daily Report (" << today << ") ---\n";
    cout << "Total Fares Collected: P" << total << "\n";
    cout << "Total Trips: " << today_fares.size() << "\n";
  }
}

// Read trip history
void view_history() {
  cout << "Enter date to view (YYYY-MM-DD) or press Enter for today: ";
  string date_str;
  getline(cin, date_str);

  if (date_str.empty()) {
    date_str = get_current_date();
  }

  ifstream file(EARNINGS_FILE);
  if (!file.is_open()) {
    cout << "No trip records found.\n";
    return;
  }

  vector<vector<string>> transactions;
  string line;
  while (getline(file, line)) {
    vector<string> row = split(line, ",");
    if (row.size() >= 4) {
      string timestamp = row[0];
      if (timestamp.length() >= 10 && timestamp.substr(0, 10) == date_str) {
        transactions.push_back(row);
      }
    }
  }
  file.close();

  // Sort by timestamp descending (newest first)
  sort(transactions.begin(), transactions.end(),
       [](const vector<string> &a, const vector<string> &b) {
         return a[0] > b[0];
       });

  cout << "\n--- Trip History for " << date_str << " ---\n";
  if (transactions.empty()) {
    cout << "No trips recorded on this date.\n";
  } else {
    for (const auto &row : transactions) {
      string time = row[0];
      string body_num = row[1];
      string ride_type = row[2];
      string fare = row[3];

      string driver_name = "Unknown";
      if (drivers_db.find(body_num) != drivers_db.end()) {
        driver_name = drivers_db[body_num].name;
      }

      cout << time << " | Body: " << body_num << " | " << driver_name << " | "
           << ride_type << " | P" << fare << "\n";
    }
  }
}

// --- Admin Functions ---

void add_new_driver() {
  cout << "Enter Body Number: ";
  string body_num;
  getline(cin, body_num);
  body_num = trim(body_num);

  if (body_num.empty()) {
    cout << "[!] Body Number cannot be empty!\n";
    return;
  }

  if (drivers_db.find(body_num) != drivers_db.end()) {
    cout << "[!] Body Number " << body_num << " is already registered.\n";
    return;
  }

  cout << "Enter Driver Name: ";
  string name;
  getline(cin, name);
  name = sanitize(name);

  if (name.empty()) {
    cout << "[!] Driver Name cannot be empty (commas/pipes not allowed)!\n";
    return;
  }

  Driver d;
  d.name = name;
  d.standing = "Good";
  d.resolved = "Yes";
  drivers_db[body_num] = d;

  save_drivers();
  cout << "[SUCCESS] Driver " << body_num << " saved!\n";
}

void view_registered_drivers() {
  cout << "\n--- Registered Drivers ---\n";
  for (const auto &[body_num, data] : drivers_db) {
    cout << body_num << " - " << data.name << "\n";
  }
}

void delete_driver() {
  cout << "Enter Body Number to delete: ";
  string body_num;
  getline(cin, body_num);

  if (drivers_db.find(body_num) != drivers_db.end()) {
    drivers_db.erase(body_num);
    
    // Also remove from active queue if they are there
    shift_line.erase(
        remove_if(shift_line.begin(), shift_line.end(), [&](const QueueItem& item) {
            return item.body_num == body_num;
        }), 
        shift_line.end()
    );

    save_drivers();
    cout << "[SUCCESS] Driver deleted from database and removed from queue.\n";
  } else {
    cout << "[!] Driver not found.\n";
  }
}

void search_driver_profile() {
  cout << "Enter Body Number to search: ";
  string body_num;
  getline(cin, body_num);

  if (drivers_db.find(body_num) != drivers_db.end()) {
    const Driver &d = drivers_db[body_num];
    cout << "\n--- Profile for " << body_num << " ---\n";
    cout << "Name: " << d.name << "\n";
    cout << "Standing: " << d.standing << "\n\n";

    if (d.resolved == "Yes") {
      cout << "--- Records [CLEARED] ---\n";
    } else {
      cout << "--- Records [ACTIVE ISSUES] ---\n";
    }

    if (d.incidents.empty()) {
      cout << "No incidents on record.\n";
    } else {
      for (size_t i = 0; i < d.incidents.size(); ++i) {
        cout << "\n#" << i + 1 << "\n";
        cout << "Type: " << d.incidents[i].type << "\n";
        cout << "Details: " << d.incidents[i].desc << "\n";
      }
    }
  } else {
    cout << "[!] Driver not found.\n";
  }
}

void add_incident() {
  cout << "Enter Body Number: ";
  string body_num;
  getline(cin, body_num);

  if (drivers_db.find(body_num) == drivers_db.end()) {
    cout << "[!] Driver not found.\n";
    return;
  }

  cout << "Select Type (1. Complaint/Report, 2. Lost Item): ";
  string type_choice;
  getline(cin, type_choice);
  string type = (type_choice == "2") ? "Lost Item" : "Complaint / Report";

  cout << "Explain the issue/item: ";
  string desc;
  getline(cin, desc);

  if (desc.empty()) {
    cout << "[!] Description cannot be empty.\n";
    return;
  }

  Incident inc = {type, desc};
  drivers_db[body_num].incidents.push_back(inc);
  drivers_db[body_num].resolved = "No";
  drivers_db[body_num].standing = "Review Needed";

  save_drivers();
  cout << "[SUCCESS] New issue added to Driver " << body_num << "'s profile.\n";
}

void resolve_incident() {
  cout << "Enter Body Number: ";
  string body_num;
  getline(cin, body_num);

  if (drivers_db.find(body_num) == drivers_db.end()) {
    cout << "[!] Driver not found.\n";
    return;
  }

  Driver &d = drivers_db[body_num];
  if (d.incidents.empty()) {
    cout << "[INFO] This driver has no incidents to resolve.\n";
    return;
  }

  cout << "Enter issue number to resolve: ";
  string issue_num_str;
  getline(cin, issue_num_str);

  try {
    int issue_num = stoi(issue_num_str) - 1; // Convert 1-based to 0-based index
    if (issue_num >= 0 && issue_num < d.incidents.size()) {
      string removed_type = d.incidents[issue_num].type;
      d.incidents.erase(d.incidents.begin() + issue_num);

      // If no issues left, clear their record
      if (d.incidents.empty()) {
        d.resolved = "Yes";
        d.standing = "Good";
      }

      save_drivers();
      cout << "[SUCCESS] Issue #" << issue_num + 1 << " (" << removed_type
           << ") is removed!\n";
    } else {
      cout << "[!] Issue number not found.\n";
    }
  } catch (...) {
    cout << "[!] Invalid input.\n";
  }
}

// --- Menu Systems ---

// Sub-menu for Admin functions
void admin_menu() {
  while (true) {
    cout << "\n=== Admin Registry Menu ===\n";
    cout << "1. Add New Driver\n";
    cout << "2. View Registered Drivers\n";
    cout << "3. Remove Driver from Database\n";
    cout << "4. Look Up Driver Profile\n";
    cout << "5. File a Report or Lost Item\n";
    cout << "6. Resolve Specific Issue\n";
    cout << "0. Back to Main Menu\n";
    cout << "Choice: ";

    string choice;
    getline(cin, choice);

    if (choice == "1")
      add_new_driver();
    else if (choice == "2")
      view_registered_drivers();
    else if (choice == "3")
      delete_driver();
    else if (choice == "4")
      search_driver_profile();
    else if (choice == "5")
      add_incident();
    else if (choice == "6")
      resolve_incident();
    else if (choice == "0")
      break; // Exit loop to return to main menu
    else
      cout << "Invalid choice. Please try again.\n";
  }
}

// Main program entry point
int main() {
  load_drivers(); // Load existing data on startup

  cout << "Welcome to B.A.R.K.E.R. - Tricycle Dispatch System (Console Edition)\n";
  cout << "(Basic Automated Ride Kontrol & Earnings Registry)\n";

  while (true) {
    cout << "\n=== Tricycle Dashboard Menu ===\n";
    cout << "1. Check In Driver\n";
    cout << "2. View Queue\n";
    cout << "3. Dispatch Tricycle\n";
    cout << "4. Complete Ride (Return to Queue)\n";
    cout << "5. Remove Driver from Queue\n";
    cout << "6. Calculate Daily Total\n";
    cout << "7. View Trip History\n";
    cout << "8. Admin Registry Menu\n";
    cout << "0. Exit\n";
    cout << "Choice: ";

    string choice;
    getline(cin, choice);

    if (choice == "1")
      check_in_driver();
    else if (choice == "2")
      view_queue();
    else if (choice == "3")
      dispatch_driver();
    else if (choice == "4")
      complete_ride();
    else if (choice == "5")
      remove_from_queue();
    else if (choice == "6")
      show_daily_total();
    else if (choice == "7")
      view_history();
    else if (choice == "8")
      admin_menu();
    else if (choice == "0")
      break; // Exit application
    else
      cout << "Invalid choice. Please try again.\n";
  }

  cout << "Exiting system. Goodbye!\n";
  return 0;
}
