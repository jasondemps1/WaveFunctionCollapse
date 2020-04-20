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

#define MAP_WIDTH 32
#define MAP_HEIGHT 32

tile map[MAP_WIDTH * MAP_HEIGHT];

std::unordered_map<char, int> frequencies;

std::unordered_map<const char*, dir> dirs;
std::vector<rule> rules;

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
}

void CollapseWaveFunction(position currentTile)
{
}

double ShannonEntropy(position tilePosition)
{
    int sum_weights = 0;
    int sum_weights_log = 0;

    for (auto& tile_possibility : map[tilePosition.x]) {
        int weight = frequencies[tile_possibility];

        sum_weights += weight;
        sum_weights_log += weight * log(weight);
    }

    return log(sum_weights) - (sum_weights_log / sum_weights);
}

bool IsCollapsed(position tilePosition)
{
    return map[tilePosition.x * MAP_WIDTH + tilePosition.y].size() == 1;
}

// Ensure the neighbor position is correct. Also ensure the position isnt fully collapsed yet.
bool TestNeighbor(position tilePosition, std::vector<position>& neighbors)
{
    if (tilePosition.x > 0 && tilePosition.y > 0 && !IsCollapsed(tilePosition)) {
        neighbors.push_back(position{ tilePosition.x, tilePosition.y });
        return true;
    }

    return false;
}

// Return a list of tile positions which are neighbors
std::vector<position> FindAvailableNeighbors(position tilePosition)
{
    // Find neighbors,
    std::vector<position> neighbors;

    // Start top left and go clock-wise
    TestNeighbor(position{ tilePosition.x - 1, tilePosition.y - 1 }, neighbors); // Top-Left
    TestNeighbor(position{ tilePosition.x, tilePosition.y - 1 }, neighbors); // Top
    TestNeighbor(position{ tilePosition.x + 1, tilePosition.y - 1 }, neighbors); // Top-Right
    TestNeighbor(position{ tilePosition.x + 1, tilePosition.y }, neighbors); // Right
    TestNeighbor(position{ tilePosition.x + 1, tilePosition.y + 1 }, neighbors); // Bottom-Right
    TestNeighbor(position{ tilePosition.x, tilePosition.y + 1 }, neighbors); // Bottom
    TestNeighbor(position{ tilePosition.x - 1, tilePosition.y + 1 }, neighbors); // Bottom-Left

    return neighbors;
}

// Start with max possibilities
// Loop map
// If new_tile.size < current
//  current = new_tile
// if new_tile == current
//  add to equivalent map
// Once done: randomly select 1 out of equivalent map
//std::vector<char>& FindLowestEntropy()
position FindLowestEntropy(position tilePosition)
{
    std::vector<position> neighbors = FindAvailableNeighbors(tilePosition);

    std::vector<position> equiv_entropy;
    double lowest_entropy = std::numeric_limits<double>::max();

    position lowest_entropy_pos{ std::numeric_limits<int>::min(), std::numeric_limits<int>::min() };

    for (auto& neighbor : neighbors) {
        auto neighbor_entropy = ShannonEntropy(neighbor);

        if (neighbor_entropy > lowest_entropy)
            continue;
        else if (neighbor_entropy < lowest_entropy) {
            lowest_entropy = neighbor_entropy;
            lowest_entropy_pos = neighbor;
            equiv_entropy.clear();
        }
        else
            equiv_entropy.push_back(neighbor);
    }

    return equiv_entropy[std::rand() % equiv_entropy.size()];
}

void Engine(position startTile)
{
    while (true) // TODO: Loop until contradiction/paradox or complete
    {
        position nextTile = FindLowestEntropy(startTile);

        try {
            CollapseWaveFunction(nextTile);
        }
        catch (std::exception & e) {
        }
        // foreach neighbor, collapse its wave function until nothing to do, and so on.
    }
}

unsigned int InitRand()
{
    unsigned int seed = std::time(nullptr);
    std::srand(seed);

    return seed;
}

int main(int argc, char** argv)
{
    InitRand();
    InitRules();
    GenerateRules();
    PrintRules();

    InitMap();

    Engine(position{ rand() % MAP_WIDTH, rand() % MAP_HEIGHT });

    return 0;
}