#include <algorithm>

struct Sprite {
    const char* image_path = "C:\\Users\\Martin\\source\\repos\\SDL2_Prototype\\resources\\";
    Sprite(SDL_Renderer *renderer, const std::string filename, const int width) : width(width) {
        SDL_Surface *surface = SDL_LoadBMP((std::string(image_path) + filename).c_str());
        if (!surface) {
            std::cerr << "Blad w SDL_LoadBMP: " << SDL_GetError() << std::endl;
            return;
        }
        if (!(surface->w%width) && surface->w/width) { // szerokosc obrazu musi byc wielokrotnoscia szerokosci sprite'a
            height  = surface->h;
            nframes = surface->w/width;
            texture = SDL_CreateTextureFromSurface(renderer, surface);
        } else
            std::cerr << "Niewlasciwa wielkosc sprite'a" << std::endl;
        SDL_FreeSurface(surface);
    }

    SDL_Rect rect(const int idx) const { // wybieranie ID sprite'a z tekstury
        return { idx*width, 0, width, height };
    }

    ~Sprite() { // zwalnianie pamieci
        if (texture) SDL_DestroyTexture(texture);
    }

    SDL_Texture *texture = nullptr; // obraz
    int width   = 0; // szerokosc sprite'a (szerokosc tekstury = width * nframes)
    int height  = 0; // wysokosc sprite'a
    int nframes = 0; // liczba klatek w animacji
};

using Clock = std::chrono::high_resolution_clock;
using TimeStamp = std::chrono::time_point<Clock>;

struct Animation : public Sprite {
    Animation(SDL_Renderer *renderer, const std::string filename, const int width, const double duration, const bool repeat) :
        Sprite(renderer, filename, width), duration(duration), repeat(repeat) {}

    bool animation_ended(const TimeStamp timestamp) const { // czy animacja jest w trakcie odtwarzania?
        double elapsed = std::chrono::duration<double>(Clock::now() - timestamp).count(); // sekundy od licznika do teraz
        return !repeat && elapsed >= duration;
    }

    int frame(const TimeStamp timestamp) const { // oblicz ilosc klatek obecnie dla animacji zaczętej od licznika
        double elapsed = std::chrono::duration<double>(Clock::now() - timestamp).count(); // sekundy od licznika do teraz
        int idx = static_cast<int>(nframes*elapsed/duration);
        return repeat ? idx % nframes : std::min(idx, nframes-1);
    }

    SDL_Rect rect(const TimeStamp timestamp) const { // wybierz wlasciwa klatke z tekstury
        return { frame(timestamp)*width, 0, width, height };
    }

    const double duration = 1; // dlugosc animacji w sekundach
    const bool repeat = false; // czy powtorzyc animacje?
};

