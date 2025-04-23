//
// Created by blackrat on 02.04.25.
//
#include <iostream>
#include <fstream>
#include <climits>
#include <sstream>
#include <chrono> // Laufzeitmessung

using namespace std;

auto readSignalFile(const string &filename) -> string
{
    // Einlesen des Summensignals aus der Datei
    // und Umwandeln in einen Vektor von Integern
    // (Die Datei wird hier geöffnet)
    // Die Datei muss im Format 0/1 sein, also nur 0 und 1 enthalten.
    // Zeilenweise wird die Datei eingelesen und in den Vektor geschrieben.
    // Am Ende wird der Vektor zurückgegeben.

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

auto printData(const int satellite_id, const int sent_bit, const int delta) -> void
{
    // Ausgabe der Satelliten mti ihrem gesendeten Bit
    // und dem Delta (Position im Summensignal,
    // an denen die Chipsequenz des betreffenden Satelliten beginnt)
    cout << "\nSatellite " << satellite_id << " has sent bit " << sent_bit << " (delta = " << delta << ")";
}

auto createPrintData(const string &sum_signal_str, const int (&chip_sequences)[24][1023]) -> void
{
    auto const start = chrono::high_resolution_clock::now(); // Start timer
    // Summensignal von String zu Integer Vektor umwandeln
    int sum_signal[1023];
    istringstream iss(sum_signal_str); // Stream aus dem Eingabe String
    int val;
    
    int i=0;
    while (iss >> val) // Integer aus String herausholen
    {
        sum_signal[i] = val;
        i++;
    }

    const unsigned long signal_length = sizeof(sum_signal) / sizeof(sum_signal[0]);
    constexpr short satellite_amount = 24;
    float treshold = 823;

    // Für jeden Satelliten die (Kreuz)Korrelation berechnen.
    // Also Bit Erkennung
    // Schieben Chipsequenz über Summensignal und rechnen an jeder Position obs passt.
    // Also Summe der Produkte → Hohe Werte = hohe Ähnlichkeit
    for (int sat = 0; sat < satellite_amount; ++sat)
    {
        const int (&chip)[1023] = chip_sequences[sat]; // Chipfolge für Satellit
        int max_corr = 0;                              // Größte gefundene Korrelation
        int delta = 0;                                 // Position (Offset), an der Korrelation maximal war. (an der die Chipsequenz beginnt)
        int chip_mapped[signal_length];        // Chip 0/1 → -1/1 für Korrelation

        for (int i = 0; i < signal_length; ++i)
        {
            chip_mapped[i] = chip[i] == 1 ? 1 : -1;
        }

        for (int d = 0; d <= signal_length; ++d)
        {
            int correlation = 0;

            // Korrelation zwischen Chips und Signal Ausschnitt berechnen
            for (int i = 0; i < signal_length; ++i)
            {
                correlation += sum_signal[(d + i) % 1023] * chip_mapped[i]; // (Skalarprodukt)
            }

            // Maximum und Position speichern.
            // Also mit Betrag vergleichen (-1 und +1 beide relevant)
            if (abs(correlation) > abs(max_corr))
            {
                max_corr = correlation;
                delta = d;
            }
        }

        if (abs(max_corr) >= treshold)
        {
            // Aus Vorzeichen der höchsten Korrelation das gesendete Bit ableiten.
            // Also Positiv → Bit = 1, Negativ → Bit = 0
            const int sent_bit = max_corr > 0 ? 1 : 0;
            // Ausgabe der Satellitennummer mit dem gesendeten Bit und Delta Offset
            printData(sat + 1, sent_bit, delta);
        }
    }

    auto const end = chrono::high_resolution_clock::now(); // End timer
    cout << "\n\ncreatePrintData executed in "
         << chrono::duration_cast<chrono::milliseconds>(end - start).count()
         << " ms" << endl;
}

auto getSequenceBit(const int variable_input[10], const int fixed_input[10],
                    const int register_sum[2]) -> int
{
    return fixed_input[9] ^ (variable_input[register_sum[0] - 1] ^ variable_input[register_sum[1] - 1]);
}

auto updateSequences(int (&variable_input)[10], int (&fixed_input)[10]) -> void
{
    int new_variable_bit = variable_input[1] ^ variable_input[2] ^ variable_input[5] ^
                           variable_input[7] ^ variable_input[8] ^ variable_input[9];
    for (int i = 9; i > 0; --i)
    {
        variable_input[i] = variable_input[i - 1];
    }
    variable_input[0] = new_variable_bit;

    int new_fixed_bit = fixed_input[2] ^ fixed_input[9];
    for (int i = 9; i > 0; --i)
    {
        fixed_input[i] = fixed_input[i - 1];
    }
    fixed_input[0] = new_fixed_bit;
}

auto goldCodeGenerator(const int (&satellite_register_sums)[24][2], int chip_sequence[24][1023]) -> void
{
    auto const start = chrono::high_resolution_clock::now(); // Start timer
    int chip_sequences[24][1023];                      // end-output

    for (int sat = 0; sat < 24; ++sat)
    {
        const auto &register_sum = satellite_register_sums[sat];
        int variable_input[10] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}; // variable input sequence
        int fixed_input[10] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1};    // fixed input sequence

        for (int i = 0; i < 1023; i++)
        {
            chip_sequence[sat][i] = getSequenceBit(variable_input, fixed_input, register_sum);
            updateSequences(variable_input, fixed_input);
        }
    }

    auto const end = chrono::high_resolution_clock::now(); // End timer
    cout << "\ngoldCodeGenerator executed in "
         << chrono::duration_cast<chrono::milliseconds>(end - start).count()
         << " ms" << endl;
}

auto main(const int argc, char *argv[]) -> int
// argc: Number of command-line arguments
// argv: Array of command-line arguments
{
    auto const start = chrono::high_resolution_clock::now(); // Start timer

    if (argc != 2)
    {
        std::cerr << "Error, Arguments must be exactly one";

        return 1;
    }

    const string filename = argv[1];
    cout << "Filename: " << filename;
    const string sum_signal = readSignalFile(filename);
    const int satellite_register_sums[24][2] = {
        {2, 6}, {3, 7}, {4, 8}, {5, 9}, {1, 9}, {2, 10}, {1, 8}, {2, 9}, {3, 10}, {2, 3}, {3, 4}, {5, 6}, {6, 7}, {7, 8}, {8, 9}, {9, 10}, {1, 4}, {2, 5}, {3, 6}, {4, 7}, {5, 8}, {6, 9}, {1, 3}, {4, 6}};

    int chip_sequences[24][1023];
    goldCodeGenerator(satellite_register_sums, chip_sequences);
    createPrintData(sum_signal, chip_sequences);
    auto const end = chrono::high_resolution_clock::now(); // End timer
    cout << "\nmain executed in "
         << chrono::duration_cast<chrono::milliseconds>(end - start).count()
         << " ms" << endl;

    return 0;
}