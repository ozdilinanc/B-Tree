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
// This function skips records marked with "Soft Delete" (*),
// writes clean records to a new file, deletes the old file, and resets indexes.
// ==============================================================================
void vacuumDatabase(LibraryCRUD **dbPtr)
{
    ifstream inFile("books_dataset.txt", ios::binary);
    ofstream outFile("books_dataset_temp.txt", ios::binary);

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

        // Index to new trees instantly (Very fast!)
        Book parsedBook = parseLine(line);
        if (parsedBook.isValid)
        {
            newDb->insertBook(parsedBook, currentOffset);
        }
    }

    inFile.close();
    outFile.close();

    // 1. Delete the old file from disk (Hard Delete)
    remove("books_dataset.txt");
    // 2. Rename the new clean file to the original name
    rename("books_dataset_temp.txt", "books_dataset.txt");

    // 3. Delete old B+ Trees from memory, attach the new one to the system
    delete *dbPtr;
    *dbPtr = newDb;

    cout << "\n[SYSTEM INFO] Vacuum completed. " << cleanedRecords << " garbage records were completely deleted from disk.\n";
}

// ==============================================================================
// BACKGROUND THREAD WORKER
// ==============================================================================
void autoVacuumWorker(LibraryCRUD **dbPtr, int intervalSeconds)
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
        cout << "\n\n*** [AUTO MAINTENANCE] Background Hard-Delete (Vacuum) process started... ***\n";
        vacuumDatabase(dbPtr);
        cout << ">>> Please re-enter your choice: ";
    }
}

// ==============================================================================
// INITIAL LOAD FUNCTION
// ==============================================================================
void initialLoad(LibraryCRUD *db)
{
    ifstream file("books_dataset.txt", ios::binary);
    if (!file.is_open())
        return;

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
    LibraryCRUD *db = new LibraryCRUD();

    cout << "==========================================\n";
    cout << "LIBRARY B+ TREE SYSTEM STARTING...\n";
    cout << "Loading data into memory, please wait...\n";

    auto startIdx = high_resolution_clock::now();
    initialLoad(db);
    auto endIdx = high_resolution_clock::now();
    cout << "Indexing Time: " << duration_cast<milliseconds>(endIdx - startIdx).count() << " ms\n";
    cout << "==========================================\n";

    // Start background Vacuum worker (e.g., runs every 120 seconds)
    thread vacuumThread(autoVacuumWorker, &db, 120);

    string choice;
    while (true)
    {

        cout << "Select process:";
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
                vector<streampos> offsets = db->searchId(id);
                if (!offsets.empty())
                {
                    fstream file("books_dataset.txt", ios::in | ios::binary);
                    file.seekg(offsets[0]);
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
            else if (searchChoice == "1")
            {
                cout << "  Author Name: " << flush;
                while (getline(cin, keyword) && keyword.empty())
                {
                }
                startSearch = high_resolution_clock::now();
                offsets = db->searchAuthor(keyword);
                endSearch = high_resolution_clock::now();
            }
            else if (searchChoice == "2")
            {
                cout << "  Category Name: " << flush;
                while (getline(cin, keyword) && keyword.empty())
                {
                }
                startSearch = high_resolution_clock::now();
                offsets = db->searchCategory(keyword);
                endSearch = high_resolution_clock::now();
            }
            else if (searchChoice == "3")
            {
                cout << "  Book Name: " << flush;
                while (getline(cin, keyword) && keyword.empty())
                {
                }
                startSearch = high_resolution_clock::now();
                offsets = db->searchBookName(keyword);
                endSearch = high_resolution_clock::now();
            }
            else if (searchChoice == "4")
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

            // Eğer geçerli bir arama yapıldıysa sonuçları bastır (Kod tekrarından kurtulduk!)
            if (validSearch)
            {
                cout << "\nFound Count: " << offsets.size() << " | Time: " << duration_cast<microseconds>(endSearch - startSearch).count() << " microseconds\n";

                int limit = 0;
                ifstream file("books_dataset.txt", ios::binary);
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
                // HATA 1 ÇÖZÜMÜ: ios::out eklendi ki Update (Yazma) yapılabilsin!
                fstream file("books_dataset.txt", ios::in | ios::out | ios::binary);

                file.seekg(offsets[0]);
                string line;
                getline(file, line);
                if (line[0] == '*')
                    cout << "This record has been deleted.\n";
                else
                {
                    Book tempBook;

                    // HATA 2 ÇÖZÜMÜ: Orijinal ID'yi ve Geçerlilik Durumunu yeni objeye aktar
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

                    // Artık dosya hem okumaya hem yazmaya açık, güncelleme başarıyla diske yazılacak
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
            fstream file("books_dataset.txt", ios::in | ios::out | ios::binary);
            if (db->deleteBook(file, id))
                cout << "Book deleted successfully (Soft Delete).\n";
            else
                cout << "Book not found!\n";
        }
        else if (choice == "4")
        {
            // User manually initiates Hard Delete
            lock_guard<mutex> lock(dbMutex);
            cout << "Manual Hard-Delete started...\n";
            vacuumDatabase(&db);
        }
        else
        {
            cout << "Invalid choice!\n";
        }
    }

    // Wait for background worker and clean up memory on exit
    vacuumThread.join();
    delete db;
    cout << "System safely shut down.\n";
    return 0;
}