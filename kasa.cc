#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <regex>
#include <climits>


namespace {
	// typy komend
	enum CommandTypes {
		route,
		ticket,
		question,
		empty,
		incorrect
	};

	// ustala typ komendy
	CommandTypes commandType(const std::string& line) {
		if (line.empty())
			return empty;
		else {
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
	}

	void printError(int lineNumber, const std::string& text) {
		std::cerr << "Error in line " << lineNumber << ": " << text << std::endl;
	}

	// przystanek: (kolejność na kursie, czas w minutach)
	using Stop = std::pair<int, int>;
	// kurs: klucz - nazwa przystanku
	using Route = std::unordered_map<std::string, Stop>;
	// zbiór kursów: klucz - numer kursu
	using Routes = std::unordered_map<int, Route>;
	// bilet: (cena w groszach, ważność w minutach)
	using Ticket = std::pair<long long int, int>;
	// zbiór biletów: klucz - nazwa biletu
	using Tickets = std::unordered_map<std::string, Ticket>;

	// pomocnicza w pełni statyczna klasa, używana do parsowania
	class Patterns {
	private:
		static const std::string stopNamePattern; // nazwa przystanku
		static const std::string routeNumberPattern; // numer kursu
		static const std::string timePattern; // czas przyjazdu (godziny i minuty)
		static const std::string ticketNamePattern; // nazwa biletu
		static const std::string pricePattern; // cena
		static const std::string validityPattern; // czas ważności (tylko minuty)

		// sprawdza, czy czas jest między 5:55 a 21:21
		static bool isValidTime(int timeInMinutes) {
			return timeInMinutes >= 5 * 60 + 55 && timeInMinutes <= 21 * 60 + 21;
		}

	public:
		// parsuje [line] jako kurs i odpowiednie wartości zapisuje w [routeId] i [newRoute]
		// wyjście: true - gdy sukces; false - wpp.
		static bool parseRoute(const std::string& line, int& routeId, Route& newRoute) {
			static const std::regex idRegex("^" + routeNumberPattern);

			std::smatch idMatch;
			if (std::regex_search(line.begin(), line.end(), idMatch, idRegex)) {
				try {
					routeId = stoi(idMatch[1]);
				}
				catch (std::out_of_range& e) {
					return false;
				}
			}
			else {
				return false;
			}

			unsigned int orderInRoute = 0;
			int lastTime = 0;

			static const std::string segmentPattern = "^ " + timePattern + " " + stopNamePattern;
			static const std::regex segmentRegex(segmentPattern);

			std::smatch segmentMatch;
			for (auto it = idMatch.suffix().first; it != line.end(); ) {
				if (std::regex_search(it, line.end(), segmentMatch, segmentRegex)) {
					std::string stopName = segmentMatch[3];

					// jeśli przystanek się powtarza, to błąd
					if (newRoute.find(stopName) != newRoute.end())
						return false;

					// nie musimy łapać std::out_of_range, bo nasz regex zapewnia, że nigdy się to nie stanie
					int hours = stoi(segmentMatch[1]);
					int minutes = stoi(segmentMatch[2]);
					int currentTime = hours * 60 + minutes;
					if (!isValidTime(currentTime))
						return false;

					// jeśli przystanek wcześniejszy jest na trasie niewcześniej niż późniejszy, to błąd
					if (lastTime >= currentTime)
						return false;
					lastTime = currentTime;

					Stop newStop(orderInRoute++, currentTime);
					newRoute[stopName] = newStop;

					it = segmentMatch.suffix().first;
				}
				else
					return false;
			}

			return !newRoute.empty();
		}

		// parsuje [line] jako bilet i odpowiednie wartości zapisuje w [ticketName] i [newTicket]
		// wyjście: true - gdy sukces; false - wpp.
		static bool parseTicket(const std::string& line, std::string& ticketName, Ticket& newTicket) {
			static const std::regex ticketRegex(ticketNamePattern + " " + pricePattern + " " + validityPattern);

			std::smatch ticketMatch;
			if (std::regex_match(line.begin(), line.end(), ticketMatch, ticketRegex)) {
				ticketName = ticketMatch[1];
				try {
					newTicket.first = stoll(ticketMatch[2]) * 100 + stoll(ticketMatch[3]);
					newTicket.second = stoi(ticketMatch[4]);
				}
				catch (std::out_of_range &e) {
					return false;
				}
				return true;
			}
			else
				return false;
		}

		// parsuje [line] jako podróż i odpowiednie wartości zapisuje w [stopNames] i [routeNumbers]
		// wyjście: true - gdy sukces; false - wpp.
		static bool parseJourney(const std::string& line, std::vector<std::string>& stopNames,
						std::vector<int>& routeNumbers) {
			if (line[0] != '?')
				return false;

			static const std::string routeNumberOrNothingPattern = "(?:$| " + routeNumberPattern + ")";
			static const std::string segmentPattern = "^ " + stopNamePattern + routeNumberOrNothingPattern;
			static const std::regex segmentRegex(segmentPattern);

			std::smatch segmentMatch;
			for (auto it = line.begin() + 1; it != line.end(); ) {
				if (std::regex_search(it, line.end(), segmentMatch, segmentRegex)) {
					stopNames.push_back(segmentMatch[1]);
					if (segmentMatch[2].length() != 0) {
						try {
							routeNumbers.push_back(stoi(segmentMatch[2]));
						}
						catch (std::out_of_range& e) {
							return false;
						}
					}
					it = segmentMatch.suffix().first;
				}
				else
					return false;
			}

			// sprawdzamy, czy przystanków jest dokładnie o jeden więcej niż kursów
			// i czy jest choć jeden kurs
			return stopNames.size() == routeNumbers.size() + 1 && !routeNumbers.empty();
		}

	};

	const std::string Patterns::stopNamePattern = "([A-Za-z_^]+)";
	const std::string Patterns::routeNumberPattern = "0*([0-9]+)";
	const std::string Patterns::timePattern = "(1?[0-9]|2[0-3]):([0-5][0-9])";
	const std::string Patterns::ticketNamePattern = "([A-Za-z ]+)";
	const std::string Patterns::pricePattern = "([0-9]+)\\.([0-9]{2})";
	const std::string Patterns::validityPattern = "([1-9][0-9]*)";

	// na podstawie [line] tworzy i dodaje odpowiedni kurs do [routes]
	// wyjście: true - gdy sukces; false - wpp.
	bool addRoute(const std::string& line, Routes& routes) {
		int routeId;
		Route newRoute;

		if (!Patterns::parseRoute(line, routeId, newRoute))
			return false;

		// jeśli kurs o takim numerze już istnieje, to błąd
		if (routes.find(routeId) != routes.end())
			return false;

		routes[routeId] = newRoute;
		return true;
	}

	// na podstawie [line] tworzy i dodaje odpowiedni bilet do [tickets]
	// wyjście: true - gdy sukces; false - wpp.
	bool addTicket(const std::string& line, Tickets& tickets) {
		std::string ticketName;
		Ticket newTicket;

		if (!Patterns::parseTicket(line, ticketName, newTicket))
			return false;

		// jeśli bilet o takiej nazwie już istnieje, to błąd
		if (tickets.find(ticketName) != tickets.end())
			return false;

		tickets[ticketName] = newTicket;
		return true;
	}

	// na podstawie dostępnych kursów [routes] oblicza czas potrzebny na pokonanie podróży
	// określonej przez [stopNames] i [routeNumbers]
	// wyjście: potrzebny czas(dodatni); -1 - gdy błąd; -2 - gdy trzeba czekać na przystanku
	int computeTimeNeededForJourney(const Routes& routes, const std::vector<std::string>& stopNames,
					const std::vector<int>& routeNumbers) {
		int timeNeeded = 0;
		std::string stopWithWaiting;
		int lastTime = -1;
		for (size_t i = 0; i < routeNumbers.size(); ++i) {
			try {
				Route route = routes.at(routeNumbers[i]);

				Stop stop1 = route.at(stopNames[i]);
				Stop stop2 = route.at(stopNames[i + 1]);

				int order1 = stop1.first;
				int order2 = stop2.first;
				int time1 = stop1.second;
				int time2 = stop2.second;

				// jeśli przystanek wcześniejszy jest na trasie później niż późniejszy, to błąd
				if (order1 >= order2)
					return -1;

				// jeśli czas odjazdu z przystanku jest wcześniej niż czas przyjazdu nań, to błąd
				if (time1 < lastTime) {
					return -1;
				}
				else if (time1 > lastTime && lastTime != -1) {
					if (stopWithWaiting.empty())
						stopWithWaiting = stopNames[i];
				}
				lastTime = time2;

				timeNeeded += time2 - time1;
			}
			catch (std::out_of_range& e) {
				return -1;
			}
		}

		if (!stopWithWaiting.empty()) {
			std::cout << ":-( " + stopWithWaiting << std::endl;
			return -2;
		}
		else
			return timeNeeded + 1;
	}

	// znajduje najtańszy zestaw biletów z [tickets], pozwalający przejechać [timeNeeded] minut
	// wyjście: liczba biletów w zestawie(1-3); 0 - gdy nie da się kupić
	int findCheapestTickets(int timeNeeded, const Tickets& tickets) {
		long long bestPrice = LLONG_MAX;
		std::vector<std::string> optimalTicketSet;
		std::string ticketNames[3];
		Ticket ticket[3];

		for (auto& it0 : tickets) {
			ticketNames[0] = it0.first;
			ticket[0] = it0.second;

			for (auto& it1 : tickets) {
				ticketNames[1] = it1.first;
				ticket[1] = it1.second;

				for (auto& it2 : tickets) {
					ticketNames[2] = it2.first;
					ticket[2] = it2.second;

					if (timeNeeded <= ticket[0].second + ticket[1].second + ticket[2].second) {
						long long int newPrice = ticket[0].first + ticket[1].first + ticket[2].first;
						if (newPrice < bestPrice) {
							bestPrice = newPrice;

							optimalTicketSet.clear();
							for (const auto& it : ticketNames) {
								if (!it.empty())
									optimalTicketSet.push_back(it);
							}
						}
					}
				}
			}
		}

		if (!optimalTicketSet.empty()) {
			std::cout << "! " << optimalTicketSet[0];
			for (size_t i = 1; i < optimalTicketSet.size(); ++i)
				std::cout << "; " << optimalTicketSet[i];

			std::cout << std::endl;
		}

		// wielkość jest co najwyżej 3, więc możemy bezpiecznie zwrócić w incie
		return optimalTicketSet.size();
	}

	// na podstawie dostępnych kursów [routes] i biletów [tickets] znajduje i wypisuje na ekran
	// najtańszy zestaw biletów, pozwalający przebyć podróż zawartą w [line]
	// wyjście: liczba biletów w zestawie(1-3); 0 - gdy nie da się kupić; -1 - gdy błąd
	int findTickets(const std::string& line, const Routes& routes, const Tickets& tickets) {
		std::vector<std::string> stopNames;
		std::vector<int> routeNumbers;
		if (!Patterns::parseJourney(line, stopNames, routeNumbers))
			return -1;

		int timeNeeded = computeTimeNeededForJourney(routes, stopNames, routeNumbers);
		if (timeNeeded == -1)
			return -1;
		else if (timeNeeded == -2)
			return 0;

		int ticketsFound = findCheapestTickets(timeNeeded, tickets);
		if (ticketsFound == 0)
			std::cout << ":-|" << std::endl;

		return ticketsFound;
	}
}


int main() {
	std::string line;
	Routes routes;
	Tickets tickets;
	tickets[""] = Ticket(0, 0);
	int allFoundTickets = 0;

	for (int i = 1; std::getline(std::cin, line); ++i) {
		switch (commandType(line)) {
			case route:
				if (!addRoute(line, routes))
					printError(i, line);
				break;

			case ticket:
				if (!addTicket(line, tickets))
					printError(i, line);
				break;

			case question:
			{
				int foundTickets = findTickets(line, routes, tickets);
				if (foundTickets == -1)
					printError(i, line);
				else
					allFoundTickets += foundTickets;
				break;
			}

			case empty:
				break;

			case incorrect:
				printError(i, line);
				break;
		}
	}

	std::cout << allFoundTickets << std::endl;

	return 0;
}