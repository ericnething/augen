const int32 SCREEN_WIDTH = 1920/2;
const int32 SCREEN_HEIGHT = 1080/2;

// Initialize SDL and create window
SDL_Window*
initializeSDL()
{
        // Initialize SDL
        bool initStatus = SDL_Init( SDL_INIT_VIDEO );
        if ( initStatus < 0 )
        {
                printf( "SDL could not be initialized. SDL_Error: %s\n",
                        SDL_GetError() );
        }

        // Create window
        SDL_Window* window = SDL_CreateWindow( "SDL Tutorial", 0, 0,
                                    SCREEN_WIDTH, SCREEN_HEIGHT,
                                    SDL_WINDOW_SHOWN );

        return window;
}

// Create renderer
SDL_Renderer*
createRenderer( SDL_Window *window )
{
        // Create renderer for window
        SDL_Renderer* renderer =
                SDL_CreateRenderer( window, -1,
                                    SDL_RENDERER_ACCELERATED |
                                    SDL_RENDERER_PRESENTVSYNC );

        return renderer;
}

// Shutdown SDL
void
shutdownSDL( SDL_Window* window,
             SDL_Renderer* renderer )
{
        // Destroy window and renderer
        SDL_DestroyRenderer( renderer );
        SDL_DestroyWindow( window );

        // Quit SDL Subsystems
        SDL_Quit();
}

// Load a surface from a file
SDL_Surface*
loadSurface( std::string path )
{
        // Load image from path
        SDL_Surface* loadedSurface = SDL_LoadBMP( path.c_str() );
        if ( loadedSurface == NULL )
        {
                printf( "Unable to load image %s. SDL Error: %s\n",
                        path.c_str(), SDL_GetError() );
        }

        return loadedSurface;
}

// Handle events on the queue
bool
parseEvents()
{
        SDL_Event event;
        bool quit = true;

        while ( SDL_PollEvent( &event ) != 0 )
        {
                // Quit event
                if ( event.type == SDL_QUIT )
                {
                        return quit;
                }
                else if ( event.type == SDL_KEYDOWN )
                {
                        if ( event.key.keysym.sym == SDLK_ESCAPE )
                        {
                                return quit;
                        }
                }
        }
        
        return !quit;
}

internal void
setRenderDrawColor( SDL_Renderer* renderer,
                    real32 colorR,
                    real32 colorG,
                    real32 colorB,
                    real32 colorA )
{
        uint8 R = colorReal32ToUint8( colorR );
        uint8 G = colorReal32ToUint8( colorG );
        uint8 B = colorReal32ToUint8( colorB );
        uint8 A = colorReal32ToUint8( colorA );

        SDL_SetRenderDrawColor( renderer, R, G, B, A );
}

internal void
drawRectangle( SDL_Renderer* renderer,
               V2 position,
               V2 size,
               real32 colorR,
               real32 colorG,
               real32 colorB,
               real32 colorA )
{
        SDL_Rect rectangle = { (int32)position.x, (int32)position.y,
                               (int32)size.x, (int32)size.y };
        
        setRenderDrawColor( renderer, colorR, colorG, colorB, colorA );
        SDL_RenderFillRect( renderer, &rectangle );
}
