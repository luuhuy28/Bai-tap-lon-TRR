#include <SFML/Graphics.hpp>
#include <vector>
#include <queue>
#include <stack>
#include <optional>
#include <cmath>
#include <iostream>
#include <string>
#include <algorithm>

using namespace std;

// ================== CẤU TRÚC DỮ LIỆU ==================
struct Node {
    int id;
    sf::Vector2f pos;
    sf::Color color = sf::Color::White;
    bool deleted = false;
};

struct GraphState {
    vector<Node> nodes;
    vector<vector<int>> adj;
    int nodeCount;
    vector<bool> visited;
    vector<int> parent;
    queue<int> q;
    string status;
    int startId, endId;
};

enum Mode { NONE, ADD_NODE, ADD_EDGE, DEL_NODE, DEL_EDGE };

// ================== BIẾN TOÀN CỤC ==================
vector<Node> nodes;
vector<vector<int>> adj;
int nodeCount = 0;
queue<int> q;
vector<bool> visited;
vector<int> parent;
sf::Font font;
Mode currentMode = NONE;
string connectivityStatus = "Cho duyet...";
stack<GraphState> history;
int startNodeId = -1;
int endNodeId = -1;
bool foundPath = false;
bool isRunningBFS = false;

sf::Clock flashClock;
int flashingBtnIdx = -1;
float flashDuration = 0.1f; 

// ================== HÀM TOÁN HỌC ==================
float distToSegment(sf::Vector2f p, sf::Vector2f v, sf::Vector2f w) {
    float l2 = pow(v.x - w.x, 2) + pow(v.y - w.y, 2);
    if (l2 == 0.0) return hypot(p.x - v.x, p.y - v.y);
    float t = max(0.f, min(1.f, ((p.x - v.x) * (w.x - v.x) + (p.y - v.y) * (w.y - v.y)) / l2));
    sf::Vector2f proj = v + t * (w - v);
    return hypot(p.x - proj.x, p.y - proj.y);
}

// ================== HÀM HỖ TRỢ ==================
void saveState() {
    GraphState s;
    s.nodes = nodes; s.adj = adj; s.nodeCount = nodeCount;
    s.visited = visited; s.parent = parent; s.q = q; 
    s.status = connectivityStatus; s.startId = startNodeId; s.endId = endNodeId;
    history.push(s);
}

void undo() {
    if (!history.empty()) {
        GraphState last = history.top(); history.pop();
        nodes = last.nodes; adj = last.adj; nodeCount = last.nodeCount;
        visited = last.visited; parent = last.parent; q = last.q;
        connectivityStatus = last.status; startNodeId = last.startId; endNodeId = last.endId;
        foundPath = false; isRunningBFS = false;
    }
}

string getQueueString() {
    if (q.empty()) return "Queue: []";
    string res = "Queue: [ ";
    queue<int> temp = q;
    while (!temp.empty()) { res += to_string(temp.front()) + " "; temp.pop(); }
    return res + "]";
}

string getPathString() {
    if (startNodeId == -1 || visited.empty()) return "Path: ";
    int target = (endNodeId != -1 && endNodeId < (int)visited.size() && visited[endNodeId]) ? endNodeId : -1;
    if (target == -1) {
        for(int i = (int)nodes.size()-1; i >= 0; i--) 
            if (!nodes[i].deleted && i < (int)visited.size() && visited[i]) { target = i; break; }
    }
    if (target == -1 || target >= (int)parent.size()) return "Path: ";
    vector<int> path;
    int curr = target;
    while(curr != -1) {
        path.push_back(curr);
        curr = parent[curr];
        if (path.size() > nodes.size()) break;
    }
    reverse(path.begin(), path.end());
    string res = "Path: ";
    if (startNodeId != -1 && endNodeId != -1) res += to_string(startNodeId) + " -> " + to_string(endNodeId) + "\nDetail: ";
    for (size_t i = 0; i < path.size(); i++) res += to_string(path[i]) + (i == path.size()-1 ? "" : " -> ");
    return res;
}

// ================== THUẬT TOÁN ==================
void startBFS() {
    if (nodes.empty()) return;
    saveState();
    foundPath = false; isRunningBFS = true; currentMode = NONE;
    visited.assign(nodes.size(), false);
    parent.assign(nodes.size(), -1);
    while(!q.empty()) q.pop();

    int startIdx = (startNodeId != -1) ? startNodeId : -1;
    if (startIdx == -1 || nodes[startIdx].deleted) {
        for(int i=0; i<(int)nodes.size(); i++) if(!nodes[i].deleted) { startIdx = i; break; }
    }
    if (startIdx == -1) { isRunningBFS = false; return; }

    for (auto& n : nodes) n.color = n.deleted ? sf::Color::Transparent : sf::Color::White;
    visited[startIdx] = true; q.push(startIdx);
    nodes[startIdx].color = sf::Color::Red;
    connectivityStatus = "Dang duyet...";
}

void stepBFS() {
    if (q.empty() || foundPath) {
        isRunningBFS = false; 
        return;
    }
    saveState();
    int u = q.front(); q.pop();
    
    if (u == startNodeId) nodes[u].color = sf::Color::Cyan;
    else if (u == endNodeId) nodes[u].color = sf::Color::Magenta;
    else nodes[u].color = sf::Color::Green;

    // KIỂM TRA ĐÍCH NGAY LẬP TỨC
    if (u == endNodeId) { 
        foundPath = true; 
        isRunningBFS = false; 
        connectivityStatus = "TIM THAY duong di tu " + to_string(startNodeId) + " den " + to_string(endNodeId);
        return; 
    }

    for (int v : adj[u]) {
        if (v < (int)nodes.size() && !nodes[v].deleted && !visited[v]) {
            visited[v] = true; parent[v] = u; q.push(v);
            if (v != endNodeId) nodes[v].color = sf::Color::Red;
        }
    }

    // Nếu sau khi duyệt kề mà Queue trống và chưa thấy đích
    if (q.empty() && !foundPath) {
        isRunningBFS = false;
        if (startNodeId != -1 && endNodeId != -1)
            connectivityStatus = "KHONG tim thay duong di!";
        else connectivityStatus = "Duyet xong do thi!";
    }
}

// ================== UI ==================
struct Button {
    sf::RectangleShape shape; sf::Text text; Mode mode;
    Button(string txt, sf::Vector2f pos, Mode m, const sf::Font& f) : text(f) {
        shape.setSize({180.f, 40.f}); shape.setPosition(pos);
        shape.setFillColor(sf::Color(60, 60, 60)); shape.setOutlineThickness(1.f);
        text.setString(txt); text.setCharacterSize(14);
        text.setPosition({pos.x + 10.f, pos.y + 10.f}); mode = m;
    }
};

int main() {
    if (!font.openFromFile("arial.ttf")) return -1;
    sf::RenderWindow window(sf::VideoMode({1000u, 600u}), "BFS Visualizer");
    window.setFramerateLimit(60);

    vector<Button> buttons;
    buttons.push_back(Button("1. Them Dinh", {810.f, 20.f}, ADD_NODE, font));
    buttons.push_back(Button("2. Them Canh", {810.f, 65.f}, ADD_EDGE, font));
    buttons.push_back(Button("3. Xoa Canh (Line)", {810.f, 110.f}, DEL_EDGE, font));
    buttons.push_back(Button("4. Xoa Dinh", {810.f, 155.f}, DEL_NODE, font));
    buttons.push_back(Button("5. HOAN TAC (Z)", {810.f, 200.f}, NONE, font));
    buttons.push_back(Button("6. START BFS (S)", {810.f, 245.f}, NONE, font));
    buttons.push_back(Button("7. STEP (Space)", {810.f, 290.f}, NONE, font));
    buttons.push_back(Button("8. RESET (R)", {810.f, 335.f}, NONE, font));

    int firstSelected = -1;

    while (window.isOpen()) {
        while (const optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();
            
            if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
                if (key->code == sf::Keyboard::Key::Z) { undo(); flashingBtnIdx = 4; flashClock.restart(); }
                if (key->code == sf::Keyboard::Key::S) { startBFS(); flashingBtnIdx = 5; flashClock.restart(); }
                if (key->code == sf::Keyboard::Key::Space) { stepBFS(); flashingBtnIdx = 6; flashClock.restart(); }
            }

            if (const auto* mouse = event->getIf<sf::Event::MouseButtonPressed>()) {
                sf::Vector2f mPos(static_cast<float>(mouse->position.x), static_cast<float>(mouse->position.y));
                if (mPos.x > 800.f) {
                    for (size_t i = 0; i < buttons.size(); i++) {
                        if (buttons[i].shape.getGlobalBounds().contains(mPos)) {
                            if (buttons[i].text.getString() == "5. HOAN TAC (Z)") { undo(); flashingBtnIdx = 4; flashClock.restart(); }
                            else if (buttons[i].text.getString() == "6. START BFS (S)") { startBFS(); }
                            else if (buttons[i].text.getString() == "7. STEP (Space)") { stepBFS(); flashingBtnIdx = 6; flashClock.restart(); }
                            else if (buttons[i].text.getString() == "8. RESET (R)") { saveState(); nodes.clear(); adj.clear(); nodeCount = 0; startNodeId = -1; endNodeId = -1; isRunningBFS = false; flashingBtnIdx = 7; flashClock.restart(); }
                            else {
                                if (!isRunningBFS) currentMode = (currentMode == buttons[i].mode) ? NONE : buttons[i].mode;
                            }
                        }
                    }
                } else if (!isRunningBFS) {
                    int clickedId = -1;
                    for (int i = 0; i < (int)nodes.size(); i++)
                        if (!nodes[i].deleted && hypot(nodes[i].pos.x - mPos.x, nodes[i].pos.y - mPos.y) < 20.f) { clickedId = i; break; }

                    if (currentMode == ADD_NODE && clickedId == -1) {
                        saveState(); nodes.push_back({nodeCount++, mPos, sf::Color::White, false}); adj.push_back({});
                    } else if (currentMode == ADD_EDGE && clickedId != -1) {
                        if (firstSelected == -1) firstSelected = clickedId;
                        else if (firstSelected != clickedId) { saveState(); adj[firstSelected].push_back(clickedId); adj[clickedId].push_back(firstSelected); firstSelected = -1; }
                    } else if (currentMode == DEL_EDGE) {
                        int eU = -1, eV = -1;
                        for (int u = 0; u < (int)nodes.size(); u++) {
                            if (nodes[u].deleted) continue;
                            for (int v : adj[u]) if (!nodes[v].deleted && u < v && distToSegment(mPos, nodes[u].pos, nodes[v].pos) < 7.f) { eU = u; eV = v; break; }
                        }
                        if (eU != -1) { saveState(); adj[eU].erase(remove(adj[eU].begin(), adj[eU].end(), eV), adj[eU].end()); adj[eV].erase(remove(adj[eV].begin(), adj[eV].end(), eU), adj[eV].end()); }
                    } else if (currentMode == DEL_NODE && clickedId != -1) {
                        saveState(); nodes[clickedId].deleted = true; if (clickedId == startNodeId) startNodeId = -1; if (clickedId == endNodeId) endNodeId = -1;
                    } else if (currentMode == NONE && clickedId != -1) {
                        if (clickedId == startNodeId) { startNodeId = -1; nodes[clickedId].color = sf::Color::White; }
                        else if (clickedId == endNodeId) { endNodeId = -1; nodes[clickedId].color = sf::Color::White; }
                        else if (startNodeId == -1) { startNodeId = clickedId; nodes[clickedId].color = sf::Color::Cyan; }
                        else if (endNodeId == -1) { endNodeId = clickedId; nodes[clickedId].color = sf::Color::Magenta; }
                    }
                }
            }
        }

        for (size_t i = 0; i < buttons.size(); i++) {
            bool isFlash = ((int)i == flashingBtnIdx && flashClock.getElapsedTime().asSeconds() < flashDuration);
            bool isStartActive = (buttons[i].text.getString() == "6. START BFS (S)" && isRunningBFS);
            bool isModeActive = (currentMode != NONE && buttons[i].mode == currentMode && !isRunningBFS);
            if (isFlash || isStartActive || isModeActive) buttons[i].shape.setFillColor(sf::Color(0, 120, 215));
            else buttons[i].shape.setFillColor(sf::Color(60, 60, 60));
        }

        window.clear(sf::Color(30, 30, 30));
        for (int u = 0; u < (int)nodes.size(); u++) {
            if (nodes[u].deleted) continue;
            for (int v : adj[u]) {
                if (v < (int)nodes.size() && !nodes[v].deleted && u < v) {
                    sf::Vector2f p1 = nodes[u].pos, p2 = nodes[v].pos;
                    float len = hypot(p2.x - p1.x, p2.y - p1.y);
                    float ang = atan2(p2.y - p1.y, p2.x - p1.x) * 180 / 3.14159f;
                    sf::RectangleShape line({len, 3.f}); line.setOrigin({0, 1.5f});
                    line.setPosition(p1); line.setRotation(sf::degrees(ang));
                    if (u < (int)visited.size() && v < (int)visited.size() && visited[u] && visited[v]) line.setFillColor(sf::Color(0, 150, 255));
                    else line.setFillColor(sf::Color(100, 100, 100));
                    window.draw(line);
                }
            }
        }
        for (auto& n : nodes) {
            if (n.deleted) continue;
            sf::CircleShape c(20.f); c.setOrigin({20.f, 20.f}); c.setPosition(n.pos);
            if (n.id == startNodeId) c.setFillColor(sf::Color::Cyan);
            else if (n.id == endNodeId) c.setFillColor(sf::Color::Magenta);
            else c.setFillColor(n.color);
            c.setOutlineThickness(2.f); c.setOutlineColor(sf::Color::Black);
            window.draw(c);
            sf::Text idTxt(font, to_string(n.id), 18); idTxt.setFillColor(sf::Color::Black);
            sf::FloatRect tr = idTxt.getLocalBounds();
            idTxt.setOrigin({tr.position.x + tr.size.x/2.f, tr.position.y + tr.size.y/2.f});
            idTxt.setPosition(n.pos); window.draw(idTxt);
        }

        sf::Text qt(font, getQueueString(), 18); qt.setFillColor(sf::Color::Cyan); qt.setPosition({20.f, 440.f}); window.draw(qt);
        sf::Text pt(font, getPathString(), 18); pt.setFillColor(sf::Color(255, 165, 0)); pt.setPosition({20.f, 480.f}); window.draw(pt);
        sf::Text st(font, "Trang thai: " + connectivityStatus, 18); st.setFillColor(sf::Color::Yellow); st.setPosition({20.f, 560.f}); window.draw(st);
        sf::RectangleShape panel({200.f, 600.f}); panel.setPosition({800.f, 0.f}); panel.setFillColor(sf::Color(45, 45, 45)); window.draw(panel);
        for (auto& b : buttons) { window.draw(b.shape); window.draw(b.text); }
        window.display();
    }
    return 0;
}