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

	using Stop = std::pair<int, int>; // first = order in the route, second = time in minutes
	using Route = std::unordered_map<std::string, Stop>; // key = name of the stop
	using Routes = std::unordered_map<unsigned int, Route>; // key = number of the route

	void findTickets(const std::string& line, const Routes& routes, tickets) {
		parseJourney();
		int timeNeeded = computeTimeNeededForJourney();

	}

}



int main() {
	std::string line;
	Routes routes;
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