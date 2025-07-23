#include <windows.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
//doruk yıldırım 2019 eylül
// Game constants
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 650
#define PADDLE_WIDTH 100
#define PADDLE_HEIGHT 20
#define BALL_SIZE 20
#define PADDLE_SPEED 12
#define PADDLE_SPEED_FAST 24

// Score multipliers for each difficulty level
const int SCORE_MULTIPLIERS[] = {1, 2, 3, 4, 5};

// Game states
enum GameState {
    MENU_STATE,
    GAME_STATE,
    GAME_OVER_STATE
};

// Game objects
struct Ball {
    float x, y;
    float dx, dy;
    float speed;
};

struct Paddle {
    float x, y;
};

// Global variables
HWND g_hWnd;
GameState g_gameState = MENU_STATE;
int g_difficulty = 0; // 0: Easy, 1: Medium, 2: Hard, 3: Expert, 4: Insane
int g_score = 0;
int g_highScore = 0;
bool g_keys[256] = {false};
int g_scoreMultiplier = 1;

Ball g_ball;
Paddle g_paddle;

// Menu button rectangles
RECT g_button1 = {300, 240, 500, 280};
RECT g_button2 = {300, 290, 500, 330};
RECT g_button3 = {300, 340, 500, 380};
RECT g_button4 = {300, 390, 500, 430};
RECT g_button5 = {300, 440, 500, 480};

// Function declarations
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void InitGame();
void UpdateGame();
void RenderGame(HDC hdc);
void RenderMenu(HDC hdc);
void RenderGameOver(HDC hdc);
void ResetBall();
void LoadHighScore();
void SaveHighScore();

// Window procedure
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            srand(time(NULL));
            LoadHighScore();
            InitGame();
            SetTimer(hwnd, 1, 8, NULL); // ~120 FPS
            break;
            
        case WM_TIMER:
            if (g_gameState == GAME_STATE) {
                UpdateGame();
            }
            InvalidateRect(hwnd, NULL, TRUE);
            break;
            
        case WM_ERASEBKGND:
            // Prevent background erasing to reduce flicker
            return 1;
            
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            // Get client rect for proper sizing
            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            
            // Create memory DC for double buffering
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP memBitmap = CreateCompatibleBitmap(hdc, clientRect.right, clientRect.bottom);
            HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);
            
            // Clear background with solid color
            HBRUSH bgBrush = CreateSolidBrush(RGB(240, 240, 240));
            FillRect(memDC, &clientRect, bgBrush);
            DeleteObject(bgBrush);
            
            // Render game state
            switch (g_gameState) {
                case MENU_STATE:
                    RenderMenu(memDC);
                    break;
                case GAME_STATE:
                    RenderGame(memDC);
                    break;
                case GAME_OVER_STATE:
                    RenderGameOver(memDC);
                    break;
            }
            
            // Copy to screen
            BitBlt(hdc, 0, 0, clientRect.right, clientRect.bottom, memDC, 0, 0, SRCCOPY);
            
            // Clean up
            SelectObject(memDC, oldBitmap);
            DeleteObject(memBitmap);
            DeleteDC(memDC);
            
            EndPaint(hwnd, &ps);
            break;
        }
        
        case WM_LBUTTONDOWN: {
            if (g_gameState == MENU_STATE) {
                POINT pt = {LOWORD(lParam), HIWORD(lParam)};
                
                if (PtInRect(&g_button1, pt)) {
                    g_difficulty = 0;
                    g_gameState = GAME_STATE;
                    InitGame();
                } else if (PtInRect(&g_button2, pt)) {
                    g_difficulty = 1;
                    g_gameState = GAME_STATE;
                    InitGame();
                } else if (PtInRect(&g_button3, pt)) {
                    g_difficulty = 2;
                    g_gameState = GAME_STATE;
                    InitGame();
                } else if (PtInRect(&g_button4, pt)) {
                    g_difficulty = 3;
                    g_gameState = GAME_STATE;
                    InitGame();
                } else if (PtInRect(&g_button5, pt)) {
                    g_difficulty = 4;
                    g_gameState = GAME_STATE;
                    InitGame();
                }
            }
            break;
        }
        
        case WM_KEYDOWN:
            g_keys[wParam] = true;
            
            if (g_gameState == MENU_STATE) {
                if (wParam == '1') {
                    g_difficulty = 0;
                    g_gameState = GAME_STATE;
                    InitGame();
                } else if (wParam == '2') {
                    g_difficulty = 1;
                    g_gameState = GAME_STATE;
                    InitGame();
                } else if (wParam == '3') {
                    g_difficulty = 2;
                    g_gameState = GAME_STATE;
                    InitGame();
                } else if (wParam == '4') {
                    g_difficulty = 3;
                    g_gameState = GAME_STATE;
                    InitGame();
                } else if (wParam == '5') {
                    g_difficulty = 4;
                    g_gameState = GAME_STATE;
                    InitGame();
                }
            } else if (g_gameState == GAME_OVER_STATE) {
                if (wParam == VK_SPACE) {
                    g_gameState = MENU_STATE;
                }
            }
            break;
            
        case WM_KEYUP:
            g_keys[wParam] = false;
            break;
            
        case WM_DESTROY:
            SaveHighScore();
            PostQuitMessage(0);
            break;
            
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

void InitGame() {
    g_score = 0;
    g_scoreMultiplier = SCORE_MULTIPLIERS[g_difficulty];
    
    // Initialize paddle
    g_paddle.x = (WINDOW_WIDTH - PADDLE_WIDTH) / 2.0f;
    g_paddle.y = WINDOW_HEIGHT - 50;
    
    // Initialize ball
    ResetBall();
}

void ResetBall() {
    g_ball.x = WINDOW_WIDTH / 2.0f;
    g_ball.y = WINDOW_HEIGHT / 2.0f;
    g_ball.speed = 3.0f + (g_difficulty * 2.0f);
    
    // Random direction
    float angle = (rand() % 120 + 30) * 3.14159f / 180.0f; // 30-150 degrees
    g_ball.dx = cos(angle) * g_ball.speed;
    g_ball.dy = sin(angle) * g_ball.speed;
}

void UpdateGame() {
    // Update paddle
    float currentSpeed = (g_keys[VK_SHIFT]) ? PADDLE_SPEED_FAST : PADDLE_SPEED;
    
    if (g_keys['A'] || g_keys['a']) {
        g_paddle.x -= currentSpeed;
        if (g_paddle.x < 0) g_paddle.x = 0;
    }
    if (g_keys['D'] || g_keys['d']) {
        g_paddle.x += currentSpeed;
        if (g_paddle.x > WINDOW_WIDTH - PADDLE_WIDTH) {
            g_paddle.x = WINDOW_WIDTH - PADDLE_WIDTH;
        }
    }
    
    // Update ball
    g_ball.x += g_ball.dx;
    g_ball.y += g_ball.dy;
    
    // Ball collision with walls
    if (g_ball.x <= 0 || g_ball.x >= WINDOW_WIDTH - BALL_SIZE) {
        g_ball.dx = -g_ball.dx;
        if (g_ball.x <= 0) g_ball.x = 0;
        if (g_ball.x >= WINDOW_WIDTH - BALL_SIZE) g_ball.x = WINDOW_WIDTH - BALL_SIZE;
    }
    
    if (g_ball.y <= 0) {
        g_ball.dy = -g_ball.dy;
        g_ball.y = 0;
    }
    
    // Ball collision with paddle
    if (g_ball.y + BALL_SIZE >= g_paddle.y && 
        g_ball.y <= g_paddle.y + PADDLE_HEIGHT &&
        g_ball.x + BALL_SIZE >= g_paddle.x && 
        g_ball.x <= g_paddle.x + PADDLE_WIDTH &&
        g_ball.dy > 0) { // Only count when ball is moving down
        
        g_ball.dy = -abs(g_ball.dy); // Always bounce up
        g_score += g_scoreMultiplier; // Add score based on difficulty multiplier
        
        // Add horizontal variation based on where ball hits paddle
        float hitPos = (g_ball.x + BALL_SIZE/2 - g_paddle.x) / PADDLE_WIDTH;
        
        if (g_difficulty == 4) { // Triangular paddle - more unpredictable
            // More extreme angles for triangular paddle
            if (hitPos < 0.3f) {
                g_ball.dx = -g_ball.speed * (1.5f + (rand() % 100) / 100.0f);
            } else if (hitPos > 0.7f) {
                g_ball.dx = g_ball.speed * (1.5f + (rand() % 100) / 100.0f);
            } else {
                // Center hit - random direction
                g_ball.dx = (rand() % 200 - 100) / 50.0f * g_ball.speed;
            }
        } else {
            g_ball.dx = (hitPos - 0.5f) * g_ball.speed * 2;
        }
        
        // Ensure ball doesn't get stuck in paddle
        g_ball.y = g_paddle.y - BALL_SIZE;
        
        // Update high score
        if (g_score > g_highScore) {
            g_highScore = g_score;
        }
    }
    
    // Game over if ball passes paddle
    if (g_ball.y > WINDOW_HEIGHT) {
        g_gameState = GAME_OVER_STATE;
    }
}

void RenderGame(HDC hdc) {
    // Set up drawing
    HPEN ballPen = CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
    HPEN paddlePen = CreatePen(PS_SOLID, 2, RGB(0, 0, 255));
    HBRUSH ballBrush = CreateSolidBrush(RGB(255, 100, 100));
    HBRUSH paddleBrush = CreateSolidBrush(RGB(100, 100, 255));
    
    // Draw ball
    SelectObject(hdc, ballPen);
    SelectObject(hdc, ballBrush);
    Ellipse(hdc, (int)g_ball.x, (int)g_ball.y, 
            (int)g_ball.x + BALL_SIZE, (int)g_ball.y + BALL_SIZE);
    
    // Draw paddle
    SelectObject(hdc, paddlePen);
    SelectObject(hdc, paddleBrush);
    
    if (g_difficulty == 4) { // Triangular paddle for hardest difficulty
        POINT triangle[3];
        triangle[0].x = (int)g_paddle.x + PADDLE_WIDTH/2; // Top point
        triangle[0].y = (int)g_paddle.y;
        triangle[1].x = (int)g_paddle.x; // Bottom left
        triangle[1].y = (int)g_paddle.y + PADDLE_HEIGHT;
        triangle[2].x = (int)g_paddle.x + PADDLE_WIDTH; // Bottom right
        triangle[2].y = (int)g_paddle.y + PADDLE_HEIGHT;
        Polygon(hdc, triangle, 3);
    } else {
        Rectangle(hdc, (int)g_paddle.x, (int)g_paddle.y,
                  (int)g_paddle.x + PADDLE_WIDTH, (int)g_paddle.y + PADDLE_HEIGHT);
    }
    
    // Draw score
    SetTextColor(hdc, RGB(0, 0, 0));
    SetBkMode(hdc, TRANSPARENT);
    char scoreText[150];
    const char* difficultyNames[] = {"Kolay", "Orta", "Zor", "Uzman", "Imkansiz"};
    sprintf(scoreText, "Skor: %d (x%d)   En Yuksek: %d   Zorluk: %s", 
            g_score, g_scoreMultiplier, g_highScore, difficultyNames[g_difficulty]);
    TextOut(hdc, 10, 10, scoreText, strlen(scoreText));
    
    // Draw controls
    char controls[100];
    if (g_keys[VK_SHIFT]) {
        sprintf(controls, "A/D: Hareket   SHIFT: Hizli mod AKTIF!");
    } else {
        sprintf(controls, "A/D: Hareket   SHIFT: Hizli mod");
    }
    TextOut(hdc, 10, WINDOW_HEIGHT - 30, controls, strlen(controls));
    
    // Clean up
    DeleteObject(ballPen);
    DeleteObject(paddlePen);
    DeleteObject(ballBrush);
    DeleteObject(paddleBrush);
}

void RenderMenu(HDC hdc) {
    SetTextColor(hdc, RGB(0, 0, 0));
    SetBkMode(hdc, TRANSPARENT);
    
    // Title
    HFONT titleFont = CreateFont(48, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Arial");
    SelectObject(hdc, titleFont);
    
    char title[] = "TOP OYUNU";
    RECT titleRect = {0, 100, WINDOW_WIDTH, 200};
    DrawText(hdc, title, -1, &titleRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    
    // Menu buttons
    HFONT menuFont = CreateFont(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                               DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                               DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Arial");
    SelectObject(hdc, menuFont);
    
    // Draw button backgrounds
    HBRUSH buttonBrush = CreateSolidBrush(RGB(220, 220, 220));
    HPEN buttonPen = CreatePen(PS_SOLID, 2, RGB(100, 100, 100));
    SelectObject(hdc, buttonBrush);
    SelectObject(hdc, buttonPen);
    
    Rectangle(hdc, g_button1.left, g_button1.top, g_button1.right, g_button1.bottom);
    Rectangle(hdc, g_button2.left, g_button2.top, g_button2.right, g_button2.bottom);
    Rectangle(hdc, g_button3.left, g_button3.top, g_button3.right, g_button3.bottom);
    Rectangle(hdc, g_button4.left, g_button4.top, g_button4.right, g_button4.bottom);
    Rectangle(hdc, g_button5.left, g_button5.top, g_button5.right, g_button5.bottom);
    
    // Draw button text
    char menu1[100], menu2[100], menu3[100], menu4[100], menu5[100];
    sprintf(menu1, "1 - Kolay (Yavas - %dx Puan)", SCORE_MULTIPLIERS[0]);
    sprintf(menu2, "2 - Orta (Normal - %dx Puan)", SCORE_MULTIPLIERS[1]);
    sprintf(menu3, "3 - Zor (Hizli - %dx Puan)", SCORE_MULTIPLIERS[2]);
    sprintf(menu4, "4 - Uzman (Cok Hizli - %dx Puan)", SCORE_MULTIPLIERS[3]);
    sprintf(menu5, "5 - Imkansiz (Ucgen - %dx Puan)", SCORE_MULTIPLIERS[4]);
    
    DrawText(hdc, menu1, -1, &g_button1, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    DrawText(hdc, menu2, -1, &g_button2, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    DrawText(hdc, menu3, -1, &g_button3, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    DrawText(hdc, menu4, -1, &g_button4, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    DrawText(hdc, menu5, -1, &g_button5, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    
    // High score
    char highScoreText[100];
    sprintf(highScoreText, "En Yuksek Skor: %d", g_highScore);
    RECT scoreRect = {0, 500, WINDOW_WIDTH, 530};
    DrawText(hdc, highScoreText, -1, &scoreRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    
    // Instructions
    HFONT instructFont = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                   DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                   DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Arial");
    SelectObject(hdc, instructFont);
    
    char instruct1[] = "Klavye: 1-5 tuslari   |   Mouse: Butonlara tikla";
    RECT instructRect = {0, 550, WINDOW_WIDTH, 580};
    DrawText(hdc, instruct1, -1, &instructRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    
    DeleteObject(titleFont);
    DeleteObject(menuFont);
    DeleteObject(instructFont);
    DeleteObject(buttonBrush);
    DeleteObject(buttonPen);
}

void RenderGameOver(HDC hdc) {
    SetTextColor(hdc, RGB(255, 0, 0));
    SetBkMode(hdc, TRANSPARENT);
    
    // Game Over title
    HFONT titleFont = CreateFont(48, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Arial");
    SelectObject(hdc, titleFont);
    
    char gameOver[] = "OYUN BITTI!";
    RECT titleRect = {0, 200, WINDOW_WIDTH, 300};
    DrawText(hdc, gameOver, -1, &titleRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    
    // Score
    HFONT scoreFont = CreateFont(24, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Arial");
    SelectObject(hdc, scoreFont);
    
    char scoreText[100];
    sprintf(scoreText, "Skorunuz: %d", g_score);
    RECT scoreRect = {0, 320, WINDOW_WIDTH, 360};
    DrawText(hdc, scoreText, -1, &scoreRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    
    if (g_score == g_highScore && g_score > 0) {
        SetTextColor(hdc, RGB(0, 255, 0));
        char newRecord[] = "YENI REKOR!";
        RECT recordRect = {0, 360, WINDOW_WIDTH, 400};
        DrawText(hdc, newRecord, -1, &recordRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
    
    // Instructions
    SetTextColor(hdc, RGB(0, 0, 0));
    char instruct[] = "Ana menuye donmek icin SPACE tusuna basin";
    RECT instructRect = {0, 450, WINDOW_WIDTH, 490};
    DrawText(hdc, instruct, -1, &instructRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    
    DeleteObject(titleFont);
    DeleteObject(scoreFont);
}

void LoadHighScore() {
    FILE* file = fopen("highscore.txt", "r");
    if (file) {
        fscanf(file, "%d", &g_highScore);
        fclose(file);
    }
}

void SaveHighScore() {
    FILE* file = fopen("highscore.txt", "w");
    if (file) {
        fprintf(file, "%d", g_highScore);
        fclose(file);
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Register window class
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "BallGameWindow";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    
    RegisterClass(&wc);
    
    // Create window
    g_hWnd = CreateWindowEx(
        0,
        "BallGameWindow",
        "Top Oyunu - 5 Zorluk Seviyesi",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT,
        WINDOW_WIDTH + 16, WINDOW_HEIGHT + 39, // Account for window borders
        NULL, NULL, hInstance, NULL
    );
    
    if (!g_hWnd) {
        return 0;
    }
    
    ShowWindow(g_hWnd, nCmdShow);
    UpdateWindow(g_hWnd);
    
    // Message loop
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return 0;
}
