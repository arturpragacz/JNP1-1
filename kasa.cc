#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

namespace {
	enum CommandTypes {
		route,
		ticket,
		question
	};

	CommandTypes commandType(const std::string& line);

}



int main() {
	std::string line;
	std::unordered_map routes;
	std::vector tickets;

	while (std::getline(std::cin, line)) {
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
		}
	}
	return 0;
}