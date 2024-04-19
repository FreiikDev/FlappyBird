#include <Grapic.h>
#include <math.h>

using namespace grapic;
using namespace std;

const int DIMW = 1000;
const int DIMH = 750;
const int TUNNEL_WIDTH = DIMW / 8;
const int TUNNEL_HEIGHT = DIMH / 4;
const int TUNNEL_GAP = DIMH / 3;
const int TUNNELS_PER_FRAME = 2;
const int FPS = 10;
const float GRAVITY = -0.1;
const float JUMP_FORCE = 15.0;

struct Position {
    int x, y;
};

struct Bird {
    Position pos;
    float velocity = 0.0;
};

struct Tunnel {
    Position upperPos;
    Position lowerPos;
    bool isActive = false;
};

struct World {
    Bird bird;
    Tunnel tunnels[TUNNELS_PER_FRAME];
    int score = 0;
};

void init(World &w) {
    w.bird.pos = {DIMW / 4, DIMH / 2};
    for (int i = 0; i < TUNNELS_PER_FRAME; ++i) {
        w.tunnels[i].isActive = false;
    }
}

void resizeAndDraw(Image &im, int x, int y, int w = -1, int h = -1) {
    if (w != -1 && h != -1) {
        float aspectRatio = DIMW > DIMH ? float(w) / h : float(h) / w;
        w = DIMW * aspectRatio;
        h = DIMH * aspectRatio;
    }
    image_draw(im, x, y, w, h);
}

void drawTunnel(Tunnel tunnel) {
    Image topTunnelImg = image("data/tunnel_top.png");
    Image bottomTunnelImg = image("data/tunnel_bottom.png");
    image_draw(topTunnelImg, tunnel.upperPos.x - TUNNEL_WIDTH, tunnel.upperPos.y - 500, TUNNEL_WIDTH, DIMH);
    image_draw(bottomTunnelImg, tunnel.lowerPos.x - TUNNEL_WIDTH, tunnel.lowerPos.y, TUNNEL_WIDTH, DIMH);
}

void drawBackground() {
    Image background = image("data/background.jpg");
    resizeAndDraw(background, 0, 0, 1920, 1080);
}

void waitForStart(int score = -1) {
    Image titleImg = image("data/title.png");
    
    bool start = false;
    bool pressSpace = false;
    while (!start) {
        winClear();
        drawBackground();
        resizeAndDraw(titleImg, 0, DIMH / 2);
        
        color(255, 255, 255);
        fontSize(DIMW * 0.05);
        selectFont("data/_decterm.ttf");

        if (pressSpace) {
            print(DIMW * 0.15, DIMH / 8, "Appuyez sur espace pour commencer");
        }

        if (score != -1) {
            char text[100];
            sprintf(text, "Perdu ! Score : %d", score);
            print(DIMW * 0.35, DIMH / 3, text);
        }

        pressSpace = !pressSpace;
        start = isKeyPressed(SDLK_SPACE);
        winDisplay();
    }
}

void drawBird(Bird &bird) {
    Image birdImg = image("data/bird.png");
    image_draw(birdImg, bird.pos.x, bird.pos.y, 50, 50);
}

void drawScore(int score) {
    color(255, 255, 255);
    fontSize(DIMW / 10);
    selectFont("data/_decterm.ttf");
    print(DIMW / 2, DIMH - DIMW / 10 - 50, score);
}

void drawAll(World &w) {
    drawBackground();
    drawBird(w.bird);
    for (int i = 0; i < TUNNELS_PER_FRAME; ++i) {
        if (w.tunnels[i].isActive) {
            drawTunnel(w.tunnels[i]);
        }
    }
    drawScore(w.score);
}

void update(World &w) {
    if (isKeyPressed(SDLK_SPACE)) {
        w.bird.velocity = JUMP_FORCE;
    }
    
    w.bird.pos.y += w.bird.velocity;
    w.bird.velocity += GRAVITY * FPS;

    bool gameOver = false;
    for (int i = 0; i < TUNNELS_PER_FRAME && !gameOver; ++i) {
        if (w.tunnels[i].isActive) {
            w.tunnels[i].upperPos.x -= 5 * FPS;
            w.tunnels[i].lowerPos.x -= 5 * FPS;

            if (w.tunnels[i].upperPos.x < -TUNNEL_WIDTH) {
                w.tunnels[i].isActive = false;
                w.tunnels[i].upperPos.x = DIMW;
                w.tunnels[i].lowerPos.x = DIMW;
                int gapPosition = rand() % (DIMH - 2 * TUNNEL_HEIGHT - TUNNEL_GAP);
                w.tunnels[i].upperPos.y = gapPosition;
                w.tunnels[i].lowerPos.y = gapPosition + TUNNEL_HEIGHT + TUNNEL_GAP;
            }

            if (w.bird.pos.y < -50 || w.bird.pos.y > DIMH + 50 ||
                (w.bird.pos.x + 50 > w.tunnels[i].upperPos.x &&
                 w.bird.pos.x + 50 < w.tunnels[i].upperPos.x + TUNNEL_WIDTH &&
                 (w.bird.pos.y - 50 < w.tunnels[i].upperPos.y + TUNNEL_HEIGHT ||
                  w.bird.pos.y + 50 > w.tunnels[i].lowerPos.y))) {
                gameOver = true;
            }
            
            if (-10 < w.bird.pos.x - w.tunnels[i].upperPos.x && 10 > w.bird.pos.x - w.tunnels[i].upperPos.x) {
                w.score++;
                cout << '\a'; // Son bip
            }
        }
    }

    bool tunnelExists = false;
    for (int i = 0; i < TUNNELS_PER_FRAME && !tunnelExists; ++i) {
        if (w.tunnels[i].isActive && w.tunnels[i].upperPos.x > DIMW - DIMW / TUNNELS_PER_FRAME) {
            tunnelExists = true;
        }
    }

    if (!tunnelExists) {
        for (int i = 0; i < TUNNELS_PER_FRAME; ++i) {
            if (!w.tunnels[i].isActive) {
                w.tunnels[i].isActive = true;
                int gapPosition = rand() % (DIMH - 2 * TUNNEL_HEIGHT - TUNNEL_GAP);
                w.tunnels[i].upperPos = {DIMW, gapPosition};
                w.tunnels[i].lowerPos = {DIMW, gapPosition + TUNNEL_HEIGHT + TUNNEL_GAP};
                break;
            }
        }
    }

    if (gameOver) {
        int score = w.score;
        w = World();
        init(w);
        waitForStart(score);
    }
}

int main(int, char**) {
    srand(time(NULL));
    bool stop = false;
    winInit("Flappy Bird", DIMW, DIMH);
    World w;
    init(w);
    waitForStart();

    while (!stop) {
        winClear();
        drawAll(w);
        update(w);
        stop = winDisplay();
    }

    winQuit();
    return 0;
}
