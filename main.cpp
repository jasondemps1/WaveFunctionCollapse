#include <stdio.h>
#include <algorithm>
#include <vector>
#include <tuple>
#include <unordered_map>
#include <string>

#define EXAMPLE_W 3
#define EXAMPLE_H 3

char example[EXAMPLE_W * EXAMPLE_H] = {
    'S', 'S', 'S',
    'C', 'C', 'S',
    'L', 'L', 'C'
};

typedef std::tuple<int, int> dir;
typedef std::tuple<char, char, dir> rule;

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
}

int main(int argc, char** argv)
{
    InitRules();
    GenerateRules();
    PrintRules();

    return 0;
}