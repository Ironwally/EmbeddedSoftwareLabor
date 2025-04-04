//
// Created by blackrat on 02.04.25.
//

#include "cdma_decoder.h"

#include <array>
#include <iostream>
#include <fstream>
#include <climits>
#include <vector>
#include <sstream>
using namespace std;

string readSignalFile(const string& filename)
{
    ifstream signalFile(filename); // Datei wird hier bereits geöffnet

    if (!signalFile)
    {
        throw runtime_error("Failed to open the signal file");
    }

    string sum_signal;
    string line;

    while (getline(signalFile, line))
    {
        sum_signal += line;
    }

    signalFile.close();
    cout << "\nImported lines: " << sum_signal << endl;

    return sum_signal;
}


int xor_i(const vector<int>& input)
{
    int result = 0;

    for (const int num : input)
    {
        result ^= num;
    }

    return result;
}

void getSatelliteBits(const int satellite_id, const int sent_bit, const int delta)
{
    // Ausgabe der Satelliten mti ihrem gesendeten Bit
    // und dem Delta (Position im Summensignal,
    // an denen die Chipsequenz des betreffenden Satelliten beginnt)
    cout << "\nSatellite " << satellite_id << " has sent bit " << sent_bit << " (delta = " << delta << ")";
}

void createPrintData(const string& sum_signal_str, const vector<vector<int>>& chip_sequences)
{
    // Summensignal von String zu Integer Vektor umwandeln
    vector<int> sum_signal;
    istringstream iss(sum_signal_str); // Stream aus dem Eingabe String
    int val;

    while (iss >> val) // Integer aus String herausholen
    {
        sum_signal.push_back(val); // Diese in Vektor einfügen
    }

    const unsigned long signal_length = sum_signal.size();

    // Für jeden Satelliten die (Kreuz)Korrelation berechnen
    // Also Bit Erkennung
    // Schieben Chipsequenz über Summensignal und rechnen an jeder Position obs passt
    // Also Summe der Produkte -> Hohe Werte = hohe Ähnlichkeit
    for (int sat = 0; sat < 24; ++sat)
    {
        const vector<int>& chip = chip_sequences[sat]; // Chipfolge für Satellit
        int max_corr = INT_MIN; // Größte gefundene Korrelation
        int delta = 0; // Position (Offset), an der Korrelation maximal war

        for (int d = 0; d <= signal_length - 1023; ++d) // TODO Delta immer null
        {
            int correlation = 0;

            // Korrelation zwischen Chips und Signal Ausschnitt berechnen
            for (int i = 0; i < 1023; ++i)
            {
                correlation += sum_signal[d + i] * chip[i];
            }

            // Maximum und Position speichern
            // Also mit Betrag vergleichen (-1 und +1 beide relevant)
            if (abs(correlation) > abs(max_corr))
            {
                max_corr = correlation;
                delta = d;
            }
        }

        // Aus Vorzeichen der höchsten Korrelation das gesendete Bit ableiten
        // Also Positiv -> Bit = 1, Negativ -> Bit = 0
        const int sent_bit = max_corr > 0 ? 1 : 0;
        // Ausgabe der Satellitennummer mit dem gesendeten Bit und Delta Offset
        getSatelliteBits(sat + 1, sent_bit, delta);
    }
}

int get_sequence_bit(const vector<int>& variable_input, const vector<int>& fixed_input, const vector<int>& register_sum)
{
    return fixed_input.back() ^ (variable_input[register_sum[0] - 1] ^ variable_input[register_sum[1] - 1]);
}

void update_sequences(vector<int>& variable_input, vector<int>& fixed_input)
{
    variable_input.insert(variable_input.begin(), xor_i({
                              variable_input[1], variable_input[2], variable_input[5],
                              variable_input[7], variable_input[8], variable_input[9]
                          }));
    variable_input.pop_back();
    fixed_input.insert(fixed_input.begin(), fixed_input[2] ^ fixed_input.back());
    fixed_input.pop_back();
}

vector<vector<int>> gold_code_generator(const vector<vector<int>>& satellite_register_sums)
{
    vector<vector<int>> chip_sequences; // end-output

    for (const auto& register_sum : satellite_register_sums)
    {
        vector<int> chip_sequence = {};
        vector<int> variable_input(10, 1); // variable input sequence
        vector<int> fixed_input(10, 1); // fixed input sequence

        for (int i = 0; i < 1023; i++)
        {
            chip_sequence.push_back(get_sequence_bit(variable_input, fixed_input, register_sum));
            // if not work emplace at front instead
            update_sequences(variable_input, fixed_input);
        }

        chip_sequences.emplace_back(chip_sequence);
    }

    return chip_sequences;
}


int main(const int argc, char* argv[])
// argc: Number of command-line arguments
// argv: Array of command-line arguments
{
    if (argc != 2)
    {
        std::cerr << "Error, Arguments must be exactly one";

        return 1;
    }

    const string filename = argv[1];
    cout << "Filename: " << filename;
    const string sum_signal = readSignalFile(filename);

    const vector<vector<int>> satellite_register_sums = {
        {2, 6}, {3, 7}, {4, 8}, {5, 9}, {1, 9}, {2, 10}, {1, 8}, {2, 9}, {3, 10}, {2, 3}, {3, 4}, {5, 6},
        {6, 7}, {7, 8}, {8, 9}, {9, 10}, {1, 4}, {2, 5}, {3, 6}, {4, 7}, {5, 8}, {6, 9}, {1, 3}, {4, 6}
    };

    const vector<vector<int>> chip_sequences = gold_code_generator(satellite_register_sums);
    createPrintData(sum_signal, chip_sequences);

    return 0;
}
