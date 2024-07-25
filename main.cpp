#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <ctime>
#include <cstdlib>
#include <cmath>
#include <algorithm>
#include "./ascii_art.h"

using namespace std;

#define VERSION "1.0.0"

struct RGB {
	int r;
	int g;
	int b;

	RGB(int r, int g, int b) : r(r), g(g), b(b) {}
	operator string () const {
		return "\033[38;2;" + to_string(r) + ";" + to_string(g) + ";" + to_string(b) + "m";
	}
	RGB operator +(const RGB& other) const {
		return { r | other.r, g | other.g, b | other.b };
	}
	RGB operator +=(const RGB& other) {
		return *this = *this + other;
	}
	RGB getInterpolation(const RGB& other, float t) const {
		int r = this->r + (other.r - this->r) * t;
		int g = this->g + (other.g - this->g) * t;
		int b = this->b + (other.b - this->b) * t;
		return { r, g, b };
	}
};
int getIncrementValue(const vector<int>& values, int maxIncrements) {
	int max = 0;
	for (int value : values) {
		if (value > max) {
			max = value;
		}
	}

	int increment = 1;
	while (max / increment > maxIncrements) {
		increment *= 5;
	}
	return increment;
}
void displayGraph(const vector<int>& x, const vector<int>& y) {
	const string blocks[9] = { "_", "▁", "▂", "▃", "▄", "▅", "▆", "▇", "█" };
	int increment = getIncrementValue(y, 8);
	for (int i = 0; i < y.size(); i++) {
		int blocksIndex = y[i] / increment;
		RGB color = { 255, 0, 0 };
		color = color.getInterpolation({ 0, 255, 0 }, static_cast<float>(blocksIndex) / 8);
		cout << (string)color;
		cout << blocks[blocksIndex];
		cout << "\033[0m";
	}
}
struct Point {
	int x;
	int y;

	Point(int x, int y) : x(x), y(y) {}
};
struct Line {
	Point start;
	Point end;

	Line(Point start, Point end) : start(start), end(end) {}
	bool isOnLine(const Point& point) {
		int dx = end.x - start.x;
		int dy = end.y - start.y;
		int dx1 = point.x - start.x;
		int dy1 = point.y - start.y;
		int dx2 = point.x - end.x;
		int dy2 = point.y - end.y;
		return dx * dy1 == dx1 * dy;
	}
};
struct ComplexLine {
	vector<Point> points;

	ComplexLine(const vector<Point>& points) : points(points) {}
	bool isOnLine(const Point& point) {
		for (int i = 0; i < points.size() - 1; i++) {
			Line line(points[i], points[i + 1]);
			if (line.isOnLine(point)) {
				return true;
			}
		}
		return false;
	}
};
class Graph {
private:
	int incrementX, incrementY;
	int maxX, maxY;
	float scaleX, scaleY;

	void setIncrements() {
		vector<int> xValues;
		vector<int> yValues;
		for (const auto& line : lines) {
			for (const auto& point : line.points) {
				xValues.push_back(point.x);
				yValues.push_back(point.y);
			}
		}
		incrementX = getIncrementValue(xValues, maxX);
		incrementY = getIncrementValue(yValues, maxY);
	}

	void setLineColor() {
		int n = lines.size();

		vector<RGB> baseColors = {
			{255, 0, 0},
			{0, 255, 0},
			{0, 0, 255},
			{255, 255, 0},
			{0, 255, 255},
			{255, 0, 255}
		};

		if (n <= baseColors.size()) {
			for (int i = 0; i < n; ++i) {
				lineColors.push_back(baseColors[i]);
			}
		}
		else {
			for (int i = 0; i < n; ++i) {
				float t = static_cast<float>(i) / (n - 1);
				int r = static_cast<int>((1 - t) * 255);
				int g = static_cast<int>(t * 255);
				int b = 0;

				if (i < n / 3) {
					g = static_cast<int>((static_cast<float>(i) / (n / 3)) * 255);
				} else if (i < 2 * n / 3) {
					r = 0;
					g = 255;
					b = static_cast<int>((static_cast<float>(i - n / 3) / (n / 3)) * 255);
				} else {
					r = static_cast<int>((static_cast<float>(i - 2 * n / 3) / (n / 3)) * 255);
					g = 255 - r;
					b = 255;
				}

				lineColors.push_back({ r, g, b });
			}
		}
	}
public:
	vector<ComplexLine> lines;
	vector<string> labels;
	vector<RGB> lineColors;

	Graph(int maxX = 10, int maxY = 10) {
		this->maxX = maxX;
		this->maxY = maxY;
		this->scaleX = 1;
		this->scaleY = 1;
	}

	void addLine(const vector<Point>& points, const string& label = "") {
		lines.push_back(ComplexLine(points));
		labels.push_back(label);
	}

	void resize(int maxX, int maxY, bool force = false) {
		this->maxX = maxX;
		this->maxY = maxY;
		float xScale = 1;
		float yScale = 1;
		if (force) {
			for (const auto& line : lines) {
				for (const auto& point : line.points) {
					xScale = static_cast<float>(point.x) / maxX;
					yScale = static_cast<float>(point.y) / maxY;
				}
			}
		}
		else {
			for (const auto& line : lines) {
				for (const auto& point : line.points) {
					if (point.x > maxX) {
						xScale = max(xScale, static_cast<float>(point.x) / maxX);
					}
					if (point.y > maxY) {
						yScale = max(yScale, static_cast<float>(point.y) / maxY);
					}
				}
			}
		}
		for (auto& line : lines) {
			for (auto& point : line.points) {
				point.x /= xScale;
				point.y /= yScale;
			}
		}
		scaleX = xScale;
		scaleY = yScale;
	}

	operator string () {
		setLineColor();
		setIncrements();

		string graph;
		int width = 0;
		int height = 0;
		for (const auto& line : lines) {
			for (const auto& point : line.points) {
				if (point.x > width) {
					width = point.x;
				}
				if (point.y > height) {
					height = point.y;
				}
			}
		}
		width++;
		height++;

		vector<vector<char>> grid(height, vector<char>(width, ' '));
		for (const auto& line : lines) {
			for (int i = 0; i < line.points.size() - 1; i++) {
				int x1 = line.points[i].x;
				int y1 = line.points[i].y;
				int x2 = line.points[i + 1].x;
				int y2 = line.points[i + 1].y;
				if (x1 == x2) {
					for (int y = min(y1, y2); y <= max(y1, y2); y++) {
						grid[height - y - 1][x1] = '|';
					}
				} else if (y1 == y2) {
					for (int x = min(x1, x2); x <= max(x1, x2); x++) {
						grid[height - y1 - 1][x] = '-';
					}
				} else {
					int dx = abs(x2 - x1);
					int dy = abs(y2 - y1);
					int sx = x1 < x2 ? 1 : -1;
					int sy = y1 < y2 ? 1 : -1;
					int err = dx - dy;
					int x = x1;
					int y = y1;
					while (true) {
						int angle = atan2(y2 - y1, x2 - x1) * 180 / M_PI;
						if (angle < 0) {
							angle += 360;
						}
						char c = (angle >= 270 ? '\\' : '/');
						grid[height - y - 1][x] = c;

						if (x == x2 && y == y2) {
							break;
						}
						int e2 = 2 * err;
						if (e2 > -dy) {
							err -= dy;
							x += sx;
						}
						if (e2 < dx) {
							err += dx;
							y += sy;
						}
					}
				}
			}
		}

		for (int i = 0; i < height; i++) {
			if (i % incrementY == 0) {
				graph += to_string((int)(((height - i) - 1) * scaleY));
			}
			graph += "\t|";
			for (int j = 0; j < width; j++) {
				RGB color = { 0, 0, 0 };
				for (int k = 0; k < lines.size(); k++) {
					if (lines[k].isOnLine({ j, height - i - 1 })) {
						color += lineColors[k];
					}
				}
				graph += (string)color;
				graph += grid[i][j];
				if (grid[i][j] == ' ') {
					graph += grid[i][j];
				} else if (grid[i][j] == '-') {
					graph += "-";
				}
			}
			graph += "\033[0m\n";
		}
		graph += "\t ";
		for (int i = 0; i < width * 2; i++) {
			graph += '-';
		}
		graph += "\n\t ";
		int xSpacing = width / incrementX;
		for (int i = 0; i < width; i++) {
			if (i % incrementX == 0) {
				graph += to_string((int)(i * scaleX)) + ' ';
			} else {
				for (int j = 0; j < xSpacing / 2; j++) {
					graph += ' ';
				}
			}
		}
		graph += "\n\n";
		for (int i = 0; i < labels.size(); i++) {
			graph += "\033[38;2;" + to_string(lineColors[i].r) + ";" + to_string(lineColors[i].g) + ";" + to_string(lineColors[i].b) + "m";
			graph += "\t" + labels[i] + "\033[0m\n";
		}
		return graph;
	}
};

class Transaction {
public:
	enum Type { Income, Deposit, Withdraw };
	Transaction(Type type, double amount, const string& date)
		: type(type), amount(amount), date(date) {}
	
	void print() const {
		string typeStr = (type == Income) ? "Income" : (type == Deposit) ? "Deposit" : "Withdraw";
		cout << date << " - " << typeStr << ": $" << amount << endl;
	}

private:
	Type type;
	double amount;
	string date;
};

class Account {
public:
	Account() : balance(0.0) {}
	Account(const string& owner, double balance = 0.0) 
		: owner(owner), balance(balance) {}

	virtual void deposit(double amount, const string& date) {
		balance += amount;
		transactions.emplace_back(Transaction::Deposit, amount, date);
	}

	virtual bool withdraw(double amount, const string& date) {
		if (amount <= balance) {
			balance -= amount;
			transactions.emplace_back(Transaction::Withdraw, amount, date);
			return true;
		}
		cout << "Insufficient funds!" << endl;
		return false;
	}

	void addIncome(double amount, const string& date) {
		balance += amount;
		transactions.emplace_back(Transaction::Income, amount, date);
	}

	double getBalance() const { return balance; }

	virtual void printAccountInfo() const {
		cout << "Owner: " << owner << "\nBalance: $" << balance << endl;
		for (const auto& transaction : transactions) {
			transaction.print();
		}
	}

	string getOwner() const { return owner; }

protected:
	string owner;
	double balance;
	vector<Transaction> transactions;
};

class InvestmentAccount : public Account {
public:
	InvestmentAccount(const string& owner, double balance = 0.0)
		: Account(owner, balance) {}

	virtual void buyStock(const string& stock, int shares, double price, const string& date) {
		double cost = shares * price;
		if (cost <= balance) {
			balance -= cost;
			investments[stock] += shares;
			transactions.emplace_back(Transaction::Withdraw, cost, date);
		} else {
			cout << "Insufficient funds to buy stock!" << endl;
		}
	}

	virtual void sellStock(const string& stock, int shares, double price, const string& date) {
		if (investments[stock] >= shares) {
			double earnings = shares * price;
			balance += earnings;
			investments[stock] -= shares;
			transactions.emplace_back(Transaction::Deposit, earnings, date);
		} else {
			cout << "Not enough shares to sell!" << endl;
		}
	}

	virtual void printAccountInfo() const override {
		Account::printAccountInfo();
		cout << "Investments:" << endl;
		for (const auto& investment : investments) {
			cout << investment.first << ": " << investment.second << " shares" << endl;
		}
	}

protected:
	map<string, int> investments;
};

class BankAccount : public Account {
public:
	BankAccount(const string& owner, double balance = 0.0)
		: Account(owner, balance) {}
};

class Stock {
public:
	Stock(const string& symbol, double price) : symbol(symbol), price(price) {}

	void updatePrice(double newPrice) {
		price = newPrice;
	}

	double getPrice() const {
		return price;
	}

	string getSymbol() const {
		return symbol;
	}

private:
	string symbol;
	double price;
};

class FinanceSimulator {
public:
	FinanceSimulator() {
		srand(time(nullptr));
	}

	void run() {
		printAsciiArt("FINANCE");
		printAsciiArt("SIMULATOR");
		cout << string(50, '-') << endl;
		string choice;
coda:
		if (currentAccount->getOwner() != ""){
			do {
				currentAccount->printAccountInfo();
				cout<<string(50, '-')<<endl;
				cout<<"1. Add Income\n2. Deposit\n3. Withdraw\n4. Buy Stock\n5. Sell Stock\n6. View Accounts\n7. Update Stock Prices\n8. Show stock price history\n9. Logout\n10. Rename\n";
				cout<<"> ";
				cin>>choice;
				if (choice == "1") {
					addIncome();
				} else if (choice == "2") {
					deposit();
				} else if (choice == "3") {
					withdraw();
				} else if (choice == "4") {
					buyStock();
				} else if (choice == "5") {
					sellStock();
				} else if (choice == "6") {
					viewAccounts();
				} else if (choice == "7") {
					updateStockPrices();
				} else if (choice == "8") {
					showStockHistory();
				} else if (choice == "9") {
					currentAccount = new Account();
					cout << "\033[2J\033[1;1H";
					goto coda;
				} else if (choice == "10") {
					renameAccount();
				}
			} while (choice != "9");
		}
		else {
			do {
				cout << "1. Create Account\n2. Login\n3. Exit\n";
				cout << "Enter choice: ";
				cin >> choice;
				if (choice == "1") {
					createAccount();
					cout << "\033[2J\033[1;1H";
					goto coda;
				} else if (choice == "2") {
					string owner;
					cout << "Enter account owner: ";
					cin >> owner;
					for (auto account : accounts) {
						if (account->getOwner() == owner) {
							currentAccount = account;
							break;
						}
					}
					if (currentAccount->getOwner() == "") {
						cout << "Account not found." << endl;
						cout << "Existing accounts:"<<endl;
						for (auto account : accounts) {
							cout << " * " << account->getOwner() << endl;
						}
					}
					else {
						cout << "\033[2J\033[1;1H";
						goto coda;
					}
				}
			} while (choice != "3");
		}
	}

private:
	Account* currentAccount = new Account();
	vector<Account*> accounts;
	vector<Stock> stocks = { Stock("AAPL", 150.0), Stock("GOOGL", 2800.0), Stock("TSLA", 700.0) };
	vector<Stock> stockHistory;

	string getCurrentDate() {
		time_t t = time(nullptr);
		char buffer[100];
		strftime(buffer, sizeof(buffer), "%Y-%m-%d", localtime(&t));
		return buffer;
	}

	void createAccount() {
		string owner;
		cout << "Enter account owner: ";
		cin >> owner;
		for (auto account : accounts) {
			if (account->getOwner() == owner) {
				cout << "Account already exists." << endl;
				return;
			}
		}
		accounts.push_back(new BankAccount(owner, 0));
		currentAccount = accounts.back();
	}

	void addIncome() {
		double amount;
		cout << "Enter income amount: ";
		cin >> amount;
		string date = getCurrentDate();

		currentAccount->addIncome(amount, date);
		cout << "\033[2J\033[1;1H";
	}

	void deposit() {
		double amount;
		cout << "Enter deposit amount: ";
		cin >> amount;
		string date = getCurrentDate();

		currentAccount->deposit(amount, date);
		cout << "\033[2J\033[1;1H";
	}

	void withdraw() {
		double amount;
		cout << "Enter withdrawal amount: ";
		cin >> amount;
		string date = getCurrentDate();

		currentAccount->withdraw(amount, date);
		cout << "\033[2J\033[1;1H";
	}

	void buyStock() {
		string stockSymbol;
		int shares;
		for (auto& stock : stocks) {
			cout << stock.getSymbol() << " - $" << stock.getPrice() << endl;
		}
		cout << "Enter stock symbol: ";
		cin >> stockSymbol;
		bool validSymbol = false;
		for (auto& stock : stocks) {
			if (stock.getSymbol() == stockSymbol) {
				validSymbol = true;
				break;
			}
		}
		if (!validSymbol) {
			cout << "Invalid stock symbol." << endl;
			return;
		}
		InvestmentAccount* invAccount = new InvestmentAccount(currentAccount->getOwner(), currentAccount->getBalance());
		cout << "Enter number of shares: ";
		cin >> shares;
		string date = getCurrentDate();

		for (auto& stock : stocks) {
			if (stock.getSymbol() == stockSymbol) {
				invAccount->buyStock(stockSymbol, shares, stock.getPrice(), date);
				for (int i = 0; i < accounts.size(); i++) {
					if (accounts[i]->getOwner() == currentAccount->getOwner()) {
						accounts.erase(accounts.begin() + i);
						break;
					}
				}
				accounts.push_back(invAccount);
				currentAccount = accounts.back();
				return;
			}
		}
		cout << "\033[2J\033[1;1H";
	}

	void sellStock() {
		string stockSymbol;
		int shares;
		cout << "Enter stock symbol: ";
		cin >> stockSymbol;
		bool validSymbol = false;
		for (auto& stock : stocks) {
			if (stock.getSymbol() == stockSymbol) {
				validSymbol = true;
				break;
			}
		}
		if (!validSymbol) {
			cout << "Invalid stock symbol." << endl;
			return;
		}
		InvestmentAccount* invAccount = new InvestmentAccount(currentAccount->getOwner(), currentAccount->getBalance());
		cout << "Enter number of shares: ";
		cin >> shares;
		string date = getCurrentDate();

		for (auto& stock : stocks) {
			if (stock.getSymbol() == stockSymbol) {
				invAccount->sellStock(stockSymbol, shares, stock.getPrice(), date);
				for (int i = 0; i < accounts.size(); i++) {
					if (accounts[i]->getOwner() == currentAccount->getOwner()) {
						accounts.erase(accounts.begin() + i);
						break;
					}
				}
				accounts.push_back(invAccount);
				currentAccount = accounts.back();
				return;
			}
		}
		cout << "\033[2J\033[1;1H";
	}

	void viewAccounts() {
		for (auto account : accounts) {
			if (account->getOwner() == currentAccount->getOwner()) {
				continue;
			}
			account->printAccountInfo();
			cout<<string(50, '-')<<endl;
		}
	}

	void updateStockPrices() {
		for (auto& stock : stocks) {
			stockHistory.push_back(stock);
			double change = (rand() % 2001 - 1000) / 100.0;
			double newPrice = stock.getPrice() + change;
			stock.updatePrice(newPrice);
			cout << "Updated " << stock.getSymbol() << " to $" << newPrice << endl;
		}
	}

	void showStockHistory() {
		if (stockHistory.size() < stocks.size() + 1) {
			cout << "Not enough data to display stock history." << endl;
			return;
		}
		Graph graph;
		for (int i = 0; i < stocks.size(); i++) {
			vector<int> x;
			vector<int> y;
			for (int j = i; j < stockHistory.size(); j += stocks.size()) {
				x.push_back(j / stocks.size());
				y.push_back(stockHistory[j].getPrice());
			}
			cout << stocks[i].getSymbol() << "\t";
			displayGraph(x, y);
			cout << endl;
			vector<Point> points;
			for (int j = 0; j < x.size(); j++) {
				points.push_back({ x[j], y[j] });
			}
			graph.addLine(points, stocks[i].getSymbol());
		}
		graph.resize(20, 20);
		cout << (string)graph << endl;
	}

	void renameAccount() {
		string newName;
		cout << "Enter new name: ";
		cin >> newName;
		for (int i = 0; i < accounts.size(); i++) {
			if (accounts[i]->getOwner() == currentAccount->getOwner()) {
				accounts.erase(accounts.begin() + i);
				break;
			}
		}
		accounts.push_back(new BankAccount(newName, currentAccount->getBalance()));
		currentAccount = accounts.back();
		cout << "\033[2J\033[1;1H";
	}
};

int main(int argc, char** argv) {
	for (int i = 1; i < argc; i++) {
		if (string(argv[i]) == "-h" || string(argv[i]) == "--help") {
			cout << "Usage: " << argv[0] << " [OPTION]\n";
			cout << "Options:\n";
			cout << "-h, --help:\t\tDisplay this information\n";
			cout << "--version:\t\tDisplay version information\n";
			return 0;
		}
		else if (string(argv[i]) == "--version") {
			cout << VERSION << endl;
			return 0;
		}
	}
	FinanceSimulator simulator;
	simulator.run();
	return 0;
}
// FIXME: casting account to investing account not working properly because user loses stocks.
// FIXME: graph digits on x-axis are not aligned with the graph once double digits are present
// FIXME: fix graph visualization (all graphs go white after like 15 updated stock prices)
// TODO: add text coloring