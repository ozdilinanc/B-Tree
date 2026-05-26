B+ TREE LIBRARY DATABASE MANAGEMENT SYSTEM
==========================================

Bu proje, C++ ile sifirdan gelistirilmis, yuksek performansli ve disk tabanli bir B+ Tree (B-Arti Agaci) veritabani yonetim motorudur. Geleneksel RAM ici arama algoritmalari yerine, verileri disk ofsetleri uzerinden indeksleyerek buyuk veri setleri (1M+ kayit) uzerinde logaritmik zaman karmasikliginda (O(log n)) okuma, yazma, guncelleme ve silme (CRUD) islemleri yapabilmektedir.


TEMEL OZELLIKLER
----------------

- Dinamik Veri Seti Secimi: Program baslatildiginda 10K, 100K veya 1M kayitlik farkli veri setleri arasindan interaktif olarak secim yapilabilmesini ve arka plan islemlerinin bu secime gore izole calismasini saglar.
- Coklu B+ Tree Indeksleme: Tek bir veri kaydi eklendiginde sistem; ID, Yazar, Kategori, Kitap Adi ve Yayin Yili olmak uzere 5 farkli B+ Agaci uzerinde eszamanli indeksleme yapar.
- Soft Delete (Mantiksal Silme): Veri silme islemleri sirasinda disk uzerinde buyuk veri kaydirmalari (shifting) yapmaktan kacinmak icin, silinen kaydin basina '*' isareti konularak veri mantiksal olarak silinir. Islem maliyeti O(1) suresine indirgenir.
- Append-Only Guncelleme: Guncellenen veriler, dosya butunlugunu korumak amaciyla MVCC (Multi-Version Concurrency Control) mantigina benzer sekilde dosyanin sonuna eklenir ve eski kayit mantiksal olarak silinir.
- Auto-Vacuum (Arka Plan Cop Toplayici): std::thread kullanilarak arka planda asenkron calisan bir bakim is parcacigi (worker thread) mevcuttur. Belirli araliklarla (orn. 120 saniyede bir) diskteki mantiksal olarak silinmis verileri temizleyerek yeni ve optimize edilmis bir dosya olusturur (Hard Delete).
- Thread Safety (Is Parcacigi Guvenligi): Dosya okuma ve Vacuum (temizlik) islemlerinin cakismasini (Race Condition) onlemek amaciyla std::mutex ve std::atomic kilit mekanizmalari kullanilmistir.


KURULUM VE DERLEME TALIMATLARI
------------------------------

Proje std::thread ve std::mutex gibi modern C++11 coklu is parcacigi (multithreading) kutuphanelerini kullandigi icin derleme asamasinda bazi sistemlerde POSIX thread (-pthread) bayraginin eklenmesi gerekebilir.

1. On Kosullar:
Sisteminizde bir C++ derleyicisi (GCC, Clang veya MSVC) kurulu olmalidir.
Proje dizininde kaynak kodlarla birlikte veri setlerinizin (10KBooks_dataset.txt vb.) bulundugundan emin olun.

2. Projeyi Derleme:

- Linux (Ubuntu, Debian, Fedora vb.) ve macOS Icin:
  Terminali proje dizininde acin ve asagidaki komutu calistirin:
  g++ main.cpp LibraryCRUD.cpp StringParser.cpp BPlusTree.cpp -o db_engine -pthread

- Windows Icin (MinGW / GCC):
  Komut satirini (CMD veya PowerShell) proje dizininde acin ve asagidaki komutu calistirin:
  g++ main.cpp LibraryCRUD.cpp StringParser.cpp BPlusTree.cpp -o db_engine.exe

3. Projeyi Calistirma:

- Linux ve macOS Icin:
  ./db_engine

- Windows Icin:
  db_engine.exe


KULLANIM REHBERI
----------------

Program calistirildiginda, oncelikle uzerinde calismak istediginiz veri setini secmeniz istenecektir. Secim yapildiktan sonra veriler diske okunacak ve B+ Agaclarina indekslenecektir. Indeksleme suresi sistem donanimina bagli olarak ekrana milisaniye cinsinden yansitilir. Yukleme tamamlandiktan sonra interaktif konsol menusu acilir:

- [1] Search Book (Menu): 5 farkli kritere gore (ID, Yazar, Kategori, Ad, Yil) arama yapmanizi saglar. Arama sureleri mikrosaniye cinsinden raporlanir.
- [2] Update Record: Belirtilen ID'ye sahip kaydi bulur ve yeni verileri girmenizi ister. Guncelleme islemi disk butunlugunu bozmadan guvenli bir sekilde yapilir.
- [3] Delete Book (Soft Delete): Hedeflenen ID'yi sistemden mantiksal olarak siler ve kullanicinin arama sonuclarindan aninda gizler.
- [4] Manual Vacuum (Hard Delete): Normalde arka planda otomatik calisan cop toplayiciyi (Vacuum) manuel olarak tetikler. Diskteki Soft Delete yapilmis tum verileri kalici olarak siler ve RAM'deki B+ Agaclarini sifirdan, en optimize haliyle yeniden insa eder.
- [0] Exit: Arka plan is parcaciklarina (Thread) durdurma sinyali gonderir, bellek sizintilarini (Memory Leak) onleyerek sistemi guvenli bir sekilde kapatir.


PROJE MIMARISI (DOSYA YAPISI)
-----------------------------

- main.cpp: Kullanici arayuzunu (CLI), veri seti secimini, multithreading yapilarini, Vacuum islemlerini ve performans olcumlerini barindiran ana kontrolcudur.
- LibraryCRUD.h / .cpp: Veritabani islemlerini (Insert, Search, Update, Delete) yoneten orta katmandir. B+ Agaclarinin somutlastirildigi alandir.
- StringParser.h / .cpp: Diskteki CSV formatli metinleri parse eden veri ayristirici moduldur.
- BPlusTree.h / .cpp: Disk ofsetlerini agac yapisi uzerinde tutan ve logaritmik arama imkani sunan cekirdek veri yapisi motorudur.
