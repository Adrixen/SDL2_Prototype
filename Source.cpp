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
#include <SDL_mixer.h>
#include <vector>
#include <sstream>
#include <regex>

/**
 * Gra platformowa napisana w języku C++ przy użyciu biblioteki SDL.
 *
 * @authors Adrian Kowalski, Paweł Czyżewski, Kamil Adaś
 */

bool quit = false;
int xMouse = 0;
int yMouse = 0;
int menuflag = 1;
int startflag = 1;
int scoresflag = 1;
int leaderboardflag = 1;
int poziom = 1;
int zniszczonebeczki = 0;
double czasspeedruna = 0;
int scrollPosition = 0;
int charWidth = 24;
int charHeight = 30;
bool destroyed1 = false;
bool destroyed2 = false;
float timetosave = 0;
float dodanieCzasu = 0;
int pokazzapisanabeczke1 = 0;
int pokazzapisanabeczke2 = 0;

/**
* Struktura sprite'ów.
* Zdefiniowana tutaj szerokosc obrazu jest zaprogramowana tak by byc wielokrotnoscia szerokosci sprite'a
* W tej strukturze znajduje sie sciezka do wykorzystywanych zasobow i tekstur.
* @param image_path sciezka do plikow takich jak np tekstury
* @param renderer renderer SDL'a
* @param filename nazwa pliku ze sciezki
* @param width szerokosc tekstury
*/

struct Sprite 

{    
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
        { //!< szerokosc obrazu musi byc wielokrotnoscia szerokosci sprite'a
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
     * @param idx id tekstury
     */
    SDL_Rect rect(const int idx) const 
    { //!< wybieranie ID sprite'a z tekstury
        return { idx * width, 0, width, height };
    }
    /**
    * Zwalnianie pamieci przez niszczenie tekstury
    */
    ~Sprite() 
    { //!< zwalnianie pamieci
        if (texture) SDL_DestroyTexture(texture);
    }

    SDL_Texture* texture = nullptr; //!< obraz
    int width = 0; //!< szerokosc sprite'a (szerokosc tekstury = width * nframes)
    int height = 0; //!< wysokosc sprite'a
    int nframes = 0; //!< liczba klatek w animacji
    Sprite() {}
};

using Clock = std::chrono::high_resolution_clock;
using TimeStamp = std::chrono::time_point<Clock>;

/**
* Struktura odpowiadająca za animacje
*/

struct Animation : public Sprite 
{
    Animation(SDL_Renderer* renderer, const std::string filename, const int width, const double duration, const bool repeat) :
        Sprite(renderer, filename, width), duration(duration), repeat(repeat) {}
    /**
    * Sprawdzenie czy animacja sie skonczyla
    */
    bool animation_ended(const TimeStamp timestamp) const 
    { //!< czy animacja jest w trakcie odtwarzania?
        double elapsed = std::chrono::duration<double>(Clock::now() - timestamp).count(); //!< sekundy od licznika do teraz
        return !repeat && elapsed >= duration;
    }
    /**
    * Obliczanie ilosci klatek dla animacji.
    */
    int frame(const TimeStamp timestamp) const 
    { //!< oblicz ilosc klatek obecnie dla animacji zaczętej od licznika
        double elapsed = std::chrono::duration<double>(Clock::now() - timestamp).count(); //!< sekundy od licznika do teraz
        int idx = static_cast<int>(nframes * elapsed / duration);
        return repeat ? idx % nframes : std::min(idx, nframes - 1);
    }
    /**
    * Wybranie wlasciwej klatki z tekstury
    */
    SDL_Rect rect(const TimeStamp timestamp) const 
    { //!< wybierz wlasciwa klatke z tekstury
        return { frame(timestamp) * width, 0, width, height };
    }

    const double duration = 1; //!< dlugosc animacji w sekundach
    const bool repeat = false; //!< czy powtorzyc animacje?
};

/**
* Struktura odpowiadająca za pociski
*/

struct Bullet
{
    double x, y;
    double vx, vy;
    bool hit;
    int width, height; // Width and height of the bullet
    bool isHitBarrel() const
    {
        return hit && hitBarrel;
    }

    bool hitBarrel = false;
    Bullet(double x, double y, double vx, double vy, int width, int height) : x(x), y(y), vx(vx), vy(vy), hit(false), width(width), height(height) {}

    // Add method to update bullet position
    void update(double dt)
    {
        x += vx * dt;
        y += vy * dt;
    }
};

/**
* Struktura odpowiadająca za beczki
*/

struct Barrel
{
    double x;
    double y;
    double vx;
    double vy;
    Sprite sprite;
    bool destroyed;
    // Add any other properties as needed

    Barrel(double initialX, double initialY, double initialVx, double initialVy, const Sprite& barrelSprite)
        : x(initialX), y(initialY), vx(initialVx), vy(initialVy), sprite(barrelSprite), destroyed(false)
    {
        // Initialize any other properties
    }

    // Update method
    void update_state(double dt)
    {
        // Implement logic to update barrel state
        // For example, move the barrel, check for collisions, etc.
        // You need to fill in the details based on your game logic.
    }
    void update_state1(double dt)
    {
        // Implement logic to update barrel state
        // For example, move the barrel, check for collisions, etc.
        // You need to fill in the details based on your game logic.
    }

    void draw(SDL_Renderer* renderer) const
    {
        if (!destroyed)
        {
            SDL_Rect dst = { static_cast<int>(x), static_cast<int>(y), sprite.width, sprite.height };
            int result = SDL_RenderCopy(renderer, sprite.texture, nullptr, &dst);
            if (result != 0)
            {
                std::cerr << "SDL_RenderCopy error: " << SDL_GetError() << std::endl;
            }
        }
    }
};

/**
* Struktura odpowiadająca za mapę, po której porusza się gracz
*/

struct Map 
{
    std::vector<Barrel> barrels1;
    std::vector<Barrel> barrels2;
    const Sprite barrelTexture1;
    const Sprite barrelTexture2;
    Map(SDL_Renderer* renderer) : renderer(renderer), textures(renderer, "ground.bmp", 128), barrelTexture1(renderer, "barrel.bmp", 128), barrelTexture2(renderer, "barrel2.bmp", 128)
    {
        assert(sizeof(level) == w * h + 1); //!< +1 dla usuwania koncowki string'a
        int window_w, window_h;
        if (destroyed1 == false && pokazzapisanabeczke1 == 0)
        {
            barrels1.emplace_back(330, 374, 0, 0, barrelTexture1);
        }
        if (destroyed2 == false && pokazzapisanabeczke2 == 0)
        {
            barrels2.emplace_back(550, 422, 0, 0, barrelTexture2);
        }
        if (!SDL_GetRendererOutputSize(renderer, &window_w, &window_h)) 
        {
            tile_w = window_w / w;
            tile_h = window_h / h;
        }
        else
            std::cerr << "Blad pobierania rozmiaru renderer'a. " << SDL_GetError() << std::endl;
    }

    void update_state(double dt)
    {
        for (auto& barrel : barrels1)
        {
            barrel.update_state(dt);
        }
    }
    void update_state1(double dt)
    {
        for (auto& barrel : barrels2)
        {
            barrel.update_state(dt);
        }
    }
    /**
    * Rysowanie mapy
    */
    void draw()
    { //!< rysowanie poziomu

        for (int j = 0; j < h; j++)
            for (int i = 0; i < w; i++) {
                if (is_empty(i, j)) continue;
                SDL_Rect dst = { tile_w * i, tile_h * j, tile_w, tile_h };
                SDL_Rect src = textures.rect(get(i, j));
                SDL_RenderCopy(renderer, textures.texture, &src, &dst);
            }
        if (poziom == 1 && destroyed1 == false && pokazzapisanabeczke1 == 0) {
        for (const auto& barrel : barrels1)
        {
            barrel.draw(renderer);
        }
    }
        if (poziom == 2 && destroyed2 == false && pokazzapisanabeczke2 == 0) {
            for (const auto& barrel : barrels2)
            {
                barrel.draw(renderer);
            }
        }
    }

    int get(const int i, const int j) const 
    { //!< pobierz i przetransformuj do indexu
        assert(i >= 0 && j >= 0 && i < w&& j < h);
        if (poziom == 1) 
        {
            return level[i + j * w] - '0';
        }
        else if (poziom == 2)
        {
            return level2[i + j * w] - '0';
        }

    }

    bool is_empty(const int i, const int j) const 
    {
        assert(i >= 0 && j >= 0 && i < w&& j < h);
        if (poziom == 1)
        {
            return level[i + j * w] == ' ';
        }
        else if (poziom == 2)
        {
            return level2[i + j * w] == ' ';
        }

    }
    /**
    * Zdefiniowane rozmiary pojedyńczych części mapy w oknie, tekstury do narysowania oraz rozmiary mapy.
    * Tablica level określa, w których miejscach na mapie znajduje się jaka tekstura.
    * @param tile_w szerokosc jednego z kwadratow mapy
    * @param tile_h wysokosc jednego z kwadratow mapy
    * @param w szerokosc calej mapy (ilosc kwadratow)
    * @param h wysokosc calej mapy (ilosc kwadratow)
    * @param level pierwszy poziom
    * @param level2 drugi poziom
    */
    SDL_Renderer* renderer; //!< renderer
    int tile_w = 0, tile_h = 0; //!< rozmiar pojedynczej czesci mapy w oknie

    const Sprite textures;       //!< tekstury do narysowania
    static constexpr int w = 16; //!< rozmiary mapy, tablica level[] ma dlugosc w*h+1 (+1 na usuwanie koncowki stringa)
    static constexpr int h = 12; //!< puste czesci mapy, cyfry oznaczaja indeks tekstur na poszczegolnych czesciach
    static constexpr char level[w * h + 1] = 

    "1111111111111111"\
    "2              2"\
    "2              2"\
    "2              2"\
    "2              2"\
    "2              2"\
    "2              2"\
    "2              2"\
    "2              2"\
    "2        3    52"\
    "2    33332333332"\
    "2333322222222222"; //!<poziom pierwszy

    static constexpr char level2[w * h + 1] =

        "1111111111111111"\
        "2              2"\
        "2              2"\
        "2              2"\
        "2              2"\
        "2              2"\
        "2              2"\
        "2   3          2"\
        "2  323         2"\
        "2              2"\
        "2              2"\
        "2333333333333332"; //!<poziom drugi
};


/**
* Struktura odpowiadająca za obliczanie ilości klatek na sekundę, klatki są ustawione na maksymalny limit 50.
* Limit jest ustawiony, głównie po to, aby zużycie CPU nie było wysokie.
*/

struct FPS_Counter //!< licznik FPS ktory mozna ewentualnie wyswietlac, fps zlimitowane do 50
{
    FPS_Counter(SDL_Renderer* renderer) : renderer(renderer), numbers(renderer, "numbers.bmp", 24) {}

    void draw() 
    {
        fps_cur++;
        double dt = std::chrono::duration<double>(Clock::now() - timestamp).count();
        if (dt >= .3) 
        { //!< co 300ms odśwież licznik fps'ow
            fps_prev = fps_cur / dt;
            fps_cur = 0;
            timestamp = Clock::now();
        }
        SDL_Rect dst = { 4, 16, numbers.width, numbers.height }; //!< rysowanie pierwszej postaci
        for (const char c : std::to_string(fps_prev)) 
        { //!< osobne cyfry od fps_prev
            SDL_Rect src = numbers.rect(c - '0'); //!< konwersja znakow numerycznych na int: '7'-'0'=7
            SDL_RenderCopy(renderer, numbers.texture, &src, &dst); //!< rysuj obecna cyfre
            dst.x += numbers.width + 4; //!< rysuj postacie od lewej do prawej, odstepy miedzy cyframi = 4
        }
    }

    int fps_cur = 0; //!< liczba wykonania funkcji draw() od ostatniego odczytu
    int fps_prev = 0; //!< ostatnie odczytanie fps
    TimeStamp timestamp = Clock::now(); //!< ostatnia aktualizacja fps_prev
    SDL_Renderer* renderer; //!< renderer
    const Sprite numbers;   //!< sprite cyfr
};

/**
* Struktura odpowiadająca za sprite pocisku, dziedziczy glowna strukture sprite'a.
*/

struct BulletSprite : public Sprite
{
    BulletSprite(SDL_Renderer* renderer, const std::string filename, const int width) : Sprite(renderer, filename, width) {}
};

/**
* Struktura odpowiadająca za animacje niszczenia beczek
*/

struct Destruction
{
    enum States { REST = 0, DESTRUCTION = 1 };
    Destruction(SDL_Renderer* renderer) :
        renderer(renderer),
        sprites
    { Animation(renderer, "barreldestructionanimationrest.bmp", 250, 1.0, true),
      Animation(renderer, "barreldestructionanimation.bmp", 250, 1.0, false)
    }
    {
    }

    void startDestructionAnimation()
    {
        state = DESTRUCTION;
        timestamp = Clock::now();
    }

    void draw()
    {
        if ((destroyed1 == true || destroyed2 == true) && (poziom == 1 || poziom == 2) && (startflag==1))
        {
            SDL_Rect src = sprites[state].rect(timestamp);
            SDL_Rect dest = { int(x) - sprite_w / 2, int(y) - sprite_h, sprite_w, sprite_h };
            SDL_RenderCopyEx(renderer, sprites[state].texture, &src, &dest, 0, nullptr, SDL_FLIP_NONE);
        }
        if (state == DESTRUCTION && sprites[state].animation_ended(timestamp))
        {
            // Animation has ended, reset state to REST
            state = REST;
        }
    }

    double x = 330, y = 374; //!< koordynaty

    int state = DESTRUCTION;         //!< obecny sprite
    TimeStamp timestamp = Clock::now();
    SDL_Renderer* renderer;   //!< rysowanie tutaj

    const int sprite_w = 250; //!< wielkosc sprite'a na ekranie
    const int sprite_h = 128;
    const std::array<Animation, 2> sprites; //!< sekwencje sprite' do narysowania
};

/**
* Struktura odpowiadająca za gracza, są w niej zdefiniowane jego stany i metody odpowiadające za ich zmianę i aktualizacje.
*/

struct Player 
{
    int framesSinceLastShot = 0;
    const int shotCooldownFrames = 30;  // Adjust the cooldown frames as needed
    double lastShootTime;
    double shootCooldown;
    enum States { REST = 0, TAKEOFF = 1, FLIGHT = 2, LANDING = 3, WALK = 4, FALL = 5 };
    std::vector <Bullet> bullets;
    BulletSprite bulletTexture;
    Player(SDL_Renderer* renderer) :
        renderer(renderer),
        sprites
              { Animation(renderer, "rest.bmp",    256, 1.0, true),
                Animation(renderer, "takeoff.bmp", 256, 0.3, false),
                Animation(renderer, "flight.bmp",  256, 1.3, false),
                Animation(renderer, "landing.bmp", 256, 0.3, false),
                Animation(renderer, "walk.bmp",    256, 1.0, true),
                Animation(renderer, "fall.bmp",    256, 1.0, true) 
              },
        bulletTexture(renderer, "bullet.bmp", 32),
        lastShootTime(0.0),  // Initialize the last shoot time to 0
        shootCooldown(0.2)    // Set the desired cooldown time in seconds 
     {
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
        double currentTime = std::chrono::duration<double>(Clock::now() - timestamp).count();
        const Uint8* kbstate = SDL_GetKeyboardState(NULL);
        if (state == WALK && startflag == 1 && !kbstate[SDL_SCANCODE_RIGHT] && !kbstate[SDL_SCANCODE_LEFT])
            set_state(REST);
        if ((state == REST || state == WALK) && kbstate[SDL_SCANCODE_UP] && startflag == 1)
        {
            if (kbstate[SDL_SCANCODE_LEFT] || kbstate[SDL_SCANCODE_RIGHT]) 
            {
                jumpvx = 200; //!< daleki skok
                jumpvy = -200;
            }
            else 
            {
                jumpvx = 50; //!< wysoki skok
                jumpvy = -300;
            }
            Mix_Chunk* jumpEffect = Mix_LoadWAV("sounds\\jump.wav"); //!<dzwiek skoku
            Mix_PlayChannel(-1, jumpEffect, 0);
            set_state(TAKEOFF);

        }
        if (state == REST && startflag == 1 && (kbstate[SDL_SCANCODE_LEFT] || kbstate[SDL_SCANCODE_RIGHT]))
        {
            backwards = kbstate[SDL_SCANCODE_LEFT];
            Mix_Chunk* runEffect = Mix_LoadWAV("sounds\\run.wav"); //!<dzwiek biegu
            Mix_PlayChannel(-1, runEffect, 0);
            set_state(WALK);
        }
        if (state == REST && (kbstate[SDL_SCANCODE_P] && startflag == 1)) //!<aby zapisac gre wcisnij przycisk "p"
        {
            printf("Saving...");
            std::ofstream file;
            file.open("SavedCoordinateX.txt"); //!<zapisywanie koordynatu X
            file << kordgetterx();
            file.close();
            file.open("SavedCoordinateY.txt"); //!<zapisywanie koordynatu Y
            file << kordgettery();
            file.close();
            file.open("SavedLevel.txt"); //!<zapisywanie poziomu
            file << poziom;
            file.close();
            file.open("SavedTime.txt"); //!<zapisywanie czasu
            file << timetosave;
            file.close();
            file.open("DestroyedBarrels.txt"); //!<zapisywanie ilosci zniszczonych beczek
            if(destroyed1==true)
            {
                int iloscdestroyow = 1;
                file << iloscdestroyow;
            }
            else 
            {
                int iloscdestroyow = 0;
                file << iloscdestroyow;
            }
            file.close();
        }
        if (state == REST && startflag == 1 && (kbstate[SDL_SCANCODE_U]))
        {
            startflag = 2;
        }
        if (kbstate[SDL_SCANCODE_SPACE] && startflag==1)
        {
            if (framesSinceLastShot >= shotCooldownFrames)
            {
                // Shoot a bullet
                double bulletSpeed = 600.0;
                Bullet bullet(x, y - sprite_h / 2, backwards ? -bulletSpeed : bulletSpeed, 0.0, bulletTexture.width, bulletTexture.height);
                bullets.push_back(bullet);

                // Play shooting sound effect
                Mix_Chunk* shootEffect = Mix_LoadWAV("sounds\\shoot.wav");
                Mix_PlayChannel(2, shootEffect, 0);
                Mix_Volume(2,32);

                // Reset the frames since last shot
                framesSinceLastShot = 0;
            }
        }
        framesSinceLastShot++;
    }

    /**
    * Metoda, która aktualizuje stan gracza, jeśli spełnione są poszczególne warunki.
    */

    void update_state(const double dt, const Map& map) 
    {
        if (state == TAKEOFF && sprites[state].animation_ended(timestamp))
            set_state(FLIGHT); //!< takeoff -> flight
        if (state == LANDING && sprites[state].animation_ended(timestamp))
            set_state(REST);   //!< landing -> rest
        if (state != FLIGHT && map.is_empty(x / map.tile_w, y / map.tile_h + 1))
            set_state(FALL);   //!< sprite fall jesli nie ma pod nogami ziemi

        x += dt * vx; //!< przed kolizją
        if (!map.is_empty(x / map.tile_w, y / map.tile_h)) //!< pozioma kolizja
        { 
            int snap = std::round(x / map.tile_w) * map.tile_w; //!< zmiana koordynatu na granice kolizji
            x = snap + (snap > x ? 1 : -1);                    //!< na gore czy dol wolnej czesci mapy?
            vx = 0; //!< stop
        }

        y += dt * vy;  //!< przed kolizją
        vy += dt * 300; //!< grawitacja
        if (!map.is_empty(x / map.tile_w, y / map.tile_h)) //!< pionowa kolizja
        { 
            int snap = std::round(y / map.tile_h) * map.tile_h;   //!< zmiana koordynatu na granice kolizji
            y = snap + (snap > y ? 1 : -1);                     //!< na gore czy dol wolnej czesci mapy?
            vy = 0; //!< stop
            if (state == FLIGHT || state == FALL)
                set_state(LANDING);
        }

    }

    /**
    * Metoda, która wyświetla teksture odpowiadającą danemu stanowi gracza.
    * @param x jeden z koordynatow postaci
    * @param y jeden z koordynatow postaci
    */

    void draw() 
    {
        SDL_Rect src = sprites[state].rect(timestamp);
        SDL_Rect dest = { int(x) - sprite_w / 2, int(y) - sprite_h, sprite_w, sprite_h };
        SDL_RenderCopyEx(renderer, sprites[state].texture, &src, &dest, 0, nullptr, backwards ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
        for (const auto& bullet : bullets)
        {
            SDL_Rect bulletRect = { static_cast<int>(bullet.x), static_cast<int>(bullet.y), bulletTexture.width, bulletTexture.height };
            SDL_RenderCopy(renderer, bulletTexture.texture, nullptr, &bulletRect);
        }
    }

    double x = 150, y = 200; //!< koordynaty postaci
    double vx = 0, vy = 0;   //!< szybkosc
    bool backwards = false;  //!< kierunek lewo lub prawo
    double jumpvx = 0, jumpvy = 0; //!< czy skok jest daleki czy wysoki?

    int state = REST;         //!< obecny sprite
    TimeStamp timestamp = Clock::now();
    SDL_Renderer* renderer;   //!< rysowanie tutaj

    const int sprite_w = 256; //!< wielkosc sprite'a na ekranie
    const int sprite_h = 128;
    const std::array<Animation, 6> sprites; //!< sekwencje sprite' do narysowania

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
bool checkCollision(const Bullet& bullet, const Barrel& barrel)
{
    // Define the dimensions of the bullet and barrel rectangles
    SDL_Rect bulletRect = { static_cast<int>(bullet.x), static_cast<int>(bullet.y), bullet.width, bullet.height };
    SDL_Rect barrelRect = { static_cast<int>(barrel.x), static_cast<int>(barrel.y), barrel.sprite.width, barrel.sprite.height };

    // Check for collision using SDL's collision function
    return SDL_HasIntersection(&bulletRect, &barrelRect) == SDL_TRUE;
}

std::string readTextFile(const std::string& filename) {
    std::ifstream file(filename);
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return content;
}

void renderNumbers(SDL_Renderer* renderer, const std::string& text, int x, int y, int charWidth, int charHeight) {
    // Load your bitmap font texture
    SDL_Surface* fontSurface = SDL_LoadBMP("C:\\Users\\Martin\\source\\repos\\SDL2_Prototype-v0.01\\resources\\numberspro.bmp");
    if (!fontSurface) {
        // Handle error (print an error message, etc.)
        std::cerr << "Failed to load font surface: " << SDL_GetError() << std::endl;
        return;
    }

    SDL_Texture* fontTexture = SDL_CreateTextureFromSurface(renderer, fontSurface);
    SDL_FreeSurface(fontSurface);
    if (!fontTexture) {
        // Handle error (print an error message, etc.)
        std::cerr << "Failed to create font texture: " << SDL_GetError() << std::endl;
        return;
    }


    int currentX = x;
    int currentY = y;
    int charIndex;
    // Render each character of the text
    for (char c : text) {
        if (c == '\n') {
            // Move to the next line
            currentX = x;
            currentY += charHeight + 2; // Adjust the vertical spacing as needed
        }
        else if (c >= '0' && c <= '9') {
            // Calculate the source rect for the character in the bitmap font
            int charIndex = c - '0'; // Adjust the index based on the character '0'
            SDL_Rect srcRect = { charIndex * charWidth, 0, charWidth, charHeight };

            // Calculate the destination rect for rendering
            SDL_Rect destRect = { currentX, currentY, charWidth, charHeight };

            // Render the character
            SDL_RenderCopy(renderer, fontTexture, &srcRect, &destRect);

            // Move to the next position
            currentX += charWidth + 2; // Adjust the horizontal spacing as needed
        }
        else if (c == '.') {
            // Calculate the source rect for the dot
            SDL_Rect srcRect = { 10 * charWidth, 0, charWidth, charHeight };

            // Calculate the destination rect for rendering
            SDL_Rect destRect = { currentX, currentY, charWidth, charHeight };

            // Render the dot
            SDL_RenderCopy(renderer, fontTexture, &srcRect, &destRect);

            // Move to the next position
            currentX += charWidth + 2; // Adjust the horizontal spacing as needed
        }
        else if (c == '-') {
            // Calculate the source rect for the dash
            SDL_Rect srcRect = { 11 * charWidth, 0, charWidth, charHeight };

            // Calculate the destination rect for rendering
            SDL_Rect destRect = { currentX, currentY, charWidth, charHeight };

            // Render the dash
            SDL_RenderCopy(renderer, fontTexture, &srcRect, &destRect);

            // Move to the next position
            currentX += charWidth + 2; // Adjust the horizontal spacing as needed
        }
    }

    // Clean up
    SDL_DestroyTexture(fontTexture);
}

bool compareLines(const std::string& s1, const std::string& s2) {
    // Extract the numeric part of each line
    size_t pos1, pos2;
    float num1, num2;

    try {
        num1 = std::stof(s1.substr(s1.find('-') + 1), &pos1);
        num2 = std::stof(s2.substr(s2.find('-') + 1), &pos2);
    }
    catch (const std::invalid_argument& e) {
        // Handle conversion errors (e.g., if a line doesn't contain a valid float)
        std::cerr << "Invalid float conversion: " << e.what() << std::endl;
        return false;
    }

    // If the numeric parts are equal, compare the parts before the dash
    if (num1 == num2) {
        return s1 < s2;
    }

    // Compare the two floating-point numbers
    return num1 < num2;
}


void findAndSaveFirstNumber(const std::string& filename, float targetNumber) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return;
    }

    std::string line;
    int lineNumber = 0;

    // Regular expression to match a floating-point number
    std::regex numberRegex(R"(\b([-+]?\d*\.?\d+([eE][-+]?\d+)?)\b)");

    while (std::getline(file, line)) {
        lineNumber++;

        // Use std::sregex_iterator to iterate over all matches in the line
        auto matchIterator = std::sregex_iterator(line.begin(), line.end(), numberRegex);
        auto endIterator = std::sregex_iterator();

        for (; matchIterator != endIterator; ++matchIterator) {
            std::smatch match = *matchIterator;
            float number = std::stof(match.str());

            // Check if the extracted number matches the target number
            const float epsilon = 0.0001;
            if (std::abs(number - targetNumber) < epsilon) {
                std::cout << "Found target number in line " << lineNumber << ": " << line << std::endl;

                // Save the line to yourscore.txt
                std::ofstream yourScoreFile("yourscore.txt", std::ios::app);
                yourScoreFile << '\n' << '\n' << '\n' << '\n' << '\n' << '\n' << '\n' << '\n' << '\n' << '\n' << '\n' << '\n' << '\n' << '\n' << '\n' << lineNumber << '\n';
                yourScoreFile.close();
                std::cout << "Saved to yourscore.txt" << std::endl;
                return;  // Stop searching after finding the first match
            }
        }
    }

    std::cout << "Target number not found in the file." << std::endl;
}



void sortFile()
{
    // Read lines from input.txt
    std::ifstream inputFile("Scoreboard.txt");
    std::vector<std::string> lines;
    std::string line;

    while (std::getline(inputFile, line)) {
        lines.push_back(line);
    }

    // Sort the lines using the custom comparison function
    std::sort(lines.begin(), lines.end(), compareLines);

    // Write the sorted lines to output.txt
    std::ofstream outputFile("sorted_scoreboard.txt");
    int lineNumber = 1;

    for (const auto& sortedLine : lines) {
        outputFile << lineNumber << "- " << sortedLine << '\n';
        ++lineNumber;
    }
}

/**
* Metoda, która jest główną pętlą gry.
*/

void main_loop(SDL_Renderer* renderer) 
{
    Map map(renderer);
    Player player(renderer);
    Destruction destruction(renderer);
    TimeStamp timestamp = Clock::now();
    SDL_Surface* Loading_Surf;
    SDL_Texture* Background_Tx;
    Loading_Surf = SDL_LoadBMP("Background.bmp");
    Background_Tx = SDL_CreateTextureFromSurface(renderer, Loading_Surf);
    SDL_FreeSurface(Loading_Surf);

    SDL_Surface* Loading_Surf1;
    SDL_Texture* Background_Tx1;
    Loading_Surf1 = SDL_LoadBMP("Background1.bmp");
    Background_Tx1 = SDL_CreateTextureFromSurface(renderer, Loading_Surf1);
    SDL_FreeSurface(Loading_Surf1);

    SDL_Surface* Loading_Surf2;
    SDL_Texture* Background_Tx2;
    Loading_Surf2 = SDL_LoadBMP("Background2.bmp");
    Background_Tx2 = SDL_CreateTextureFromSurface(renderer, Loading_Surf2);
    SDL_FreeSurface(Loading_Surf2);
    std::vector<Destruction> destructions;
    /**
    * Pętla menu.
    */
    while (menuflag==1)
    {
        SDL_Event e;

        if (SDL_PollEvent(&e) && (SDL_KEYDOWN == e.type && SDLK_RETURN == e.key.keysym.sym))
        {
            printf("Zaczynamy!");
            startflag = 1;
            menuflag = 2;
            auto t1 = std::chrono::high_resolution_clock::now();
            break; //!< po kliknieciu ENTER zacznij gre
        }
        if (e.type == SDL_KEYDOWN || SDL_QUIT == e.type || SDL_MOUSEBUTTONDOWN == e.type) {
            switch (e.type) 
            {
            case SDL_QUIT:
                printf("program stopped");
                startflag = 2;
                menuflag = 2;
                scoresflag = 2;
                leaderboardflag = 2;
                break;
            //case SDL_MOUSEMOTION:
            //    x = event.motion.x;
            //    y = event.motion.y;
            case SDL_MOUSEBUTTONDOWN:
                mousePress(e.button);
                if ((xMouse >= 309 && xMouse <= 500) && (yMouse >= 289 && yMouse <= 351)) //!< klikniecie na przycisk start
                {
                    printf("startujemy");
                    startflag = 1;
                    menuflag = 2;
                    auto t1 = std::chrono::high_resolution_clock::now();
                }
                else if ((xMouse >= 309 && xMouse <= 500) && (yMouse >= 224 && yMouse <= 280)) //!<klikniecie na przycisk continue
                {
                    printf("Loading Save!");
                    double loadedx, loadedy;
                    int loadedpoziom;
                    std::ifstream file("SavedCoordinateX.txt"); //!<odczytywanie koordynatu X
                    file >> loadedx;
                    std::ifstream file2("SavedCoordinateY.txt"); //!<odczytywanie koordynatu Y
                    file2 >> loadedy;
                    std::ifstream file3("SavedLevel.txt"); //!<odczytywanie poziomu
                    file3 >> loadedpoziom;
                    std::ifstream file4("SavedTime.txt");
                    file4 >> dodanieCzasu;
                    std::ifstream file5("DestroyedBarrels.txt");
                    file5 >> pokazzapisanabeczke1;
                    if (pokazzapisanabeczke1 == 1)
                    {
                        zniszczonebeczki = 1;
                    }
                    player.x = loadedx;
                    player.y = loadedy;
                    poziom = loadedpoziom;
                    menuflag = 2;
                    startflag = 1;
                }
                else if ((xMouse >= 309 && xMouse <= 500) && (yMouse >= 370 && yMouse <= 432)) //!< klikniecie na przycisk quit
                {
                    printf("Wychodzimy");
                    menuflag = 2;
                    startflag = 2;
                    scoresflag = 2;
                    leaderboardflag = 2;
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
    const double fixedTimeStep = 0.02; // 50 FPS
    double accumulator = 0.0;
    TimeStamp currentTime = Clock::now();
    TimeStamp newTime;
    auto startSpeedRunTime = std::chrono::high_resolution_clock::now();
    auto lastFrameTimeSpeedRun = startSpeedRunTime;
    while (startflag == 1)
    {
        newTime = Clock::now();
        double frameTime = std::chrono::duration<double>(newTime - currentTime).count();
        currentTime = newTime;
        accumulator += frameTime;
        auto currentSpeedRunTime = std::chrono::high_resolution_clock::now();
        auto duration = currentSpeedRunTime - lastFrameTimeSpeedRun;
        auto deltaTime = std::chrono::duration<double>(duration).count(); // Convert to seconds
        lastFrameTimeSpeedRun = currentSpeedRunTime;
        
        if (zniszczonebeczki >= 2)
        {
            startflag = 2;
            scoresflag = 1;
            auto t2 = std::chrono::high_resolution_clock::now();
        }
        if (poziom == 1 && zniszczonebeczki >=1 && player.x >= 699 && player.y >= 449) //!<jesli gracz wejdzie w portal, przenies go na poziom 2
        {
            Mix_Chunk* portalEffect = Mix_LoadWAV("sounds\\portal.wav"); //!<dzwiek portalu
            Mix_PlayChannel(-1, portalEffect, 0);
            poziom = 2;
            player.x = 150;
            player.y = 200;
        }
        while (accumulator >= fixedTimeStep)
        {
            SDL_Event event;
            if (SDL_PollEvent(&event))
            {
                if (SDL_PollEvent(&event) && (SDL_QUIT == event.type || (SDL_KEYDOWN == event.type && SDLK_ESCAPE == event.key.keysym.sym)))
                    {
                        printf("Konczymy2!");
                        menuflag = 2;
                        startflag = 2;
                        scoresflag = 2;
                        leaderboardflag = 2;
                        break; //!< po kliknieciu ESC lub zamknieciu okna wyjdz z gry
                    }
            }
            const auto dt = std::chrono::duration<double>(Clock::now() - timestamp).count();

        player.handle_keyboard(); //!< odczytywanie stanu z klawiatury

        
        //if (dt < 0.02) { //!< max 50 FPS (convert milliseconds to seconds)
        //    std::this_thread::sleep_for(std::chrono::milliseconds(1));
        //    continue;
        //}
        timestamp = Clock::now();
        for (auto& bullet : player.bullets)
        {
            bullet.update(dt);
            // Check for collisions with barrels
            if (poziom == 1 && destroyed1 == false && pokazzapisanabeczke1 == 0) {
            for (auto& barrel : map.barrels1)
            {
                if (!bullet.hit && !barrel.destroyed && checkCollision(bullet, barrel))
                {
                    destruction.x = 390;
                        destruction.y = 500;
                    destructions.push_back(destruction); // Add it to the vector
                    // Bullet has hit the barrel
                    bullet.hit = true;
                    bullet.hitBarrel = true;  // Set hitBarrel to true
                    destroyed1 = true;
                    barrel.destroyed = true;
                    destruction.startDestructionAnimation();
                    zniszczonebeczki++;
                    Mix_Chunk* destroybarreleffect = Mix_LoadWAV("sounds\\destroybarrel.wav");
                    Mix_PlayChannel(-1, destroybarreleffect, 0);
                    // Add any other logic you want when a barrel is hit by a bullet
                }
            }
        }
            if (poziom == 2 && destroyed2 == false && pokazzapisanabeczke2 == 0) {
            for (auto& barrel : map.barrels2)
            {
                if (!bullet.hit && !barrel.destroyed && checkCollision(bullet, barrel))
                {
                    destruction.x = 530;
                    destruction.y = 300;
                    destructions.push_back(destruction); // Add it to the vector
                    // Bullet has hit the barrel
                    bullet.hit = true;
                    bullet.hitBarrel = true;  // Set hitBarrel to true
                    destroyed2 = true;
                    barrel.destroyed = true;
                    destruction.startDestructionAnimation();
                    zniszczonebeczki++;
                    Mix_Chunk* destroybarreleffect = Mix_LoadWAV("sounds\\destroybarrel.wav");
                    Mix_PlayChannel(-1, destroybarreleffect, 0);
                    // Add any other logic you want when a barrel is hit by a bullet
                }
            }
            }
        }
        player.bullets.erase(std::remove_if(player.bullets.begin(), player.bullets.end(),
            [](const Bullet& bullet) {
                return bullet.x < 0 || bullet.x > 800 || bullet.y < 0 || bullet.y > 600 || (bullet.hit && bullet.isHitBarrel());
            }),
            player.bullets.end());
        player.update_state(fixedTimeStep, map);
        map.update_state(fixedTimeStep);
        map.update_state1(fixedTimeStep);

        accumulator -= fixedTimeStep;
        }
        SDL_RenderClear(renderer); //!< wyczysc ostatnia klatke

        player.draw();
        destruction.draw();
        map.draw();
        SDL_RenderPresent(renderer);
        // Cap the frame rate to achieve a consistent frame rate
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        auto currentTimeTotal = std::chrono::high_resolution_clock::now();
        auto totalTime = std::chrono::duration<double>(currentTimeTotal - startSpeedRunTime).count();
        //std::cout << "Elapsed Time: " << totalTime << " seconds" << std::endl;
        czasspeedruna = totalTime;
        timetosave = czasspeedruna;
    }
    if (scoresflag == 1) 
    {
        float finalSpeedRunTime = 0;
        finalSpeedRunTime = czasspeedruna + dodanieCzasu;
        std::cout << "Final Time: " << finalSpeedRunTime << " seconds" << std::endl;
        printf("Saving your score...");
        std::ofstream fileScoreboard;
        fileScoreboard.open("Scoreboard.txt", std::fstream::app);
        fileScoreboard << finalSpeedRunTime << std::endl;
        fileScoreboard.close();
        sortFile();
        std::ofstream fileYourScore;
        fileYourScore.open("yourscore.txt");
        fileYourScore << '\n' << finalSpeedRunTime << std::endl;
        fileYourScore.close();
        std::cout << "Before findAndSaveFirstNumber call" << std::endl;
        findAndSaveFirstNumber("sorted_scoreboard.txt", finalSpeedRunTime);
        std::cout << "After findAndSaveFirstNumber call" << std::endl;
    }
    std::string textContent = readTextFile("sorted_scoreboard.txt");
    std::string textContent1 = readTextFile("yourscore.txt");
    //std::cout << textContent << std::endl;
    while (scoresflag == 1) 
    {

        SDL_Event event;
        if (SDL_PollEvent(&event) && (SDL_KEYDOWN == event.type && SDLK_RETURN == event.key.keysym.sym))
        {
            printf("show leaderboards!");
            leaderboardflag = 1;
            scoresflag = 0;
            break; //!< po kliknieciu ENTER pokaz leaderboardy
        }
        if (SDL_PollEvent(&event))
        {
            if (SDL_PollEvent(&event) && (SDL_QUIT == event.type || (SDL_KEYDOWN == event.type && SDLK_ESCAPE == event.key.keysym.sym)))
            {
                printf("Konczymy3!");
                leaderboardflag = 0;
                scoresflag = 0;
                break; //!< po kliknieciu ESC lub zamknieciu okna wyjdz z gry
            }
            else if (event.type == SDL_MOUSEWHEEL) {
                // Handle mouse wheel events to scroll
                if (event.wheel.y > 0) {
                    // Scroll up
                    scrollPosition += 30;
                }
                else if (event.wheel.y < 0) {
                    // Scroll down
                    scrollPosition -= 30;
                }
            }
        }
        player.handle_keyboard(); //!< odczytywanie stanu z klawiatury
        //SDL_SetRenderDrawColor(renderer, 255, 0, 0, 0);
        SDL_RenderClear(renderer);

        // Render visible portion of the text based on scroll position
        int renderY = scrollPosition;


        // Update the screen
        SDL_RenderCopy(renderer, Background_Tx1, NULL, NULL);
        renderNumbers(renderer, textContent1, 10, renderY, charWidth, charHeight);
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    while (leaderboardflag == 1)
    {

        SDL_Event event;
        if (SDL_PollEvent(&event))
        {
            if (SDL_PollEvent(&event) && (SDL_QUIT == event.type || (SDL_KEYDOWN == event.type && SDLK_ESCAPE == event.key.keysym.sym)))
            {
                printf("Konczymy4!");
                leaderboardflag = 0;
                break; //!< po kliknieciu ESC lub zamknieciu okna wyjdz z gry
            }
            else if (event.type == SDL_MOUSEWHEEL) {
                // Handle mouse wheel events to scroll
                if (event.wheel.y > 0) {
                    // Scroll up
                    scrollPosition += 30;
                }
                else if (event.wheel.y < 0) {
                    // Scroll down
                    scrollPosition -= 30;
                }
            }
        }
        player.handle_keyboard(); //!< odczytywanie stanu z klawiatury
        //SDL_SetRenderDrawColor(renderer, 255, 0, 0, 0);
        SDL_RenderClear(renderer);


        // Render visible portion of the text based on scroll position
        int renderY = scrollPosition;


        // Update the screen
        SDL_RenderCopy(renderer, Background_Tx2, NULL, NULL);
        renderNumbers(renderer, textContent, 10, renderY, charWidth, charHeight);
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

}



/**
* Główna metoda, która jest używana, aby uruchomić program.
*/

int main() 
{
    quit = false;
    SDL_SetMainReady();
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) <0 )
    {
        std::cerr << "Blad podczas inicjalizacji SDL'a: " << SDL_GetError() << std::endl;
        return -1;
    }

    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);

    Mix_Music *backgroundSound = Mix_LoadMUS("music\\background.wav");
    Mix_Chunk *jumpEffect = Mix_LoadWAV("sounds\\jump.wav");
    Mix_Chunk *portalEffect = Mix_LoadWAV("sounds\\portal.wav");
    Mix_Chunk *runningEffect = Mix_LoadWAV("sounds\\run.wav");

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    if (SDL_CreateWindowAndRenderer(800, 600, SDL_WINDOW_SHOWN | SDL_WINDOW_INPUT_FOCUS, &window, &renderer)<0) 
    {
        std::cerr << "Blad przy tworzeniu okna i renderer'a: " << SDL_GetError() << std::endl;
        return -1;
    }
    SDL_SetWindowTitle(window, "Platformer 4ID13B");
    SDL_SetRenderDrawColor(renderer, 210, 255, 179, 255);
    Mix_PlayMusic(backgroundSound, -1);
    main_loop(renderer);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    Mix_FreeMusic(backgroundSound);
    Mix_FreeChunk(jumpEffect);
    Mix_FreeChunk(portalEffect);
    Mix_FreeChunk(runningEffect);
    Mix_CloseAudio();
    SDL_Quit();
    return 0;
}