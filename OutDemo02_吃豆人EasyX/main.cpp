#include <graphics.h>
#include <windows.h>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cstring>

// A self-contained Pac-Man game for Dev-C++ 5.11 and EasyX.
// No image or sound files are needed.

const int COLS = 21;
const int ROWS = 21;
const int CELL = 28;
const int BOARD_X = 86;
const int BOARD_Y = 102;
const int WIN_W = 760;
const int WIN_H = 760;

const char *MAZE[ROWS] = {
    "#####################",
    "#.........#.........#",
    "#.###.###.#.###.###.#",
    "#o..#.....#.....#..o#",
    "###.#.###...###.#.###",
    "#...#...#...#...#...#",
    "#.#####.#.#.#.#####.#",
    "#.......#.#.#.......#",
    "#.###.###...###.###.#",
    "#...#...........#...#",
    "###.#.##.....##.#.###",
    "#...#...........#...#",
    "#.###.###...###.###.#",
    "#.......#.#.#.......#",
    "#.#####.#.#.#.#####.#",
    "#...#...#...#...#...#",
    "###.#.###...###.#.###",
    "#o..#.....#.....#..o#",
    "#.###.###.#.###.###.#",
    "#.........#.........#",
    "#####################"
};

enum Direction { UP = 0, RIGHT = 1, DOWN = 2, LEFT = 3, NONE = 4 };
enum Mode { TITLE, PLAYING, PAUSED, CLEARED, GAME_OVER };

const int DX[4] = { 0, 1, 0, -1 };
const int DY[4] = { -1, 0, 1, 0 };
const COLORREF BG = RGB(6, 10, 27);
const COLORREF FLOOR = RGB(10, 16, 40);
const COLORREF GOLD = RGB(255, 216, 67);

struct Ghost {
    int x, y, homeX, homeY, dir;
    COLORREF color;
    DWORD safeUntil;
};

char board[ROWS][COLS + 1];
Ghost ghosts[4];
int playerX, playerY, playerDir, wantedDir;
int score = 0, lives = 3, level = 1, dotsLeft = 0;
Mode mode = TITLE;
DWORD powerUntil = 0, invulnerableUntil = 0;
DWORD lastPlayerStep = 0, lastGhostStep = 0;
DWORD pauseStarted = 0;
bool previousKey[256] = { false };

bool isDown(int key) {
    return (GetAsyncKeyState(key) & 0x8000) != 0;
}

bool justPressed(int key) {
    bool down = isDown(key);
    bool pressed = down && !previousKey[key];
    previousKey[key] = down;
    return pressed;
}

bool timerActive(DWORD until, DWORD now) {
    return until != 0 && static_cast<LONG>(until - now) > 0;
}

bool openTile(int x, int y) {
    return x >= 0 && x < COLS && y >= 0 && y < ROWS && board[y][x] != '#';
}

void resetPositions(DWORD now) {
    playerX = 10;
    playerY = 15;
    playerDir = LEFT;
    wantedDir = LEFT;

    const int hx[4] = { 9, 10, 11, 10 };
    const int hy[4] = { 10, 10, 10, 9 };
    const COLORREF colors[4] = {
        RGB(255, 81, 113), RGB(255, 162, 214),
        RGB(66, 220, 240), RGB(255, 170, 74)
    };
    for (int i = 0; i < 4; ++i) {
        ghosts[i].x = ghosts[i].homeX = hx[i];
        ghosts[i].y = ghosts[i].homeY = hy[i];
        ghosts[i].dir = i;
        ghosts[i].color = colors[i];
        ghosts[i].safeUntil = 0;
    }
    powerUntil = 0;
    invulnerableUntil = now + 1600;
    lastPlayerStep = lastGhostStep = now;
}

void initLevel(bool newGame) {
    if (newGame) {
        score = 0;
        lives = 3;
        level = 1;
    }
    for (int y = 0; y < ROWS; ++y) {
        std::strcpy(board[y], MAZE[y]);
    }
    board[15][10] = ' ';
    board[10][9] = board[10][10] = board[10][11] = ' ';
    board[9][10] = ' ';
    dotsLeft = 0;
    for (int y = 0; y < ROWS; ++y) {
        for (int x = 0; x < COLS; ++x) {
            if (board[y][x] == '.' || board[y][x] == 'o') ++dotsLeft;
        }
    }
    resetPositions(GetTickCount());
}

void eatTile(DWORD now) {
    char &tile = board[playerY][playerX];
    if (tile == '.') {
        score += 10;
        --dotsLeft;
        tile = ' ';
    } else if (tile == 'o') {
        score += 50;
        --dotsLeft;
        tile = ' ';
        powerUntil = now + 7000;
    }
    if (dotsLeft == 0) mode = CLEARED;
}

void checkCollision(DWORD now) {
    if (mode != PLAYING) return;
    for (int i = 0; i < 4; ++i) {
        Ghost &g = ghosts[i];
        if (g.x != playerX || g.y != playerY || timerActive(g.safeUntil, now)) continue;
        if (timerActive(powerUntil, now)) {
            score += 200;
            g.x = g.homeX;
            g.y = g.homeY;
            g.safeUntil = now + 1200;
        } else if (!timerActive(invulnerableUntil, now)) {
            --lives;
            if (lives == 0) mode = GAME_OVER;
            else resetPositions(now);
            return;
        }
    }
}

void movePlayer(DWORD now) {
    if (wantedDir != NONE && openTile(playerX + DX[wantedDir], playerY + DY[wantedDir])) {
        playerDir = wantedDir;
    }
    if (openTile(playerX + DX[playerDir], playerY + DY[playerDir])) {
        playerX += DX[playerDir];
        playerY += DY[playerDir];
        eatTile(now);
        checkCollision(now);
    }
}

int ghostDirection(const Ghost &g, int index, DWORD now) {
    int choices[4], count = 0;
    for (int d = 0; d < 4; ++d) {
        if (openTile(g.x + DX[d], g.y + DY[d])) choices[count++] = d;
    }
    if (count == 0) return NONE;
    int candidates[4], n = 0;
    for (int i = 0; i < count; ++i) {
        if (count == 1 || choices[i] != (g.dir + 2) % 4) candidates[n++] = choices[i];
    }
    if (n == 0) {
        for (int i = 0; i < count; ++i) candidates[n++] = choices[i];
    }

    // Each ghost has a slightly different pursuit style.
    if (index == 2 && std::rand() % 100 < 35) return candidates[std::rand() % n];
    int targetX = playerX, targetY = playerY;
    if (index == 1) {
        targetX += DX[playerDir] * 3;
        targetY += DY[playerDir] * 3;
    } else if (index == 3 && std::abs(g.x - playerX) + std::abs(g.y - playerY) > 7) {
        targetX = 1;
        targetY = 19;
    }

    bool frightened = timerActive(powerUntil, now);
    int best = candidates[0], bestValue = frightened ? -100000 : 100000;
    for (int i = 0; i < n; ++i) {
        int d = candidates[i];
        int distance = std::abs(g.x + DX[d] - targetX) +
                       std::abs(g.y + DY[d] - targetY);
        int value = distance * 20 + std::rand() % 12;
        if ((frightened && value > bestValue) || (!frightened && value < bestValue)) {
            best = d;
            bestValue = value;
        }
    }
    return best;
}

void moveGhosts(DWORD now) {
    for (int i = 0; i < 4 && mode == PLAYING; ++i) {
        Ghost &g = ghosts[i];
        int d = ghostDirection(g, i, now);
        if (d != NONE) {
            g.dir = d;
            g.x += DX[d];
            g.y += DY[d];
        }
        checkCollision(now);
    }
}

void handleInput() {
    if (isDown(VK_UP) || isDown('W')) wantedDir = UP;
    else if (isDown(VK_RIGHT) || isDown('D')) wantedDir = RIGHT;
    else if (isDown(VK_DOWN) || isDown('S')) wantedDir = DOWN;
    else if (isDown(VK_LEFT) || isDown('A')) wantedDir = LEFT;

    if (justPressed('R')) {
        initLevel(true);
        mode = PLAYING;
    }
    if (justPressed(VK_RETURN)) {
        if (mode == TITLE) {
            DWORD now = GetTickCount();
            invulnerableUntil = now + 1600;
            lastPlayerStep = lastGhostStep = now;
            mode = PLAYING;
        }
        else if (mode == CLEARED) {
            ++level;
            initLevel(false);
            mode = PLAYING;
        } else if (mode == GAME_OVER) {
            initLevel(true);
            mode = PLAYING;
        }
    }
    if (justPressed('P')) {
        if (mode == PLAYING) {
            pauseStarted = GetTickCount();
            mode = PAUSED;
        }
        else if (mode == PAUSED) {
            DWORD now = GetTickCount();
            DWORD pauseTime = now - pauseStarted;
            if (timerActive(powerUntil, pauseStarted)) powerUntil += pauseTime;
            if (timerActive(invulnerableUntil, pauseStarted)) invulnerableUntil += pauseTime;
            for (int i = 0; i < 4; ++i) {
                if (timerActive(ghosts[i].safeUntil, pauseStarted)) {
                    ghosts[i].safeUntil += pauseTime;
                }
            }
            mode = PLAYING;
            lastPlayerStep = lastGhostStep = now;
        }
    }
}

void drawGhost(const Ghost &g, DWORD now) {
    int cx = BOARD_X + g.x * CELL + CELL / 2;
    int cy = BOARD_Y + g.y * CELL + CELL / 2;
    bool frightened = timerActive(powerUntil, now) && !timerActive(g.safeUntil, now);
    COLORREF color = frightened ? RGB(69, 93, 255) : g.color;
    if (frightened && static_cast<LONG>(powerUntil - now) < 1800 && (now / 170) % 2) {
        color = RGB(220, 227, 255);
    }
    setfillcolor(color);
    solidcircle(cx, cy - 3, 10);
    solidrectangle(cx - 10, cy - 3, cx + 10, cy + 8);
    POINT hem[8] = {
        { cx - 10, cy + 7 }, { cx - 7, cy + 11 },
        { cx - 3, cy + 7 }, { cx, cy + 11 },
        { cx + 3, cy + 7 }, { cx + 7, cy + 11 },
        { cx + 10, cy + 7 }, { cx + 10, cy + 4 }
    };
    solidpolygon(hem, 8);
    if (frightened) {
        setfillcolor(WHITE);
        solidcircle(cx - 4, cy - 1, 2);
        solidcircle(cx + 4, cy - 1, 2);
    } else {
        setfillcolor(WHITE);
        solidcircle(cx - 4, cy - 3, 4);
        solidcircle(cx + 4, cy - 3, 4);
        setfillcolor(RGB(30, 52, 151));
        solidcircle(cx - 4 + DX[g.dir], cy - 3 + DY[g.dir], 2);
        solidcircle(cx + 4 + DX[g.dir], cy - 3 + DY[g.dir], 2);
    }
}

void drawPlayer(DWORD now) {
    if (timerActive(invulnerableUntil, now) && (now / 120) % 2) return;
    int cx = BOARD_X + playerX * CELL + CELL / 2;
    int cy = BOARD_Y + playerY * CELL + CELL / 2;
    setfillcolor(GOLD);
    solidcircle(cx, cy, 11);
    int opening = ((now / 120) % 2) ? 5 : 8;
    POINT mouth[3] = {
        { cx, cy },
        { cx + DX[playerDir] * 13 + DY[playerDir] * opening,
          cy + DY[playerDir] * 13 - DX[playerDir] * opening },
        { cx + DX[playerDir] * 13 - DY[playerDir] * opening,
          cy + DY[playerDir] * 13 + DX[playerDir] * opening }
    };
    setfillcolor(FLOOR);
    solidpolygon(mouth, 3);
}

void drawMaze(DWORD now) {
    setfillcolor(FLOOR);
    solidroundrect(BOARD_X - 8, BOARD_Y - 8,
                   BOARD_X + COLS * CELL + 8, BOARD_Y + ROWS * CELL + 8, 18, 18);
    for (int y = 0; y < ROWS; ++y) {
        for (int x = 0; x < COLS; ++x) {
            int px = BOARD_X + x * CELL;
            int py = BOARD_Y + y * CELL;
            if (board[y][x] == '#') {
                setfillcolor(RGB(21, 51, 144));
                solidroundrect(px + 1, py + 1, px + CELL - 1, py + CELL - 1, 8, 8);
                setfillcolor(RGB(24, 72, 194));
                solidroundrect(px + 4, py + 4, px + CELL - 4, py + CELL - 4, 7, 7);
            } else if (board[y][x] == '.') {
                setfillcolor(RGB(255, 203, 171));
                solidcircle(px + CELL / 2, py + CELL / 2, 3);
            } else if (board[y][x] == 'o') {
                setfillcolor(GOLD);
                int radius = (now / 280) % 2 ? 6 : 7;
                solidcircle(px + CELL / 2, py + CELL / 2, radius);
            }
        }
    }
}

void drawHUD(DWORD now) {
    setbkcolor(BG);
    cleardevice();
    setbkmode(TRANSPARENT);
    settextstyle(37, 0, "Arial");
    settextcolor(GOLD);
    outtextxy(BOARD_X, 18, "PAC-MAN");

    char text[100];
    settextstyle(18, 0, "Consolas");
    settextcolor(RGB(210, 226, 255));
    std::sprintf(text, "SCORE  %06d", score);
    outtextxy(BOARD_X + 315, 22, text);
    std::sprintf(text, "LEVEL  %d", level);
    outtextxy(BOARD_X + 315, 49, text);
    settextstyle(16, 0, "Consolas");
    outtextxy(BOARD_X, 70, "LIVES");
    for (int i = 0; i < lives; ++i) {
        setfillcolor(GOLD);
        solidcircle(BOARD_X + 75 + i * 25, 79, 8);
    }

    drawMaze(now);
    for (int i = 0; i < 4; ++i) drawGhost(ghosts[i], now);
    drawPlayer(now);

    settextstyle(15, 0, "Consolas");
    settextcolor(RGB(161, 181, 224));
    outtextxy(BOARD_X, 713, "WASD / ARROWS  MOVE    P  PAUSE    R  RESTART    ESC  QUIT");
    if (timerActive(powerUntil, now) && mode == PLAYING) {
        int seconds = static_cast<int>((powerUntil - now + 999) / 1000);
        std::sprintf(text, "POWER %ds", seconds);
        settextcolor(RGB(139, 162, 255));
        outtextxy(BOARD_X + 472, 72, text);
    }
}

void drawOverlay(const char *headline, const char *line1, const char *line2) {
    const int left = 145, top = 288, right = 615, bottom = 477;
    setfillcolor(RGB(12, 22, 59));
    solidroundrect(left, top, right, bottom, 22, 22);
    setlinecolor(RGB(68, 117, 238));
    roundrect(left, top, right, bottom, 22, 22);
    setbkmode(TRANSPARENT);
    settextcolor(GOLD);
    settextstyle(38, 0, "Arial");
    outtextxy(left + 38, top + 28, headline);
    settextstyle(20, 0, "Consolas");
    settextcolor(RGB(224, 233, 255));
    outtextxy(left + 38, top + 102, line1);
    settextstyle(16, 0, "Consolas");
    settextcolor(RGB(160, 185, 242));
    outtextxy(left + 38, top + 144, line2);
}

void render(DWORD now) {
    drawHUD(now);
    if (mode == TITLE) {
        drawOverlay("PAC-MAN", "Eat dots. Dodge ghosts.", "ENTER start  |  P pause  |  ESC quit");
    } else if (mode == PAUSED) {
        drawOverlay("PAUSED", "Take your time.", "Press P to continue");
    } else if (mode == CLEARED) {
        drawOverlay("MAZE CLEARED!", "All dots collected!", "ENTER next level  |  R new game");
    } else if (mode == GAME_OVER) {
        drawOverlay("GAME OVER", "The ghosts caught you.", "ENTER or R to try again");
    }
    FlushBatchDraw();
}

int main() {
    std::srand(static_cast<unsigned int>(std::time(NULL)));
    initLevel(true);
    mode = TITLE;
    HWND window = initgraph(WIN_W, WIN_H);
    BeginBatchDraw();
    while (IsWindow(window)) {
        if (justPressed(VK_ESCAPE)) break;
        handleInput();
        DWORD now = GetTickCount();
        if (mode == PLAYING) {
            int playerDelay = std::max(70, 135 - (level - 1) * 4);
            int ghostDelay = std::max(95, 200 - (level - 1) * 9);
            if (now - lastPlayerStep >= static_cast<DWORD>(playerDelay)) {
                lastPlayerStep = now;
                movePlayer(now);
            }
            if (mode == PLAYING && now - lastGhostStep >= static_cast<DWORD>(ghostDelay)) {
                lastGhostStep = now;
                moveGhosts(now);
            }
        }
        render(mode == PAUSED ? pauseStarted : now);
        Sleep(16);
    }
    EndBatchDraw();
    closegraph();
    return 0;
}
