#include <windows.h>
#include <commdlg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <shlobj.h>

// Constants
#define IDC_ENCRYPT_BTN 101
#define IDC_DECRYPT_BTN 102
#define KEY "mykey"  // Fixed key for XOR operation
#define MAX_FILE_SIZE 1048576  // 1MB max file size

// Global variables
HWND hwndButtonEncrypt;
HWND hwndButtonDecrypt;
HFONT hFont;
char desktopPath[MAX_PATH];

// Function prototypes
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
int XORFile(const char* inputFile, const char* outputFile);
void GetDesktopPath();
void ShowError(const char* message);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Get desktop path
    GetDesktopPath();
    if (desktopPath[0] == '\0') {
        MessageBox(NULL, "Failed to get desktop path", "Error", MB_ICONERROR);
        return 1;
    }

    // Register window class
    WNDCLASSEX wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = "XORAppClass";

    if (!RegisterClassEx(&wc)) {
        ShowError("Window registration failed");
        return 1;
    }

    // Create main window
    HWND hwnd = CreateWindowEx(
        0, "XORAppClass", "File Encryption Tool",
        WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
        CW_USEDEFAULT, CW_USEDEFAULT, 400, 250,
        NULL, NULL, hInstance, NULL
    );

    if (!hwnd) {
        ShowError("Window creation failed");
        return 1;
    }

    // Create font
    hFont = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                      DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS,
                      CLEARTYPE_QUALITY, VARIABLE_PITCH, "Arial");

    // Create buttons
    hwndButtonEncrypt = CreateWindow(
        "BUTTON", "Sifrele",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        100, 50, 200, 40, hwnd, (HMENU)IDC_ENCRYPT_BTN,
        hInstance, NULL
    );

    hwndButtonDecrypt = CreateWindow(
        "BUTTON", "Sifre Coz",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        100, 120, 200, 40, hwnd, (HMENU)IDC_DECRYPT_BTN,
        hInstance, NULL
    );

    // Set font for buttons
    SendMessage(hwndButtonEncrypt, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hwndButtonDecrypt, WM_SETFONT, (WPARAM)hFont, TRUE);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // Message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    DeleteObject(hFont);
    return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_COMMAND: {
            if (LOWORD(wParam) == IDC_ENCRYPT_BTN || LOWORD(wParam) == IDC_DECRYPT_BTN) {
                // Configure the open file dialog
                OPENFILENAME ofn;
                char fileName[MAX_PATH] = "";
                char outputFile[MAX_PATH];
                
                ZeroMemory(&ofn, sizeof(ofn));
                ofn.lStructSize = sizeof(ofn);
                ofn.hwndOwner = hwnd;
                ofn.lpstrFile = fileName;
                ofn.nMaxFile = MAX_PATH;
                ofn.lpstrFilter = "Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
                ofn.nFilterIndex = 1;
                ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

                if (GetOpenFileName(&ofn)) {
                    // Create output file path
                    sprintf(outputFile, "%s\\dosya_%s.txt", desktopPath, 
                           (LOWORD(wParam) == IDC_ENCRYPT_BTN ? "sifreli" : "cozulmus"));

                    // Debug: Show the desktop path being used
                    char debugMsg[512];
                    sprintf(debugMsg, "Desktop path: %s\nOutput file: %s", desktopPath, outputFile);
                    MessageBox(hwnd, debugMsg, "Debug Info", MB_ICONINFORMATION | MB_OK);

                    if (XORFile(fileName, outputFile) == 0) {
                        char message[512];
                        sprintf(message, "File successfully %s and saved to:\n%s", 
                               (LOWORD(wParam) == IDC_ENCRYPT_BTN ? "encrypted" : "decrypted"), 
                               outputFile);
                        MessageBox(hwnd, message, "Success", MB_ICONINFORMATION | MB_OK);
                    } else {
                        MessageBox(hwnd, "Error processing file!", "Error", MB_ICONERROR | MB_OK);
                    }
                }
            }
            break;
        }

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        case WM_CTLCOLORBTN: {
            HDC hdc = (HDC)wParam;
            SetBkColor(hdc, RGB(240, 240, 240));
            return (LRESULT)GetStockObject(WHITE_BRUSH);
        }

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int XORFile(const char* inputFile, const char* outputFile) {
    FILE* inFile = fopen(inputFile, "rb");
    if (!inFile) {
        return 1; // Error: could not open input file
    }

    // Get file size
    fseek(inFile, 0, SEEK_END);
    long fileSize = ftell(inFile);
    fseek(inFile, 0, SEEK_SET);

    if (fileSize > MAX_FILE_SIZE || fileSize <= 0) {
        fclose(inFile);
        return 1; // Error: file too large or empty
    }

    // Allocate buffer and read file
    char* buffer = (char*)malloc(fileSize);
    if (!buffer) {
        fclose(inFile);
        return 1; // Error: memory allocation failed
    }

    if (fread(buffer, 1, fileSize, inFile) != fileSize) {
        free(buffer);
        fclose(inFile);
        return 1; // Error: could not read file
    }
    fclose(inFile);

    // XOR operation
    const char* key = KEY;
    int keyLength = strlen(key);
    for (long i = 0; i < fileSize; i++) {
        buffer[i] ^= key[i % keyLength];
    }

    // Write output file
    FILE* outFile = fopen(outputFile, "wb");
    if (!outFile) {
        free(buffer);
        return 1; // Error: could not create output file
    }

    if (fwrite(buffer, 1, fileSize, outFile) != fileSize) {
        free(buffer);
        fclose(outFile);
        return 1; // Error: could not write file
    }

    free(buffer);
    fclose(outFile);
    return 0; // Success
}

void GetDesktopPath() {
    // Try to get desktop path using SHGetSpecialFolderPath
    if (SHGetSpecialFolderPath(NULL, desktopPath, CSIDL_DESKTOP, FALSE)) {
        // Successfully got desktop path
        return;
    }
    
    // Fallback: Try environment variable
    char* userProfile = getenv("USERPROFILE");
    if (userProfile) {
        sprintf(desktopPath, "%s\\Desktop", userProfile);
    } else {
        // Last resort: current directory
        GetCurrentDirectory(MAX_PATH, desktopPath);
    }
}

void ShowError(const char* message) {
    MessageBox(NULL, message, "Error", MB_ICONERROR | MB_OK);
}
