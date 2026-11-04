#include <graphics.h>
#include <windows.h>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <queue>
#include <string>
#include <vector>

// Pac-Man style game for the EasyX-enabled Dev-C++ installation.
// The maze is generated as one connected area, so every dot is reachable.

const int COLS = 25;
const int ROWS = 19;
const int TILE = 24;
const int BOARD_X = 32;
const int BOARD_Y = 98;
const int WINDOW_W = 910;
const int WINDOW_H = 625;
const int DX[4] = { 0, 1, 0, -1 };
const int DY[4] = { -1, 0, 1, 0 };

struct Point {
    int x, y;
    Point(int xx = 0, int yy = 0) : x(xx), y(yy) {}
};

struct Ghost {
    int x, y, homeX, homeY;
    COLORREF color;
};

class PacManGame {
public:
    PacManGame() : quit(false) {
        std::srand(static_cast<unsigned int>(std::time(0)));
        initgraph(WINDOW_W, WINDOW_H);
        window = GetHWnd();
        SetWindowTextA(window, "OutDemo01_ - Pac-Man");
        BeginBatchDraw();
        restart();
    }

    ~PacManGame() {
        EndBatchDraw();
        if (IsWindow(window)) closegraph();
    }

    void run() {
        while (!quit && IsWindow(window)) {
            readInput();
            if (started && !paused && !lost && !won) {
                ++tick;
                movePlayer();
                if (!won) checkCollisions();
                if (!lost && !won && tick % 3 == 0) {
                    moveGhosts();
                    checkCollisions();
                }
                if (powerTicks > 0) --powerTicks;
                if (safeTicks > 0) --safeTicks;
            }
            if (IsWindow(window)) draw();
            Sleep(100);
        }
    }

private:
    HWND window;
    std::vector<std::string> maze;
    std::vector<Ghost> ghosts;
    int playerX, playerY;
    int directionX, directionY, wantedX, wantedY;
    int score, lives, pellets, powerTicks, safeTicks, tick;
    bool started, paused, lost, won, quit;

    bool inside(int x, int y) const {
        return x > 0 && x < COLS - 1 && y > 0 && y < ROWS - 1;
    }

    bool open(int x, int y) const {
        return inside(x, y) && maze[y][x] != '#';
    }

    void makeMaze() {
        maze.assign(ROWS, std::string(COLS, '#'));
        std::vector<Point> stack;
        maze[1][1] = '.';
        stack.push_back(Point(1, 1));
        while (!stack.empty()) {
            Point p = stack.back();
            int order[4] = { 0, 1, 2, 3 };
            for (int i = 3; i > 0; --i) {
                int j = std::rand() % (i + 1);
                std::swap(order[i], order[j]);
            }
            bool found = false;
            for (int i = 0; i < 4; ++i) {
                int d = order[i];
                int nx = p.x + DX[d] * 2, ny = p.y + DY[d] * 2;
                if (inside(nx, ny) && maze[ny][nx] == '#') {
                    maze[p.y + DY[d]][p.x + DX[d]] = '.';
                    maze[ny][nx] = '.';
                    stack.push_back(Point(nx, ny));
                    found = true;
                    break;
                }
            }
            if (!found) stack.pop_back();
        }

        // Extra passages make routes less like a one-way tree.
        int gaps = 0;
        for (int attempt = 0; attempt < 600 && gaps < 34; ++attempt) {
            int x = 1 + std::rand() % (COLS - 2);
            int y = 1 + std::rand() % (ROWS - 2);
            if (maze[y][x] != '#') continue;
            if ((open(x - 1, y) && open(x + 1, y)) ||
                (open(x, y - 1) && open(x, y + 1))) {
                maze[y][x] = '.';
                ++gaps;
            }
        }
    }

    std::vector<int> distances(int x, int y) const {
        std::vector<int> result(COLS * ROWS, -1);
        std::queue<Point> pending;
        result[y * COLS + x] = 0;
        pending.push(Point(x, y));
        while (!pending.empty()) {
            Point p = pending.front();
            pending.pop();
            for (int d = 0; d < 4; ++d) {
                int nx = p.x + DX[d], ny = p.y + DY[d];
                if (open(nx, ny) && result[ny * COLS + nx] == -1) {
                    result[ny * COLS + nx] = result[p.y * COLS + p.x] + 1;
                    pending.push(Point(nx, ny));
                }
            }
        }
        return result;
    }

    void addGhosts() {
        ghosts.clear();
        std::vector<int> dist = distances(1, 1);
        const COLORREF colors[3] = {
            RGB(255, 80, 115), RGB(230, 115, 255), RGB(58, 227, 230)
        };
        for (int i = 0; i < 3; ++i) {
            int best = -100000, bx = -1, by = -1;
            for (int y = 1; y < ROWS - 1; ++y) {
                for (int x = 1; x < COLS - 1; ++x) {
                    if (!open(x, y) || (x == 1 && y == 1)) continue;
                    int value = dist[y * COLS + x];
                    bool used = false;
                    for (size_t j = 0; j < ghosts.size(); ++j) {
                        int gap = std::abs(x - ghosts[j].x) + std::abs(y - ghosts[j].y);
                        if (gap == 0) used = true;
                        if (gap < 6) value -= 100;
                    }
                    if (!used && value > best) {
                        best = value;
                        bx = x;
                        by = y;
                    }
                }
            }
            Ghost g;
            g.x = g.homeX = bx;
            g.y = g.homeY = by;
            g.color = colors[i];
            ghosts.push_back(g);
            maze[by][bx] = ' ';
        }
    }

    void restart() {
        makeMaze();
        playerX = playerY = 1;
        maze[1][1] = ' ';
        addGhosts();
        Point corners[3] = {
            Point(COLS - 2, 1), Point(1, ROWS - 2),
            Point(COLS - 2, ROWS - 2)
        };
        for (int i = 0; i < 3; ++i)
            if (maze[corners[i].y][corners[i].x] == '.')
                maze[corners[i].y][corners[i].x] = 'o';

        pellets = 0;
        for (int y = 0; y < ROWS; ++y)
            for (int x = 0; x < COLS; ++x)
                if (maze[y][x] == '.' || maze[y][x] == 'o') ++pellets;

        directionX = directionY = wantedX = wantedY = 0;
        score = 0;
        lives = 3;
        powerTicks = safeTicks = tick = 0;
        started = paused = lost = won = false;
        flushmessage(EX_KEY);
    }

    void readInput() {
        ExMessage msg;
        while (peekmessage(&msg, EX_KEY)) {
            if (msg.message != WM_KEYDOWN) continue;
            int key = msg.vkcode;
            if (key == VK_ESCAPE) { quit = true; return; }
            if ((key == 'R') && !msg.prevdown) { restart(); return; }
            if ((key == 'P') && !msg.prevdown) { paused = !paused; continue; }
            if (lost || won) continue;
            if (key == VK_UP || key == 'W') { wantedX = 0; wantedY = -1; started = true; }
            if (key == VK_RIGHT || key == 'D') { wantedX = 1; wantedY = 0; started = true; }
            if (key == VK_DOWN || key == 'S') { wantedX = 0; wantedY = 1; started = true; }
            if (key == VK_LEFT || key == 'A') { wantedX = -1; wantedY = 0; started = true; }
        }
    }

    void movePlayer() {
        if ((wantedX != 0 || wantedY != 0) &&
            open(playerX + wantedX, playerY + wantedY)) {
            directionX = wantedX;
            directionY = wantedY;
        }
        int nx = playerX + directionX, ny = playerY + directionY;
        if (!open(nx, ny)) return;
        playerX = nx;
        playerY = ny;
        char &tile = maze[ny][nx];
        if (tile == '.' || tile == 'o') {
            score += tile == 'o' ? 50 : 10;
            if (tile == 'o') powerTicks = 75;
            --pellets;
            tile = ' ';
        }
        if (pellets == 0) won = true;
    }

    void moveGhosts() {
        std::vector<int> dist = distances(playerX, playerY);
        for (size_t i = 0; i < ghosts.size(); ++i) {
            Ghost &g = ghosts[i];
            int choices[4], count = 0;
            for (int d = 0; d < 4; ++d)
                if (open(g.x + DX[d], g.y + DY[d])) choices[count++] = d;
            if (count == 0) continue;
            int chosen = choices[std::rand() % count];
            if (std::rand() % 4 != 0) {
                int best = powerTicks > 0 ? -100000 : 100000;
                for (int k = 0; k < count; ++k) {
                    int d = choices[k];
                    int value = dist[(g.y + DY[d]) * COLS + g.x + DX[d]];
                    if ((powerTicks > 0 && value > best) ||
                        (powerTicks == 0 && value < best)) {
                        best = value;
                        chosen = d;
                    }
                }
            }
            g.x += DX[chosen];
            g.y += DY[chosen];
        }
    }

    void checkCollisions() {
        for (size_t i = 0; i < ghosts.size(); ++i) {
            Ghost &g = ghosts[i];
            if (g.x != playerX || g.y != playerY) continue;
            if (powerTicks > 0) {
                score += 200;
                g.x = g.homeX;
                g.y = g.homeY;
            } else if (safeTicks == 0) {
                --lives;
                if (lives == 0) { lost = true; return; }
                playerX = playerY = 1;
                directionX = directionY = wantedX = wantedY = 0;
                powerTicks = 0;
                safeTicks = 15;
                for (size_t j = 0; j < ghosts.size(); ++j) {
                    ghosts[j].x = ghosts[j].homeX;
                    ghosts[j].y = ghosts[j].homeY;
                }
                return;
            }
        }
    }

    void label(int x, int y, int size, COLORREF color, const char *value) const {
        settextstyle(size, 0, "Arial");
        settextcolor(color);
        outtextxy(x, y, value);
    }

    void drawGhost(const Ghost &g) const {
        int cx = BOARD_X + g.x * TILE + TILE / 2;
        int cy = BOARD_Y + g.y * TILE + TILE / 2;
        setfillcolor(powerTicks > 0 ? RGB(63, 98, 220) : g.color);
        solidcircle(cx, cy - 3, 9);
        solidrectangle(cx - 9, cy - 3, cx + 9, cy + 8);
        solidcircle(cx - 6, cy + 6, 3);
        solidcircle(cx + 6, cy + 6, 3);
        setfillcolor(WHITE);
        solidcircle(cx - 4, cy - 4, 3);
        solidcircle(cx + 4, cy - 4, 3);
        setfillcolor(RGB(20, 38, 85));
        solidcircle(cx - 4, cy - 4, 1);
        solidcircle(cx + 4, cy - 4, 1);
    }

    void drawPlayer() const {
        if (safeTicks > 0 && tick % 2 == 0) return;
        int cx = BOARD_X + playerX * TILE + TILE / 2;
        int cy = BOARD_Y + playerY * TILE + TILE / 2;
        setfillcolor(RGB(255, 218, 65));
        solidcircle(cx, cy, 10);
        int dx = directionX, dy = directionY;
        if (dx == 0 && dy == 0) dx = 1;
        POINT mouth[3];
        mouth[0].x = cx; mouth[0].y = cy;
        mouth[1].x = cx + dx * 11 - dy * 5;
        mouth[1].y = cy + dy * 11 + dx * 5;
        mouth[2].x = cx + dx * 11 + dy * 5;
        mouth[2].y = cy + dy * 11 - dx * 5;
        setfillcolor(RGB(15, 20, 43));
        solidpolygon(mouth, 3);
    }

    void draw() const {
        const COLORREF background = RGB(10, 14, 31);
        const COLORREF white = RGB(236, 239, 255);
        setbkcolor(background);
        cleardevice();
        setbkmode(TRANSPARENT);
        label(32, 20, 34, RGB(255, 218, 65), "PAC-MAN");
        label(215, 33, 17, RGB(160, 177, 214), "OUTDEMO01_  /  EAT ALL DOTS");

        setfillcolor(RGB(15, 20, 43));
        solidroundrect(BOARD_X - 8, BOARD_Y - 8,
                       BOARD_X + COLS * TILE + 8, BOARD_Y + ROWS * TILE + 8, 16, 16);
        for (int y = 0; y < ROWS; ++y) {
            for (int x = 0; x < COLS; ++x) {
                int left = BOARD_X + x * TILE;
                int top = BOARD_Y + y * TILE;
                char tile = maze[y][x];
                if (tile == '#') {
                    setfillcolor(RGB(36, 79, 185));
                    solidroundrect(left + 1, top + 1, left + TILE - 1,
                                   top + TILE - 1, 7, 7);
                } else if (tile == '.') {
                    setfillcolor(RGB(255, 205, 164));
                    solidcircle(left + TILE / 2, top + TILE / 2, 3);
                } else if (tile == 'o') {
                    setfillcolor(RGB(68, 235, 235));
                    solidcircle(left + TILE / 2, top + TILE / 2, 7);
                }
            }
        }
        for (size_t i = 0; i < ghosts.size(); ++i) drawGhost(ghosts[i]);
        drawPlayer();

        setfillcolor(RGB(18, 28, 56));
        solidroundrect(655, 98, 882, 554, 20, 20);
        char buffer[80];
        label(678, 123, 19, RGB(167, 185, 220), "SCORE");
        std::sprintf(buffer, "%d", score);
        label(678, 151, 32, RGB(255, 218, 65), buffer);
        label(678, 206, 19, RGB(167, 185, 220), "LIVES");
        std::sprintf(buffer, "%d", lives);
        label(678, 233, 30, white, buffer);
        label(678, 288, 19, RGB(167, 185, 220), "DOTS LEFT");
        std::sprintf(buffer, "%d", pellets);
        label(678, 315, 30, white, buffer);
        label(678, 371, 19, RGB(167, 185, 220), "POWER");
        std::sprintf(buffer, "%d sec", (powerTicks + 9) / 10);
        label(678, 397, 26, RGB(68, 235, 235), buffer);

        if (lost) label(678, 470, 23, RGB(255, 80, 115), "GAME OVER");
        else if (won) label(678, 470, 23, RGB(255, 218, 65), "YOU WIN!");
        else if (paused) label(678, 470, 23, RGB(255, 218, 65), "PAUSED");
        else if (!started) label(678, 470, 18, RGB(255, 218, 65), "PRESS A KEY");
        else label(678, 470, 18, white, "CHASE THE DOTS");

        label(32, 575, 19, white, "ARROWS / WASD  MOVE       P  PAUSE       R  RESTART       ESC  QUIT");
        FlushBatchDraw();
    }
};

int main() {
    PacManGame game;
    game.run();
    return 0;
}
