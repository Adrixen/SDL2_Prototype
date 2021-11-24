#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include "fps_counter.h"
#include "map.h"
#include "player.h"
bool quit = false;
int xMouse = 0;
int yMouse = 0;
SDL_MouseButtonEvent b;
int menuflag = 1;
int startflag = 1;
int x = 0;
int y = 0;

void mousePress(SDL_MouseButtonEvent& b) 
{
    if (b.button == SDL_BUTTON_LEFT) 
    {
        SDL_GetMouseState(&xMouse, &yMouse);
        SDL_GetGlobalMouseState(&xMouse, &yMouse);
        printf("%d, %d", xMouse, yMouse);
        //if ((xMouse >= 737 && xMouse <= 897) && (yMouse >= 349 && yMouse <= 423))
        //{
        //    printf("LOLOLOLOLOLO!");
        //}
    }
}

void main_loop(SDL_Renderer* renderer) {
    FPS_Counter fps_counter(renderer);
    Map map(renderer);
    Player player(renderer);
    TimeStamp timestamp = Clock::now();
    SDL_Surface* Loading_Surf;
    SDL_Texture* Background_Tx;
    Loading_Surf = SDL_LoadBMP("Background.bmp");
    Background_Tx = SDL_CreateTextureFromSurface(renderer, Loading_Surf);
    SDL_FreeSurface(Loading_Surf);

    while (menuflag==1) // petla menu
    {
        SDL_Event e;

        if (SDL_PollEvent(&e) && (SDL_KEYDOWN == e.type && SDLK_RETURN == e.key.keysym.sym))
        {
            printf("Zaczynamy!");
            break; // po kliknieciu ENTER zacznij gre
        }
        if (e.type == SDL_KEYDOWN || SDL_QUIT == e.type || SDL_MOUSEBUTTONDOWN == e.type) {
            // zamknij gre po wcisnieciu esc lub q
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
                if ((xMouse >= 737 && xMouse <= 897) && (yMouse >= 349 && yMouse <= 423))
                {
                    printf("startujemy");
                    menuflag = 2;
                    startflag = 1;
                }
                else if ((xMouse >= 746 && xMouse <= 889) && (yMouse >= 477 && yMouse <= 546)) 
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

    while (startflag == 1) // główna pętla gry
    {
        SDL_Event event;
        if (SDL_PollEvent(&event) && (SDL_QUIT == event.type || (SDL_KEYDOWN == event.type && SDLK_ESCAPE == event.key.keysym.sym)))
        {
            printf("Konczymy2!");
        break; // po kliknieciu ESC lub zamknieciu okna wyjdz z gry
        }
        //if (quit == true)
        //    startflag = 2;
        //    break;
        player.handle_keyboard(); // odczytywanie stanu z klawiatury

        const auto dt = Clock::now() - timestamp;
        if (dt < std::chrono::milliseconds(20)) { // max 50 FPS
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }
        timestamp = Clock::now();

        player.update_state(std::chrono::duration<double>(dt).count(), map); // grawitacja, poruszanie sie, kolizja

        SDL_RenderClear(renderer); // wyczysc ostatnia klatke
        fps_counter.draw();
        player.draw();
        map.draw();
        SDL_RenderPresent(renderer);
    }
}

int main() {
    quit = false;
    SDL_SetMainReady();
    if (SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "Blad podczas inicjalizacji SDL'a: " << SDL_GetError() << std::endl;
        return -1;
    }

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    if (SDL_CreateWindowAndRenderer(800, 600, SDL_WINDOW_SHOWN | SDL_WINDOW_INPUT_FOCUS, &window, &renderer)) {
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

