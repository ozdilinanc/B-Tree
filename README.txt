B+ TREE LIBRARY DATABASE MANAGEMENT SYSTEM
==========================================

Bu proje, C++ ile sifirdan gelistirilmis, yuksek performansli ve disk tabanli bir B+ Tree (B-Arti Agaci) veritabani yonetim motorudur. Geleneksel RAM ici arama algoritmalari yerine, verileri disk ofsetleri uzerinden indeksleyerek buyuk veri setleri (1M+ kayit) uzerinde logaritmik zaman karmasikliginda (O(log n)) okuma, yazma, guncelleme ve silme (CRUD) islemleri yapabilmektedir.


TEMEL OZELLIKLER
----------------

- Dinamik Veri Seti Secimi: Program baslatildiginda 10K, 100K veya 1M kayitlik farkli veri setleri arasindan interaktif olarak secim yapilabilmesini ve arka plan islemlerinin bu secime gore izole calismasini saglar.
- Cross-Platform RAM Profiling: Windows (<windows.h>) ve Linux (<sys/resource.h>) sistemleri icin derleyici makrolari kullanilarak gercek zamanli, capraz platform RAM tuketimi analizi yapar.
- Coklu B+ Tree Indeksleme: Tek bir veri kaydi eklendiginde sistem; ID, Yazar, Kategori, Kitap Adi ve Yayin Yili olmak uzere 5 farkli B+ Agaci uzerinde eszamanli indeksleme yapar.
- Soft Delete & Auto-Vacuum: Veriler aninda mantiksal olarak silinir (Soft Delete - O(1)). Arka planda calisan asenkron cop toplayici is parcacigi (Vacuum Thread) belirli araliklarla diskteki silinmis verileri fiziki olarak temizleyerek sistemi optimize eder.


SISTEM PERFORMANSI (BENCHMARKS)
-------------------------------
5 farkli B+ Agacinin (ID, Yazar, Kategori, Ad, Yil) eszamanli indeksleme performansi gercek test ortaminda asagidaki gibi olculmustur:

- 10.000 Kayit   | Indeksleme: ~193 ms    | RAM Tuketimi: ~1.3 MB
- 100.000 Kayit  | Indeksleme: ~1994 ms   | RAM Tuketimi: ~22.4 MB
- 1.000.000 Kayit| Indeksleme: ~21495 ms  | RAM Tuketimi: ~229.3 MB

* Arama (Search) sureleri veri boyutundan bagimsiz olarak mikrosaniye (Milisaniye alti) seviyelerinde gerceklesmektedir.


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
  Komut satirini proje dizininde acin ve asagidaki komutu calistirin:
  g++ main.cpp LibraryCRUD.cpp StringParser.cpp BPlusTree.cpp -o db_engine.exe

3. Projeyi Calistirma:
- Linux ve macOS Icin: ./db_engine
- Windows Icin: db_engine.exe


KULLANIM REHBERI
----------------

Program calistirildiginda oncelikle yuklemek istediginiz veri setini (10K, 100K, 1M) secmeniz istenir. Yukleme bitip RAM ve Sure metrikleri ekrana basildiktan sonra ana menu acilir:

- [1] Search Book (Menu): 5 farkli kritere gore (ID, Yazar, vb.) logaritmik hizda arama.
- [2] Update Record: Disk butunlugunu bozmadan Append-Only mantigiyla kayit guncelleme.
- [3] Delete Book (Soft Delete): Kaydi sistemden mantiksal olarak gizleme.
- [4] Manual Vacuum (Hard Delete): Diskteki Soft Delete yapilmis tum verileri temizleyip agaci defragmente etme (Birlestirme).
- [0] Exit: Arka plan is parcaciklarini guvenli bir sekilde kapatir ve programdan cikar.