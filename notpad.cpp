#include <windows.h>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <direct.h>

// Kontrol ID'leri
#define IDC_EDIT 1001
#define IDC_LISTBOX 1002
#define IDC_TITLE_EDIT 1003
#define IDC_BTN_NEW_NOTE 1004
#define IDC_BTN_OLD_NOTES 1005
#define IDC_BTN_SAVE 1006
#define IDC_BTN_DELETE_ALL 1007
#define IDC_BTN_DELETE_CURRENT 1008
#define IDC_BTN_BACK 1009

// Uygulama durumları
enum AppState {
    STATE_MAIN_MENU,
    STATE_NOTE_EDITOR,
    STATE_NOTE_LIST
};

// Not yapısı
struct Note {
    std::string title;
    std::string content;
    std::string filename;
    
    Note(const std::string& t = "Yeni Not", const std::string& c = "") 
        : title(t), content(c) {
        // Dosya adını başlıktan oluştur
        filename = t + ".txt";
        // Geçersiz karakterleri temizle
        for (char& ch : filename) {
            if (ch == '\\' || ch == '/' || ch == ':' || ch == '*' || 
                ch == '?' || ch == '"' || ch == '<' || ch == '>' || ch == '|') {
                ch = '_';
            }
        }
    }
};

// Global değişkenler
HWND hMainWindow;
HWND hEdit;
HWND hListBox;
HWND hTitleEdit;
HWND hBtnNewNote, hBtnOldNotes, hBtnSave, hBtnDeleteAll, hBtnDeleteCurrent, hBtnBack;
HWND hLabelTitle, hLabelWelcome;
std::vector<Note> notes;
int currentNoteIndex = -1;
AppState currentState = STATE_MAIN_MENU;
HFONT hBigFont, hMediumFont, hSmallFont;

// Fonksiyon prototipleri
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void CreateControls(HWND hwnd);
void CreateFonts();
void ShowMainMenu();
void ShowNoteEditor();
void ShowNoteList();
void NewNote();
void SaveCurrentNote();
void LoadNote(int index);
void LoadAllNotes();
void DeleteCurrentNote();
void DeleteAllNotes();
void UpdateNotesList();
void ResizeControls(HWND hwnd);
std::string GetNotesDirectory();

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    const char* CLASS_NAME = "ModernNotepadV2Window";
    
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    
    RegisterClass(&wc);
    
    hMainWindow = CreateWindowEx(
        0, CLASS_NAME, "Modern Notepad v2.0",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1000, 700,
        NULL, NULL, hInstance, NULL
    );
    
    if (!hMainWindow) return 0;
    
    CreateFonts();
    CreateControls(hMainWindow);
    
    // Notlar klasorunu olustur
    std::string notesDir = GetNotesDirectory();
    _mkdir(notesDir.c_str());
    
    // Mevcut notlari yukle
    LoadAllNotes();
    
    // Ana menuyu goster
    ShowMainMenu();
    
    ShowWindow(hMainWindow, nCmdShow);
    UpdateWindow(hMainWindow);
    
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return (int)msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CREATE:
            break;
            
        case WM_SIZE:
            ResizeControls(hwnd);
            break;
            
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDC_BTN_NEW_NOTE:
                    NewNote();
                    ShowNoteEditor();
                    break;
                    
                case IDC_BTN_OLD_NOTES:
                    ShowNoteList();
                    break;
                    
                case IDC_BTN_SAVE:
                    SaveCurrentNote();
                    MessageBox(hwnd, "Not kaydedildi!", "Bilgi", MB_ICONINFORMATION);
                    break;
                    
                case IDC_BTN_DELETE_CURRENT:
                    DeleteCurrentNote();
                    break;
                    
                case IDC_BTN_DELETE_ALL:
                    DeleteAllNotes();
                    break;
                    
                case IDC_BTN_BACK:
                    if (currentState == STATE_NOTE_EDITOR) {
                        SaveCurrentNote();
                    }
                    ShowMainMenu();
                    break;
                    
                case IDC_LISTBOX:
                    if (HIWORD(wParam) == LBN_DBLCLK)
                    {
                        int sel = SendMessage(hListBox, LB_GETCURSEL, 0, 0);
                        if (sel != LB_ERR) {
                            LoadNote(sel);
                            ShowNoteEditor();
                        }
                    }
                    break;
            }
            break;
            
        case WM_CLOSE:
            if (currentState == STATE_NOTE_EDITOR) {
                SaveCurrentNote();
            }
            DestroyWindow(hwnd);
            break;
            
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
            
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

void CreateFonts()
{
    // Büyük font (başlık için)
    hBigFont = CreateFont(28, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        TURKISH_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
    
    // Orta font (butonlar için)
    hMediumFont = CreateFont(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        TURKISH_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
    
    // Küçük font (metin editörü için)
    hSmallFont = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        TURKISH_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Consolas");
}

void CreateControls(HWND hwnd)
{
    // Hos geldin etiketi
    hLabelWelcome = CreateWindow("STATIC", "Modern Not Defteri'ne Hos Geldiniz!",
        WS_CHILD | SS_CENTER,
        0, 0, 0, 0, hwnd, NULL, GetModuleHandle(NULL), NULL);
    SendMessage(hLabelWelcome, WM_SETFONT, (WPARAM)hBigFont, TRUE);
    
    // Ana menu butonlari
    hBtnNewNote = CreateWindow("BUTTON", "Yeni Not Olustur",
        WS_CHILD | BS_PUSHBUTTON,
        0, 0, 0, 0, hwnd, (HMENU)IDC_BTN_NEW_NOTE, GetModuleHandle(NULL), NULL);
    SendMessage(hBtnNewNote, WM_SETFONT, (WPARAM)hMediumFont, TRUE);
    
    hBtnOldNotes = CreateWindow("BUTTON", "Kayitli Notlarim",
        WS_CHILD | BS_PUSHBUTTON,
        0, 0, 0, 0, hwnd, (HMENU)IDC_BTN_OLD_NOTES, GetModuleHandle(NULL), NULL);
    SendMessage(hBtnOldNotes, WM_SETFONT, (WPARAM)hMediumFont, TRUE);
    
    // Not editörü kontrolleri
    hLabelTitle = CreateWindow("STATIC", "Not Basligi:",
        WS_CHILD, 0, 0, 0, 0, hwnd, NULL, GetModuleHandle(NULL), NULL);
    SendMessage(hLabelTitle, WM_SETFONT, (WPARAM)hMediumFont, TRUE);
    
    hTitleEdit = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "",
        WS_CHILD | ES_AUTOHSCROLL,
        0, 0, 0, 0, hwnd, (HMENU)IDC_TITLE_EDIT, GetModuleHandle(NULL), NULL);
    SendMessage(hTitleEdit, WM_SETFONT, (WPARAM)hMediumFont, TRUE);
    
    hEdit = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "",
        WS_CHILD | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
        0, 0, 0, 0, hwnd, (HMENU)IDC_EDIT, GetModuleHandle(NULL), NULL);
    SendMessage(hEdit, WM_SETFONT, (WPARAM)hSmallFont, TRUE);
    
    // Editör butonları
    hBtnSave = CreateWindow("BUTTON", "Notu Kaydet",
        WS_CHILD | BS_PUSHBUTTON,
        0, 0, 0, 0, hwnd, (HMENU)IDC_BTN_SAVE, GetModuleHandle(NULL), NULL);
    SendMessage(hBtnSave, WM_SETFONT, (WPARAM)hMediumFont, TRUE);
    
    hBtnDeleteCurrent = CreateWindow("BUTTON", "Bu Notu Sil",
        WS_CHILD | BS_PUSHBUTTON,
        0, 0, 0, 0, hwnd, (HMENU)IDC_BTN_DELETE_CURRENT, GetModuleHandle(NULL), NULL);
    SendMessage(hBtnDeleteCurrent, WM_SETFONT, (WPARAM)hMediumFont, TRUE);
    
    hBtnBack = CreateWindow("BUTTON", "Ana Sayfa",
        WS_CHILD | BS_PUSHBUTTON,
        0, 0, 0, 0, hwnd, (HMENU)IDC_BTN_BACK, GetModuleHandle(NULL), NULL);
    SendMessage(hBtnBack, WM_SETFONT, (WPARAM)hMediumFont, TRUE);
    
    // Not listesi
    hListBox = CreateWindowEx(WS_EX_CLIENTEDGE, "LISTBOX", "",
        WS_CHILD | WS_VSCROLL | LBS_NOTIFY,
        0, 0, 0, 0, hwnd, (HMENU)IDC_LISTBOX, GetModuleHandle(NULL), NULL);
    SendMessage(hListBox, WM_SETFONT, (WPARAM)hMediumFont, TRUE);
    
    hBtnDeleteAll = CreateWindow("BUTTON", "Tum Notlari Sil",
        WS_CHILD | BS_PUSHBUTTON,
        0, 0, 0, 0, hwnd, (HMENU)IDC_BTN_DELETE_ALL, GetModuleHandle(NULL), NULL);
    SendMessage(hBtnDeleteAll, WM_SETFONT, (WPARAM)hMediumFont, TRUE);
}

void ShowMainMenu()
{
    currentState = STATE_MAIN_MENU;
    
    // Tüm kontrolleri gizle
    ShowWindow(hLabelWelcome, SW_HIDE);
    ShowWindow(hBtnNewNote, SW_HIDE);
    ShowWindow(hBtnOldNotes, SW_HIDE);
    ShowWindow(hLabelTitle, SW_HIDE);
    ShowWindow(hTitleEdit, SW_HIDE);
    ShowWindow(hEdit, SW_HIDE);
    ShowWindow(hBtnSave, SW_HIDE);
    ShowWindow(hBtnDeleteCurrent, SW_HIDE);
    ShowWindow(hBtnBack, SW_HIDE);
    ShowWindow(hListBox, SW_HIDE);
    ShowWindow(hBtnDeleteAll, SW_HIDE);
    
    // Ana menü kontrollerini göster
    ShowWindow(hLabelWelcome, SW_SHOW);
    ShowWindow(hBtnNewNote, SW_SHOW);
    ShowWindow(hBtnOldNotes, SW_SHOW);
    
    ResizeControls(hMainWindow);
}

void ShowNoteEditor()
{
    currentState = STATE_NOTE_EDITOR;
    
    // Tüm kontrolleri gizle
    ShowWindow(hLabelWelcome, SW_HIDE);
    ShowWindow(hBtnNewNote, SW_HIDE);
    ShowWindow(hBtnOldNotes, SW_HIDE);
    ShowWindow(hListBox, SW_HIDE);
    ShowWindow(hBtnDeleteAll, SW_HIDE);
    
    // Editör kontrollerini göster
    ShowWindow(hLabelTitle, SW_SHOW);
    ShowWindow(hTitleEdit, SW_SHOW);
    ShowWindow(hEdit, SW_SHOW);
    ShowWindow(hBtnSave, SW_SHOW);
    ShowWindow(hBtnDeleteCurrent, SW_SHOW);
    ShowWindow(hBtnBack, SW_SHOW);
    
    SetFocus(hEdit);
    ResizeControls(hMainWindow);
}

void ShowNoteList()
{
    currentState = STATE_NOTE_LIST;
    
    // Tüm kontrolleri gizle
    ShowWindow(hLabelWelcome, SW_HIDE);
    ShowWindow(hBtnNewNote, SW_HIDE);
    ShowWindow(hBtnOldNotes, SW_HIDE);
    ShowWindow(hLabelTitle, SW_HIDE);
    ShowWindow(hTitleEdit, SW_HIDE);
    ShowWindow(hEdit, SW_HIDE);
    ShowWindow(hBtnSave, SW_HIDE);
    ShowWindow(hBtnDeleteCurrent, SW_HIDE);
    
    // Liste kontrollerini göster
    ShowWindow(hListBox, SW_SHOW);
    ShowWindow(hBtnDeleteAll, SW_SHOW);
    ShowWindow(hBtnBack, SW_SHOW);
    
    UpdateNotesList();
    ResizeControls(hMainWindow);
}

void NewNote()
{
    static int noteCounter = 1;
    char title[50];
    sprintf(title, "Yeni Not %d", noteCounter++);
    
    Note newNote(title, "");
    notes.push_back(newNote);
    currentNoteIndex = notes.size() - 1;
    
    SetWindowText(hTitleEdit, title);
    SetWindowText(hEdit, "");
}

void SaveCurrentNote()
{
    if (currentNoteIndex < 0 || currentNoteIndex >= notes.size()) return;
    
    // Başlığı al
    char titleBuffer[256];
    GetWindowText(hTitleEdit, titleBuffer, sizeof(titleBuffer));
    notes[currentNoteIndex].title = titleBuffer;
    
    // İçeriği al
    int textLen = GetWindowTextLength(hEdit);
    char* contentBuffer = new char[textLen + 1];
    GetWindowText(hEdit, contentBuffer, textLen + 1);
    notes[currentNoteIndex].content = contentBuffer;
    
    // Dosya adını güncelle
    notes[currentNoteIndex].filename = notes[currentNoteIndex].title + ".txt";
    for (char& ch : notes[currentNoteIndex].filename) {
        if (ch == '\\' || ch == '/' || ch == ':' || ch == '*' || 
            ch == '?' || ch == '"' || ch == '<' || ch == '>' || ch == '|') {
            ch = '_';
        }
    }
    
    // Dosyaya kaydet
    std::string fullPath = GetNotesDirectory() + "\\" + notes[currentNoteIndex].filename;
    std::ofstream file(fullPath);
    if (file.is_open()) {
        file << notes[currentNoteIndex].content;
        file.close();
    }
    
    delete[] contentBuffer;
}

void LoadNote(int index)
{
    if (index < 0 || index >= notes.size()) return;
    
    currentNoteIndex = index;
    SetWindowText(hTitleEdit, notes[index].title.c_str());
    SetWindowText(hEdit, notes[index].content.c_str());
}

void LoadAllNotes()
{
    notes.clear();
    std::string notesDir = GetNotesDirectory();
    
    WIN32_FIND_DATA findData;
    std::string searchPath = notesDir + "\\*.txt";
    HANDLE hFind = FindFirstFile(searchPath.c_str(), &findData);
    
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            std::string filename = findData.cFileName;
            std::string fullPath = notesDir + "\\" + filename;
            
            std::ifstream file(fullPath);
            if (file.is_open()) {
                std::stringstream buffer;
                buffer << file.rdbuf();
                
                std::string title = filename.substr(0, filename.find_last_of('.'));
                Note note(title, buffer.str());
                notes.push_back(note);
                
                file.close();
            }
        } while (FindNextFile(hFind, &findData));
        FindClose(hFind);
    }
}

void DeleteCurrentNote()
{
    if (currentNoteIndex < 0 || currentNoteIndex >= notes.size()) return;
    
    int result = MessageBox(hMainWindow, 
        "Bu notu silmek istediginizden emin misiniz?",
        "Not Sil", MB_YESNO | MB_ICONQUESTION);
        
    if (result == IDYES) {
        // Dosyayı sil
        std::string fullPath = GetNotesDirectory() + "\\" + notes[currentNoteIndex].filename;
        DeleteFile(fullPath.c_str());
        
        // Listeden çıkar
        notes.erase(notes.begin() + currentNoteIndex);
        currentNoteIndex = -1;
        
        ShowMainMenu();
    }
}

void DeleteAllNotes()
{
    int result = MessageBox(hMainWindow, 
        "Tüm notları silmek istediginizden emin misiniz?",
        "Tüm Notları Sil", MB_YESNO | MB_ICONQUESTION);
        
    if (result == IDYES) {
        // Tüm dosyaları sil
        for (const auto& note : notes) {
            std::string fullPath = GetNotesDirectory() + "\\" + note.filename;
            DeleteFile(fullPath.c_str());
        }
        
        notes.clear();
        currentNoteIndex = -1;
        ShowMainMenu();
    }
}

void UpdateNotesList()
{
    SendMessage(hListBox, LB_RESETCONTENT, 0, 0);
    
    for (size_t i = 0; i < notes.size(); i++) {
        SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)notes[i].title.c_str());
    }
}

void ResizeControls(HWND hwnd)
{
    RECT rect;
    GetClientRect(hwnd, &rect);
    int width = rect.right;
    int height = rect.bottom;
    
    if (currentState == STATE_MAIN_MENU) {
        // Ana menu duzeni
        SetWindowPos(hLabelWelcome, NULL, 50, height/4, width-100, 50, SWP_NOZORDER);
        SetWindowPos(hBtnNewNote, NULL, width/2-120, height/2-30, 240, 60, SWP_NOZORDER);
        SetWindowPos(hBtnOldNotes, NULL, width/2-120, height/2+50, 240, 60, SWP_NOZORDER);
    }
    else if (currentState == STATE_NOTE_EDITOR) {
        // Not editoru duzeni
        SetWindowPos(hLabelTitle, NULL, 20, 20, 100, 25, SWP_NOZORDER);
        SetWindowPos(hTitleEdit, NULL, 130, 20, width-280, 30, SWP_NOZORDER);
        SetWindowPos(hEdit, NULL, 20, 60, width-40, height-150, SWP_NOZORDER);
        SetWindowPos(hBtnSave, NULL, 20, height-80, 140, 50, SWP_NOZORDER);
        SetWindowPos(hBtnDeleteCurrent, NULL, 180, height-80, 140, 50, SWP_NOZORDER);
        SetWindowPos(hBtnBack, NULL, width-140, height-80, 120, 50, SWP_NOZORDER);
    }
    else if (currentState == STATE_NOTE_LIST) {
        // Not listesi duzeni
        SetWindowPos(hListBox, NULL, 20, 20, width-40, height-100, SWP_NOZORDER);
        SetWindowPos(hBtnDeleteAll, NULL, 20, height-80, 160, 50, SWP_NOZORDER);
        SetWindowPos(hBtnBack, NULL, width-140, height-80, 120, 50, SWP_NOZORDER);
    }
}

std::string GetNotesDirectory()
{
    char buffer[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, buffer);
    return std::string(buffer) + "\\MyNotes";
}
