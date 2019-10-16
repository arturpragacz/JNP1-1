#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <regex>
#include <climits>


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

	using Stop = std::pair<int, int>; //Przystanek; (kolejnosc na kursie, czas w minutach)
	using Route = std::unordered_map<std::string, Stop>; //Kurs; Klucz - nazwa przystanku
	using Routes = std::unordered_map<unsigned int, Route>; //Zbior kursow; Klucz - numer kursu
	using Ticket = std::pair<long long, int>; //Bilet; (cena w groszach, waznosc w minutach)
	using Tickets = std::unordered_map<std::string, Ticket>; //Zbior biletow; Klucz - nazwa biletu

	//Sprawdza czy o danej godzinie tramwaje moga byc jeszcze czynne.
	bool validTime(int timeInMinutes) {
		return timeInMinutes >= 5 * 60 + 55 && timeInMinutes <= 21 * 60 + 21;
	}

	class Patterns {
	private:
		static const std::string stopNamePattern;
		static const std::string routeNumberPattern;
		static const std::string timePattern;
		static const std::string ticketNamePattern;
		static const std::string pricePattern;
		static const std::string validityPattern;

	public:
		//Bierze linie wejscia i tworzy na jej podstawie nowy kurs pod newRoute. Zapisuje
		//jego numer pod routeId. Zwraca true jesli zakonczono sukcesem, false jesli wystapil blad.
		bool parseRoute(const std::string& line, unsigned int& routeId, Route& newRoute) {
			//Parsowanie numeru kursu.
			auto it = line.begin();

			static const std::regex idRegex("^" + routeNumberPattern);
			std::smatch idMatch;
			if (std::regex_search(line.begin(), line.end(), idMatch, idRegex))
				routeId = stoi(idMatch[1]);
			else
				return false;

			it = idMatch.suffix().first;

			//Parsowanie trasy kursu.
			unsigned int orderInRoute = 0; //Kolejnosc aktualnie parsowanego przystanku na trasie kursu.
			int lastTime = 0; //Czas przyjazdu na poprzedni przystanek w minutach.

			static const std::string segmentPattern = "^ " + timePattern + " " + stopNamePattern;
			static const std::regex segmentRegex(segmentPattern);

			std::smatch segmentMatch;
			while (it != line.end()) {
				if (std::regex_search(it, line.end(), segmentMatch, segmentRegex)) {
					std::string stopName = segmentMatch[3];
					if (newRoute.find(stopName) != newRoute.end())
						return false; //Przystanek powtarza sie na trasie kursu.

					int hours = stoi(segmentMatch[1]);
					int minutes = stoi(segmentMatch[2]);
					int currentTime =  hours * 60 + minutes;
					if (!validTime(currentTime))
						return false;

					//Sprawdzamy czy godziny przyjazdow sa rosnace.
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

			return true;
		}

		bool parseTicket(const std::string& line, std::string& ticketName, Ticket& newTicket) {
			static const std::regex ticketRegex("^" + ticketNamePattern + " " + pricePattern
																					+ " " + validityPattern + "$");
			std::smatch ticketMatch;
			if (std::regex_search(line.begin(), line.end(), ticketMatch, ticketRegex)) {
				ticketName = ticketMatch[1];
				newTicket.first = stoll(ticketMatch[2]) * 100 + stoll(ticketMatch[3]);
				newTicket.second = stoi(ticketMatch[4]);
				return true;
			}
			return false;
		}

		bool parseJourney(const std::string& line, std::vector<std::string>& stopNames, std::vector<int>& routeNumbers) {
			if (line[0] != '?')
				return false;

			static const std::string routeNumberOrNothingPattern = "(?:$| " + routeNumberPattern + ")";
			static const std::string segmentPattern = "^ " + stopNamePattern + routeNumberOrNothingPattern;
			static const std::regex segmentRegex(segmentPattern);

			std::smatch segmentMatch;
			for (auto it = line.begin() + 1; it != line.end();) {
				if (std::regex_search(it, line.end(), segmentMatch, segmentRegex)) {
					stopNames.push_back(segmentMatch[1]);
					if (segmentMatch.size() >= 3 && !segmentMatch[2].str().empty())
						routeNumbers.push_back(stoi(segmentMatch[2]));

					it = segmentMatch.suffix().first;
				}
				else
					return false;
			}

			return stopNames.size() == routeNumbers.size() + 1;
		}

	} patterns;

	const std::string Patterns::stopNamePattern = "([A-Za-z_^]+)";
	const std::string Patterns::routeNumberPattern = "0*([0-9]+)";
	const std::string Patterns::timePattern = "(1?[0-9]|2[0-3]):([0-5][0-9])";
	const std::string Patterns::ticketNamePattern = "([A-Za-z ]+)";
	const std::string Patterns::pricePattern = "([0-9]+).([0-9]{2})";
	const std::string Patterns::validityPattern = "([1-9][0-9]*)";

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

	//Na podstawie linii wejscia, tworzy nowy kurs i dodaje go do routes.
	//Zwraca true jesli zakonczono sukcesem, false jesli wystapil blad.
	bool addRoute(const std::string& line, Routes& routes) {
		//Parsujemy linie i bierzemy z niej informacje o kursie.
		unsigned int routeId;
		Route newRoute;

		if (!patterns.parseRoute(line, routeId, newRoute))
			return false;

		//Juz istnieje kurs o takim numerze.
		if (routes.find(routeId) != routes.end())
			return false;
		//Wrzucamy nowy kurs do mapy kursow (routes).
		routes[routeId] = newRoute;
		return true;
	}

	bool addTicket(const std::string& line, Tickets& tickets) {
		//Parsujemy linie i bierzemy z niej informacje o bilecie.
		std::string ticketName;
		Ticket newTicket;

		if (!patterns.parseTicket(line, ticketName, newTicket))
			return false;

		//Juz istnieje bilet o takiej nazwie.
		if (tickets.find(ticketName) != tickets.end())
			return false;
		//Wrzucamy nowy bilet do mapy biletow (tickets).
		tickets[ticketName] = newTicket;
		return true;
	}

	bool findCheapestTickets(int timeNeeded, const Tickets& tickets) {
		long long bestPrice = LLONG_MAX;
		std::vector<std::string> optimalTicketSet;
		std::string ticketName[3];
		Ticket ticket[3];

		for (auto& it0 : tickets) {
			ticketName[0] = it0.first;
			ticket[0] = it0.second;

			for (auto& it1 : tickets) {
				ticketName[1] = it1.first;
				ticket[1] = it1.second;

				for (auto& it2 : tickets) {
					ticketName[2] = it2.first;
					ticket[2] = it2.second;

					if (timeNeeded <= ticket[0].second + ticket[1].second + ticket[2].second) {
						long long newPrice = ticket[0].first + ticket[1].first + ticket[2].first;
						if (newPrice < bestPrice) {
							bestPrice = newPrice;

							optimalTicketSet = std::vector<std::string>();
							for (auto it : ticketName) {
								if (!it.empty())
									optimalTicketSet.push_back(it);
							}
						}
					}
				}
			}
		}

		if (bestPrice == LLONG_MAX)
			return false;

		std::cout<<"!";
		for (auto& it : optimalTicketSet)
			std::cout<<" "<<it;

		return true;
	}

	bool findTickets(const std::string& line, const Routes& routes, const Tickets& tickets) {
		std::vector<std::string> stopNames;
		std::vector<int> routeNumbers;
		if (!patterns.parseJourney(line, stopNames, routeNumbers))
			return false;

		int timeNeeded = computeTimeNeededForJourney(routes, stopNames, routeNumbers);
		if (timeNeeded == -1)
			return false;

		if (!findCheapestTickets(timeNeeded, tickets))
			std::cout<<":-|\n";

		return true;
	}
}


int main() {
	std::string line;
	Routes routes;
	Tickets tickets;
	tickets[""] = Ticket(0, 0); //Pusty bilet.

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
				if (!findTickets(line, routes, tickets))
					printError(i, line);
				break;

			case incorrect:
				printError(i, line);
		}
	}
	return 0;
}