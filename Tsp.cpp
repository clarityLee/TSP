#include <chrono>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include "Tsp.hpp"
using namespace std;

Tsp::~Tsp() {};
bool Tsp::hasError() {return errorFlag;}
void Tsp::read(int &argc, char* argv[]) {
    constexpr auto &&now = std::chrono::high_resolution_clock::now;
    auto start = now();

    if (argc < 2) {
        errorFlag = true;
        cout <<  "Missing command argument. Please specify the path to tp.data." << endl;
        return;
    }

    filePath = string(argv[1]);
    ifstream in(filePath + "/tp.data");
    if (!in.is_open()) {
        errorFlag = true;
        cout << "Unable to open \"" 
            << filePath << "/tp.data\". Please check existance of file or path." << endl;
        return;
    }

    string _tmp;
    getline(in, _tmp);
    stringstream ss(_tmp);
    ss >> nodes >> edgeNum >> totalTimeQuota >> startTime;
    cities.reserve(nodes);
    edges.reserve(edgeNum);
    
    cout << "Reading Cities...";
    map<string, int> cityNameAndIndex;
    short count = nodes;
    while(count--) {
        getline(in, _tmp);
        ss.clear(); ss.str(_tmp);
        string name;
        int hapiness, start, close;
        ss >> name >> hapiness >> start >> close;
        int index = cities.size();
        cities.emplace_back(index, name, hapiness, start, close);
        cityNameAndIndex[name] = index;

    }
    cout << "complete." << endl;
    
    
    shortestTravelTime.resize(nodes);
    cameFrom.resize(nodes);
    for (int i = 0 ; i < nodes ; ++i) {
        shortestTravelTime[i].resize(nodes);
        cameFrom[i].resize(nodes);
        for (int j = 0 ; j < nodes ; ++j) {
            if (i == j) shortestTravelTime[i][j] = 0;
            else shortestTravelTime[i][j] = 2147483647;
            cameFrom[i][j] = -1;
        }
    }
    
    cout << "Reading Edges...";
    count = edgeNum;
    while(count--) {
        getline(in, _tmp);
        ss.clear(); ss.str(_tmp);
        string name1, name2;
        int travelTime;
        ss >> name1 >> name2 >> travelTime;

        int index_city1 = cityNameAndIndex.find(name1)->second;
        City &c1 = cities[index_city1];

        int index_city2 = cityNameAndIndex.find(name2)->second;
        City &c2 = cities[index_city2];
        edges.emplace_back(&c1, &c2, travelTime);

        shortestTravelTime[c1.index][c2.index] = shortestTravelTime[c2.index][c1.index] = travelTime;
        cameFrom[c1.index][c2.index] = c1.index;
        cameFrom[c2.index][c1.index] = c2.index;
    }
    cout << "complete." << endl;

    chrono::duration<double> elapsed = now() - start;
    cout << "readData, completed in : " << (int) elapsed.count() * 1000 << "ms." << endl;

    // printCities();
    // printEdges();
    // printShortestTravelTime();
    // printCamefrom();

    start = now();
    calcWarshall();
    elapsed = now() - start;
    cout << "calculate Warshall, completed in : " << (int) elapsed.count() * 1000 << "ms." << endl;
};

void Tsp::openTime(bool b) {
    takeOpentime = b;
};

// void Tsp::recordPath(Record &record, City* city, int arriving, int leaving) {
//     record.path.emplace_back(city->name, arriving, leaving);
// };

void Tsp::sub1_greedy() {
    if (!reseted) reset();

    // set current position to starting city
    City* start = &cities[0];
    City* current = &cities[0];
    int totalEarned = 0, timeQuota = totalTimeQuota, currentTime = startTime;
    map<float, City*> cpRatio; // highest cp element is at the end
    vector<City*> unvisited; unvisited.reserve(cities.size());
    for (int i = 0 ; i < cities.size() ; ++i) {
        unvisited.push_back(&cities[i]);
    }

    // record start position;
    start->visited = true;
    totalEarned += start->hapiness;
    greedyResult.path.emplace_back(start->name, currentTime, currentTime);
    
    // 1. while loop:
    int loopCount = 0;
    const int limit = nodes + 3;
    while(true) {

        // 1b. remove destination that do not fit to remaining time quota
        greedy_filterUnvisited(unvisited, start, current, timeQuota);
        if (unvisited.empty()) break;

        // 1c. calculate c/p ratio of earnable hapiness vs travel time for a not visited city
        cpRatio.clear();
        greedy_calcCP(unvisited, cpRatio, current);

        // 1e. take highest cp city as destination, go to that city and record path
        auto it = cpRatio.rbegin();
        City* dest = it->second;


        greedy_goto(current, dest, currentTime, totalEarned, timeQuota);
        current = dest;

        if (loopCount++ > limit) {
            string message = string("Tsp::greedySolution, while loop exceed limit times(")
                + to_string(limit) + ").";
            throw runtime_error(message);
        }
    }

    // 2. go back to starting point.
    greedy_goto(current, start, currentTime, totalEarned, timeQuota);

    // 3. save greedy result
    greedyResult.hapiness = totalEarned;
    greedyResult.time = greedyResult.path.back().leaving - startTime;
};

void Tsp::greedy_filterUnvisited(vector<City*> &unvisited, City* start, City* current, int timeQuota) {
    for (int i = unvisited.size() - 1 ; i >= 0 ; --i) {
        City* dest = unvisited[i];
        bool erase = false;
        if (dest->visited) {
            erase = true;
        } else {
            int requiredTime = shortestTravelTime[current->index][dest->index]
                + shortestTravelTime[dest->index][start->index];
            if (timeQuota < requiredTime) erase = true;
        }
        if (erase) {
            unvisited.erase(unvisited.begin() + i);
        }
    }
};

void Tsp::greedy_calcCP(vector<City*> &unvisited, map<float, City*> &cpRatio, City* source) {
    for (int i = 0 ; i < unvisited.size(); ++i) {
        int earnable = 0;
        City* dest = unvisited[i];
        City* current = unvisited[i];

        int loops = 0;
        const int limit = nodes + 3;
        while(current != source) {

            if (!current->visited) earnable += current->hapiness;

            // move current to cameFrom
            current = &cities[cameFrom[source->index][current->index]];

            if (loops++ > limit) {
                string message = string("Tsp::greedy_calcCP while loop exceeds limit times(")
                    + to_string(limit) + ")";
                throw runtime_error(message);
            };
        }

        float cp = earnable / shortestTravelTime[source->index][dest->index];
        cpRatio[cp] = dest;
    }
};

void Tsp::greedy_goto(City* source, City* destination,
        int &currentTime, int &totalEarned, int &timeQuota) {
    vector<City*> passingCities;
    City* current = destination;

    int loops = 0;
    const int limit = nodes + 3;
    while(current != source) {

        if (!current->visited) {
            current->visited = true;
            totalEarned += current->hapiness;
        }

        // move current to cameFrom
        passingCities.push_back(current);
        current = &cities[cameFrom[source->index][current->index]];

        if (loops++ > limit) {
            string message = string("Tsp::greedy_goto while loop exceeds limit times: "
                + to_string(limit));
            throw runtime_error(message);
        }
    }

    // current is source now
    for (int i = passingCities.size() - 1 ; i >= 0 ; --i) {
        // record next city
        City* next = passingCities[i];
        currentTime += shortestTravelTime[current->index][next->index];
        greedyResult.path.emplace_back(next->name, currentTime, currentTime);
        current = next;
    }
    timeQuota -= shortestTravelTime[source->index][destination->index];
};

void Tsp::findSolution() {
    sub1_greedy();
    saveFileSub1(greedyResult);
    showVisitStatus();
};

void Tsp::reset() {
    reseted = true;
    for (int i = 0; i < cities.size(); ++i) {
        cities[i].visited = false;
    }
};

void Tsp::saveFileSub1(Record &record) {
    ofstream o(filePath + "/ans1.txt");
    stringstream ss;
    ss << record.hapiness << ' ' << record.time << endl;
    for (int i = 0 ; i < record.path.size() ; ++i) {
        FootPrint &f = record.path[i];
        ss << f.name << ' ' << f.arriving << ' ' << f.leaving << endl;
    }
    o << ss.rdbuf();
}

void Tsp::calcWarshall() {
    cout << "Start to calculate Warshall" << endl;
    for (short k = 0 ; k < nodes ; ++k) {
        for (short i = 0 ; i < nodes ; ++i) {
            for (short j = 0 ; j < nodes; ++j) {
                if (shortestTravelTime[i][j] > shortestTravelTime[i][k] + shortestTravelTime[k][j]) {
                    shortestTravelTime[i][j] = shortestTravelTime[i][k] + shortestTravelTime[k][j];
                    cameFrom[i][j] = cameFrom[k][j];
                }
            }
        }
    }
    cout << "Warshall calculate complete.";
    // saveShortestTravelTime();
    // saveCameFrom();
};

void Tsp::printData() {
    cout << "Cities: " << endl;
    for (int i = 0 ; i < nodes ; ++i) {
        City &c = cities[i];
        if (i + 1 < 10) cout << "  ";
        else if (i + 1 < 100) cout << ' ';

        cout << i+1 << ". " << c.name;

        if (c.name.length() == 2) cout << "  ";
        else if (c.name.length() == 3) cout <<  ' ';

        cout << "  hapiness: ";
        if (c.hapiness < 10) cout << "  ";
        else if (c.hapiness < 100) cout << " ";
        
        cout << c.hapiness;
        cout << endl;
    }

    cout << "Edges:" << endl;
    for (int i = 0 ; i < edgeNum ; ++i) {
        Edge &e = edges[i];
        cout << "  " << i+1 << ". " << e.city1->name << "  " << e.city2->name << "  " << e.travelTime << endl;
    }
};

void Tsp::printCities() {
    for (int i = 0 ; i < cities.size() ; ++i) {
        City &c = cities[i];
        cout << c.index << ". " << c.name << ", " << c.hapiness << endl;
    }
    cout << endl;
};


void Tsp::printEdges() {
    for (int i = 0 ; i < edges.size() ; ++i) {
        Edge &e = edges[i];
        cout << i << ". " << e.city1->name << " " << e.city2->name << ", " << e.travelTime << endl;
    }
    cout << endl;
};

void Tsp::printShortestTravelTime() {
    cout << "Printing shortestTravelTime..." << endl;
    for (short i = 0 ; i < nodes ; ++i) {
        for (short j = 0 ; j < nodes ; ++j) {
            cout << shortestTravelTime[i][j];
            if (j < nodes - 1) cout << ' ';
            else cout << endl;
        }
    }
    cout << endl;
};

void Tsp::printCamefrom() {
    cout << "Print cameFrom...." << endl;
    for (short i = 0 ; i < nodes ; ++i) {
        for (short j = 0 ; j < nodes ; ++j) {
            cout << cameFrom[i][j];
            if (j < nodes - 1) cout << ' ';
            else cout << endl;
        }
    }
    cout << endl;
};

void Tsp::saveShortestTravelTime() {
    ofstream o(filePath + "/shortestTravelTime.txt");
    for (short i = 0 ; i < nodes ; ++i) {
        for (short j = 0 ; j < nodes ; ++j) {
            o << shortestTravelTime[i][j];
            if (j < nodes - 1) o << ' ';
            else o << endl;
        }
    }
    cout << filePath << "/shortestTravelTime.txt saved." << endl;
};
void Tsp::saveCameFrom() {
    ofstream o(filePath + "/cameFrom.txt");
    for (short i = 0 ; i < nodes ; ++i) {
        for (short j = 0 ; j < nodes ; ++j) {
            o << cameFrom[i][j];
            if (j < nodes - 1) o << ' ';
            else o << endl;
        }
    }
    cout << filePath << "/cameFrom.txt saved." << endl;
};

void Tsp::showVisitStatus() {
    int visited = 0 , unvisited = 0;
    for (int i = 0 ; i < cities.size() ; ++i) {
        if (cities[i].visited) ++visited;
        else ++unvisited;
    };
    cout << "Total City: " << cities.size() << ", visited/unvisited: "
         << visited << "/" << unvisited << endl;
}