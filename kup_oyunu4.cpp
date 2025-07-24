#include <windows.h>
#include <math.h>
#include <time.h>
#include <stdio.h>

// Oyun değişkenleri
struct Kup {
    float x, y, z;
    float rotX, rotY, rotZ;
    float r, g, b;
    int aktif;
    float boyut;
};

#define MAX_KUP 15
Kup kupler[MAX_KUP];
int puan = 0;
int seviye = 1;
int oyunSuresi = 60;
DWORD baslangicZamani;
int oyunBitti = 0;

// Zorluk sistemi
int zorlukSeviyesi = 1; // 1=Kolay, 2=Orta, 3=Zor
int hareketHizi = 50;   // Timer hızı (düşük = hızlı)
DWORD sonHareketZamani = 0;

// Pencere değişkenleri
HWND hWnd;
HDC hDC;
int genislik = 1920;
int yukseklik = 1080;
int fullScreen = 0;

// Oyun durumları
#define DURUM_MENU 0
#define DURUM_OYUN 1
int oyunDurumu = DURUM_MENU;

// Menu buton alanları
struct MenuButon {
    int x, y, genislik, yukseklik;
    int zorlukSeviyesi;
};

MenuButon menuButonlari[3];

// Fonksiyon bildirimleri
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void OyunuBaslat();
void KupleriOlustur();
void KupleriCiz(HDC hdc);
void Kup3DCiz(HDC hdc, Kup* k);
void OyunuGuncelle();
void PuanGoster(HDC hdc);
int KupTiklandi(int mouseX, int mouseY);
void FullScreenGecis();
void MenuCiz(HDC hdc);
void ZorlukAyarla(int seviye);
void TumKupleriYenidenDagit();
void MenuButonlariOlustur();
int MenuButonTiklandi(int mouseX, int mouseY);
int KupCakisiyorMu(int index, float x, float y, float boyut);
void GuvenliKonumBul(int index);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Pencere sınıfı
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KupOyunu4";
    wc.hCursor = LoadCursor(NULL, IDC_CROSS);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    
    RegisterClass(&wc);
    
    // Pencere oluştur (tam ekran için özel ayarlar)
    hWnd = CreateWindow(
        "KupOyunu4",
        "3D Kup Yakalama Oyunu v4 - Fare ve Klavye ile Kontrol!",
        WS_POPUP,  // Tam ekran için WS_POPUP kullan
        0, 0,      // Tam ekran konumu
        GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),  // Tam ekran boyutu
        NULL, NULL, hInstance, NULL
    );
    
    ShowWindow(hWnd, SW_MAXIMIZE);  // Tam ekran göster
    UpdateWindow(hWnd);
    
    // Tam ekran durumunu ayarla
    fullScreen = 1;
    genislik = GetSystemMetrics(SM_CXSCREEN);
    yukseklik = GetSystemMetrics(SM_CYSCREEN);
    
    // Menu butonlarını oluştur
    MenuButonlariOlustur();
    
    // Oyunu başlat
    OyunuBaslat();
    
    // Ana döngü
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE:
            SetTimer(hWnd, 1, hareketHizi, NULL); // Zorluk seviyesine göre hız
            break;
            
        case WM_TIMER:
            if (!oyunBitti) {
                OyunuGuncelle();
                InvalidateRect(hWnd, NULL, FALSE);
            }
            break;
            
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            
            // Double buffering için memory DC oluştur
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP memBitmap = CreateCompatibleBitmap(hdc, genislik, yukseklik);
            HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);
            
            // Arka plan
            HBRUSH arkaPlan = CreateSolidBrush(RGB(20, 30, 60));
            RECT fullRect = {0, 0, genislik, yukseklik};
            FillRect(memDC, &fullRect, arkaPlan);
            DeleteObject(arkaPlan);
            
            if (oyunDurumu == DURUM_MENU) {
                MenuCiz(memDC);
            } else if (!oyunBitti) {
                KupleriCiz(memDC);
                PuanGoster(memDC);
            } else {
                // Oyun bitti mesajı
                SetTextColor(memDC, RGB(255, 255, 0));
                SetBkMode(memDC, TRANSPARENT);
                HFONT buyukFont = CreateFont(48, 0, 0, 0, FW_BOLD, 0, 0, 0, 
                    TURKISH_CHARSET, 0, 0, 0, 0, "Arial");
                SelectObject(memDC, buyukFont);
                
                char mesaj[100];
                sprintf(mesaj, "OYUN BITTI! PUAN: %d", puan);
                TextOut(memDC, genislik/2 - 150, yukseklik/2 - 50, mesaj, strlen(mesaj));
                
                SetTextColor(memDC, RGB(255, 255, 255));
                HFONT normalFont = CreateFont(24, 0, 0, 0, FW_NORMAL, 0, 0, 0, 
                    TURKISH_CHARSET, 0, 0, 0, 0, "Arial");
                SelectObject(memDC, normalFont);
                TextOut(memDC, genislik/2 - 120, yukseklik/2 + 20, 
                    "SPACE: Yeniden Oyna  |  M: Ana Menu", 35);
                
                DeleteObject(buyukFont);
                DeleteObject(normalFont);
            }
            
            // Memory DC'den ana DC'ye kopyala (double buffering)
            BitBlt(hdc, 0, 0, genislik, yukseklik, memDC, 0, 0, SRCCOPY);
            
            // Memory nesnelerini temizle
            SelectObject(memDC, oldBitmap);
            DeleteObject(memBitmap);
            DeleteDC(memDC);
            
            EndPaint(hWnd, &ps);
            break;
        }
        
        case WM_LBUTTONDOWN: {
            int mouseX = LOWORD(lParam);
            int mouseY = HIWORD(lParam);
            
            if (oyunDurumu == DURUM_MENU) {
                // Menu'da mouse tıklaması - zorluk seçimi
                int secilenZorluk = MenuButonTiklandi(mouseX, mouseY);
                if (secilenZorluk > 0) {
                    ZorlukAyarla(secilenZorluk);
                }
            } else if (oyunDurumu == DURUM_OYUN && !oyunBitti) {
                // Oyunda mouse tıklaması - kup yakalama
                if (KupTiklandi(mouseX, mouseY)) {
                    puan += 10 * zorlukSeviyesi;
                } else {
                    // Yanlış tıklama - tüm küpleri yeniden dağıt
                    TumKupleriYenidenDagit();
                }
            }
            break;
        }
        
        case WM_KEYDOWN:
            if (oyunDurumu == DURUM_MENU) {
                if (wParam == '1') {
                    ZorlukAyarla(1); // Kolay
                } else if (wParam == '2') {
                    ZorlukAyarla(2); // Orta
                } else if (wParam == '3') {
                    ZorlukAyarla(3); // Zor
                }
            } else if (wParam == VK_SPACE && oyunBitti) {
                OyunuBaslat(); // Yeniden başlat
                InvalidateRect(hWnd, NULL, FALSE);
            } else if (wParam == 'M' && oyunBitti) {
                oyunDurumu = DURUM_MENU; // Ana menüye dön
                InvalidateRect(hWnd, NULL, FALSE);
            } else if (wParam == VK_ESCAPE) {
                if (oyunDurumu == DURUM_OYUN) {
                    oyunDurumu = DURUM_MENU;
                    InvalidateRect(hWnd, NULL, FALSE);
                } else {
                    PostQuitMessage(0);
                }
            } else if (wParam == VK_F11) {
                FullScreenGecis(); // F11 ile full ekran
                InvalidateRect(hWnd, NULL, FALSE);
            }
            break;
            
        case WM_SIZE:
            genislik = LOWORD(lParam);
            yukseklik = HIWORD(lParam);
            MenuButonlariOlustur(); // Pencere boyutu değiştiğinde butonları yeniden hesapla
            if (!oyunBitti) {
                KupleriOlustur(); // Pencere boyutu değiştiğinde küpleri yeniden dağıt
            }
            break;
            
        case WM_DESTROY:
            KillTimer(hWnd, 1);
            PostQuitMessage(0);
            break;
            
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void OyunuBaslat() {
    srand(time(NULL));
    puan = 0;
    oyunBitti = 0;
    baslangicZamani = GetTickCount();
    KupleriOlustur();
}

void KupleriOlustur() {
    // Responsive margin hesaplama
    int marjin = (genislik + yukseklik) / 40;
    if (marjin < 30) marjin = 30;
    if (marjin > 80) marjin = 80;
    
    for (int i = 0; i < MAX_KUP; i++) {
        // Responsive boyut
        kupler[i].boyut = (genislik + yukseklik) / 40;
        if (kupler[i].boyut < 20) kupler[i].boyut = 20;
        if (kupler[i].boyut > 60) kupler[i].boyut = 60;
        
        // Çakışmayan güvenli konum bul
        int denemeSayisi = 0;
        do {
            kupler[i].x = rand() % (genislik - 2*marjin) + marjin;
            kupler[i].y = rand() % (yukseklik - 2*marjin - 100) + marjin + 80;
            denemeSayisi++;
        } while (KupCakisiyorMu(i, kupler[i].x, kupler[i].y, kupler[i].boyut) && denemeSayisi < 50);
        
        kupler[i].z = (rand() % 100) + 50;
        kupler[i].rotX = rand() % 360;
        kupler[i].rotY = rand() % 360;
        kupler[i].rotZ = rand() % 360;
        kupler[i].aktif = 1;
        
        // Önceden tanımlı renklerden birini seç (beyaz hariç)
        int renkler[8][3] = {
            {255, 100, 100}, {100, 255, 100}, {100, 100, 255}, {255, 255, 100},
            {255, 100, 255}, {100, 255, 255}, {255, 150, 100}, {150, 100, 255}
        };
        int renkIndex = rand() % 8;
        kupler[i].r = renkler[renkIndex][0];
        kupler[i].g = renkler[renkIndex][1];
        kupler[i].b = renkler[renkIndex][2];
    }
}

void KupleriCiz(HDC hdc) {
    for (int i = 0; i < MAX_KUP; i++) {
        if (kupler[i].aktif) {
            Kup3DCiz(hdc, &kupler[i]);
        }
    }
}

void Kup3DCiz(HDC hdc, Kup* k) {
    // 2D kare olarak çiz (3D efekti için gölge)
    int boyut = (int)k->boyut;
    
    // Gölge çiz
    HBRUSH golgeBrush = CreateSolidBrush(RGB(0, 0, 0));
    RECT golgeRect = {k->x - boyut/2 + 3, k->y - boyut/2 + 3, 
                      k->x + boyut/2 + 3, k->y + boyut/2 + 3};
    FillRect(hdc, &golgeRect, golgeBrush);
    DeleteObject(golgeBrush);
    
    // Ana kareyi çiz
    HBRUSH kupBrush = CreateSolidBrush(RGB((int)k->r, (int)k->g, (int)k->b));
    RECT kupRect = {k->x - boyut/2, k->y - boyut/2, 
                    k->x + boyut/2, k->y + boyut/2};
    FillRect(hdc, &kupRect, kupBrush);
    DeleteObject(kupBrush);
    
    // Kenar çizgisi
    HPEN kenarPen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
    SelectObject(hdc, kenarPen);
    SelectObject(hdc, GetStockObject(NULL_BRUSH));
    Rectangle(hdc, k->x - boyut/2, k->y - boyut/2, 
              k->x + boyut/2, k->y + boyut/2);
    DeleteObject(kenarPen);
}

void OyunuGuncelle() {
    DWORD simdikiZaman = GetTickCount();
    
    // Zorluk seviyesine göre hareket hızı kontrolü
    if (simdikiZaman - sonHareketZamani > hareketHizi) {
        for (int i = 0; i < MAX_KUP; i++) {
            if (kupler[i].aktif) {
                kupler[i].rotX += 2.0f;
                kupler[i].rotY += 1.5f;
                kupler[i].rotZ += 1.0f;
                
                if (kupler[i].rotX > 360) kupler[i].rotX -= 360;
                if (kupler[i].rotY > 360) kupler[i].rotY -= 360;
                if (kupler[i].rotZ > 360) kupler[i].rotZ -= 360;
            }
        }
        sonHareketZamani = simdikiZaman;
    }
    
    // Oyun süresi kontrolü
    if (simdikiZaman - baslangicZamani > oyunSuresi * 1000) {
        oyunBitti = 1;
    }
}

void PuanGoster(HDC hdc) {
    SetTextColor(hdc, RGB(255, 255, 255));
    SetBkMode(hdc, TRANSPARENT);
    
    HFONT font = CreateFont(28, 0, 0, 0, FW_BOLD, 0, 0, 0, 
        TURKISH_CHARSET, 0, 0, 0, 0, "Arial");
    SelectObject(hdc, font);
    
    char puanStr[50];
    sprintf(puanStr, "PUAN: %d", puan);
    TextOut(hdc, 20, 20, puanStr, strlen(puanStr));
    
    // Zorluk seviyesi göster
    char zorlukStr[50];
    char* zorlukAdi[] = {"", "KOLAY", "ORTA", "ZOR"};
    sprintf(zorlukStr, "ZORLUK: %s", zorlukAdi[zorlukSeviyesi]);
    TextOut(hdc, 20, 55, zorlukStr, strlen(zorlukStr));
    
    // Kalan süre
    DWORD gecenSure = (GetTickCount() - baslangicZamani) / 1000;
    int kalanSure = oyunSuresi - gecenSure;
    if (kalanSure < 0) kalanSure = 0;
    
    char sureStr[50];
    sprintf(sureStr, "SURE: %d", kalanSure);
    TextOut(hdc, genislik - 150, 20, sureStr, strlen(sureStr));
    
    DeleteObject(font);
}

int KupTiklandi(int mouseX, int mouseY) {
    for (int i = 0; i < MAX_KUP; i++) {
        if (kupler[i].aktif) {
            int boyut = (int)kupler[i].boyut;
            if (mouseX >= kupler[i].x - boyut/2 && mouseX <= kupler[i].x + boyut/2 &&
                mouseY >= kupler[i].y - boyut/2 && mouseY <= kupler[i].y + boyut/2) {
                
                // Küpü yeniden konumlandır (çakışma kontrolü ile)
                GuvenliKonumBul(i);
                kupler[i].z = (rand() % 100) + 50;
                
                // Önceden tanımlı renklerden birini seç
                int renkler[8][3] = {
                    {255, 100, 100}, {100, 255, 100}, {100, 100, 255}, {255, 255, 100},
                    {255, 100, 255}, {100, 255, 255}, {255, 150, 100}, {150, 100, 255}
                };
                int renkIndex = rand() % 8;
                kupler[i].r = renkler[renkIndex][0];
                kupler[i].g = renkler[renkIndex][1];
                kupler[i].b = renkler[renkIndex][2];
                
                return 1; // Başarılı tıklama
            }
        }
    }
    return 0; // Boş tıklama
}

void FullScreenGecis() {
    static WINDOWPLACEMENT wp = { sizeof(wp) };
    static DWORD dwStyle = 0;
    
    if (!fullScreen) {
        // Normal pencereden full ekrana geç
        GetWindowPlacement(hWnd, &wp);
        dwStyle = GetWindowLong(hWnd, GWL_STYLE);
        
        SetWindowLong(hWnd, GWL_STYLE, dwStyle & ~(WS_CAPTION | WS_THICKFRAME));
        SetWindowPos(hWnd, HWND_TOP, 0, 0, 
                     GetSystemMetrics(SM_CXSCREEN), 
                     GetSystemMetrics(SM_CYSCREEN),
                     SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        fullScreen = 1;
    } else {
        // Full ekrandan normal pencereye geç
        SetWindowLong(hWnd, GWL_STYLE, dwStyle);
        SetWindowPlacement(hWnd, &wp);
        SetWindowPos(hWnd, NULL, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                     SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        fullScreen = 0;
    }
}

void MenuCiz(HDC hdc) {
    SetTextColor(hdc, RGB(255, 255, 0));
    SetBkMode(hdc, TRANSPARENT);
    
    HFONT buyukFont = CreateFont(64, 0, 0, 0, FW_BOLD, 0, 0, 0, 
        TURKISH_CHARSET, 0, 0, 0, 0, "Arial");
    SelectObject(hdc, buyukFont);
    
    TextOut(hdc, genislik/2 - 200, 100, "KUP YAKALAMA", 13);
    
    SetTextColor(hdc, RGB(255, 255, 255));
    HFONT normalFont = CreateFont(32, 0, 0, 0, FW_NORMAL, 0, 0, 0, 
        TURKISH_CHARSET, 0, 0, 0, 0, "Arial");
    SelectObject(hdc, normalFont);
    
    TextOut(hdc, genislik/2 - 150, 250, "ZORLUK SEVIYESI SECIN:", 22);
    
    // Zorluk seçeneklerini çiz ve buton alanlarını vurgula
    for (int i = 0; i < 3; i++) {
        // Buton arka planı çiz
        HBRUSH butonBrush;
        if (i == 0) {
            butonBrush = CreateSolidBrush(RGB(40, 80, 40)); // Kolay - yeşil ton
            SetTextColor(hdc, RGB(100, 255, 100));
        } else if (i == 1) {
            butonBrush = CreateSolidBrush(RGB(80, 80, 40)); // Orta - sarı ton
            SetTextColor(hdc, RGB(255, 255, 100));
        } else {
            butonBrush = CreateSolidBrush(RGB(80, 40, 40)); // Zor - kırmızı ton
            SetTextColor(hdc, RGB(255, 100, 100));
        }
        
        RECT butonRect = {menuButonlari[i].x, menuButonlari[i].y, 
                          menuButonlari[i].x + menuButonlari[i].genislik, 
                          menuButonlari[i].y + menuButonlari[i].yukseklik};
        FillRect(hdc, &butonRect, butonBrush);
        DeleteObject(butonBrush);
        
        // Buton kenarı
        HPEN kenarPen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
        SelectObject(hdc, kenarPen);
        SelectObject(hdc, GetStockObject(NULL_BRUSH));
        Rectangle(hdc, butonRect.left, butonRect.top, butonRect.right, butonRect.bottom);
        DeleteObject(kenarPen);
        
        // Buton metni
        char* metinler[] = {"1 - KOLAY", "2 - ORTA", "3 - ZOR"};
        int metinUzunlugu[] = {10, 9, 8};
        TextOut(hdc, menuButonlari[i].x + 20, menuButonlari[i].y + 8, 
                metinler[i], metinUzunlugu[i]);
    }
    
    SetTextColor(hdc, RGB(200, 200, 200));
    HFONT kucukFont = CreateFont(20, 0, 0, 0, FW_NORMAL, 0, 0, 0, 
        TURKISH_CHARSET, 0, 0, 0, 0, "Arial");
    SelectObject(hdc, kucukFont);
    
    TextOut(hdc, genislik/2 - 180, 500, "Yanlis tiklama = Tum kupler yer degistirir!", 44);
    TextOut(hdc, genislik/2 - 150, 530, "Klavye (1,2,3) veya Fare ile secin", 35);
    TextOut(hdc, genislik/2 - 120, 560, "F11: Full Ekran  |  ESC: Cikis", 31);
    
    DeleteObject(buyukFont);
    DeleteObject(normalFont);
    DeleteObject(kucukFont);
}

void ZorlukAyarla(int seviye) {
    zorlukSeviyesi = seviye;
    switch(seviye) {
        case 1: hareketHizi = 80; break;  // Kolay - yavas
        case 2: hareketHizi = 50; break;  // Orta
        case 3: hareketHizi = 25; break;  // Zor - hizli
    }
    // Timer'i yeni hizla yeniden ayarla
    KillTimer(hWnd, 1);
    SetTimer(hWnd, 1, hareketHizi, NULL);
    
    oyunDurumu = DURUM_OYUN;
    OyunuBaslat();
}

void TumKupleriYenidenDagit() {
    KupleriOlustur(); // Tum kupleri yeniden konumlandir
}

// Çakışma kontrolü fonksiyonu
int KupCakisiyorMu(int index, float x, float y, float boyut) {
    for (int i = 0; i < index; i++) {
        if (kupler[i].aktif) {
            float mesafe = sqrt((x - kupler[i].x) * (x - kupler[i].x) + 
                               (y - kupler[i].y) * (y - kupler[i].y));
            float minMesafe = (boyut + kupler[i].boyut) / 2 + 10; // 10 piksel boşluk
            if (mesafe < minMesafe) {
                return 1; // Çakışıyor
            }
        }
    }
    return 0; // Çakışmıyor
}

// Güvenli konum bulma fonksiyonu
void GuvenliKonumBul(int index) {
    int marjin = (genislik + yukseklik) / 40;
    if (marjin < 30) marjin = 30;
    if (marjin > 80) marjin = 80;
    
    int denemeSayisi = 0;
    do {
        kupler[index].x = rand() % (genislik - 2*marjin) + marjin;
        kupler[index].y = rand() % (yukseklik - 2*marjin - 100) + marjin + 80;
        denemeSayisi++;
    } while (KupCakisiyorMu(index, kupler[index].x, kupler[index].y, kupler[index].boyut) && denemeSayisi < 50);
}

void MenuButonlariOlustur() {
    // Buton boyutları ve konumları (responsive)
    int butonGenislik = 200;
    int butonYukseklik = 50;
    int baslangicY = 320;
    int aralik = 60;
    
    for (int i = 0; i < 3; i++) {
        menuButonlari[i].x = genislik/2 - butonGenislik/2;
        menuButonlari[i].y = baslangicY + (i * aralik);
        menuButonlari[i].genislik = butonGenislik;
        menuButonlari[i].yukseklik = butonYukseklik;
        menuButonlari[i].zorlukSeviyesi = i + 1;
    }
}

int MenuButonTiklandi(int mouseX, int mouseY) {
    for (int i = 0; i < 3; i++) {
        if (mouseX >= menuButonlari[i].x && 
            mouseX <= menuButonlari[i].x + menuButonlari[i].genislik &&
            mouseY >= menuButonlari[i].y && 
            mouseY <= menuButonlari[i].y + menuButonlari[i].yukseklik) {
            return menuButonlari[i].zorlukSeviyesi;
        }
    }
    return 0; // Hiçbir butona tıklanmadı
}
