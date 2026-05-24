#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <windows.h>
#include <psapi.h>

#include "StringParser.h"
#include "LibraryCRUD.h"

using namespace std;
using namespace std::chrono;

// RAM Tüketimini MB cinsinden hesaplayan fonksiyon
double getMemoryUsageMB()
{
    PROCESS_MEMORY_COUNTERS_EX pmc;
    GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS *)&pmc, sizeof(pmc));
    return (double)pmc.WorkingSetSize / (1024 * 1024);
}

int main()
{
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);

    double startRAM = getMemoryUsageMB();
    LibraryCRUD db;

    fstream file("books_dataset.txt", ios::in | ios::out | ios::binary);
    if (!file.is_open())
        return 1;

    cout << "[SISTEM] 1 Milyon kayit indeksleniyor..." << endl;
    string satir;
    int satirSayisi = 0;

    // ==========================================
    // 1. İNDEKSLEME SÜRESİ ÖLÇÜMÜ BAŞLANGICI
    // ==========================================
    auto startIdxTime = high_resolution_clock::now();

    while (true)
    {
        streampos currentOffset = file.tellg();
        if (!getline(file, satir))
            break;

        Book parsedBook = parseLine(satir);
        if (parsedBook.isValid)
        {
            db.insertBook(parsedBook, currentOffset);
            satirSayisi++;
        }
    }

    auto endIdxTime = high_resolution_clock::now();
    double endIdxRAM = getMemoryUsageMB();
    // ==========================================

    // ==========================================
    // 2. ARAMA (SEARCH) SÜRESİ ÖLÇÜMÜ
    // ==========================================
    auto startSearch = high_resolution_clock::now();
    // Performans testi için rastgele bir yazarı arıyoruz
    vector<streampos> authorOffsets = db.searchAuthor("Stephen Crane");
    auto endSearch = high_resolution_clock::now();
    // ==========================================

    // ==========================================
    // 3. SİLME VE GÜNCELLEME İŞLEMİ (CRUD)
    // ==========================================
    cout << "\n[ISLEM] 10 ID'li kitap siliniyor ve 112 ID'li kitap guncelleniyor..." << endl;
    db.deleteBook(file, "10");
    Book guncelKitap = {"112", "The Red Badge of Courage (REMASTERED)", "Stephen Crane", "Classic", "2026", true};
    db.updateBook(file, "112", guncelKitap);

    // ==========================================
    // 4. DOĞRULAMA (VERIFICATION) ÇIKTILARI
    // ==========================================
    cout << "\n--- TEST 1: SILINEN KAYDI ARAMA ---" << endl;
    vector<streampos> deletedOffsets = db.searchId("10");
    int gecerliKayit10 = 0;
    for (streampos off : deletedOffsets)
    {
        file.clear();
        file.seekg(off);
        getline(file, satir);
        if (satir[0] == '*')
        {
            cout << "   -> [SISTEM] Kayit bulundu ama silinmis (*) isaretli. (Diskteki: " << satir << ")" << endl;
        }
        else
        {
            gecerliKayit10++;
        }
    }
    if (gecerliKayit10 == 0)
        cout << "   -> Kullaniciya gosterilecek sonuc: BULUNAMADI (0 Kayit)" << endl;

    cout << "\n--- TEST 2: GUNCELLENEN KAYDI ARAMA ---" << endl;
    vector<streampos> updatedOffsets = db.searchId("112");
    for (streampos off : updatedOffsets)
    {
        file.clear();
        file.seekg(off);
        getline(file, satir);
        if (satir[0] == '*')
        {
            cout << "   -> [COPE ATILAN ESKI KAYIT] " << satir << endl;
        }
        else
        {
            cout << "   -> [GUNCEL YENI KAYIT] " << satir << endl;
        }
    }
    file.close();

    // ==========================================
    // 5. PERFORMANS HESAPLAMALARI VE ÇIKTISI
    // ==========================================
    auto sureIdx = duration_cast<milliseconds>(endIdxTime - startIdxTime);
    auto sureSearch = duration_cast<microseconds>(endSearch - startSearch);

    cout << "\n==========================================" << endl;
    cout << "          PERFORMANS RAPORU               " << endl;
    cout << "==========================================" << endl;
    cout << "Okunan Toplam Kayit   : " << satirSayisi << endl;
    cout << "Indeksleme Suresi     : " << sureIdx.count() << " milisaniye" << endl;
    cout << "Agaclarin RAM Tuketimi: " << (endIdxRAM - startRAM) << " MB" << endl;
    cout << "Arama Suresi (Read)   : " << sureSearch.count() << " mikrosaniye" << endl;
    cout << "Bulunan Yazar Kaydi   : " << authorOffsets.size() << " adet" << endl;
    cout << "==========================================\n"
         << endl;

    return 0;
}