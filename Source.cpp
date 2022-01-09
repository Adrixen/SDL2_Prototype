#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <cassert>
#include <stdio.h>
#include <array>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <strstream>
#include <string>
#define SDL_MAIN_HANDLED
#include <SDL.h>

/**
 * Gra platformowa napisana w języku C++ przy użyciu biblioteki SDL.
 *
 * @authors Adrian Kowalski, Paweł Czyżewski.
 */

bool quit = false;
int xMouse = 0;
int yMouse = 0;
int menuflag = 1;
int startflag = 1;

struct Sprite 

{    
     /**
     * Struktura sprite'ów.
     * Zdefiniowana tutaj szerokosc obrazu jest zaprogramowana tak by byc wielokrotnoscia szerokosci sprite'a
     * W tej strukturze znajduje sie sciezka do wykorzystywanych zasobow i tekstur.
     */
    const char* image_path = "C:\\Users\\Martin\\source\\repos\\SDL2_Prototype-v0.01\\resources\\";
    Sprite(SDL_Renderer* renderer, const std::string filename, const int width) : width(width) 
    {
        SDL_Surface* surface = SDL_LoadBMP((std::string(image_path) + filename).c_str());
        if (!surface) 
        {
            std::cerr << "Blad w SDL_LoadBMP: " << SDL_GetError() << std::endl;
            return;
        }
        if (!(surface->w % width) && surface->w / width) 
        { // szerokosc obrazu musi byc wielokrotnoscia szerokosci sprite'a
            height = surface->h;
            nframes = surface->w / width;
            texture = SDL_CreateTextureFromSurface(renderer, surface);
        }
        else
            std::cerr << "Niewlasciwa wielkosc sprite'a" << std::endl;
        SDL_FreeSurface(surface);
    }
     /**
     * Wybieranie ID sprite'a z tekstury
     */
    SDL_Rect rect(const int idx) const 
    { // wybieranie ID sprite'a z tekstury
        return { idx * width, 0, width, height };
    }
    /**
    * Zwalnianie pamieci przez niszczenie tekstury
    */
    ~Sprite() 
    { // zwalnianie pamieci
        if (texture) SDL_DestroyTexture(texture);
    }

    SDL_Texture* texture = nullptr; // obraz
    int width = 0; // szerokosc sprite'a (szerokosc tekstury = width * nframes)
    int height = 0; // wysokosc sprite'a
    int nframes = 0; // liczba klatek w animacji
};

using Clock = std::chrono::high_resolution_clock;
using TimeStamp = std::chrono::time_point<Clock>;


struct Animation : public Sprite 
{
    /**
    * Struktura odpowiadająca za animacje
    */
    Animation(SDL_Renderer* renderer, const std::string filename, const int width, const double duration, const bool repeat) :
        Sprite(renderer, filename, width), duration(duration), repeat(repeat) {}
    /**
    * Sprawdzenie czy animacja sie skonczyla
    */
    bool animation_ended(const TimeStamp timestamp) const 
    { // czy animacja jest w trakcie odtwarzania?
        double elapsed = std::chrono::duration<double>(Clock::now() - timestamp).count(); // sekundy od licznika do teraz
        return !repeat && elapsed >= duration;
    }
    /**
    * Obliczanie ilosci klatek dla animacji.
    */
    int frame(const TimeStamp timestamp) const 
    { // oblicz ilosc klatek obecnie dla animacji zaczętej od licznika
        double elapsed = std::chrono::duration<double>(Clock::now() - timestamp).count(); // sekundy od licznika do teraz
        int idx = static_cast<int>(nframes * elapsed / duration);
        return repeat ? idx % nframes : std::min(idx, nframes - 1);
    }
    /**
    * Wybranie wlasciwej klatki z tekstury
    */
    SDL_Rect rect(const TimeStamp timestamp) const 
    { // wybierz wlasciwa klatke z tekstury
        return { frame(timestamp) * width, 0, width, height };
    }

    const double duration = 1; // dlugosc animacji w sekundach
    const bool repeat = false; // czy powtorzyc animacje?
};

/**
* Struktura odpowiadająca za mapę, po której porusza się gracz
*/
struct Map 
{
    Map(SDL_Renderer* renderer) : renderer(renderer), textures(renderer, "ground.bmp", 128) 
    {
        assert(sizeof(level) == w * h + 1); // +1 dla usuwania koncowki string'a
        int window_w, window_h;
        if (!SDL_GetRendererOutputSize(renderer, &window_w, &window_h)) 
        {
            tile_w = window_w / w;
            tile_h = window_h / h;
        }
        else
            std::cerr << "Blad pobierania rozmiaru renderer'a. " << SDL_GetError() << std::endl;
    }
    /**
    * Rysowanie mapy
    */
    void draw() 
    { // rysowanie poziomu
        for (int j = 0; j < h; j++)
            for (int i = 0; i < w; i++) {
                if (is_empty(i, j)) continue;
                SDL_Rect dst = { tile_w * i, tile_h * j, tile_w, tile_h };
                SDL_Rect src = textures.rect(get(i, j));
                SDL_RenderCopy(renderer, textures.texture, &src, &dst);
            }
    }

    int get(const int i, const int j) const 
    { // pobierz i przetransformuj do indexu
        assert(i >= 0 && j >= 0 && i < w&& j < h);
        return level[i + j * w] - '0';
    }

    bool is_empty(const int i, const int j) const 
    {
        assert(i >= 0 && j >= 0 && i < w&& j < h);
        return level[i + j * w] == ' ';
    }
    /**
    * Zdefiniowane rozmiary pojedyńczych części mapy w oknie, tekstury do narysowania oraz rozmiary mapy.
    * Tablica level określa, w których miejscach na mapie znajduje się jaka tekstura.
    */
    SDL_Renderer* renderer; // renderer
    int tile_w = 0, tile_h = 0; // rozmiar pojedynczej czesci mapy w oknie

    const Sprite textures;       // tekstury do narysowania
    static constexpr int w = 16; // rozmiary mapy, tablica level[] ma dlugosc w*h+1 (+1 na usuwanie koncowki stringa)
    static constexpr int h = 12; // puste czesci mapy, cyfry oznaczaja indeks tekstur na poszczegolnych czesciach
    static constexpr char level[w * h + 1] = 

    "1111111111111111"\
    "2              2"\
    "2              2"\
    "2              2"\
    "2              2"\
    "2              2"\
    "2              2"\
    "2              2"\
    "2           3  2"\
    "2        3     2"\
    "2      332333332"\
    "2333333222222222";
};

/**
* Struktura odpowiadająca za obliczanie ilości klatek na sekundę, klatki są ustawione na maksymalny limit 50.
* Limit jest ustawiony, głównie po to, aby zużycie CPU nie było wysokie.
*/

struct FPS_Counter // licznik FPS ktory mozna ewentualnie wyswietlac, fps zlimitowane do 50
{
    FPS_Counter(SDL_Renderer* renderer) : renderer(renderer), numbers(renderer, "numbers.bmp", 24) {}

    void draw() 
    {
        fps_cur++;
        double dt = std::chrono::duration<double>(Clock::now() - timestamp).count();
        if (dt >= .3) 
        { // co 300ms odśwież licznik fps'ow
            fps_prev = fps_cur / dt;
            fps_cur = 0;
            timestamp = Clock::now();
        }
        SDL_Rect dst = { 4, 16, numbers.width, numbers.height }; // rysowanie pierwszej postaci
        for (const char c : std::to_string(fps_prev)) 
        { // osobne cyfry od fps_prev
            SDL_Rect src = numbers.rect(c - '0'); // konwersja znakow numerycznych na int: '7'-'0'=7
            SDL_RenderCopy(renderer, numbers.texture, &src, &dst); // rysuj obecna cyfre
            dst.x += numbers.width + 4; // rysuj postacie od lewej do prawej, odstepy miedzy cyframi = 4
        }
    }

    int fps_cur = 0; // liczba wykonania funkcji draw() od ostatniego odczytu
    int fps_prev = 0; // ostatnie odczytanie fps
    TimeStamp timestamp = Clock::now(); // ostatnia aktualizacja fps_prev
    SDL_Renderer* renderer; // renderer
    const Sprite numbers;   // sprite cyfr
};

/**
* Struktura odpowiadająca za gracza, są w niej zdefiniowane jego stany i metody odpowiadające za ich zmianę i aktualizacje.
*/

struct Player 
{
    enum States { REST = 0, TAKEOFF = 1, FLIGHT = 2, LANDING = 3, WALK = 4, FALL = 5 };

    Player(SDL_Renderer* renderer) :
        renderer(renderer),
        sprites
              { Animation(renderer, "rest.bmp",    256, 1.0, true),
                Animation(renderer, "takeoff.bmp", 256, 0.3, false),
                Animation(renderer, "flight.bmp",  256, 1.3, false),
                Animation(renderer, "landing.bmp", 256, 0.3, false),
                Animation(renderer, "walk.bmp",    256, 1.0, true),
                Animation(renderer, "fall.bmp",    256, 1.0, true) 
              } {
    }
    /**
    * Metoda ustawiająca stan gracza
    */
    void set_state(int s) 
    {
        timestamp = Clock::now();
        state = s;
        if (state != FLIGHT && state != WALK)
            vx = 0;
        else if (state == WALK)
            vx = backwards ? -150 : 150;
        else if (state == FLIGHT) 
        {
            vy = jumpvy;
            vx = backwards ? -jumpvx : jumpvx;
        }
    }

    /**
    * Metoda, która pobiera przyciski z klawiatury, dzięki którym gracz może się poruszać.
    */

    void handle_keyboard() 
    {
        const Uint8* kbstate = SDL_GetKeyboardState(NULL);
        if (state == WALK && !kbstate[SDL_SCANCODE_RIGHT] && !kbstate[SDL_SCANCODE_LEFT])
            set_state(REST);
        if ((state == REST || state == WALK) && kbstate[SDL_SCANCODE_UP]) 
        {
            if (kbstate[SDL_SCANCODE_LEFT] || kbstate[SDL_SCANCODE_RIGHT]) 
            {
                jumpvx = 200; // daleki skok
                jumpvy = -200;
            }
            else 
            {
                jumpvx = 50; // wysoki skok
                jumpvy = -300;
            }
            set_state(TAKEOFF);
        }
        if (state == REST && (kbstate[SDL_SCANCODE_LEFT] || kbstate[SDL_SCANCODE_RIGHT])) 
        {
            backwards = kbstate[SDL_SCANCODE_LEFT];
            set_state(WALK);
        }
        if (state == REST && (kbstate[SDL_SCANCODE_P]))
        {
            printf("Saving...");
            std::ofstream file;
            file.open("SavedCoordinateX.txt");
            file << kordgetterx();
            file.close();
            file.open("SavedCoordinateY.txt");
            file << kordgettery();
            file.close();
        }
    }

    /**
    * Metoda, która aktualizuje stan gracza, jeśli spełnione są poszczególne warunki.
    */

    void update_state(const double dt, const Map& map) 
    {
        if (state == TAKEOFF && sprites[state].animation_ended(timestamp))
            set_state(FLIGHT); // takeoff -> flight
        if (state == LANDING && sprites[state].animation_ended(timestamp))
            set_state(REST);   // landing -> rest
        if (state != FLIGHT && map.is_empty(x / map.tile_w, y / map.tile_h + 1))
            set_state(FALL);   // sprite fall jesli nie ma pod nogami ziemi

        x += dt * vx; // przed kolizją
        if (!map.is_empty(x / map.tile_w, y / map.tile_h)) // pozioma kolizja
        { 
            int snap = std::round(x / map.tile_w) * map.tile_w; // zmiana koordynatu na granice kolizji
            x = snap + (snap > x ? 1 : -1);                    // na gore czy dol wolnej czesci mapy?
            vx = 0; // stop
        }

        y += dt * vy;  // przed kolizją
        vy += dt * 300; // grawitacja
        if (!map.is_empty(x / map.tile_w, y / map.tile_h)) // pionowa kolizja
        { 
            int snap = std::round(y / map.tile_h) * map.tile_h;   // zmiana koordynatu na granice kolizji
            y = snap + (snap > y ? 1 : -1);                     // na gore czy dol wolnej czesci mapy?
            vy = 0; // stop
            if (state == FLIGHT || state == FALL)
                set_state(LANDING);
        }
    }

    /**
    * Metoda, która wyświetla teksture odpowiadającą danemu stanowi gracza.
    */

    void draw() 
    {
        SDL_Rect src = sprites[state].rect(timestamp);
        SDL_Rect dest = { int(x) - sprite_w / 2, int(y) - sprite_h, sprite_w, sprite_h };
        SDL_RenderCopyEx(renderer, sprites[state].texture, &src, &dest, 0, nullptr, backwards ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
    }

    double x = 150, y = 200; // koordynaty postaci
    double vx = 0, vy = 0;   // szybkosc
    bool backwards = false;  // kierunek lewo lub prawo
    double jumpvx = 0, jumpvy = 0; // czy skok jest daleki czy wysoki?

    int state = REST;         // obecny sprite
    TimeStamp timestamp = Clock::now();
    SDL_Renderer* renderer;   // rysowanie tutaj

    const int sprite_w = 256; // wielkosc sprite'a na ekranie
    const int sprite_h = 128;
    const std::array<Animation, 6> sprites; // sekwencje sprite' do narysowania

    double kordgetterx()
    {
        return x;
    }

    double kordgettery()
    {
        return y;
    }
};

/**
* Metoda, która wykonuje się przy kliknięciu lewym przyciskiem myszy. Była używana głównie do ustawienia koordynatów przycisków menu.
*/

void mousePress(SDL_MouseButtonEvent& b) 
{
    if (b.button == SDL_BUTTON_LEFT) 
    {
        SDL_GetMouseState(&xMouse, &yMouse);
        printf("(%d, %d)", xMouse, yMouse);
        //if ((xMouse >= 737 && xMouse <= 897) && (yMouse >= 349 && yMouse <= 423))
        //{
        //    printf("Test!");
        //}
    }
}

/**
* Metoda, która jest główną pętlą gry.
*/

void main_loop(SDL_Renderer* renderer) 
{
    Map map(renderer);
    Player player(renderer);
    TimeStamp timestamp = Clock::now();
    SDL_Surface* Loading_Surf;
    SDL_Texture* Background_Tx;
    Loading_Surf = SDL_LoadBMP("Background.bmp");
    Background_Tx = SDL_CreateTextureFromSurface(renderer, Loading_Surf);
    SDL_FreeSurface(Loading_Surf);
    /**
    * Pętla menu.
    */
    while (menuflag==1) // petla menu
    {
        SDL_Event e;

        if (SDL_PollEvent(&e) && (SDL_KEYDOWN == e.type && SDLK_RETURN == e.key.keysym.sym))
        {
            printf("Zaczynamy!");
            break; // po kliknieciu ENTER zacznij gre
        }
        if (e.type == SDL_KEYDOWN || SDL_QUIT == e.type || SDL_MOUSEBUTTONDOWN == e.type) {
            switch (e.type) 
            {
            case SDL_QUIT:
                printf("program stopped");
                startflag = 2;
                menuflag = 2;
                break;
            //case SDL_MOUSEMOTION:
            //    x = event.motion.x;
            //    y = event.motion.y;
            case SDL_MOUSEBUTTONDOWN:
                mousePress(e.button);
                if ((xMouse >= 309 && xMouse <= 500) && (yMouse >= 289 && yMouse <= 351)) // klikniecie na przycisk start
                {
                    printf("startujemy");
                    menuflag = 2;
                    startflag = 1;
                }
                else if ((xMouse >= 309 && xMouse <= 500) && (yMouse >= 224 && yMouse <= 280)) // klikniecie na przycisk quit
                {
                    printf("Loading Save!");
                    double loadedx, loadedy;
                    std::ifstream file("SavedCoordinateX.txt");
                    file >> loadedx;
                    std::ifstream file2("SavedCoordinateY.txt");
                    file2 >> loadedy;
                    player.x = loadedx;
                    player.y = loadedy;
                    menuflag = 2;
                    startflag = 1;
                }
                else if ((xMouse >= 309 && xMouse <= 500) && (yMouse >= 370 && yMouse <= 432)) // klikniecie na przycisk quit
                {
                    printf("Wychodzimy");
                    menuflag = 2;
                    startflag = 2;
                    xMouse = 0;
                    yMouse = 0;
                    break;
                }
            default:
            break;
            }

        }

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, Background_Tx, NULL, NULL);
        SDL_RenderPresent(renderer);
    }
    /**
    * Pętla gry.
    */
    while (startflag == 1) // główna pętla gry
    {
        SDL_Event event;
        if (SDL_PollEvent(&event) && (SDL_QUIT == event.type || (SDL_KEYDOWN == event.type && SDLK_ESCAPE == event.key.keysym.sym)))
        {
            printf("Konczymy2!");
        break; // po kliknieciu ESC lub zamknieciu okna wyjdz z gry
        }

        player.handle_keyboard(); // odczytywanie stanu z klawiatury

        const auto dt = Clock::now() - timestamp;
        if (dt < std::chrono::milliseconds(20)) { // max 50 FPS
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }
        timestamp = Clock::now();

        player.update_state(std::chrono::duration<double>(dt).count(), map); // grawitacja, poruszanie sie, kolizja

        SDL_RenderClear(renderer); // wyczysc ostatnia klatke
        player.draw();
        map.draw();
        SDL_RenderPresent(renderer);
    }
}

/**
* Główna metoda, która jest używana, aby uruchomić program.
*/

int main() 
{
    quit = false;
    SDL_SetMainReady();
    if (SDL_Init(SDL_INIT_VIDEO)) 
    {
        std::cerr << "Blad podczas inicjalizacji SDL'a: " << SDL_GetError() << std::endl;
        return -1;
    }

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    if (SDL_CreateWindowAndRenderer(800, 600, SDL_WINDOW_SHOWN | SDL_WINDOW_INPUT_FOCUS, &window, &renderer)) 
    {
        std::cerr << "Blad przy tworzeniu okna i renderer'a: " << SDL_GetError() << std::endl;
        return -1;
    }
    SDL_SetWindowTitle(window, "Platformer 3ID14B");
    SDL_SetRenderDrawColor(renderer, 210, 255, 179, 255);

    main_loop(renderer);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}