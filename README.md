# B.A.R.K.E.R. (Basic Automated Ride Kontrol & Earnings Registry)

Welcome to **B.A.R.K.E.R.**, a robust Tricycle Dispatch System designed for efficient queue management and earnings tracking. This version is built in C++ and optimized for console-based operations.

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
