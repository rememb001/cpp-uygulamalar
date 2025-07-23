#include <iostream>
#include <string>
#include <iomanip>
#include <locale>
#include <conio.h>
#include <ctime>
#include <cmath>

using namespace std;

// Sadece harf kontrolu fonksiyonu
bool sadeceharf(const string& str) {
    for(char c : str) {
        if(!isalpha(c) && c != ' ') {
            return false;
        }
    }
    return !str.empty();
}

// Sadece sayi kontrolu fonksiyonu
bool sadecesakam(const string& str) {
    for(char c : str) {
        if(!isdigit(c)) {
            return false;
        }
    }
    return !str.empty();
}

// Dogum tarihinden yas hesaplama fonksiyonu
int yasHesapla(int gun, int ay, int yil) {
    time_t t = time(0);
    struct tm* now = localtime(&t);
    
    int bugunYil = now->tm_year + 1900;
    int bugunAy = now->tm_mon + 1;
    int bugunGun = now->tm_mday;
    
    int yas = bugunYil - yil;
    
    // Eger dogum gunu henuz gelmemisse, yasi bir azalt
    if(bugunAy < ay || (bugunAy == ay && bugunGun < gun)) {
        yas--;
    }
    
    return yas;
}

int main() {
    // Turkce karakter destegi icin
    setlocale(LC_ALL, "Turkish");
    
    // Değişkenler
    string ad, soyad, meslek, dogumTarihi;
    int yas;
    
    cout << "=================================================" << endl;
    cout << "         KISISEL BILGI TOPLAMA PROGRAMI         " << endl;
    cout << "=================================================" << endl;
    cout << endl;
    
    // Kullanıcıdan bilgileri al
    cout << "Lutfen asagidaki bilgileri giriniz:" << endl;
    cout << endl;
    
    // Ad girisi (sadece harf)
    do {
        cout << "Adiniz: ";
        getline(cin, ad);
        if(!sadeceharf(ad)) {
            cout << "Hata: Sadece harf giriniz!" << endl;
        }
    } while(!sadeceharf(ad));
    
    // Soyad girisi (sadece harf)
    do {
        cout << "Soyadiniz: ";
        getline(cin, soyad);
        if(!sadeceharf(soyad)) {
            cout << "Hata: Sadece harf giriniz!" << endl;
        }
    } while(!sadeceharf(soyad));
    
    // Yas girisi (sadece sayi)
    string yasStr;
    do {
        cout << "Yasiniz: ";
        getline(cin, yasStr);
        if(!sadecesakam(yasStr)) {
            cout << "Hata: Sadece sayi giriniz!" << endl;
        }
    } while(!sadecesakam(yasStr));
    yas = stoi(yasStr); // String'i int'e cevir
    
    // Meslek girisi (sadece harf)
    do {
        cout << "Mesleginiz: ";
        getline(cin, meslek);
        if(!sadeceharf(meslek)) {
            cout << "Hata: Sadece harf giriniz!" << endl;
        }
    } while(!sadeceharf(meslek));
    
    // Dogum tarihi girisi (dogrulama ile)
    bool gecerliTarih = false;
    do {
        cout << "Dogum Tarihiniz: ";
        
        // Otomatik tarih girisi (GG/AA/YYYY)
        string gun, ay, yil;
        char ch;
        
        // Tarih girisi backspace destegi ile
        cout << "Tarih (GGAAYYYY): ";
        string tamTarih = "";
        int pozisyon = 0;
        
        while(pozisyon < 8) {
            ch = _getch();
            
            if(ch >= '0' && ch <= '9') {
                // Rakam girisi
                tamTarih += ch;
                cout << ch;
                
                // Otomatik / ekleme
                if(pozisyon == 1 || pozisyon == 3) {
                    cout << "/";
                }
                pozisyon++;
            }
            else if(ch == 8) { // Backspace (ASCII 8)
                if(pozisyon > 0 && !tamTarih.empty()) {
                    pozisyon--;
                    tamTarih.pop_back();
                    
                    // Ekranda geri git
                    cout << "\b \b";
                    
                    // Eger / karakterinden sonra geldik, onu da ekrandan kaldir
                    if(pozisyon == 2 || pozisyon == 4) {
                        cout << "\b \b";
                    }
                }
            }
        }
        
        // Tarihi parcalara ayir
        gun = tamTarih.substr(0, 2);
        ay = tamTarih.substr(2, 2);
        yil = tamTarih.substr(4, 4);
        
        cout << endl; // Yeni satira gec
        
        // Gun, ay ve yil dogrulamasi
        int gunSayi = stoi(gun);
        int aySayi = stoi(ay);
        int yilSayi = stoi(yil);
        
        if(gunSayi < 1 || gunSayi > 31) {
            cout << "Hata: Gun 1-31 arasinda olmalidir!" << endl;
            gecerliTarih = false;
        } else if(aySayi < 1 || aySayi > 12) {
            cout << "Hata: Ay 1-12 arasinda olmalidir!" << endl;
            gecerliTarih = false;
        } else if(yilSayi >= 2026) {
            cout << "Hata: Dogum yili 2025 ve altinda olmalidir!" << endl;
            gecerliTarih = false;
        } else if(yilSayi < 1900) {
            cout << "Hata: Dogum yili 1900 ve ustunde olmalidir!" << endl;
            gecerliTarih = false;
        } else {
            // Dogum tarihinden yas hesapla ve girilen yas ile karsilastir
            int hesaplananYas = yasHesapla(gunSayi, aySayi, yilSayi);
            int yasFarki = abs(hesaplananYas - yas);
            
            if(yasFarki > 3) {
                cout << "Hata: Girilen yas (" << yas << ") ile dogum tarihinden hesaplanan yas (" << hesaplananYas << ") arasinda 3 yildan fazla fark var!" << endl;
                gecerliTarih = false;
            } else {
                // Tarihi birlestir
                dogumTarihi = gun + "/" + ay + "/" + yil;
                gecerliTarih = true;
            }
        }
        
    } while(!gecerliTarih);
    
    cout << endl;
    cout << "Bilgiler kaydediliyor..." << endl;
    cout << endl;
    
    // Tablo başlığı
    cout << "=================================================" << endl;
    cout << "                KISISEL BILGILER               " << endl;
    cout << "=================================================" << endl;
    
    // Tablo içeriği
    cout << left << setw(20) << "| Alan" << "| Bilgi" << setw(23) << " " << "|" << endl;
    cout << "|" << setfill('-') << setw(48) << "" << "|" << endl;
    cout << setfill(' ');
    
    cout << left << setw(20) << "| Ad" << "| " << setw(25) << ad << "|" << endl;
    cout << left << setw(20) << "| Soyad" << "| " << setw(25) << soyad << "|" << endl;
    cout << left << setw(20) << "| Yas" << "| " << setw(25) << yas << "|" << endl;
    cout << left << setw(20) << "| Meslek" << "| " << setw(25) << meslek << "|" << endl;
    cout << left << setw(20) << "| Dogum Tarihi" << "| " << setw(25) << dogumTarihi << "|" << endl;
    
    cout << "=================================================" << endl;
    cout << endl;
    cout << "Tesekkurler! Bilgileriniz basariyla kaydedildi." << endl;
    cout << "Cikmak icin herhangi bir tusa basin..." << endl;
    
    cin.get();
    return 0;
}
