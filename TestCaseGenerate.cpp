#include <iostream>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <string>
#include <chrono>
#include <random>
#include <set>
using namespace std;

class coordinate {
public:
    coordinate() {};
    coordinate(double _i, double _j) : i(_i), j(_j) {};
    double i, j;
};
class coordCompare {
public:
    bool operator() (const coordinate& lhs, const coordinate& rhs) const {
        if (lhs.i != rhs.i) return lhs.i < rhs.i;
        return lhs.j < rhs.j;
    }
};
class City {
public:
    City() {};
    City(int _index, string _n, double x, double y, short _hapiness = 1,
            int _l = 4, int _s = 0, int _c = 1400)
        :index(_index), name(_n), hapiness(_hapiness), start(_s), close(_c),
        level(_l), coord(x, y) {};
    int index;
    string name;
    short hapiness;
    int start;
    int close;
    short level;
    coordinate coord;
};
class Edge {
public:
    Edge() {};
    Edge(City* _c1, City* _c2, int _level, int _distance, int _travelTime)
        : city1(_c1), city2(_c2), level(_level), distance(_distance), travelTime(_travelTime) {};
    City* city1;
    City* city2;
    int level;
    double distance;
    int travelTime;
};

class Generator {
public:
    Generator(int _n);
    void addCity(int level, int x, int y);
    void addEdge(int index1, int index2, int edgeLevel);
    void generate();
    void saveFile();
    void printTest();
    void printStatus();
private:
    int totalCities, totalEdges = 0,
        timeQuota = 4*24*60, startTime = 480;
    int currentCities = 0, currentEdges = 0;
    vector<City> cities;
    vector<Edge> edges;
    vector<string> cityNames;
    vector<coordinate> coords;

    Generator() {};
    mt19937 randomGenerator;
    uniform_int_distribution<short> unidist2To4, unidistChar,
        unidistHappy, uniTravelTime, unidist0To100, unidist1To100;
    void generateCity(int level, int count);
    void generateEdgeByCoords();
    short getHapiness(short level);
    void getStartAndclose(short level, int &start, int &close);
    int getTravelTime(short level, double distance);
    bool isDuplicatedEdge(int index1, int index2);
};

void addCityAndEdge(Generator &g);

int main(int argc, char* argv[]) {

    if (argc < 2) {
        cout << "Missing command parameter." << endl
            << "Please specify mode(coord/random) and the Node number.(2-100)" << endl;
        return 1;
    }

    string arg = argv[1];
    int nodes;
    bool convertError = false;
    try {
        std::size_t pos;
        nodes = std::stoi(arg, &pos);
        if (pos < arg.size()) {
            std::cerr << "Trailing characters after number: " << arg << '\n';
        }
    } catch (std::invalid_argument const &ex) {
        std::cerr << "Invalid number: " << arg << '\n';
        convertError = true;
    } catch (std::out_of_range const &ex) {
        std::cerr << "Number out of range: " << arg << '\n';
        convertError = true;
    }

    if (convertError) {
        cout << "The input argurment is not a valid number!" << endl;
        return 1;
    }
    if (nodes > 100) {
        cout << "The input nodes cannot be greater than 100." << endl;
        return 1;
    }
    if (nodes < 2) {
        cout << "At least two node is needed!" << endl;
    }
    cout << "** nodeNumbers : " << nodes << endl;

    Generator g(nodes);
    addCityAndEdge(g);

    cout << "** Generating map..." << endl;
    g.generate();
    cout << "** Generating map complete." << endl;
    g.printStatus();

    cout << "** Saving file..." << endl;
    string filePath = "testCase/" + to_string(nodes) + "/tp.data";
    g.saveFile();
    cout << "** File Saved. Program finished. Bye ~" << endl;

    // g.printTest();
    return 0;
};

Generator::Generator(int _n) : totalCities(_n),
        unidist2To4(2, 4), unidistChar(65, 90), unidistHappy(5, 25),
        uniTravelTime(3, 30), unidist0To100(0, 100), unidist1To100(1, 100),
        randomGenerator(chrono::high_resolution_clock::now().time_since_epoch().count()) {
    cities.reserve(totalCities);
    edges.reserve(totalCities * (totalCities-1) / 2);
    //edgeNum = nodes * (nodes - 1) / 2;
};

void Generator::addCity(int level, int x, int y) {
    int nameLength = unidist2To4(randomGenerator);
    string name;

    // generate a non-repetitive valid name
    while (true) {
        for (int k = 0 ; k < nameLength ; ++k) {
            name.push_back((char) unidistChar(randomGenerator));
        }
        bool found = false;
        for (int i = 0 ; i < cityNames.size() ; ++i) {
            if (cityNames[i] == name) {
                found = true;
                break;
            }
        }
        if (found) {
            name.clear();
        } else {
            break;
        }
    }

    coords.emplace_back(x, y);
    int hapiness = getHapiness(level);
    int start, close;
    getStartAndclose(level, start, close);
    int index = cities.size();
    cities.emplace_back(index, name, x, y, hapiness, level, start, close);
    cityNames.push_back(name);
    ++currentCities;
};
void Generator::addEdge(int index1, int index2, int edgeLevel) {
    // detect duplicated
    City &c1 = cities[index1];
    City &c2 = cities[index2];
    // cout << "adding Edge: "
    //     << "city1:cities["<<index1<<"]("<<cities[index1].name<<"), "
    //     << "city2:cities["<<index2<<"]("<<cities[index2].name<<"). " << endl;
    
    if (isDuplicatedEdge(index1, index2)) return;

    double distance = sqrt((c1.coord.i - c2.coord.i) * (c1.coord.i - c2.coord.i)
            + (c1.coord.j - c2.coord.j) * (c1.coord.j - c2.coord.j));
    int travelTime = getTravelTime(edgeLevel , distance);

    // cout << "Adding edge index: " << edges.size();
    edges.emplace_back(&c1, &c2, edgeLevel, distance, travelTime);
    Edge &e = edges[edges.size() -1];
    
    // cout << ", cities: " << e.city1->name << ", " << e.city2->name 
    //     << ", edge level: " << e.level << ", travelTime: " << e.travelTime << endl;
    ++currentEdges;
};
void Generator::generate() {
    cout << "generateCity(3, 20);" << endl;
    generateCity(3, 20);
    generateCity(4, totalCities - cities.size());
    cout << "generateEdgeByCoords();" << endl;
    generateEdgeByCoords();
};

void Generator::generateCity(int level, int count) {
    for (int i = 0 ; i < count ; ++i) {
        int nameLength = unidist2To4(randomGenerator);
        string name;

        // generate a non-repetitive valid name
        while (true) {
            for (int k = 0 ; k < nameLength ; ++k) {
                name.push_back((char) unidistChar(randomGenerator));
            }
            bool found = false;
            for (int v = 0 ; v < cityNames.size() ; ++v) {
                if (cityNames[v] == name) {
                    found = true;
                    break;
                }
            }
            if (found) {
                name.clear();
                continue;
            } else {
                break;
            }
        }
        
        // generate a non-repetitive valid coord
        short x, y;
        while (true) {
            uniform_int_distribution<short> uniDist(0, 1000);
            x = uniDist(randomGenerator);
            y = uniDist(randomGenerator);
            
            bool isOk = true;
            for (int u = 0 ; u < cities.size() ; ++u) {
                City &c = cities[u];
                if (abs(x-c.coord.i) < 3 && abs(y-c.coord.j) < 3) {
                    isOk = false;
                    break;
                }
            }
            if (isOk) break;
        }
        coords.emplace_back(x, y);

        int hapiness = getHapiness(level);
        int start, close;
        getStartAndclose(level, start, close);
        int index = cities.size();
        cities.emplace_back(index, name, x, y, hapiness, level, start, close);
        cityNames.push_back(name);
        ++currentCities;
    }
};
void Generator::generateEdgeByCoords() {
    for (short i = 0 ; i < cities.size() - 1 ; ++i) {
        for (short j = i+1 ; j < cities.size() ; ++j) {
            City &c1 = cities[i];
            City &c2 = cities[j];

            int edgeLevel = 4;
            if (c1.level <= 3 && c2.level <= 3) edgeLevel = 3;
            addEdge(i, j, edgeLevel);
        }
    }
};

short Generator::getHapiness(short level) {
    if (level == 1) {
        uniform_int_distribution<short> unidist(100, 150);
        return unidist(randomGenerator);
    }
    if (level == 2) {
        uniform_int_distribution<short> unidist(50, 70);
        return unidist(randomGenerator);
    }
    if (level == 3) {
        uniform_int_distribution<short> unidist(20, 30);
        return unidist(randomGenerator);
    }
    uniform_int_distribution<short> unidist(1, 10);
    return unidist(randomGenerator);

};

void Generator::getStartAndclose(short level, int &start, int &close) {
    if (level == 1) {
        start = 0;
        close = 24 * 60 * 7;
        return;
    }
    if (level == 2) {
        start = 24 * 60 * 1 + 8 * 60; // Mon 0800
        close = 24 * 60 * 5 + 17 * 60; // Friday 1700
        return;
    }

    if (level == 3) {
        uniform_int_distribution<short> unidist(1, 2);
        int d = unidist(randomGenerator);
        start = 24 * 60 * d + 8 * 60; // Monday 8 AM
        close = 24 * 60 * (d+3) + 17 * 60; // Thursday 17:00
        return;
    }

    uniform_int_distribution<short> unidist(1, 5);
    int d = unidist(randomGenerator);
    start = 24 * 60 * (d-1) + 8 * 60;
    close = 24 * 60 * (d+1) + 17 * 60;
}

bool Generator::isDuplicatedEdge(int index1, int index2) {
    for (int i = 0 ; i < edges.size() ; ++i) {
        Edge &e = edges[i];
        if ((index1 == edges[i].city1->index && index2 == edges[i].city2->index)
            || (index1 == edges[i].city2->index && index2 == edges[i].city1->index))  {
            // cout << "Duplicated Edge!" << endl;
            return true;
        }
    }

    // cout << "not duplicated Edge!" << endl;
    return false;
};

int Generator::getTravelTime(short level, double distance) {

    if (level == 1) {
        return distance / 10 + 60;
    }
    if (level == 2) {
        return distance / 6 + 60;
    }
    if (level == 3) {
        return distance / 2;
    }

    return distance;
};

void Generator::saveFile() {
    string filePath = "testCase/C" + to_string(totalCities)+ ".tp.data";
    
    ofstream o(filePath);

    o << totalCities << ' ' << currentEdges << ' ' << timeQuota << ' ' << startTime << endl;
    for (int i = 0 ; i < cities.size() ; ++i) {
        City &c = cities[i];
        o << c.name << ' ' << c.hapiness << ' ' << c.start <<  ' ' << c.close << endl;
    }
    o.flush();
    for (int i = 0 ; i < edges.size() ; ++i) {
        Edge &e = edges[i];
        // o << "edge index: " << i 
        //     << ", city1 index: " << e.city1->index << ", name: " << e.city1->name
        //     << ", city2 index: " << e.city2->index << ", name: " << e.city2->name << endl;
        // o   << "     edge level: " << e.level
        //     << ", distance: " << e.distance
        //     << ", travelTime: " << e.travelTime << endl;
        o << e.city1->name << ' ' << e.city2->name << ' ' << e.travelTime << endl;
        o.flush();
    }

};

void Generator::printTest() {
    /*
    cout << " -----------" << cities.size() << " cities: ----------" << endl;
    cout << totalCities << ' ' << currentEdges << ' ' << timeQuota << ' ' << startTime << endl;
    for (int i = 0 ; i < cities.size() ; ++i) {
        City &c = cities[i];
        cout << c.name << ' ' << c.hapiness << ' ' << c.level << ' ' << c.start <<  ' ' << c.close
            << " | (" << c.coord.i << ", " << c.coord.j << ")" << endl;
    }
    */

    cout << " --------- " << edges.size() << " edges --------------" << endl;

    /*
    for (int i = 0 ; i < edges.size() ; ++i) {
        Edge &e = edges[i];
        cout << "--------" << endl;
        cout << "e.city1->name: " << e.city1.name << endl;
        cout << "e.city2->name: " << e.city2->name << endl;
        cout << e.level << ' ' << e.travelTime << endl;
    }
    */
};

void Generator::printStatus() {
    cout << "cities: " << cities.size() << ", edges: " << edges.size() << endl;
    cout << "currentCities: " << currentCities << ", currentEdges: " << currentEdges<< endl;
};

void addCityAndEdge(Generator &g) {
    g.addCity(1, 270, 136); // 0
    g.addCity(1, 735, 307); // 1
    g.addCity(1, 240, 596); // 2
    g.addCity(1, 829, 804); // 3
    g.addCity(2, 179,  75); // 4

    g.addCity(2, 129, 269); // 5
    g.addCity(2, 136, 538); // 6
    g.addCity(2, 405, 233); // 7
    g.addCity(2, 370, 551); // 8
    g.addCity(2, 613, 146); // 9

    g.addCity(2, 803,  87); // 10
    g.addCity(2, 949, 141); // 11
    g.addCity(2, 852, 399); // 12
    g.addCity(2, 175, 701); // 13
    g.addCity(2, 403, 749); // 14

    g.addCity(2, 685, 825); // 15
    g.addCity(2, 903, 704); // 16
    
    // 6
    g.addEdge(0, 1, 1);
    g.addEdge(0, 2, 1);
    g.addEdge(0, 3, 1);
    g.addEdge(0, 4, 2);
    g.addEdge(0, 5, 2);
    g.addEdge(0, 7, 2);

    // 7
    g.addEdge(1, 0, 1);
    g.addEdge(1, 2, 1);
    g.addEdge(1, 3, 1);
    g.addEdge(1, 9, 2);
    g.addEdge(1, 10, 2);
    g.addEdge(1, 11, 2);
    g.addEdge(1, 12, 2);

    // 7
    g.addEdge(2, 0, 1);
    g.addEdge(2, 1, 1);
    g.addEdge(2, 3, 1);
    g.addEdge(2, 6, 2);
    g.addEdge(2, 8, 2);
    g.addEdge(2, 13, 2);
    g.addEdge(2, 14, 2);

    // 6
    g.addEdge(3, 0, 1);
    g.addEdge(3, 1, 1);
    g.addEdge(3, 2, 1);
    g.addEdge(3, 14, 2);
    g.addEdge(3, 15, 2);
    g.addEdge(3, 16, 2);

    // 3
    g.addEdge(4, 0, 2);
    g.addEdge(4, 5, 2);
    g.addEdge(4, 9, 2);

    // 5
    g.addEdge(5, 0, 2);
    g.addEdge(5, 4, 2);
    g.addEdge(5, 6, 2);
    g.addEdge(5, 7, 2);
    g.addEdge(5, 8, 2);

    // 4
    g.addEdge(6, 2, 2);
    g.addEdge(6, 5, 2);
    g.addEdge(6, 8, 2);
    g.addEdge(6, 13, 2);

    // 4
    g.addEdge(7, 0, 2);
    g.addEdge(7, 5, 2);
    g.addEdge(7, 8, 2);
    g.addEdge(7, 9, 2);

    // 6
    g.addEdge(8, 2, 2);
    g.addEdge(8, 5, 2);
    g.addEdge(8, 6, 2);
    g.addEdge(8, 7, 2);
    g.addEdge(8, 13, 2);
    g.addEdge(8, 14, 2);

    // 4
    g.addEdge(9, 1, 2);
    g.addEdge(9, 4, 2);
    g.addEdge(9, 7, 2);
    g.addEdge(9, 10, 2);

    // 4
    g.addEdge(10, 1, 2);
    g.addEdge(10, 9, 2);
    g.addEdge(10, 11, 2);
    g.addEdge(10, 12, 2);

    // 3
    g.addEdge(11, 1, 2);
    g.addEdge(11, 10, 2);
    g.addEdge(11, 12, 2);
    
    // 5
    g.addEdge(12, 1, 2);
    g.addEdge(12, 10, 2);
    g.addEdge(12, 11, 2);
    g.addEdge(12, 15, 2);
    g.addEdge(12, 16, 2);

    //4
    g.addEdge(13, 2, 2);
    g.addEdge(13, 6, 2);
    g.addEdge(13, 8, 2);
    g.addEdge(13, 14, 2);

    // 5
    g.addEdge(14, 2, 2);
    g.addEdge(14, 3, 2);
    g.addEdge(14, 8, 2);
    g.addEdge(14, 13, 2);
    g.addEdge(14, 15, 2);

    // 4
    g.addEdge(15, 3, 2);
    g.addEdge(15, 12, 2);
    g.addEdge(15, 14, 2);
    g.addEdge(15, 16, 2);

    // 3
    g.addEdge(16, 3, 2);
    g.addEdge(16, 12, 2);
    g.addEdge(16, 15, 2);

    g.printStatus();
};