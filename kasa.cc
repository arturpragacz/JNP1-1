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

	void parseJourney(const std::string& line, std::vector<std::string>& stopNames, std::vector<int>& routeNumbers) {

	}

	int computeTimeNeededForJourney(const Routes& routes, const std::vector<std::string>& stopNames,
					const std::vector<int>& routeNumbers) {
		int timeNeeded = 0;
		for (size_t i = 0; i < routeNumbers.size(); ++i) {
			try {
				Route route = routes.at(routeNumbers[i]);

				Stop stop1 = route.at(stopNames[i]);
				Stop stop2 = route.at(stopNames[i + 1]);

				int order1 = stop1.first;
				int order2 = stop2.first;
				int time1 = stop1.second;
				int time2 = stop2.second;

				if (order1 >= order2)
					return -1;
				else
					timeNeeded += time2 - time1;
			}
			catch (std::out_of_range& e) {
				return -1;
			}
		}

		return timeNeeded;
	}

	int findTickets(const std::string& line, const Routes& routes, tickets) {
		std::vector<std::string> stopNames;
		std::vector<int> routeNumbers;
		parseJourney(line, stopNames, routeNumbers);

		int timeNeeded = computeTimeNeededForJourney(routes, stopNames, routeNumbers);

		findCheapestTickets(timeNeeded);
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