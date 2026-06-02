#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>

using namespace std;

const int   ROWS = 20;
const int   COLS = 40;
const int   CELL = 24;
const float STEP = 0.1f;
const int   INF = 1'000'000;

const sf::Color COL_PINK{ 229,   0, 126 };
const sf::Color COL_ORANGE{ 247, 148,  30 };
const sf::Color COL_BG{ 12,  12,  18 };
const sf::Color COL_EMPTY{ 24,  24,  36 };
const sf::Color COL_WALL{ 55,  55,  70 };

sf::Color lerpColor(sf::Color a, sf::Color b, float t) {
    return {
        (uint8_t)(a.r + (float)(b.r - a.r) * t),
        (uint8_t)(a.g + (float)(b.g - a.g) * t),
        (uint8_t)(a.b + (float)(b.b - a.b) * t)
    };
}

vector<pair<int, int>> dijkstra(const vector<vector<bool>>& walls, int sr, int sc, int er, int ec) {
    const int dr[] = { -1,  1,  0,  0 };
    const int dc[] = { 0,  0, -1,  1 };

    vector<vector<int>> dist(ROWS, vector<int>(COLS, INF));
    vector<vector<pair<int, int>>> prev(ROWS, vector<pair<int, int>>(COLS, { -1,-1 }));

    using Node = tuple<int, int, int>;
    priority_queue<Node, vector<Node>, greater<Node>> pq;
    dist[sr][sc] = 0;
    pq.push({ 0, sr, sc });

    while (!pq.empty()) {
        auto [d, r, c] = pq.top(); pq.pop();
        if (d > dist[r][c]) continue;
        if (r == er && c == ec) break;
        for (int i = 0; i < 4; i++) {
            int nr = r + dr[i], nc = c + dc[i];
            if (nr < 0 || nr >= ROWS || nc < 0 || nc >= COLS) continue;
            if (walls[nr][nc]) continue;
            if (d + 1 < dist[nr][nc]) {
                dist[nr][nc] = d + 1;
                prev[nr][nc] = { r, c };
                pq.push({ d + 1, nr, nc });
            }
        }
    }

    vector<pair<int, int>> path;
    if (dist[er][ec] == INF) return path;
    for (pair<int, int> cur = { er, ec }; cur.first != -1; ) {
        path.push_back(cur);
        cur = prev[cur.first][cur.second];
    }
    reverse(path.begin(), path.end());
    return path;
}

enum class Mode { PlaceA, PlaceB, Wall, Running };

void refreshTitle(sf::RenderWindow& win, Mode m) {
    string text = "Ivan Rodic - Dijsktra pathfinding (spa-dz03) ";

    switch (m) {
    case Mode::PlaceA:
        win.setTitle(text + "[POSTAVLJANJE TOCKE A]"); break;
    case Mode::PlaceB:
        win.setTitle(text + "[POSTAVLJANJE TOCKE B]"); break;
    case Mode::Wall:
        win.setTitle(text + "[POSTAVLJANE ZIDOVA]"); break;
    case Mode::Running:
        win.setTitle(text + "[ANIMACIJA U TIJEKU]"); break;
    }
}

int main() {
    cout << "Kontrole:" << endl
        << "\tA - postavljanje tocke A" << endl
        << "\tB - postavljanje tocke B" << endl
        << "\tW - crtanje zidova" << endl
        << "\tLijevi klik/drag - postavi tocku / crtaj zid" << endl
        << "\tDesni klik/drag - obrisi tocku / zid" << endl
        << "\tC - obrisi sve zidove" << endl
        << "\tESC - zaustavi animaciju, natrag na uredjivanje" << endl;

    sf::RenderWindow window(
        sf::VideoMode({ (unsigned)(COLS * CELL), (unsigned)(ROWS * CELL) }),
        "Ivan Rodic - Dijsktra pathfinding (spa-dz03)"
    );
    window.setFramerateLimit(60);

    vector<vector<bool>>  walls(ROWS, vector<bool>(COLS, false));
    int   sr = -1, sc = -1;
    int   er = -1, ec = -1;
    Mode  mode = Mode::Wall;
    vector<pair<int, int>> path;
    size_t   pathIdx = 0;
    sf::Clock stepClock;

    sf::RectangleShape cell({ (float)(CELL - 1), (float)(CELL - 1) });
    sf::CircleShape    player(CELL / 2.f - 2.f);
    sf::CircleShape    glow(CELL / 2.f + 4.f);

    while (window.isOpen()) {

        while (auto ev = window.pollEvent()) {

            if (ev->is<sf::Event::Closed>()) { window.close(); continue; }

            if (auto* key = ev->getIf<sf::Event::KeyPressed>()) {
                using K = sf::Keyboard::Key;
                if (key->code == K::A && mode != Mode::Running) {
                    mode = Mode::PlaceA; refreshTitle(window, mode); cout << endl << "Postavljanje tocke A...";
                }
                else if (key->code == K::B && mode != Mode::Running) {
                    mode = Mode::PlaceB; refreshTitle(window, mode); cout << endl << "Postavljanje tocke B...";
                }
                else if (key->code == K::W && mode != Mode::Running) {
                    mode = Mode::Wall;   refreshTitle(window, mode); cout << endl << "Postavljanje zidova...";
                }
                else if (key->code == K::C) {
                    walls.assign(ROWS, vector<bool>(COLS, false));
                    path.clear(); mode = Mode::Wall; refreshTitle(window, mode); cout << endl << "Zidovi ocisceni";
                }
                else if (key->code == K::Escape) {
                    path.clear(); mode = Mode::Wall; refreshTitle(window, mode); cout << endl << "Animacija pauzirana...";
                }
                else if ((key->code == K::Space || key->code == K::Enter) && mode != Mode::Running && sr >= 0 && er >= 0) {
                    path = dijkstra(walls, sr, sc, er, ec);
                    pathIdx = 0;
                    if (!path.empty()) {
                        stepClock.restart();
                        mode = Mode::Running; refreshTitle(window, mode); cout << endl << "Animacija pokrenuta...";
                    }
                }
            }

            if (auto* click = ev->getIf<sf::Event::MouseButtonPressed>()) {
                int c = click->position.x / CELL;
                int r = click->position.y / CELL;
                if (r < 0 || r >= ROWS || c < 0 || c >= COLS) continue;

                if (mode == Mode::PlaceA) {
                    walls[r][c] = false;
                    sr = r; sc = c;
                    mode = Mode::Wall; refreshTitle(window, mode); cout << endl << "Postavljena tocka A";
                }
                else if (mode == Mode::PlaceB) {
                    walls[r][c] = false;
                    er = r; ec = c;
                    mode = Mode::Wall; refreshTitle(window, mode); cout << endl << "Postavljena tocka B";
                }
                else if (mode == Mode::Wall) {
                    bool isAB = (r == sr && c == sc) || (r == er && c == ec);
                    if (!isAB) {
                        if (click->button == sf::Mouse::Button::Left)
                            walls[r][c] = true;
                        else if (click->button == sf::Mouse::Button::Right)
                            walls[r][c] = false;
                    }
                }
            }

            if (auto* moved = ev->getIf<sf::Event::MouseMoved>()) {
                if (mode != Mode::Wall) continue;
                int c = moved->position.x / CELL;
                int r = moved->position.y / CELL;
                if (r < 0 || r >= ROWS || c < 0 || c >= COLS) continue;
                bool isAB = (r == sr && c == sc) || (r == er && c == ec);
                if (!isAB) {
                    if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
                        walls[r][c] = true;
                    else if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Right))
                        walls[r][c] = false;
                }
            }
        }

        if (mode == Mode::Running && !path.empty() && pathIdx < path.size() - 1) {
            if (stepClock.getElapsedTime().asSeconds() >= STEP) {
                stepClock.restart();
                pathIdx++;
            }
        }

        window.clear(COL_BG);

        for (int r = 0; r < ROWS; r++) {
            for (int c = 0; c < COLS; c++) {
                cell.setPosition({ (float)(c * CELL) + .5f, (float)(r * CELL) + .5f });
                cell.setFillColor(walls[r][c] ? COL_WALL : COL_EMPTY);
                window.draw(cell);
            }
        }

        if (!path.empty()) {
            for (size_t i = 0; i < path.size(); i++) {
                auto [r, c] = path[i];
                float t = (float)i / (float)(path.size() - 1);
                sf::Color pc = lerpColor(COL_PINK, COL_ORANGE, t);
                pc.a = 70;
                cell.setPosition({ (float)(c * CELL) + .5f, (float)(r * CELL) + .5f });
                cell.setFillColor(pc);
                window.draw(cell);
            }
        }

        for (size_t i = 1; i < pathIdx && !path.empty(); i++) {
            auto [r, c] = path[i];
            float t = (float)i / (float)(path.size() - 1);
            sf::Color tc = lerpColor(COL_PINK, COL_ORANGE, t);
            tc.a = 210;
            cell.setPosition({ (float)(c * CELL) + .5f, (float)(r * CELL) + .5f });
            cell.setFillColor(tc);
            window.draw(cell);
        }

        if (sr >= 0) {
            cell.setPosition({ (float)(sc * CELL) + .5f, (float)(sr * CELL) + .5f });
            cell.setFillColor(COL_PINK);
            window.draw(cell);
        }

        if (er >= 0) {
            cell.setPosition({ (float)(ec * CELL) + .5f, (float)(er * CELL) + .5f });
            cell.setFillColor(COL_ORANGE);
            window.draw(cell);
        }

        if (mode == Mode::Running && !path.empty()) {
            auto [r, c] = path[pathIdx];
            float t = (float)pathIdx / (float)(path.size() - 1);
            sf::Color pc = lerpColor(COL_PINK, COL_ORANGE, t);
            float px = (float)(c * CELL) + 2.f;
            float py = (float)(r * CELL) + 2.f;

            glow.setPosition({ px - 4.f, py - 4.f });
            glow.setFillColor({ pc.r, pc.g, pc.b, (uint8_t)65 });
            window.draw(glow);

            player.setPosition({ px, py });
            player.setFillColor(pc);
            window.draw(player);
        }

        window.display();
    }
}