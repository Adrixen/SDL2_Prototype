#include <cassert>

struct Map {
    Map(SDL_Renderer *renderer) : renderer(renderer), textures(renderer, "ground.bmp", 128) {
        assert(sizeof(level) == w*h+1); // +1 dla usuwania koncowki string'a
        int window_w, window_h;
        if (!SDL_GetRendererOutputSize(renderer, &window_w, &window_h)) {
            tile_w = window_w/w;
            tile_h = window_h/h;
        } else
            std::cerr << "Blad pobierania rozmiaru renderer'a. " << SDL_GetError() << std::endl;
    }

    void draw() { // rysowanie poziomu
        for (int j=0; j<h; j++)
            for (int i=0; i<w; i++) {
                if (is_empty(i, j)) continue;
                SDL_Rect dst = { tile_w*i, tile_h*j, tile_w, tile_h };
                SDL_Rect src = textures.rect(get(i,j));
                SDL_RenderCopy(renderer, textures.texture, &src, &dst);
            }
    }

    void menu()
    {
        SDL_RenderCopy(renderer, textures.texture, NULL, NULL);
    }

    int get(const int i, const int j) const { // pobierz i przetransformuj do indexu
        assert(i>=0 && j>=0 && i<w && j<h);
        return level[i+j*w] - '0';
    }

    bool is_empty(const int i, const int j) const {
        assert(i>=0 && j>=0 && i<w && j<h);
        return level[i+j*w] == ' ';
    }

    SDL_Renderer *renderer; // rysowanie tutaj
    int tile_w = 0, tile_h = 0; // rozmiar kwadracika mapy w oknie

    const Sprite textures;       // tekstury do narysowania
    static constexpr int w = 16; // rozmiary mapy, tablica level[] ma dlugosc w*h+1 (+1 na usuwanie koncowki stringa)
    static constexpr int h = 12; // puste kwadraciki, cyfry oznaczaja indeks tekstur pustych kwadracikow
    static constexpr char level[w*h+1] = " 123451234012340"\
                                         "5              5"\
                                         "0              0"\
                                         "5              5"\
                                         "0              0"\
                                         "5              5"\
                                         "0              0"\
                                         "5              1"\
                                         "0              2"\
                                         "5              4"\
                                         "0              5"\
                                         "1234512345152500";
};

