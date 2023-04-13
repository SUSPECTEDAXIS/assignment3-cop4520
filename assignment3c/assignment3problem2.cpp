#include <climits>
#include <iostream>
#include <memory>
#include <mutex>
#include <algorithm>
#include <functional>
#include <set>
#include <thread>
#include <random>

#define TC 8 // each thread will have ranges of 60 meaning 0-60 then 61- whatever and so on

#define MS 60 //this doesnt stand for milliseconds but minutes in this case

#ifndef H
#define H 24 // doing 1 day only
#endif

std::mutex mutex;
// doing this rng which will later allow us to actually choose a rangte for it and make the numbers
// make sense
int generateRandomNumber(int min, int max)
{
    std::random_device seed;
    std::mt19937 rng(seed());
    std::uniform_int_distribution<std::mt19937::result_type> dist(min, max);

    return dist(rng);
}
// this function below just lets me know hey sensors are all good to go
bool isR(int c, std::vector<bool> &s)
{
    for (int i = 0; i < static_cast<int>(s.size()); i++)
    {
        if (!s[i] && c != i)
        {
            return false;
        }
    }
    return true;
}

void biggestDiff(std::vector<int> &sR)
{
    int start = 0; // start interval for minutes
    int jump = 10; // will be added to our start interval to show where it starts to end
    int mD = INT_MIN;

    // this foor loop its to give us largest difference
    for (int id = 0; id < TC; id++)
    {
        int offset = id * MS;

        for (int i = offset; i < MS - jump + 1; i++)
        {
            int max = *std::max_element(sR.begin() + i, sR.begin() + i + jump); // sr will be for our readings from our sensor
            int min = *std::min_element(sR.begin() + i, sR.begin() + i + jump);
            int d = max - min;

            if (d > mD)
            {
                mD = d;
                start = i;
            }
        }
    }
    std::cout << "Biggest difference from" << start << " minutes to " << (start + 10) << " was " << mD << "F" << std::endl;
}

void hT(std::vector<int> &sR)
{
    std::set<int> t{}; // t is for temperature

    for (auto it = sR.rbegin(); it != sR.rend(); it++)
    {
        if (t.find(*it) == t.end())
        {
            t.insert(*it);
        }

        if (t.size() == 5)
        {
            break;
        }
    }

    std::cout << "The 5 highest temperatures recorded during this hour report were; ";
    for (int temperature : t)
    {
        std::cout << temperature << "F ";
    }

    std::cout << std::endl;
}

void lT(std::vector<int> &sr)
{
    std::set<int> t{};

    for (auto it = sr.begin(); it != sr.end(); it++)
    {
        if (t.find(*it) == t.end())
        {
            t.insert(*it);
        }

        if (t.size() == 5)
        {
            break;
        }
    }

    std::cout << "The 5 lowest temperatures recorded during this hour report were; ";
    for (int temperature : t)
    {
        std::cout << temperature << "F ";
    }

    std::cout << std::endl;
}

void report(int h, std::vector<int> &sr)
{
    std::cout << h + 1 << " hour has passed" << std::endl;
    std::cout << "report below " << std::endl;
    biggestDiff(sr);
    std::sort(sr.begin(), sr.end());
    hT(sr);
    lT(sr);

    std::cout << std::endl;
}

void mT(int id, std::vector<int> &sr, std::vector<bool> &isSRD)
{
    for (int h = 0; h < HS; h++)
    {
        for (int min = 0; min < MS; min++)
        {
            isSRD[id] = false;
            sr[min + (id * MS)] = generateRandomNumber(-100, 70);
            isSRD[id] = true;
            // while loop below will make it sleep to try to simulate real time
            while (!isR(id, isSRD))
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }

        if (id == 0)
        {
            mutex.lock();
            report(h, sr);
            mutex.unlock();
        }
    }
}

int main()
{
    std::vector<int> sr(TC * MS);
    std::vector<bool> isSRD(TC);
    std::thread threads[TC] = {};

    for (int i = 0; i < TC; i++)
    {
        threads[i] = std::thread(mT, i, std::ref(sr), std::ref(isSRD));
    }

    auto start = std::chrono::high_resolution_clock::now();

    for (std::thread &thread : threads)
    {
        thread.join();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<double, std::milli>(end - start);

    std::cout << "Finished in " << duration.count() << "ms" << std::endl;
}
