#include <windows.h>
#include <conio.h>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <queue>
#include <string>
#include <vector>

// OutDemo01_: a single-file Pac-Man style game for Dev-C++ on Windows.
// Uses only the C++ standard library and the Windows console API.

const int MAZE_W = 25;
const int MAZE_H = 19;
const int SCREEN_W = 64;
const int SCREEN_H = 25;
const int DX[4] = { 0, 1, 0, -1 };
const int DY[4] = { -1, 0, 1, 0 };

struct Point {
    int x;
    int y;
    Point(int px = 0, int py = 0) : x(px), y(py) {}
};

struct Ghost {
    int x;
    int y;
    int homeX;
    int homeY;
    WORD color;
};

class Game {
public:
    Game() : console(GetStdHandle(STD_OUTPUT_HANDLE)), quit(false) {
        std::srand(static_cast<unsigned int>(std::time(0)));
        SetConsoleTitleA("OutDemo01_ - Pac-Man");
        CONSOLE_CURSOR_INFO cursor;
        cursor.dwSize = 1;
        cursor.bVisible = FALSE;
        SetConsoleCursorInfo(console, &cursor);
        restart();
    }

    ~Game() {
        CONSOLE_CURSOR_INFO cursor;
        cursor.dwSize = 20;
        cursor.bVisible = TRUE;
        SetConsoleCursorInfo(console, &cursor);
        SetConsoleTextAttribute(console, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        COORD position;
        position.X = 0;
        position.Y = SCREEN_H - 1;
        SetConsoleCursorPosition(console, position);
        std::printf("\n");
    }

    void run() {
        while (!quit) {
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
            draw();
            Sleep(110);
        }
    }

private:
    HANDLE console;
    std::vector<std::string> maze;
    std::vector<Ghost> ghosts;
    int playerX, playerY;
    int directionX, directionY;
    int wantedX, wantedY;
    int score, lives, pellets, powerTicks, safeTicks, tick;
    bool started, paused, lost, won, quit;

    bool inside(int x, int y) const {
        return x > 0 && x < MAZE_W - 1 && y > 0 && y < MAZE_H - 1;
    }

    bool floorAt(int x, int y) const {
        return inside(x, y) && maze[y][x] != '#';
    }

    void buildMaze() {
        maze.assign(MAZE_H, std::string(MAZE_W, '#'));
        std::vector<Point> stack;
        maze[1][1] = '.';
        stack.push_back(Point(1, 1));

        // Carve a connected maze, so every pellet is reachable.
        while (!stack.empty()) {
            Point here = stack.back();
            int order[4] = { 0, 1, 2, 3 };
            for (int i = 3; i > 0; --i) {
                int j = std::rand() % (i + 1);
                std::swap(order[i], order[j]);
            }
            bool advanced = false;
            for (int i = 0; i < 4; ++i) {
                int d = order[i];
                int nx = here.x + 2 * DX[d];
                int ny = here.y + 2 * DY[d];
                if (inside(nx, ny) && maze[ny][nx] == '#') {
                    maze[here.y + DY[d]][here.x + DX[d]] = '.';
                    maze[ny][nx] = '.';
                    stack.push_back(Point(nx, ny));
                    advanced = true;
                    break;
                }
            }
            if (!advanced) stack.pop_back();
        }

        // Open extra connections to give the player more route choices.
        int opened = 0;
        for (int attempt = 0; attempt < 600 && opened < 34; ++attempt) {
            int x = 1 + std::rand() % (MAZE_W - 2);
            int y = 1 + std::rand() % (MAZE_H - 2);
            if (maze[y][x] != '#') continue;
            bool horizontal = floorAt(x - 1, y) && floorAt(x + 1, y);
            bool vertical = floorAt(x, y - 1) && floorAt(x, y + 1);
            if (horizontal || vertical) {
                maze[y][x] = '.';
                ++opened;
            }
        }
    }

    std::vector<int> distancesFrom(int x, int y) const {
        std::vector<int> distance(MAZE_W * MAZE_H, -1);
        std::queue<Point> pending;
        distance[y * MAZE_W + x] = 0;
        pending.push(Point(x, y));
        while (!pending.empty()) {
            Point p = pending.front();
            pending.pop();
            for (int d = 0; d < 4; ++d) {
                int nx = p.x + DX[d], ny = p.y + DY[d];
                int index = ny * MAZE_W + nx;
                if (floorAt(nx, ny) && distance[index] == -1) {
                    distance[index] = distance[p.y * MAZE_W + p.x] + 1;
                    pending.push(Point(nx, ny));
                }
            }
        }
        return distance;
    }

    void placeGhosts() {
        ghosts.clear();
        std::vector<int> distance = distancesFrom(1, 1);
        const WORD colors[3] = {
            FOREGROUND_RED | FOREGROUND_INTENSITY,
            FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
            FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY
        };
        for (int i = 0; i < 3; ++i) {
            int best = -100000, bx = -1, by = -1;
            for (int y = 1; y < MAZE_H - 1; ++y) {
                for (int x = 1; x < MAZE_W - 1; ++x) {
                    if (!floorAt(x, y) || (x == 1 && y == 1)) continue;
                    int value = distance[y * MAZE_W + x];
                    bool used = false;
                    for (size_t j = 0; j < ghosts.size(); ++j) {
                        int separation = std::abs(x - ghosts[j].x) + std::abs(y - ghosts[j].y);
                        if (separation == 0) used = true;
                        if (separation < 6) value -= 100;
                    }
                    if (!used && value > best) {
                        best = value;
                        bx = x;
                        by = y;
                    }
                }
            }
            Ghost ghost;
            ghost.x = ghost.homeX = bx;
            ghost.y = ghost.homeY = by;
            ghost.color = colors[i];
            ghosts.push_back(ghost);
            maze[by][bx] = ' ';
        }
    }

    void restart() {
        buildMaze();
        playerX = playerY = 1;
        maze[playerY][playerX] = ' ';
        placeGhosts();

        const Point corners[3] = {
            Point(MAZE_W - 2, 1), Point(1, MAZE_H - 2),
            Point(MAZE_W - 2, MAZE_H - 2)
        };
        for (int i = 0; i < 3; ++i) {
            int x = corners[i].x, y = corners[i].y;
            if (maze[y][x] == '.') maze[y][x] = 'o';
        }

        pellets = 0;
        for (int y = 0; y < MAZE_H; ++y)
            for (int x = 0; x < MAZE_W; ++x)
                if (maze[y][x] == '.' || maze[y][x] == 'o') ++pellets;

        directionX = directionY = wantedX = wantedY = 0;
        score = 0;
        lives = 3;
        powerTicks = safeTicks = tick = 0;
        started = paused = lost = won = false;
    }

    void readInput() {
        while (_kbhit()) {
            int key = _getch();
            if (key == 0 || key == 224) {
                key = _getch();
                if (key == 72) { wantedX = 0; wantedY = -1; started = true; }
                if (key == 80) { wantedX = 0; wantedY = 1; started = true; }
                if (key == 75) { wantedX = -1; wantedY = 0; started = true; }
                if (key == 77) { wantedX = 1; wantedY = 0; started = true; }
                continue;
            }
            if (key == 27) { quit = true; return; }
            if (key == 'r' || key == 'R') { restart(); continue; }
            if (key == 'p' || key == 'P') { paused = !paused; continue; }
            if (key == 'w' || key == 'W') { wantedX = 0; wantedY = -1; started = true; }
            if (key == 's' || key == 'S') { wantedX = 0; wantedY = 1; started = true; }
            if (key == 'a' || key == 'A') { wantedX = -1; wantedY = 0; started = true; }
            if (key == 'd' || key == 'D') { wantedX = 1; wantedY = 0; started = true; }
        }
    }

    void movePlayer() {
        if (floorAt(playerX + wantedX, playerY + wantedY) &&
            (wantedX != 0 || wantedY != 0)) {
            directionX = wantedX;
            directionY = wantedY;
        }
        int nx = playerX + directionX, ny = playerY + directionY;
        if (!floorAt(nx, ny)) return;
        playerX = nx;
        playerY = ny;
        char &tile = maze[playerY][playerX];
        if (tile == '.') {
            score += 10;
            --pellets;
            tile = ' ';
        } else if (tile == 'o') {
            score += 50;
            --pellets;
            powerTicks = 75;
            tile = ' ';
        }
        if (pellets == 0) won = true;
    }

    void moveGhosts() {
        std::vector<int> distance = distancesFrom(playerX, playerY);
        for (size_t i = 0; i < ghosts.size(); ++i) {
            Ghost &ghost = ghosts[i];
            int choices[4], count = 0;
            for (int d = 0; d < 4; ++d)
                if (floorAt(ghost.x + DX[d], ghost.y + DY[d])) choices[count++] = d;
            if (count == 0) continue;

            int choice = choices[std::rand() % count];
            if (std::rand() % 4 != 0) {
                int best = powerTicks > 0 ? -100000 : 100000;
                for (int k = 0; k < count; ++k) {
                    int d = choices[k];
                    int nx = ghost.x + DX[d], ny = ghost.y + DY[d];
                    int value = distance[ny * MAZE_W + nx];
                    if ((powerTicks > 0 && value > best) ||
                        (powerTicks == 0 && value < best)) {
                        best = value;
                        choice = d;
                    }
                }
            }
            ghost.x += DX[choice];
            ghost.y += DY[choice];
        }
    }

    void checkCollisions() {
        for (size_t i = 0; i < ghosts.size(); ++i) {
            Ghost &ghost = ghosts[i];
            if (ghost.x != playerX || ghost.y != playerY) continue;
            if (powerTicks > 0) {
                score += 200;
                ghost.x = ghost.homeX;
                ghost.y = ghost.homeY;
            } else if (safeTicks == 0) {
                --lives;
                if (lives <= 0) { lost = true; return; }
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

    void cell(std::vector<CHAR_INFO> &frame, int x, int y, char symbol, WORD color) const {
        if (x < 0 || x >= SCREEN_W || y < 0 || y >= SCREEN_H) return;
        CHAR_INFO &item = frame[y * SCREEN_W + x];
        item.Char.AsciiChar = symbol;
        item.Attributes = color;
    }

    void text(std::vector<CHAR_INFO> &frame, int x, int y,
              const std::string &message, WORD color) const {
        for (size_t i = 0; i < message.size(); ++i)
            cell(frame, x + static_cast<int>(i), y, message[i], color);
    }

    void draw() const {
        const WORD white = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
        const WORD bright = white | FOREGROUND_INTENSITY;
        const WORD yellow = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
        const WORD blue = FOREGROUND_BLUE | FOREGROUND_INTENSITY;
        const WORD cyan = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
        std::vector<CHAR_INFO> frame(SCREEN_W * SCREEN_H);
        for (size_t i = 0; i < frame.size(); ++i) {
            frame[i].Char.AsciiChar = ' ';
            frame[i].Attributes = white;
        }
        text(frame, 2, 0, "OUTDEMO01_  PAC-MAN", yellow);
        text(frame, 2, 1, "Eat every dot. Avoid ghosts. Power dots let you chase!", bright);

        for (int y = 0; y < MAZE_H; ++y) {
            for (int x = 0; x < MAZE_W; ++x) {
                char tile = maze[y][x];
                WORD color = tile == '#' ? blue : (tile == 'o' ? cyan : white);
                cell(frame, x + 2, y + 3, tile, color);
            }
        }
        for (size_t i = 0; i < ghosts.size(); ++i)
            cell(frame, ghosts[i].x + 2, ghosts[i].y + 3, 'G',
                 powerTicks > 0 ? blue : ghosts[i].color);
        cell(frame, playerX + 2, playerY + 3, 'C',
             safeTicks > 0 && tick % 2 == 0 ? white : yellow);

        char line[80];
        std::sprintf(line, "SCORE   %d", score);
        text(frame, 31, 4, line, yellow);
        std::sprintf(line, "LIVES   %d", lives);
        text(frame, 31, 6, line, bright);
        std::sprintf(line, "DOTS    %d", pellets);
        text(frame, 31, 8, line, bright);
        std::sprintf(line, "POWER   %ds", (powerTicks * 110 + 999) / 1000);
        text(frame, 31, 10, line, cyan);
        text(frame, 31, 13, "C = YOU", yellow);
        text(frame, 31, 14, "G = GHOST", FOREGROUND_RED | FOREGROUND_INTENSITY);
        text(frame, 31, 15, ". = DOT", white);
        text(frame, 31, 16, "o = POWER DOT", cyan);

        if (!started && !lost && !won)
            text(frame, 31, 19, "PRESS A D W S", yellow);
        else if (paused)
            text(frame, 31, 19, "PAUSED - PRESS P", yellow);
        else if (won)
            text(frame, 31, 19, "YOU WIN! PRESS R", yellow);
        else if (lost)
            text(frame, 31, 19, "GAME OVER! R", FOREGROUND_RED | FOREGROUND_INTENSITY);

        text(frame, 2, 23, "MOVE: ARROWS / WASD   PAUSE: P   RESTART: R   QUIT: ESC", bright);
        COORD bufferSize;
        bufferSize.X = SCREEN_W;
        bufferSize.Y = SCREEN_H;
        COORD bufferOrigin;
        bufferOrigin.X = 0;
        bufferOrigin.Y = 0;
        SMALL_RECT region;
        region.Left = 0;
        region.Top = 0;
        region.Right = SCREEN_W - 1;
        region.Bottom = SCREEN_H - 1;
        WriteConsoleOutputA(console, &frame[0], bufferSize, bufferOrigin, &region);
    }
};

int main() {
    Game game;
    game.run();
    return 0;
}
