#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>

using namespace std;

const int ROWS = 20;
const int COLS = 40;

void drawGrid(const vector<vector<char>>& grid) {
    cout << "\033[H";
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            cout << grid[r][c];
        }
        cout << '\n';
    }
    cout.flush();
}

vector<pair<int, int>> buildPath(int sr, int sc, int er, int ec) {
    vector<pair<int, int>> path;
    int r = sr, c = sc;
    while (r != er || c != ec) {
        path.push_back({ r, c });
        if (r < er) r++;
        else if (r > er) r--;
        if (c < ec) c++;
        else if (c > ec) c--;
    }
    path.push_back({ er, ec });
    return path;
}

int main() {
    int startRow, startCol, endRow, endCol;

    cout << "Redak tocke A (1-20): ";
    cin >> startRow;
    cout << "Stupac tocke A (1-40): ";
    cin >> startCol;
    cout << "Redak tocke B (1-20): ";
    cin >> endRow;
    cout << "Stupac tocke B (1-40): ";
    cin >> endCol;

    startRow = max(1, min(20, startRow));
    startCol = max(1, min(40, startCol));
    endRow = max(1, min(20, endRow));
    endCol = max(1, min(40, endCol));

    int sr = startRow - 1, sc = startCol - 1;
    int er = endRow - 1, ec = endCol - 1;

    vector<pair<int, int>> path = buildPath(sr, sc, er, ec);

    vector<vector<char>> grid(ROWS, vector<char>(COLS, '.'));
    grid[sr][sc] = 'A';
    grid[er][ec] = 'B';

    cout << "\033[2J\033[H";

    for (size_t i = 0; i < path.size(); i++) {
        auto [row, col] = path[i];

        if (i > 0) {
            auto [pr, pc] = path[i - 1];
            grid[pr][pc] = (pr == sr && pc == sc) ? 'A' : '*';
        }

        if (row == er && col == ec) {
            grid[row][col] = 'B';
        }
        else {
            grid[row][col] = 'X';
        }

        drawGrid(grid);
        auto t0 = chrono::steady_clock::now();
        while (chrono::steady_clock::now() - t0 < chrono::milliseconds(100)) {}
    }

    return 0;
}