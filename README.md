# Financial Data Analysis and Reporting System

A modern C++17 command-line system for processing financial transactions, calculating portfolio performance, and generating structured reports. The tool supports user-entered financial data, dynamic tax-gain optimization using a DP-based subset selection algorithm, and formatted reporting for audit or review.

## Features
- User input validation for dates, transaction types, and numeric values
- Object-oriented design with reusable `Account` and `Transaction` classes
- Dynamic programming algorithm for optimized realized-gain selection
- Automatic calculation of balance and realized P&L
- Structured, aligned report output using `iomanip`

## Build & Run
\`\`\`bash
g++ -std=c++17 -O2 -Wall -Wextra -pedantic main.cpp -o financial_engine
./financial_engine
\`\`\`

