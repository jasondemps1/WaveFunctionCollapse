#include <cstdio>
#include <iostream>
#include <algorithm>
#include <vector>
#include <tuple>
#include <unordered_map>
#include <string>
#include <cstdlib>
#include <ctime>
#include <array>
#include <limits>
#include <random>
#include <stack>
#include <exception>
#include <chrono>
#include <fstream>
#include <sstream>

#pragma warning(disable:4996)

#define ENABLE_DEBUG 1
#define ENABLE_OUTPUT 1
#define ASK_REPEAT 1
#define WRITE_MAP 1

#define MAP_POSITION(y, x, MAX_WIDTH) y * MAX_WIDTH + x

enum Direction : int {
    up, down, left, right, upleft, upright, downleft, downright, MAX
} direction;

class ContradictionException : public std::exception {
    virtual const char* what() const throw() {
        return "EXCEPTION: Contradiction";
    }
} ContradictionException;

typedef std::tuple<int, int> dir;
typedef std::tuple<char, char, dir> rule;
typedef std::vector<char> tile;

struct position {
    int x, y;
};

dir operator+(dir d1, dir d2) {
    return std::make_tuple(
        std::get<0>(d1) + std::get<0>(d2),
        std::get<1>(d1) + std::get<1>(d2)
    );
}

std::vector<char> tiles;
std::vector<char> exampleMap;
std::vector<char> outputMap;
std::unordered_map<int, int> colors;

int EXAMPLE_WIDTH = std::numeric_limits<int>::min();
int EXAMPLE_HEIGHT = std::numeric_limits<int>::min();

std::vector<tile> map;

std::unordered_map<int, int> frequencies;

std::vector<dir> dirs;
std::vector<rule> rules;

#define MAP_WIDTH 32
#define MAP_HEIGHT 32

auto seed = std::chrono::system_clock::now().time_since_epoch().count();

std::mt19937 gen;

bool diags = true;

struct TileData
{
    int c;
    int col;
};

std::vector<TileData> exampleFileData;

tile& GetMapTile(position pos)
{
    return map[MAP_POSITION(pos.y, pos.x, MAP_WIDTH)];
}

int ChooseRandomWeighted(std::vector<double> weights)
{
    std::discrete_distribution<int> dist(std::begin(weights), std::end(weights));

    return dist(gen);
}

void AddRule(char from, char to, dir dir)
{
    rule r = rule(from, to, dir);

    // Look for duplicates before push_back
    auto it = std::find(rules.begin(), rules.end(), r);
    if (it == rules.end())
        rules.push_back(r);
}

void IncreaseFrequency(char tile)
{
    frequencies[tile]++;
}

void InitRules()
{
    dirs.reserve(4);
    dirs.resize(4);

    dirs[Direction::up] = dir(0, 1);
    dirs[Direction::down] = dir(0, -1);
    dirs[Direction::left] = dir(-1, 0);
    dirs[Direction::right] = dir(1, 0);

    if (diags) {
        dirs.resize(Direction::MAX);
        dirs[Direction::upleft] = dirs[Direction::up] + dirs[Direction::left];
        dirs[Direction::upright] = dirs[Direction::up] + dirs[Direction::right];
        dirs[Direction::downleft] = dirs[Direction::down] + dirs[Direction::left];
        dirs[Direction::downright] = dirs[Direction::down] + dirs[Direction::right];
    }
}

bool IsNegativeBound(int val)
{
    return val < 0;
}

bool IsGreaterBound(int val, int max)
{
    return val >= max;
}

void ParseFileData()
{
    if (exampleFileData.empty())
        throw "No File Data to parse.";

    for (auto& tile_data : exampleFileData) {
        auto it = std::find(tiles.begin(), tiles.end(), tile_data.c);

        if (it == tiles.end())
            tiles.push_back(tile_data.c);

        exampleMap.push_back(tile_data.c);

        if (tile_data.col == -1) {
            colors[tile_data.c] = rand() % 25;
        }
        else
            colors[tile_data.c] = tile_data.col;
    }
}

void GenerateRules()
{
    ParseFileData();

    for (int y = 0; y < EXAMPLE_HEIGHT; ++y) {
        for (int x = 0; x < EXAMPLE_WIDTH; ++x) {
            char from = exampleMap[MAP_POSITION(y, x, EXAMPLE_WIDTH)];

            IncreaseFrequency(from);

            // Up Check
            if (!IsNegativeBound(y - 1))
                AddRule(from, exampleMap[MAP_POSITION((y - 1), x, EXAMPLE_HEIGHT)], dirs[Direction::up]);
            // Down Check
            if (!IsGreaterBound(y + 1, EXAMPLE_HEIGHT))
                AddRule(from, exampleMap[MAP_POSITION((y + 1), x, EXAMPLE_HEIGHT)], dirs[Direction::down]);
            // Left Check
            if (!IsNegativeBound(x - 1))
                AddRule(from, exampleMap[MAP_POSITION(y, (x - 1), EXAMPLE_WIDTH)], dirs[Direction::left]);
            // Right Check
            if (!IsGreaterBound(x + 1, EXAMPLE_WIDTH))
                AddRule(from, exampleMap[MAP_POSITION(y, (x + 1), EXAMPLE_WIDTH)], dirs[Direction::right]);

            if (diags) {
                // UpLeft
                if (!IsNegativeBound(y - 1) && !IsNegativeBound(x - 1))
                    AddRule(from, exampleMap[MAP_POSITION((y - 1), (x - 1), EXAMPLE_WIDTH)], dirs[Direction::upleft]);
                // UpRight
                if (!IsNegativeBound(y - 1) && !IsGreaterBound(x + 1, EXAMPLE_WIDTH))
                    AddRule(from, exampleMap[MAP_POSITION((y - 1), (x + 1), EXAMPLE_WIDTH)], dirs[Direction::upright]);

                // DownLeft
                if (!IsGreaterBound(y + 1, EXAMPLE_HEIGHT) && !IsNegativeBound(x - 1))
                    AddRule(from, exampleMap[MAP_POSITION((y + 1), (x - 1), EXAMPLE_WIDTH)], dirs[Direction::downleft]);
                // DownRight
                if (!IsGreaterBound(y + 1, EXAMPLE_HEIGHT) && !IsGreaterBound(x + 1, EXAMPLE_WIDTH))
                    AddRule(from, exampleMap[MAP_POSITION((y + 1), (x + 1), EXAMPLE_WIDTH)], dirs[Direction::downright]);
            }
        }
    }
}

void PrintRules()
{
#if ENABLE_DEBUG
    printf(" => Learned Rules:\n");
    for (rule r : rules) {
        printf("%d (%d, %d) %d\n", std::get<0>(r), std::get<0>(std::get<2>(r)), std::get<1>(std::get<2>(r)), std::get<1>(r));
    }
#endif

#if ENABLE_DEBUG
    printf(" => Frequencies:\n");
    for (auto& f : frequencies) {
        printf("Saw %d => %d times\n", f.first, f.second);
    }
#endif
}

void InitMap()
{
    map.clear();
    for (int i = 0; i < MAP_HEIGHT * MAP_WIDTH; ++i)
        map.push_back(tiles);

    outputMap.clear();
    outputMap.resize(map.size());
}

void CollapseWaveFunction(position currentTile)
{
    auto& tile = GetMapTile(currentTile);
    auto valid_weights = std::vector<std::pair<const char, int>>();

    for (auto& weight : frequencies) {
        auto it = std::find(tile.begin(), tile.end(), weight.first);
        if (it != tile.end())
            valid_weights.push_back(weight);
    }

    int total_weight = 0;

    for (auto& w : valid_weights)
        total_weight += w.second;

    char choiceChar = -1;

    do {
        auto rnd = (((double)rand() / (RAND_MAX))) * total_weight;

        for (auto& w : valid_weights) {
            rnd -= w.second;
            if (rnd < 0) {
                choiceChar = w.first;
                break;
            }
        }
    } while (choiceChar == -1);

    // Remove choices not matching our choice above from tile possibilities.
    auto end = std::remove_if(tile.begin(), tile.end(), [&](const char c) {return c != choiceChar;});

    // Actually remove invalid choices
    tile.resize(std::distance(tile.begin(), end));

    // Check for a contradiction
    if (tile.empty())
        throw ContradictionException;

#if ENABLE_DEBUG
    printf("Collapsed Position: (%d, %d) into %d\n", currentTile.x, currentTile.y, tile[0]);
#endif

    outputMap[MAP_POSITION(currentTile.y, currentTile.x, MAP_WIDTH)] = tile[0];
}

double ShannonEntropy(position tilePosition)
{
    int sum_weights = 0;
    int sum_weights_log = 0;

    for (auto& tile_possibility : GetMapTile(tilePosition)) {
        int weight = frequencies[tile_possibility];

        sum_weights += weight;
        sum_weights_log += weight * log(weight);
    }

    return log(sum_weights) - (sum_weights_log / sum_weights);
}

bool IsCollapsed(position tilePosition)
{
    return GetMapTile(tilePosition).size() == 1;
}

// Ensure the neighbor position is correct. Also ensure the position isnt fully collapsed yet.
bool TestNeighbor(position tilePosition, std::vector<std::pair<position, dir>>& neighbors, dir direction)
{
    if (tilePosition.x > 0 && tilePosition.x < MAP_WIDTH && tilePosition.y > 0 && tilePosition.y < MAP_HEIGHT && !IsCollapsed(tilePosition)) {
        neighbors.push_back(std::make_pair(position{ tilePosition.x, tilePosition.y }, direction));
        return true;
    }

    return false;
}

// Return a list of tile positions which are neighbors
std::vector<std::pair<position, dir>> FindAvailableNeighbors(position tilePosition)
{
    // Find neighbors,
    std::vector<std::pair<position, dir>> neighbors;

    // Start top left and go clock-wise
    TestNeighbor(position{ tilePosition.x, tilePosition.y - 1 }, neighbors, dirs[Direction::up]); // Top
    TestNeighbor(position{ tilePosition.x, tilePosition.y + 1 }, neighbors, dirs[Direction::down]); // Bottom
    TestNeighbor(position{ tilePosition.x - 1, tilePosition.y }, neighbors, dirs[Direction::left]); // Left
    TestNeighbor(position{ tilePosition.x + 1, tilePosition.y }, neighbors, dirs[Direction::right]); // Right

    if (diags) {
        TestNeighbor(position{ tilePosition.x - 1, tilePosition.y - 1 }, neighbors, dirs[Direction::upleft]); // Top-Left
        TestNeighbor(position{ tilePosition.x + 1, tilePosition.y - 1 }, neighbors, dirs[Direction::upright]); // Top-Right
        TestNeighbor(position{ tilePosition.x - 1, tilePosition.y + 1 }, neighbors, dirs[Direction::downleft]); // Bottom-Left
        TestNeighbor(position{ tilePosition.x + 1, tilePosition.y + 1 }, neighbors, dirs[Direction::downright]); // Bottom-Right
    }

    return neighbors;
}

bool CheckRules(const char current, std::pair<position, dir> neighborTile, const char neighbor)
{
    rule temp_rule = rule(current, neighbor, neighborTile.second);

    auto it = std::find(rules.begin(), rules.end(), temp_rule);

    return (it != rules.end());
}

void Constrain(position pos, const char tile)
{
    auto& possibilities = GetMapTile(pos);
    auto it = std::find(possibilities.begin(), possibilities.end(), tile);

    if (it != possibilities.end()) {
        possibilities.erase(it);

        // Check for a contradiction
        if (possibilities.empty())
            throw ContradictionException;

#if ENABLE_DEBUG
        printf("Constrained Position: (%d, %d) into ", pos.x, pos.y);

        for (auto& possibility : possibilities) {
            printf("%d ", possibility);
        }

        printf("\n");
#endif
    }

    if (possibilities.size() == 1)
        outputMap[MAP_POSITION(pos.y, pos.x, MAP_WIDTH)] = possibilities[0];
}

// Keep doing this function until the propagation has 'died down'
void PropagateWaveFunction(position currentTile)
{
    // Propagate each possible change in the map
    std::stack<position> propagation;
    propagation.push(currentTile);

    // While we still have tiles on the stack
    while (!propagation.empty()) {
        position current_pos = propagation.top();
        tile& current_tile = GetMapTile(current_pos);

        propagation.pop();

        // Look through each neighbor
        for (auto neighbor : FindAvailableNeighbors(current_pos)) {
            // Investigate neighbor's possibilities. Check if any of their possibilities are compatible with any of our possibilities.
            //  - Look through our rules for anything that could match our situation

            for (auto& neighbor_tile : GetMapTile(neighbor.first)) {
                bool other_possible = std::any_of(current_tile.begin(), current_tile.end(), [&](auto& t) { return CheckRules(t, neighbor, neighbor_tile); });

                if (!other_possible) {
                    Constrain(neighbor.first, neighbor_tile);
                    // If not, remove the possibility from neighbor and push neighbor position to stack
                    propagation.push(neighbor.first);
                }
            }
        }
    }
}

// Start with max possibilities
// Loop map
// If new_tile.size < current
//  current = new_tile
// if new_tile == current
//  add to equivalent map
// Once done: randomly select 1 out of equivalent map
position FindLowestEntropyGlobal()
{
    std::vector<position> equiv_entropy;
    double lowest_entropy = std::numeric_limits<double>::max();

    position lowest_entropy_pos{ std::numeric_limits<int>::min(), std::numeric_limits<int>::min() };

    for (int i = 0; i < MAP_HEIGHT; ++i)
        for (int j = 0; j < MAP_WIDTH; ++j) {
            position tile_pos = position{ i,j };

            // Immediately ignore any tile thats fully collapsed
            if (GetMapTile(tile_pos).size() == 1)
                continue;

            auto tile_entropy = ShannonEntropy(tile_pos);

            if (tile_entropy > lowest_entropy)
                continue;
            else if (tile_entropy < lowest_entropy) {
                lowest_entropy = tile_entropy;
                lowest_entropy_pos = tile_pos;
                equiv_entropy.clear();
            }
            equiv_entropy.push_back(tile_pos);
        }

    return equiv_entropy[std::rand() % equiv_entropy.size()];
}

bool IsGloballyCollapsed()
{
    for (auto& tile : map) {
        if (tile.size() > 1)
            return false;
    }

    return true;
}

#define COLOR_BLACK 0
#define COLOR_BLUE 1
#define COLOR_GREEN 2
#define COLOR_AQUA 3
#define COLOR_RED 4
#define COLOR_PURPLE 5
#define COLOR_WHITE 15

#ifdef _WIN32
#include <windows.h>

inline void setcolor(int textcol, int backcol)
{
    //if ((textcol % 16) == (backcol % 16))textcol++;
    textcol %= 16; backcol %= 16;
    unsigned short wAttributes = ((unsigned)backcol << 4) | (unsigned)textcol;
    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hStdOut, wAttributes);
}
#endif

void DrawMap(int xBound, int yBound, const char* map)
{
    for (int y = 0; y < yBound; ++y) {
        for (int x = 0; x < xBound; ++x) {
            char c = map[MAP_POSITION(y, x, xBound)];
            try {
                int col = colors.at(c);

                setcolor(col, col);
            }
            catch (std::exception & e) {
                continue;
            }

            printf("%c", c);
        }

        setcolor(COLOR_WHITE, COLOR_BLACK);
        printf("\n");
    }
}

void Step()
{
    position nextTile = FindLowestEntropyGlobal();
    CollapseWaveFunction(nextTile);

    PropagateWaveFunction(nextTile);
}

void Engine()
{
    while (!IsGloballyCollapsed())
    {
        // TODO: Future, we might rollback possibility stack to find a good one?
        Step();
    }
}

unsigned long long InitRand()
{
    seed = std::chrono::system_clock::now().time_since_epoch().count();

    std::srand(seed);
    gen.seed(seed);

#if ENABLE_DEBUG
    printf("Seed value: %llu\n", seed);
#endif

    return seed;
}

bool AskNewMap()
{
    printf("Generate another map? (y/n) ");

    char str[4];
    char val = 'n';

    if (fgets(str, 4, stdin) != nullptr)
        val = str[0];

    return val == 'y';
}

void OutputResult()
{
#if ENABLE_OUTPUT
    printf("EXAMPLE:\n");
    DrawMap(EXAMPLE_WIDTH, EXAMPLE_HEIGHT, exampleMap.data());

    printf("\nRESULT:\n");
    DrawMap(MAP_WIDTH, MAP_HEIGHT, outputMap.data());
#endif
}

int ParseExampleTiles(std::string line)
{
    // Each line consists of a (char, color) (..., ...) combo
    int tile;
    int count = 0;
    int offset = 0;
    std::size_t read = 0;

    while (std::sscanf(line.c_str() + offset, "%d,%n", &tile, &read) > 0) {
        offset += read;
        count++;
        exampleFileData.push_back(TileData{ tile, -1 });
    }

    return count;
}

int ParseExampleColors(std::string line)
{
    int tileNum;
    int tileColor;

    if (std::sscanf(line.c_str(), "%d=%d", &tileNum, &tileColor) < 0)
        throw "Could not read file";

    for (auto& tileData : exampleFileData) {
        if (tileData.c == tileNum) {
            tileData.col = tileColor;
        }
    }
}

void InitExampleData(int argc, char** argv)
{
    // Reading Files =>
    //  NOW: File should be : (char, color) (..., ...)
    //  LATER: YAML? JSON? CSV?
    if (argc < 2)
        throw "Please provide an input file for example";

    std::string line;
    std::ifstream file;

    file.open(argv[1]); // TODO: Add some checking on this string?

    if (file.is_open()) {
        int lines = 0;
        int count = 0;

        while (getline(file, line)) {
            count = ParseExampleTiles(line);
            lines++;
        }

        file.close();

        EXAMPLE_WIDTH = count;
        EXAMPLE_HEIGHT = lines;
    }

    // Test if we have color data
    if (argc > 2) {
        file.open(argv[2]);

        if (file.is_open()) {
            while (getline(file, line)) {
                ParseExampleColors(line);
            }
        }

        file.close();
    }
}

int main(int argc, char** argv)
{
    try {
        InitExampleData(argc, argv);
    }
    catch (std::exception & e)
    {
#if ENABLE_OUTPUT
        printf("%s\n", e.what());
#endif
        return -1;
    }

    InitRules();
    GenerateRules();
    PrintRules();

Start:
    InitRand();
    InitMap();

    try {
        Engine();
    }
    catch (std::exception & e)
    {
#if ENABLE_OUTPUT
        printf("%s\n", e.what());
#endif
        OutputResult();
        goto Start;
    }

    OutputResult();

#if ASK_REPEAT
    if (AskNewMap())
        goto Start;
#endif

    return 0;
}