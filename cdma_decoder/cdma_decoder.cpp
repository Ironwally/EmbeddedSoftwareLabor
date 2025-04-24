//
// Created by blackrat on 02.04.25.
//
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <chrono> // Für die Laufzeitmessung
using namespace std;

// Liest die Signaldatei ein und gibt den Inhalt als String zurück
auto readSignalFile(const string &filename) -> string
{
    ifstream signalFile(filename, ios::binary | ios::ate); // Datei im Binärmodus öffnen und ans Ende springen

    if (!signalFile)
    {
        throw runtime_error("Failed to open the signal file");
    }

    auto fileSize = signalFile.tellg(); // Dateigröße ermitteln
    signalFile.seekg(0, ios::beg);      // Zurück zum Anfang der Datei
    // Gesamte Datei in einen Rutsch einlesen
    string sum_signal;
    sum_signal.reserve(static_cast<size_t>(fileSize));                                       // Speicher reservieren, um unnötige Allokationen zu vermeiden
    sum_signal.assign((istreambuf_iterator<char>(signalFile)), istreambuf_iterator<char>()); // Dateiinhalt in den String einlesen
    signalFile.close();
    cout << "\nImported lines: " << sum_signal << '\n';

    return sum_signal;
}

// Gibt die Ergebnisse für einen Satelliten aus
auto printData(const int satellite_id, const int sent_bit, const int delta) -> void
{
    cout << "\nSatellite " << satellite_id << " has sent bit " << sent_bit << " (delta = " << delta << ")";
}

// Führt die Korrelation durch und erstellt die Ausgabe für printData
auto createPrintData(const string &sum_signal_str, const int (&chip_sequences)[24][1023]) -> void
{
    auto const start = chrono::high_resolution_clock::now(); // Startzeit für die Laufzeitmessung
    int sum_signal[1023];
    {
        // Signalwerte aus dem String extrahieren und in ein Array speichern
        // Der Block ist in geschweifte Klammern eingefasst, um den Gültigkeitsbereich
        // der Variablen `iss`, `val` und `idx` auf diesen Abschnitt zu beschränken.
        // Dadurch wird Speicher freigegeben, sobald die Variablen nicht mehr benötigt werden.
        istringstream iss(sum_signal_str);
        int val, idx = 0;

        while (iss >> val && idx < 1023)
        {
            sum_signal[idx++] = val;
        }
    }

    // Erweitertes Array erstellen, um Modulo-Operationen zu vermeiden
    int sum_signal_ext[2046];

    for (int i = 0; i < 1023; i++)
    {
        sum_signal_ext[i] = sum_signal[i];
        sum_signal_ext[i + 1023] = sum_signal[i];
    }

    constexpr short satellite_amount = 24;
    float treshold = 823.0f;

    for (int sat = 0; sat < satellite_amount; ++sat)
    {
        const int (&chip)[1023] = chip_sequences[sat]; // Chip-Sequenz des aktuellen Satelliten
        int max_corr = 0;
        int delta = 0;
        int chip_mapped[1023];

        for (int i = 0; i < 1023; ++i)
        {
            chip_mapped[i] = (chip[i] == 1) ? 1 : -1; // Mapping der Chip-Sequenz auf +1 und -1
        }

        // Korrelation mit Schleifenentfaltung
        for (int d = 0; d < 1023; ++d)
        {
            int correlation = 0;
            int base_idx = d;
            int i = 0;

            // Schleifenentfaltung in Schritten von 4 indem 4 Werte gleichzeitig verarbeitet werden
            for (; i + 3 < 1023; i += 4)
            {
                correlation += sum_signal_ext[base_idx + i] * chip_mapped[i];
                correlation += sum_signal_ext[base_idx + i + 1] * chip_mapped[i + 1];
                correlation += sum_signal_ext[base_idx + i + 2] * chip_mapped[i + 2];
                correlation += sum_signal_ext[base_idx + i + 3] * chip_mapped[i + 3];
            }

            // Restliche Werte berechnen, die nicht in der Schleifenentfaltung enthalten sind
            for (; i < 1023; ++i)
            {
                correlation += sum_signal_ext[base_idx + i] * chip_mapped[i];
            }

            // Maximale Korrelation und zugehöriges Delta aktualisieren
            if (abs(correlation) > abs(max_corr))
            {
                max_corr = correlation;
                delta = d;
            }
        }

        // Ergebnisse ausgeben, falls die Korrelation den Schwellenwert überschreitet
        if (abs(max_corr) >= treshold)
        {
            const int sent_bit = (max_corr > 0) ? 1 : 0;
            printData(sat + 1, sent_bit, delta);
        }
    }

    auto const end = chrono::high_resolution_clock::now(); // Endzeit für die Laufzeitmessung
    cout << "\n\ncreatePrintData executed in " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << " ms" << endl;
}

// Berechnet das nächste Bit der Chip-Sequenz basierend auf den Registerwerten
auto getSequenceBit(const int variable_input[10], const int fixed_input[10], const int register_sum[2]) -> int
{
    return fixed_input[9] ^ (variable_input[register_sum[0] - 1] ^ variable_input[register_sum[1] - 1]);
}

// Aktualisiert die Registerwerte für die nächste Iteration
auto updateSequences(int (&variable_input)[10], int (&fixed_input)[10]) -> void
{
    // Neues Bit für die variable Sequenz berechnen
    int new_variable_bit = variable_input[1] ^ variable_input[2] ^ variable_input[5] ^ variable_input[7] ^ variable_input[8] ^ variable_input[9];
    // Registerwerte verschieben
    memmove(&variable_input[1], &variable_input[0], 9 * sizeof(int));
    variable_input[0] = new_variable_bit;
    // Neues Bit für die feste Sequenz berechnen
    int new_fixed_bit = fixed_input[2] ^ fixed_input[9];
    memmove(&fixed_input[1], &fixed_input[0], 9 * sizeof(int));
    fixed_input[0] = new_fixed_bit;
}

// Generiert die Gold-Codes für alle Satelliten
auto goldCodeGenerator(const int (&satellite_register_sums)[24][2], int chip_sequence[24][1023]) -> void
{
    auto const start = chrono::high_resolution_clock::now(); // Startzeit für die Laufzeitmessung

    for (int sat = 0; sat < 24; ++sat)
    {
        const auto &register_sum = satellite_register_sums[sat]; // Register-Summen für den aktuellen Satelliten
        int variable_input[10] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}; // Variable Eingabesequenz
        int fixed_input[10] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1};    // Feste Eingabesequenz

        // Gold-Code für den aktuellen Satelliten generieren
        for (int i = 0; i < 1023; i++)
        {
            chip_sequence[sat][i] = getSequenceBit(variable_input, fixed_input, register_sum);
            updateSequences(variable_input, fixed_input);
        }
    }

    auto const end = chrono::high_resolution_clock::now(); // Endzeit für die Laufzeitmessung
    cout << "\ngoldCodeGenerator executed in " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << " ms" << endl;
}

// Hauptprogramm
// argc: Anzahl der Argumente in der Kommandozeile
// argv: Array der Argumente in der Kommandozeile
auto main(const int argc, char *argv[]) -> int
{
    auto const start = chrono::high_resolution_clock::now(); // Startzeit für die Laufzeitmessung
    // Beschleunigt Standard-I/O indem die Synchronisation mit C-Standard-I/O deaktiviert wird
    // und die Eingabe-/Ausgabepufferung deaktiviert wird
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (argc != 2)
    {
        std::cerr << "Error, Arguments must be exactly one";

        return 1;
    }

    const string filename = argv[1];
    cout << "Filename: " << filename;
    const string sum_signal = readSignalFile(filename);
    // Register-Summen für die Satelliten
    const int satellite_register_sums[24][2] = {{2, 6}, {3, 7}, {4, 8}, {5, 9}, {1, 9}, {2, 10}, {1, 8}, {2, 9}, {3, 10}, {2, 3}, {3, 4}, {5, 6}, {6, 7}, {7, 8}, {8, 9}, {9, 10}, {1, 4}, {2, 5}, {3, 6}, {4, 7}, {5, 8}, {6, 9}, {1, 3}, {4, 6}};
    int chip_sequences[24][1023]; // Speicher für die generierten Chip-Sequenzen
    goldCodeGenerator(satellite_register_sums, chip_sequences);
    createPrintData(sum_signal, chip_sequences);
    auto const end = chrono::high_resolution_clock::now(); // Endzeit für die Laufzeitmessung
    cout << "\nmain executed in " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << " ms" << endl;

    return 0;
}