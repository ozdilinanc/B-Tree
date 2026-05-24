B+ TREE INDEXING ENGINE - MIMARI DOKUMANTASYON
==============================================

Bu dokuman, C++ ile gelistirilen B+ Tree veri yapisinin arka planindaki mimari kararlari, bellek yonetimi (memory management) tekniklerini ve algoritma akisini detaylandirmaktadir.
Proje, disk tabanli veritabani yonetim sistemlerinin (DBMS) temel indeksleme mekanizmasini simule etmektedir.


1. C++ BELLEK YONETIMI VE ISARETCI (POINTER) MIMARISI
-----------------------------------------------------

1.1. Call by Reference Pointer (*&) Kullanimi
B+ Tree yapisinda, alt dugumlerde (child nodes) meydana gelen bolunme (split) islemlerinin, ust dugumlere (parent nodes) yeni dugumun bellek adresini bildirmesi gerekmektedir.
C++'ta bir fonksiyon yalnizca tek bir deger dondurebildigi icin, bu iletisim referans uzerinden saglanmistir.

KOD ORNEGI:
void BPlusTree::insertRecursive(BPlusNode *current, string key, streampos offset, BPlusNode *&newChild, string &promotedKey)

Buradaki "BPlusNode *&newChild" parametresi, isaretcinin kopyalanmasini engeller.
Dogrudan ust fonksiyonun (parent) bellek alanindaki isaretciye referans verir.
Boylece alt dugumde olusturulan yeni bellek adresi "newChild = newNode;" ile atandiginda, ust fonksiyon bu degisikligi aninda algilar.

1.2. Segmentation Fault (Cokme) Engelleme ve Kisa Devre (Short-Circuit) Mantigi
Dugum icerisindeki dizilerde arama yaparken, hedeflenen anahtarin mevcut tum anahtarlardan buyuk olma ihtimali vardir.
Bu durum, bellek sinirlarinin disina cikilmasina (Out of Bounds) sebep olabilir.

KOD ORNEGI:
while (pos < current->keys.size() && key > current->keys[pos])
    pos++;

Buradaki "pos < current->keys.size()" kontrolu hayati bir guvenlik bariyeridir.
C++'taki mantiksal AND (&&) operatorunun kisa devre (short-circuit) ozelligi sayesinde,
eger "pos" degeri vektor boyutuna ulasirsa sol taraf "false" doner ve derleyici sag taraftaki "current->keys[pos]" bellek erisimini calistirmaz.
Bu sayede olasi bir Segmentation Fault hatasi onlenmis olur.

1.3. Vektor Iteratorleri ve Bellek Kaydirma (begin() + pos)
B+ Tree dugumlerindeki anahtarlar her zaman sirali kalmak zorundadir.
Bu nedenle yeni bir eleman sona eklenmez (push_back kullanilmaz), bulunmasi gereken indekse yerlestirilir.

KOD ORNEGI:
current->keys.insert(current->keys.begin() + pos, key);

C++ std::vector::insert metodu, dogrudan indeks numarasi (int) kabul etmez.
Ilgili bellek blogunun baslangic noktasini gosteren bir iterator (begin()) talep eder.
"begin() + pos" ifadesi, bellekte tam olarak araya girilecek noktayi isaret eder.
Bu metod, hedef noktadan sonraki tum elemanlari bellekte guvenli bir sekilde birer adim saga kaydirir (memory shifting).


2. VERI YAPILARI VE ALGORITMA TASARIMI
--------------------------------------

2.1. Paralel Diziler (Parallel Arrays) Mimarisi
Dugumlerin icerisinde veri tutulurken C++ struct veya pair yapilari yerine paralel diziler tercih edilmistir.

- vector<string> keys;
- vector<vector<streampos>> dataOffsets;

Bu yapida, anahtar (key) ile o anahtara ait disk ofsetleri ayni indeks numarasinda eslesir.
dataOffsets yapisinin iki boyutlu bir vektor olmasinin sebebi,
ayni anahtara sahip (Duplicate Keys) birden fazla kaydin (ornegin ayni yazarin birden fazla kitabinin) disk adreslerini dinamik olarak tutabilmektir.

2.2. Degisken Isimlendirme Konvansiyonu
Kod okunabilirligini artirmak amaciyla dongu degiskenlerinde stratejik isimlendirmeler yapilmistir:
- "i" Degiskeni: Dumduz iterasyon ve tum elemanlari tarama (for donguleri) islemlerinde kullanilmistir.
- "pos" Degiskeni: Yeni eklenecek bir elemanin, agacin sirali yapisini bozmadan araya girecegi kesin koordinati (Position) bulmak icin tasarlanan while dongulerinde tercih edilmistir.

2.3. Linked List (Bagli Liste) Dugum Ekleme Standartlari
Yaprak dugumler (Leaf Nodes) bolundugunde, B+ Tree kurali geregi en alt katmandaki sirali bagli liste yapisinin korunmasi gerekir.

KOD ORNEGI:
newNode->next = current->next;
current->next = newNode;

Baglanti kopmalarini onlemek icin isaretci atamalari spesifik bir sirayla yapilir.
Ilk olarak yeni olusturulan dugum (newNode), mevcut dugumun sagindaki elemana baglanir.
Daha sonra mevcut dugumun isaretcisi yeni dugume yonlendirilir.
Islem sirasinin ters yapilmasi, listenin geri kalaninin bellek adresinin (memory leak) kaybolmasina yol acardi.


3. B+ TREE SPESIFIKASYONLARI VE YONLENDIRME (ROUTING)
-----------------------------------------------------

3.1. B-Tree ile B+ Tree Arasindaki Mimari Fark
Projenin temelini olusturan bu mimaride, ic dugumler (Internal Nodes) ile yaprak dugumler (Leaf Nodes) arasinda kesin bir veri ayrimi vardir.
- Ic Dugumler (isLeafNode = false): Sadece indeksleme (yol gosterme) amaciyla "keys" tutar. Iclerinde kesinlikle veri ofseti (dataOffsets) barindirmazlar.
- Yaprak Dugumler (isLeafNode = true): Agactaki tum gercek veriler (ofsetler) yalnizca bu katmanda tutulur.
Bu ayrim, ic dugumlerin bellek yukunu hafifleterek agacin derecesinin (branching factor) maksimize edilmesini ve arama hizinin logaritmik olarak artmasini saglar.

3.2. Agac Yonlendirme (Routing) Mantigi
Arama islemi kokten baslar ve anahtarlarin buyukluklerine gore uygun cocuk dugume inilir.

KOD ORNEGI:
int i = 0;
while (i < current->keys.size() && key >= current->keys[i])
    i++;
current = current->children[i];

Bu algoritmada yonlendirme kosulu ">=" olarak belirlenmistir.
Aranan anahtar, ic dugumdeki anahtardan kucukse sol taraftaki yola (i), buyuk veya esitse sag taraftaki yola (i++) yonlendirilir.


4. BOLUNME (SPLIT) ISLEMLERI VE EDGE CASELER
--------------------------------------------

Agac kapasitesi (order) asildiginda gerceklesen bolunme islemleri, dugumun tipine gore tamamen farkli iki algoritma ile ele alinir.

4.1. Yaprak (Leaf) ve Ic (Internal) Dugum Bolunme Farklari
- Yaprak Dugum Bolunmesi: Yeni olusturulan sag dugumun ilk elemani, ust dugume kopyalanir (promotedKey).
Veri yaprakta kalmaya devam eder, cunku gercek verilere sadece yapraklardan erisilebilir.
- Ic Dugum Bolunmesi: Bolunme noktasindaki ortanca eleman, bulundugu ic dugumden kalici olarak silinir (erase) ve ust dugume tasinir.
Bu islem, ic dugumlerde veri tekrarini onler.

4.2. Ic Dugum Bolunmesinde Pointer Senkronizasyonu
Bir ic dugumde N adet anahtar varsa, N+1 adet cocuk isaretcisi (children pointer) olmak zorundadir.

KOD ORNEGI:
for (int j = mid + 1; j < order; j++) {
    newNode->keys.push_back(current->keys[j]);
    newNode->children.push_back(current->children[j]);
}
newNode->children.push_back(current->children[order]); // Kritik atama

Ortadan bolunen bir ic dugumun sag yarisi yeni bir dugume aktarilirken, anahtarlari aktaran standart for dongusu, son anahtarin saginda kalan o son isaretciyi kopyalayamaz.
Yukaridaki blokta dongu sonrasina eklenen manuel push_back islemi, agacin en sagindaki bellek adreslerinin kaybolmasini onler.


5. OZYINELEMELI (RECURSIVE) ISLEYIS VE DOMINO ETKISI
----------------------------------------------------

insertRecursive fonksiyonu, B+ Tree'nin yukaridan asagiya ve asagidan yukariya dogru calismasini saglayan 4 temel faza sahiptir:

1. Arama/Inis Fazi: Fonksiyon, dogru yaprak dugumu bulana kadar kendi kendini cagirarak (Call Stack) agacin derinliklerine iner.
2. Yaprak Ekleme Fazi: Veri ilgili yaprak dugume eklenir. Tasma (overflow) kontrolu yapilir.
3. Yukari Cikis (Backtracking): Alt dugumdeki islem bittikten sonra, ozyinelemeli yapi bir ust fonksiyona (parent) geri doner.
4. Domino Etkisi (Cascading Splits):

KOD ORNEGI:
if (newChild == nullptr)
    return;

Ust dugum, alt dugumden gelen newChild isaretcisini kontrol eder. Eger nullptr ise herhangi bir tasma olmamistir ve surec tamamlanir.
Ancak isaretci dolu gelirse, alt dugumun bolundugu anlasilir. Ust dugum, alttan gelen anahtari kendi icerisine insert eder.
Eger bu ekleme islemi ust dugumun de kapasitesini asmasina sebep olursa, ust dugum de bolunur ve bu durum Kok (Root) dugume kadar ardisik bir tasma/bolunme reaksiyonu tetikler.