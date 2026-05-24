Neden Array (Dizi) Değil de Dinamik Vector Kullandık?

C++ arrayler compile edilmeden once size belirlenmeli
Yani keys[100] yazilabilir ama keys[m] yazarsan hata verir (Variable Length Array kısıtlaması)
ayrica keys.insert(begin + 1, "B") ile shifting yapmak daha rahat


