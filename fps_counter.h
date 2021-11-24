#include "sprite.h"

struct FPS_Counter {
    FPS_Counter(SDL_Renderer *renderer) : renderer(renderer), numbers(renderer, "numbers.bmp", 24) {}

    void draw() {
        fps_cur++;
        double dt = std::chrono::duration<double>(Clock::now() - timestamp).count();
        if (dt>=.3) { // co 300ms odśwież licznik FPS
            fps_prev = fps_cur/dt;
            fps_cur = 0;
            timestamp = Clock::now();
        }
        SDL_Rect dst = {4, 16, numbers.width, numbers.height}; // rysowanie pierwszej postaci
        for (const char c : std::to_string(fps_prev)) { // osobne cyfry od fps_prev
            SDL_Rect src = numbers.rect(c-'0'); // konwersja znakow numerycznych na int: '7'-'0'=7
            SDL_RenderCopy(renderer, numbers.texture, &src, &dst); // rysuj obecna cyfre
            dst.x += numbers.width + 4; // rysuj postacie od lewej do prawej, odstepy miedzy cyframi = 4
        }
    }

    int fps_cur  = 0; // liczba wykonania funkcji draw() od ostatniego odczytu
    int fps_prev = 0; // ostatnie odczytanie fps
    TimeStamp timestamp = Clock::now(); // ostatnia aktualizacja fps_prev
    SDL_Renderer *renderer; // rysowanie tutaj
    const Sprite numbers;   // sprite cyfr
};

