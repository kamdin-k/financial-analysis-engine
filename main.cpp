#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <memory>
#include <limits>
#include <algorithm>
#include <regex>

// ---------------------------------------
// ENUMS & STRUCTS
// ---------------------------------------

enum class TxType {
    Deposit,
    Withdrawal,
    Buy,
    Sell
};

struct Transaction {
    std::string date;
    TxType type;
    double amount;
    double gainLoss;

    Transaction(const std::string& d, TxType t, double amt, double gl = 0.0)
        : date(d), type(t), amount(amt), gainLoss(gl) {}
};

// ---------------------------------------
// ACCOUNT BASE CLASS
// ---------------------------------------

class Account {
public:
    virtual ~Account() = default;

    virtual void addTransaction(const Transaction& tx) {
        transactions.push_back(tx);
    }

    virtual double computeBalance() const {
        double balance = 0.0;
        for (const auto& tx : transactions) {
            if (tx.type == TxType::Deposit || tx.type == TxType::Sell) balance += tx.amount;
            else balance -= tx.amount;
        }
        return balance;
    }

    virtual double computeRealizedPnL() const {
        double pnl = 0.0;
        for (const auto& tx : transactions) pnl += tx.gainLoss;
        return pnl;
    }

    const std::vector<Transaction>& getTransactions() const {
        return transactions;
    }

    virtual std::string name() const = 0;

protected:
    std::vector<Transaction> transactions;
};

class InvestmentAccount : public Account {
public:
    std::string name() const override {
        return "Investment Account";
    }
};

// ---------------------------------------
// TAX OPTIMIZATION (DP)
// ---------------------------------------

struct TaxOptimizationResult {
    double targetLimit;
    double optimizedGain;
    std::vector<size_t> chosenIndices;
};

TaxOptimizationResult optimizeTaxLots(const std::vector<double>& gains, double targetLimit) {
    size_t n = gains.size();
    if (n == 0 || targetLimit <= 0.0) return {targetLimit, 0.0, {}};

    int scaledTarget = static_cast<int>(targetLimit * 100 + 0.5);

    std::vector<std::vector<bool>> dp(n + 1, std::vector<bool>(scaledTarget + 1, false));
    dp[0][0] = true;

    for (size_t i = 1; i <= n; ++i) {
        int val = static_cast<int>(gains[i - 1] * 100 + 0.5);
        for (int g = 0; g <= scaledTarget; ++g) {
            bool without = dp[i - 1][g];
            bool with = (g - val >= 0) && dp[i - 1][g - val];
            dp[i][g] = without || with;
        }
    }

    int best = 0;
    for (int g = scaledTarget; g >= 0; --g) {
        if (dp[n][g]) { best = g; break; }
    }

    std::vector<size_t> chosen;
    int g = best;
    for (size_t i = n; i > 0; --i) {
        int val = static_cast<int>(gains[i - 1] * 100 + 0.5);
        if (g >= val && dp[i - 1][g - val]) {
            chosen.push_back(i - 1);
            g -= val;
        }
    }
    std::reverse(chosen.begin(), chosen.end());

    return {targetLimit, best / 100.0, chosen};
}

// ---------------------------------------
// INPUT VALIDATION HELPERS
// ---------------------------------------

bool validateDate(const std::string& date) {
    std::regex pattern(R"(\d{4}-\d{2}-\d{2})");
    return std::regex_match(date, pattern);
}

int validatedInt(const std::string& prompt, int min, int max) {
    int value;
    while (true) {
        std::cout << prompt;
        if (std::cin >> value && value >= min && value <= max) return value;

        std::cout << "Invalid input. Please enter a number between "
                  << min << " and " << max << ".\n";

        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}

double validatedDouble(const std::string& prompt, bool allowNegative = false) {
    double value;
    while (true) {
        std::cout << prompt;
        if (std::cin >> value) {
            if (!allowNegative && value < 0) {
                std::cout << "Value cannot be negative.\n";
                continue;
            }
            return value;
        }
        std::cout << "Invalid number. Try again.\n";

        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}

std::string validatedDate(const std::string& prompt) {
    std::string date;
    while (true) {
        std::cout << prompt;
        std::cin >> date;
        if (validateDate(date)) return date;
        std::cout << "Invalid date format. Use YYYY-MM-DD.\n";
    }
}

// ---------------------------------------
// REPORTING
// ---------------------------------------

void printAccountReport(const Account& account, double taxLimit) {
    double balance = account.computeBalance();
    double pnl = account.computeRealizedPnL();

    std::cout << "\n===== " << account.name() << " =====\n";
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Current balance: " << std::setw(12) << balance << "\n";
    std::cout << "Realized P&L :  " << std::setw(12) << pnl << "\n\n";

    std::vector<double> gains;
    for (auto& tx : account.getTransactions())
        if (tx.gainLoss > 0) gains.push_back(tx.gainLoss);

    if (!gains.empty()) {
        auto result = optimizeTaxLots(gains, taxLimit);
        std::cout << "Tax optimization (target cap: " << taxLimit << ")\n";
        std::cout << "Optimized gains: " << result.optimizedGain << "\n";
        std::cout << "Chosen tax lots: " << result.chosenIndices.size() << "\n\n";
    }

    std::cout << "Transaction summary:\n";
    std::cout << std::left << std::setw(12) << "Date"
              << std::setw(12) << "Type"
              << std::setw(12) << "Amount"
              << std::setw(12) << "Gain/Loss" << "\n";

    for (const auto& tx : account.getTransactions()) {
        std::string typeStr;
        switch (tx.type) {
            case TxType::Deposit:    typeStr = "Deposit"; break;
            case TxType::Withdrawal: typeStr = "Withdraw"; break;
            case TxType::Buy:        typeStr = "Buy"; break;
            case TxType::Sell:       typeStr = "Sell"; break;
        }

        std::cout << std::left
                  << std::setw(12) << tx.date
                  << std::setw(12) << typeStr
                  << std::setw(12) << tx.amount
                  << std::setw(12) << tx.gainLoss
                  << "\n";
    }

    std::cout << "==============================" << std::endl;
}

// ---------------------------------------
// MAIN PROGRAM (WITH VALIDATION)
// ---------------------------------------

int main() {
    std::unique_ptr<Account> account = std::make_unique<InvestmentAccount>();

    int n = validatedInt("Enter number of transactions: ", 1, 1000);

    for (int i = 0; i < n; i++) {
        std::cout << "\n--- Transaction " << (i + 1) << " ---\n";

        std::string date = validatedDate("Date (YYYY-MM-DD): ");
        int typeChoice = validatedInt("Type (1=Deposit, 2=Withdraw, 3=Buy, 4=Sell): ", 1, 4);

        double amount = validatedDouble("Amount: ");

        double gainLoss = 0.0;
        if (typeChoice == 4) { 
            gainLoss = validatedDouble("Realized Gain/Loss (use negative for loss): ", true);
        }

        TxType type = (typeChoice == 1 ? TxType::Deposit :
                       typeChoice == 2 ? TxType::Withdrawal :
                       typeChoice == 3 ? TxType::Buy :
                                         TxType::Sell);

        account->addTransaction(Transaction{date, type, amount, gainLoss});
    }

    double taxLimit = validatedDouble("\nEnter yearly gain limit for tax optimization: ");

    printAccountReport(*account, taxLimit);

    return 0;
}
