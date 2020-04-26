#include <stdio.h>
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

class ContradictionException : public std::exception
{
    virtual const char* what() const throw()
    {
        return "EXCEPTION: Contradiction";
    }
} ContradictionException;

typedef std::tuple<int, int> dir;
typedef std::tuple<char, char, dir> rule;
typedef std::vector<char> tile;

struct position {
    int x, y;
};

#define EXAMPLE_W 3
#define EXAMPLE_H 3

char tiles[] = { 'S', 'C', 'L' };

char example[EXAMPLE_W * EXAMPLE_H] = {
    tiles[0], tiles[0], tiles[0],
    tiles[1], tiles[1], tiles[0],
    tiles[2], tiles[2], tiles[1]
};

#define MAP_WIDTH 3
#define MAP_HEIGHT 3

tile map[MAP_WIDTH * MAP_HEIGHT];

std::unordered_map<char, int> frequencies;

std::unordered_map<const char*, dir> dirs;
std::vector<rule> rules;

char outputMap[MAP_WIDTH * MAP_HEIGHT];

unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();

std::mt19937 gen;

int ChooseRandomWeighted(std::vector<double> weights)
{
    std::discrete_distribution<int> dist(std::begin(weights), std::end(weights));

    return dist(gen);
}

void AddRule(char from, char to, dir direction)
{
    rule r = rule(from, to, direction);

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
    dirs["up"] = dir(0, 1);
    dirs["down"] = dir(0, -1);
    dirs["left"] = dir(-1, 0);
    dirs["right"] = dir(1, 0);
}

void GenerateRules()
{
    for (int row = 0; row < EXAMPLE_H; ++row) {
        for (int col = 0; col < EXAMPLE_W; ++col) {
            char from = example[row * EXAMPLE_W + col];

            IncreaseFrequency(from);

            // Up Check
            if (row != 0)
                AddRule(from, example[(row - 1) * EXAMPLE_W + col], dirs["up"]);
            // Down Check
            if (row < EXAMPLE_H - 1)
                AddRule(from, example[(row + 1) * EXAMPLE_W + col], dirs["down"]);
            // Left Check
            if (col != 0)
                AddRule(from, example[row * EXAMPLE_W + (col - 1)], dirs["left"]);
            // Right Check
            if (col < EXAMPLE_W - 1)
                AddRule(from, example[row * EXAMPLE_W + (col + 1)], dirs["right"]);
        }
    }
}

void PrintRules()
{
    for (rule r : rules) {
        printf("%c (%d, %d) %c\n", std::get<0>(r), std::get<0>(std::get<2>(r)), std::get<1>(std::get<2>(r)), std::get<1>(r));
    }

    for (auto& f : frequencies) {
        printf("Saw %c => %d times\n", f.first, f.second);
    }
}

void InitMap()
{
    tile map_template(tiles, tiles + sizeof(tiles));

    // Generate a map of superposition tiles
    for (int i = 0; i < MAP_HEIGHT; ++i)
        for (int j = 0; j < MAP_WIDTH; ++j)
            map[i * MAP_WIDTH + j] = map_template;

    memset(outputMap, 0, MAP_WIDTH * MAP_HEIGHT * sizeof outputMap[0]);
}

tile& GetMapTile(position pos)
{
    return map[pos.y * MAP_WIDTH + pos.x];
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

    auto rnd = ((double)rand() / (RAND_MAX)) + 1;

    char choiceChar;
    for (auto& w : valid_weights) {
        rnd -= w.second;
        if (rnd < 0) {
            //chosen = w;
            choiceChar = w.first;
            break;
        }
    }

    // Remove choices not matching our choice above from tile possibilities.
    auto end = std::remove_if(tile.begin(), tile.end(), [&](const char c) {return c != choiceChar;});

    // Actually remove invalid choices
    tile.resize(std::distance(tile.begin(), end));

    // Check for a contradiction
    if (tile.empty())
        throw ContradictionException;

    printf("Collapsed Position: (%d, %d) into %c\n", currentTile.x, currentTile.y, tile[0]);

    outputMap[currentTile.x * MAP_WIDTH + currentTile.y] = tile[0];
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
    //TestNeighbor(position{ tilePosition.x - 1, tilePosition.y - 1 }, neighbors); // Top-Left
    TestNeighbor(position{ tilePosition.x, tilePosition.y - 1 }, neighbors, dirs["up"]); // Top
    //TestNeighbor(position{ tilePosition.x + 1, tilePosition.y - 1 }, neighbors); // Top-Right
    TestNeighbor(position{ tilePosition.x + 1, tilePosition.y }, neighbors, dirs["right"]); // Right
    //TestNeighbor(position{ tilePosition.x + 1, tilePosition.y + 1 }, neighbors); // Bottom-Right
    TestNeighbor(position{ tilePosition.x, tilePosition.y + 1 }, neighbors, dirs["down"]); // Bottom
    //TestNeighbor(position{ tilePosition.x - 1, tilePosition.y + 1 }, neighbors); // Bottom-Left
    TestNeighbor(position{ tilePosition.x - 1, tilePosition.y }, neighbors, dirs["left"]); // Left

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

        printf("Constrained Position: (%d, %d) into ", pos.x, pos.y);

        for (auto& possibility : possibilities) {
            printf("%c ", possibility);
        }

        printf("\n");
    }

    if (possibilities.size() == 1)
        outputMap[pos.x * MAP_WIDTH + pos.y] = possibilities[0];
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
            position tile_pos = position{ j,i };

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

void DrawMap()
{
    for (int i = 0; i < MAP_HEIGHT; ++i) {
        for (int j = 0; j < MAP_WIDTH; ++j) {
            //position tile_pos = position{ j,i };
            printf("%c", outputMap[i * MAP_WIDTH + j]);
        }
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

unsigned int InitRand()
{
    seed = std::chrono::system_clock::now().time_since_epoch().count();

    std::srand(seed);
    gen.seed(seed);

    printf("Seed value: %u\n", seed);

    return seed;
}

int main(int argc, char** argv)
{
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
        printf("%s\n", e.what());
        DrawMap();
        goto Start;
    }

    DrawMap();

    return 0;
}