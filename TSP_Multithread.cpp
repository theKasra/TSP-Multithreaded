#include <iostream>
#include <Windows.h>
#include <time.h>
#include <stdlib.h>
#include <vector>
#include <fstream>
#include <string>
#include <thread>

struct PathInfo {
    std::vector<int> path;
    int cost;
    int thread_id;
};

// global variables
const int cores = std::thread::hardware_concurrency();
int input_number, runtime, cities;
std::vector<std::vector<int>> map;
std::vector<PathInfo> thread_answers;
bool start_switch = false;

int GetTotalNumberOfCities(unsigned int input_number)
{
    std::ifstream f;
    std::string temp;

    if (input_number == 1)
    {
        f.open("input_1.txt");
        if (f.is_open())
        {
            std::getline(f, temp);
            f.close();
            return std::stoi(temp);
        }
        else return -1;
    }

    else if (input_number == 2)
    {
        f.open("input_2.txt");
        if (f.is_open())
        {
            std::getline(f, temp);
            f.close();
            return std::stoi(temp);
        }
        else return -1;
    }

    else return -1;
}
std::vector<std::vector<int>> ReadInput(std::string file_path, char split_char, int cities)
{
    std::vector<std::vector<int>> map;

    std::ifstream f;
    f.open(file_path);

    if (f.is_open())
    {
        std::string first_line;
        getline(f, first_line);

        std::string str;
        while (!f.eof())
        {
            std::vector<int> row;
            for (int i = 0; i < cities; i++)
            {
                getline(f, str, split_char);
                row.push_back(std::stoi(str));
            }
            map.push_back(row);
        }
    }

    else
        std::cout << "An error occured during reading the file!" << std::endl;

    return map;
}
int FindCost(std::vector<std::vector<int>> map, int curr_city, int travel_to)
{
    for (int i = 0; map.size(); i++)
    {
        if (i == curr_city)
        {
            std::vector<int> temp = map[i];
            for (int j = 0; temp.size(); j++)
            {
                if (j == travel_to)
                {
                    return temp.at(j);
                }
            }
        }
    }
}
PathInfo FindBestAnswer(std::vector<PathInfo> answers)
{
    PathInfo best_answer;

    best_answer = answers[0];
    for (int i = 0; i < answers.size(); i++)
    {
        if (best_answer.cost < answers[i].cost)
        {
            best_answer = answers[i];
        }
    }
        return best_answer;
}
void PrintAnswer(PathInfo answer)
{
    std::cout << "\n";
    for (int i = 0; i < answer.path.size(); i++)
    {
        std::cout << answer.path[i];
        if (i < answer.path.size() - 1) std::cout << " -> ";
    }
    std::cout << "\nCost: " << answer.cost << std::endl;
}
void TSP_RandomPaths(std::vector<std::vector<int>> map, int cities)
{
    int thread_id = GetCurrentThreadId();

    int curr_city = 0;
    int curr_cost = 0;
    int travel_to;

    int rand_min = 1;
    int rand_max = cities - 1;

    std::vector<int> visited;
    visited.push_back(curr_city);

    srand(time(NULL) + thread_id);
    int Time = time(NULL);

    PathInfo answer;
    int loop_counter = 0;

    while (time(NULL) - Time < runtime)
    {
        // right before the end, make it a cycle
        if (visited.size() < cities)
            travel_to = ((rand() % rand_max - rand_min + 1) + rand_min);
        else
        {
            travel_to = 0;
            curr_cost += FindCost(map, curr_city, travel_to);
            visited.push_back(travel_to);
            curr_city = travel_to;
        }

        // avoid revisiting cities
        if (std::find(visited.begin(), visited.end(), travel_to) != visited.end() && visited.size() < cities)
            continue;

        // found a hamiltonian cycle
        if (curr_cost != 0 && visited.size() == cities + 1)
        {
            if (loop_counter == 0)
            {
                answer.path = visited;
                answer.cost = curr_cost;
                answer.thread_id = thread_id;
                loop_counter++;
            }

            else if (curr_cost < answer.cost)
            {
                answer.path = visited;
                answer.cost = curr_cost;
                answer.thread_id = thread_id;
            }

            // prepare for finding another path
            visited.clear();
            visited.push_back(curr_city);
            curr_cost = 0;
            continue;
        }

        curr_cost += FindCost(map, curr_city, travel_to);
        visited.push_back(travel_to);
        curr_city = travel_to;
    }
    for (int i = 0; i < cores; i++)
    {
        if(thread_answers[i].thread_id == thread_id)
            thread_answers[i] = answer;
    }
}


DWORD WINAPI f(LPVOID Params)
{
    std::vector<std::vector<int>>& map = *(std::vector<std::vector<int>>*) Params;
    while(!start_switch);
    TSP_RandomPaths(map, cities);
    return 0;
}


int main()
{
    thread_answers.resize(cores);

    std::cout << "Which input would you like to choose?\n" << "1. input_1.txt - 5 cities\n" << "2. input_2.txt - 100 cities\n\n" << "Enter the number: ";
    std::cin >> input_number;
    std::cout << "\nPlease set the runtime value: ";
    std::cin >> runtime;

    cities = GetTotalNumberOfCities(input_number);

    if (input_number == 1)
    {
        map = ReadInput("input_1.txt", '\t', cities);
    }

    else if (input_number == 2)
    {
        map = ReadInput("input_2.txt", ' ', cities);
    }


    HANDLE* h = new HANDLE[cores];
    for (int i = 0; i < cores; i++)
    {
         h[i] = CreateThread(NULL, 0, f, &map, 0, NULL);
         thread_answers[i].thread_id = GetThreadId(h[i]);
    }

    start_switch = true;

    WaitForMultipleObjects(cores, h, true, INFINITE);

    PathInfo final_result = FindBestAnswer(thread_answers);
    PrintAnswer(final_result);

    for (int i = 0; i < cores; i++)
        CloseHandle(h[i]);

    delete[] h;
    return 0;
}
