//
// Created by blackrat on 02.04.25.
//

#include "cdma_decoder.h"
#include <iostream>
#include <fstream>
using namespace std;

string readSignalFile(const string& filename)
{
    fstream signalFile;
    string sum_signal;
    signalFile.open(filename);
    if (!signalFile.is_open()) throw runtime_error("Failed to open the signal file");

    string line;
    while ( getline (signalFile, line) )
    {
        sum_signal+= line; //possibly add /n
    }
    signalFile.close();

    cout << "Imported lines:";
    cout << sum_signal;

    return sum_signal;
}

void getSatelliteBits(const string& filename)
{
    string sum_signal = readSignalFile(filename);

}


int main(const int argc, char* argv[])
    // argc: Number of command-line arguments
    // argv: Array of command-line arguments
{
    if (argc!=2) std::cout << "Error, Arguments must be exactly one";
    const string filename = argv[1];
    getSatelliteBits(filename);

    // format
    // print

    return 0;
}
