#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <mutex>
#include <atomic>
#include <cstdio> // For std::remove and std::rename

#include "StringParser.h"
#include "LibraryCRUD.h"

using namespace std;
using namespace std::chrono;

// --- GLOBAL VARIABLES (For Thread Management) ---
mutex dbMutex;                // Prevents simultaneous reading and vacuuming
atomic<bool> isRunning(true); // Flag to stop the background process

// ==============================================================================
// DATABASE MAINTENANCE (VACUUM / HARD DELETE) FUNCTION
// ==============================================================================
void vacuumDatabase(LibraryCRUD **dbPtr, const string &filename)
{
    ifstream inFile(filename, ios::binary);

    // Gecici dosya ismini dinamik olarak belirliyoruz
    string tempFilename = "temp_" + filename;
    ofstream outFile(tempFilename, ios::binary);

    if (!inFile.is_open() || !outFile.is_open())
        return;

    // Creating fresh new trees
    LibraryCRUD *newDb = new LibraryCRUD();
    string line;
    int cleanedRecords = 0;

    while (true)
    {
        streampos currentOffset = outFile.tellp(); // Byte address in the new file
        if (!getline(inFile, line))
            break;

        // Skip without writing if the line is empty or starts with '*' (Deleted)
        if (line.empty() || line[0] == '*')
        {
            cleanedRecords++;
            continue;
        }

        // Write valid record to the new file (adding \n)
        outFile << line << "\n";

        // Index to new trees instantly
        Book parsedBook = parseLine(line);
        if (parsedBook.isValid)
        {
            newDb->insertBook(parsedBook, currentOffset);
        }
    }

    inFile.close();
    outFile.close();

    // 1. Delete the old file from disk (Hard Delete)
    remove(filename.c_str());
    // 2. Rename the new clean file to the original name
    rename(tempFilename.c_str(), filename.c_str());

    // 3. Delete old B+ Trees from memory, attach the new one to the system
    delete *dbPtr;
    *dbPtr = newDb;

    cout << "\n[SYSTEM INFO] Vacuum completed. " << cleanedRecords << " garbage records were completely deleted from disk.\n";
}

// ==============================================================================
// BACKGROUND THREAD WORKER
// ==============================================================================
void autoVacuumWorker(LibraryCRUD **dbPtr, int intervalSeconds, string filename)
{
    while (isRunning)
    {
        // Sleep for the specified seconds (frequently checking the stop flag)
        for (int i = 0; i < intervalSeconds; ++i)
        {
            if (!isRunning)
                return;
            this_thread::sleep_for(chrono::seconds(1));
        }

        // When the time is up, lock the system and perform cleanup
        lock_guard<mutex> lock(dbMutex);
        cout << "\n\n*** [AUTO MAINTENANCE] Background Hard-Delete (Vacuum) process started on " << filename << "... ***\n";
        vacuumDatabase(dbPtr, filename);
        cout << ">>> Please re-enter your choice: ";
    }
}

// ==============================================================================
// INITIAL LOAD FUNCTION
// ==============================================================================
void initialLoad(LibraryCRUD *db, const string &filename)
{
    ifstream file(filename, ios::binary);
    if (!file.is_open())
    {
        cout << "\n[KRITIK HATA] " << filename << " dosyasi bulunamadi!\n";
        cout << "Lutfen dosyanin programla ayni klasorde oldugundan emin olun.\n\n";
        return;
    }

    int processedRecords = 0;
    string line;

    while (true)
    {
        streampos currentOffset = file.tellg();
        if (!getline(file, line))
            break;

        if (!line.empty() && line[0] != '*')
        {
            Book parsedBook = parseLine(line);
            if (parsedBook.isValid)
                db->insertBook(parsedBook, currentOffset);
        }
        processedRecords++;

        // Used endl to print to screen immediately
        if (processedRecords % 100000 == 0)
        {
            cout << " -> " << processedRecords << " records added to the tree..." << endl;
        }
    }
    file.close();
}

int main()
{
    cout << "==========================================\n";
    cout << "    LIBRARY B+ TREE SYSTEM STARTING...\n";
    cout << "==========================================\n";

    // --- DATASET SEÇİM MENÜSÜ ---
    string selectedFilename = "";
    string dsChoice;

    while (true)
    {
        cout << "Select Dataset to Load:\n";
        cout << "[1] 10K Books Dataset\n";
        cout << "[2] 100K Books Dataset\n";
        cout << "[3] 1M Books Dataset\n";
        cout << "Choice: ";
        cin >> dsChoice;

        if (dsChoice == "1")
        {
            selectedFilename = "10KBooks_dataset.txt";
            break;
        }
        else if (dsChoice == "2")
        {
            selectedFilename = "100KBooks_dataset.txt";
            break;
        }
        else if (dsChoice == "3")
        {
            selectedFilename = "1MBooks_dataset.txt";
            break;
        }
        else
        {
            cout << "Invalid choice! Please enter 1, 2, or 3.\n\n";
        }
    }

    cout << "\n[SYSTEM] Selected dataset: " << selectedFilename << "\n";
    cout << "Loading data into memory, please wait...\n";

    LibraryCRUD *db = new LibraryCRUD();

    auto startIdx = high_resolution_clock::now();
    initialLoad(db, selectedFilename); // Dinamik dosya ismini fonksiyona gonderiyoruz
    auto endIdx = high_resolution_clock::now();

    cout << "Indexing Time: " << duration_cast<milliseconds>(endIdx - startIdx).count() << " ms\n";
    cout << "==========================================\n";

    // Start background Vacuum worker with the selected filename
    thread vacuumThread(autoVacuumWorker, &db, 120, selectedFilename);

    string choice;
    while (true)
    {
        cout << "\nSelect process:";
        cout << "\n[1] Search Book (Menu)\n[2] Update Record\n[3] Delete Book (Soft Delete)\n[4] Manual Vacuum (Hard Delete)\n[0] Exit\n";
        cout << "Choice: " << flush;
        cin >> choice;

        if (choice == "0")
        {
            isRunning = false; // Stop the background thread
            break;
        }
        else if (choice == "1")
        {
            cout << "\n  --- SEARCH MENU ---" << endl;
            cout << "  [1] Search by ID" << endl;
            cout << "  [2] Search by Author" << endl;
            cout << "  [3] Search by Category" << endl;
            cout << "  [4] Search by Book Name" << endl;
            cout << "  [5] Search by Publication Year" << endl;
            cout << "  Select search type: " << flush;

            string searchChoice;
            while (getline(cin, searchChoice) && searchChoice.empty())
            {
            }

            vector<streampos> offsets;
            string keyword;
            auto startSearch = high_resolution_clock::now();
            auto endSearch = high_resolution_clock::now();
            bool validSearch = true;

            if (searchChoice == "1")
            {
                string id;
                cout << "Book ID: ";
                cin >> id;

                lock_guard<mutex> lock(dbMutex);
                vector<streampos> idOffsets = db->searchId(id);
                if (!idOffsets.empty())
                {
                    fstream file(selectedFilename, ios::in | ios::binary);
                    file.seekg(idOffsets[0]);
                    string line;
                    getline(file, line);
                    if (line[0] != '*')
                        cout << "Found: " << line << "\n";
                    else
                        cout << "This record has been deleted.\n";
                }
                else
                {
                    cout << "Not found.\n";
                }
            }
            else if (searchChoice == "2")
            {
                cout << "  Author Name: " << flush;
                while (getline(cin, keyword) && keyword.empty())
                {
                }
                startSearch = high_resolution_clock::now();
                offsets = db->searchAuthor(keyword);
                endSearch = high_resolution_clock::now();
            }
            else if (searchChoice == "3")
            {
                cout << "  Category Name: " << flush;
                while (getline(cin, keyword) && keyword.empty())
                {
                }
                startSearch = high_resolution_clock::now();
                offsets = db->searchCategory(keyword);
                endSearch = high_resolution_clock::now();
            }
            else if (searchChoice == "4")
            {
                cout << "  Book Name: " << flush;
                while (getline(cin, keyword) && keyword.empty())
                {
                }
                startSearch = high_resolution_clock::now();
                offsets = db->searchBookName(keyword);
                endSearch = high_resolution_clock::now();
            }
            else if (searchChoice == "5")
            {
                cout << "  Publication Year (e.g., 1998): " << flush;
                while (getline(cin, keyword) && keyword.empty())
                {
                }
                startSearch = high_resolution_clock::now();
                offsets = db->searchDate(keyword);
                endSearch = high_resolution_clock::now();
            }
            else
            {
                cout << "  Invalid search type selected!\n";
                validSearch = false;
            }

            if (validSearch && searchChoice != "1")
            {
                cout << "\nFound Count: " << offsets.size() << " | Time: " << duration_cast<microseconds>(endSearch - startSearch).count() << " microseconds\n";

                int limit = 0;
                ifstream file(selectedFilename, ios::binary);
                for (streampos off : offsets)
                {
                    if (limit >= 5)
                    {
                        cout << "... and " << offsets.size() - 5 << " more records.\n";
                        break;
                    }
                    file.seekg(off);
                    string line;
                    getline(file, line);
                    if (line[0] != '*')
                    {
                        cout << " -> " << line << "\n";
                        limit++;
                    }
                }
                file.close();
            }
        }
        else if (choice == "2")
        {
            string id;
            cout << "Book ID: ";
            cin >> id;

            lock_guard<mutex> lock(dbMutex);
            vector<streampos> offsets = db->searchId(id);
            if (!offsets.empty())
            {
                fstream file(selectedFilename, ios::in | ios::out | ios::binary);

                file.seekg(offsets[0]);
                string line;
                getline(file, line);
                if (line[0] == '*')
                    cout << "This record has been deleted.\n";
                else
                {
                    Book tempBook;

                    tempBook.id = id;
                    tempBook.isValid = true;

                    cout << " Book Title: ";
                    while (getline(cin, tempBook.title) && tempBook.title.empty())
                    {
                    }

                    cout << " Book Author: ";
                    while (getline(cin, tempBook.author) && tempBook.author.empty())
                    {
                    }

                    cout << " Book Category: ";
                    while (getline(cin, tempBook.category) && tempBook.category.empty())
                    {
                    }

                    cout << " Book Publish Date: ";
                    while (getline(cin, tempBook.date) && tempBook.date.empty())
                    {
                    }

                    db->updateBook(file, id, tempBook);
                    cout << "Record successfully updated!\n";
                }
            }
            else
            {
                cout << "Not found.\n";
            }
        }
        else if (choice == "3")
        {
            string id;
            cout << "Book ID to Delete: ";
            cin >> id;

            lock_guard<mutex> lock(dbMutex);
            fstream file(selectedFilename, ios::in | ios::out | ios::binary);
            if (db->deleteBook(file, id))
                cout << "Book deleted successfully (Soft Delete).\n";
            else
                cout << "Book not found!\n";
        }
        else if (choice == "4")
        {
            lock_guard<mutex> lock(dbMutex);
            cout << "Manual Hard-Delete started...\n";
            vacuumDatabase(&db, selectedFilename); // Dinamik isim Vacuum'a da gonderildi
        }
        else
        {
            cout << "Invalid choice!\n";
        }
    }

    vacuumThread.join();
    delete db;
    cout << "System safely shut down.\n";
    return 0;
}