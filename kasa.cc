#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <regex>

namespace {
	enum CommandTypes {
		route,
		ticket,
		question,
		incorrect
	};

	CommandTypes commandType(const std::string& line) {
		char firstLetter = line[0];
		if (firstLetter == '?')
			return question;
		else if (firstLetter >= '0' && firstLetter <= '9')
			return route;
		else if ((firstLetter >= 'A' && firstLetter <= 'z') || firstLetter == ' ')
			return ticket;
		else
			return incorrect;
	}

	void printError(int lineNumber, const std::string& text) {
		std::cerr << "Error in line " << lineNumber << ": " << text << std::endl;
	}

	findTickets

}



int main() {
	std::string line;
	std::unordered_map routes;
	std::vector tickets;

	for (int i = 1; std::getline(std::cin, line); ++i) {
		switch (commandType(line)) {
			case route:
				addRoute(line, routes);
				break;

			case ticket:
				addTicket(line, tickets);
				break;

			case question:
				findTickets(line, routes, tickets);
				break;

			case incorrect:
				printError(i, line);
		}
	}
	return 0;
}