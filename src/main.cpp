#include "main.hpp"
#include "helper.hpp" 
#include "Core.hpp"
#include "dram.hpp"


int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cout << "Please give input file path" << std::endl;
        exit(-1);
    }

    std::ifstream inFile(argv[1], std::ofstream::in);

    if (!inFile)
    {
        std::cerr << "Input correct file path" << std::endl;
        exit(-1);
    }

    std::cout << "Input the number of files: ";

    inFile >> n >> DRAM::ROW_ACCESS_DELAY >> DRAM::COL_ACCESS_DELAY;

    optimize = true;

    if (((unsigned int)pow(2, 18)) % n != 0)
    {
        std::cout << "Invalid number of cores given: Number of cores must be in power of 2";
        exit(-1);
    }

    Core *cores[n];

    std::string temp;
    getline(inFile, temp);

    for (int i = 0; i < n; i++)
    {
        std::cout << "Input the path of file " << (i + 1) << ": ";
        std::string path;
        getline(inFile, path);
        cores[i] = new Core(path, (unsigned int)i);
        cores[i]->compile();
        cores[i]->setup();
    }

    // std::cout << "Input DRAM row access delay: ";
    // std::cin >> DRAM::ROW_ACCESS_DELAY;

    // std::cout << "Input DRAM column access delay: ";
    // std::cin >> DRAM::COL_ACCESS_DELAY;

    while (true)
    {
        bool f = false;
        for (unsigned int i = 0; i < n; i++)
        {
            f = f | cores[i]->execute();
        }
        if (!f)
        {
            break;
        }
        std::cout << totalCycles << std::endl;
    }

    for (int i = 0; i < n; i++)
    {
        delete cores[i];
    }

    return 0;
}



