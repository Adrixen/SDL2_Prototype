#include <array>
#include <cmath>

struct Player {
    enum States { REST=0, TAKEOFF=1, FLIGHT=2, LANDING=3, WALK=4, FALL=5 };

    Player(SDL_Renderer *renderer) :
        renderer(renderer),
        sprites{Animation(renderer, "rest.bmp",    256, 1.0, true ),
                Animation(renderer, "takeoff.bmp", 256, 0.3, false),
                Animation(renderer, "flight.bmp",  256, 1.3, false),
                Animation(renderer, "landing.bmp", 256, 0.3, false),
                Animation(renderer, "walk.bmp",    256, 1.0, true ),
                Animation(renderer, "fall.bmp",    256, 1.0, true )} {
    }

    void set_state(int s) {
        timestamp = Clock::now();
        state = s;
        if (state!=FLIGHT && state!=WALK)
            vx = 0;
        else if (state==WALK)
            vx = backwards ? -150 : 150;
        else if (state==FLIGHT) {
            vy = jumpvy;
            vx = backwards ? -jumpvx : jumpvx;
        }
    }

    void handle_keyboard() {
        const Uint8 *kbstate = SDL_GetKeyboardState(NULL);
        if (state==WALK && !kbstate[SDL_SCANCODE_RIGHT] && !kbstate[SDL_SCANCODE_LEFT])
            set_state(REST);
        if ((state==REST || state==WALK) && kbstate[SDL_SCANCODE_UP]) {
            if (kbstate[SDL_SCANCODE_LEFT] || kbstate[SDL_SCANCODE_RIGHT]) {
                jumpvx =  200; // daleki skok
                jumpvy = -200;
            } else {
                jumpvx =   50; // wysoki skok
                jumpvy = -300;
            }
            set_state(TAKEOFF);
        }
        if (state==REST && (kbstate[SDL_SCANCODE_LEFT] || kbstate[SDL_SCANCODE_RIGHT])) {
            backwards = kbstate[SDL_SCANCODE_LEFT];
            set_state(WALK);
        }
    }

    void update_state(const double dt, const Map &map) {
        if (state==TAKEOFF && sprites[state].animation_ended(timestamp))
            set_state(FLIGHT); // takeoff -> flight
        if (state==LANDING && sprites[state].animation_ended(timestamp))
            set_state(REST);   // landing -> rest
        if (state!=FLIGHT && map.is_empty(x/map.tile_w, y/map.tile_h + 1))
            set_state(FALL);   // sprite fall jesli nie ma pod nogami ziemi

        x += dt*vx; // przed kolizją
        if (!map.is_empty(x/map.tile_w, y/map.tile_h)) { // pozioma kolizja
            int snap = std::round(x/map.tile_w)*map.tile_w; // zmiana koordynatu na granice kolizji
            x = snap + (snap>x ? 1 : -1);                   // na gore czy dol wolnego kwadracika mapy?
            vx = 0; // stop
        }

        y  += dt*vy;  // prior to collision detection
        vy += dt*300; // gravity
        if (!map.is_empty(x/map.tile_w, y/map.tile_h)) { // pionowa kolizja
            int snap = std::round(y/map.tile_h)*map.tile_h;   // zmiana koordynatu na granice kolizji
            y = snap + (snap>y ? 1 : -1);                     // na gore czy dol wolnego kwadracika mapy?
            vy = 0; // stop
            if (state==FLIGHT || state==FALL)
                set_state(LANDING);
        }
    }

    void draw() {
        SDL_Rect src = sprites[state].rect(timestamp);
        SDL_Rect dest = { int(x)-sprite_w/2, int(y)-sprite_h, sprite_w, sprite_h };
        SDL_RenderCopyEx(renderer, sprites[state].texture, &src, &dest, 0, nullptr, backwards ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
    }

    double x = 150, y = 200; // koordynaty postaci
    double vx = 0, vy = 0;   // szybkosc
    bool backwards = false;  // kierunek lewo lub prawo
    double jumpvx = 0, jumpvy = 0; // czy skok jest daleki czy wysoki?

    int state = REST;         // obecny sprite
    TimeStamp timestamp = Clock::now();
    SDL_Renderer *renderer;   // rysowanie tutaj

    const int sprite_w = 256; // wielkosc sprite'a na ekranie
    const int sprite_h = 128;
    const std::array<Animation,6> sprites; // sekwencje sprite' do narysowania
};

