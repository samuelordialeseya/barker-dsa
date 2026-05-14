# B.A.R.K.E.R. (Basic Automated Ride Kontrol & Earnings Registry)

B.A.R.K.E.R. is a console-based management system designed to automate tricycle dispatching and earnings tracking for transport cooperatives. It uses a `std::map` data structure to link unique body numbers to driver profiles, allowing for instant data retrieval and registration. The system also manages a dispatch queue using a `std::vector` to ensure a fair "first-in, first-out" order for waiting drivers. This implementation focuses on data persistence and efficient search algorithms to provide a reliable tool for real-world tricycle terminal operations.

## 🚀 Features
- **Driver Management**: Register, search, and manage driver profiles.
- **Queue System**: Real-time queueing for drivers with "Waiting" and "Dispatched" states.
- **Earnings Tracking**: Automated recording of trip fares and daily total calculations.
- **Incident Reporting**: File and resolve complaints or lost items tied to specific driver body numbers.
- **Data Persistence**: Automatically saves data to CSV files for cross-platform compatibility.

## 🛠️ Built With
- **C++**: Core logic and data structures.
- **Standard Library**: Utilizing `map`, `vector`, and `algorithms` for efficient data handling.
- **CSV Storage**: Simple, human-readable data storage.

## 💻 How to Run
1. Ensure you have a C++ compiler installed (like `g++`).
2. Compile the source code:
   ```bash
   g++ main.cpp -o barker.exe
   ```
3. Run the executable:
   ```bash
   ./barker.exe
   ```

## 📋 Note on Data Files
The system looks for `drivers.csv` and `daily_earnings.csv` in the parent directory to maintain compatibility with other versions of the project.

---
Developed as a Final Project for Advanced Computer Programming (ACP).
