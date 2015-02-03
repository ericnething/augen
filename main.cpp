#include "main.h"

const int32 SCREEN_WIDTH = 640;
const int32 SCREEN_HEIGHT = 480;

// Initialize SDL and create window
SDL_Window* initializeSDL()
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
SDL_Renderer* createRenderer( SDL_Window *window )
{
        // Create renderer for window
        SDL_Renderer* renderer =
                SDL_CreateRenderer( window, -1,
                                    SDL_RENDERER_ACCELERATED |
                                    SDL_RENDERER_PRESENTVSYNC );

        return renderer;
}

// Shutdown SDL
void shutdownSDL( SDL_Window* window, SDL_Renderer* renderer )
{
        // Destroy window and renderer
        SDL_DestroyRenderer( renderer );
        SDL_DestroyWindow( window );

        // Quit SDL Subsystems
        SDL_Quit();
}

// Load a surface from a file
SDL_Surface* loadSurface( std::string path )
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
bool parseEvents()
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
                    real32 colorR, real32 colorG, real32 colorB, real32 colorA )
{
        uint8 R = colorReal32ToUint8( colorR );
        uint8 G = colorReal32ToUint8( colorG );
        uint8 B = colorReal32ToUint8( colorB );
        uint8 A = colorReal32ToUint8( colorA );

        SDL_SetRenderDrawColor( renderer, R, G, B, A );
}

internal void
drawRectangle( SDL_Renderer* renderer,
               V2 position, V2 size,
               real32 colorR, real32 colorG, real32 colorB, real32 colorA )
{
        SDL_Rect rectangle = { (int32)position.x, (int32)position.y,
                               (int32)size.x, (int32)size.y };
        
        setRenderDrawColor( renderer, colorR, colorG, colorB, colorA );
        SDL_RenderFillRect( renderer, &rectangle );
}

internal void
drawBackground( SDL_Renderer* renderer, const GameState gameState )
{
        const int32 tileRows = 36;
        const int32 tileCols = 16;
        const int32 tileSize = 64;

        const Camera camera = gameState.camera;
        
        uint32 tiles[tileRows][tileCols] =
        {
                { 1, 1, 1, 1,  1, 0, 0, 0,  1, 1, 1, 1,  1, 1, 1, 1 },
                { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1 },
                { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1 },
                { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1 },
                { 0, 0, 0, 0,  0, 0, 0, 0,  0, 1, 1, 1,  0, 0, 0, 1 },
                { 0, 0, 0, 0,  0, 0, 1, 0,  0, 0, 0, 1,  0, 0, 0, 1 },
                { 0, 0, 0, 0,  0, 0, 1, 0,  0, 0, 0, 1,  0, 0, 0, 1 },
                { 0, 0, 0, 0,  0, 1, 1, 1,  0, 0, 0, 1,  0, 0, 0, 1 },
                { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1,  1, 0, 0, 1 },
                { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1 },
                { 1, 0, 0, 0,  0, 0, 0, 1,  0, 0, 0, 0,  0, 0, 0, 1 },
                { 1, 0, 0, 0,  0, 0, 0, 1,  0, 0, 0, 0,  0, 0, 0, 1 },

                { 1, 1, 1, 1,  0, 0, 1, 1,  0, 0, 0, 0,  0, 1, 1, 1 },
                { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1 },
                { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1 },
                { 1, 0, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 0, 1 },
                { 0, 0, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 0, 1 },
                { 0, 0, 0, 0,  1, 1, 1, 1,  1, 0, 0, 0,  0, 0, 0, 1 },
                { 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1 },
                { 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1 },
                { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1, 0, 0, 1 },
                { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1 },
                { 1, 0, 0, 0,  0, 0, 1, 1,  0, 0, 0, 0,  0, 0, 0, 1 },
                { 1, 0, 0, 0,  0, 0, 1, 0,  0, 0, 0, 0,  0, 0, 0, 1 },

                { 1, 0, 0, 0,  0, 0, 0, 1,  0, 0, 0, 0,  0, 1, 1, 1 },
                { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1 },
                { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1 },
                { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1 },
                { 0, 0, 0, 0,  0, 1, 1, 1,  1, 1, 0, 0,  0, 0, 0, 1 },
                { 0, 0, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0,  0, 1, 0, 1 },
                { 0, 0, 0, 0,  1, 0, 0, 0,  0, 1, 0, 0,  0, 1, 0, 1 },
                { 0, 0, 0, 0,  1, 0, 0, 0,  0, 1, 0, 0,  1, 1, 0, 1 },
                { 1, 0, 0, 0,  1, 1, 1, 0,  1, 1, 0, 1,  1, 0, 0, 1 },
                { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1 },
                { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1 },
                { 1, 1, 1, 1,  0, 0, 1, 1,  0, 0, 0, 0,  0, 1, 1, 1 }

        };
        
        for ( int row = 0; row < tileRows; ++row )
        {
                for ( int col = 0; col < tileCols; ++col )
                {
                        int32 tileValue = tiles[row][col];
                        V2 position = {
                                col*tileSize - camera.position.x,
                                row*tileSize - camera.position.y
                        };
                        V2 size = { tileSize, tileSize };
                        V2 screenSize = { SCREEN_WIDTH, SCREEN_HEIGHT };

                        // Only render tiles that are within view of the camera
                        if ( position > (-1 * size) && position < screenSize )
                        {
                                if ( tileValue == 0 )
                                {
                                        drawRectangle( renderer, position, size,
                                                       0.5, 0.5, 0.5, 1.0 );
                                }
                                if ( tileValue == 1 )
                                {
                                        drawRectangle( renderer, position, size,
                                                       1.0, 1.0, 1.0, 1.0 );
                                }
                        }
                }
        }
}

// Draw to the screen
void draw( SDL_Window* window, SDL_Renderer* renderer, const GameState gameState )
{
        Player player = gameState.player;
        Camera camera = gameState.camera;
        
        //Clear screen
        setRenderDrawColor( renderer, 0.0, 0.0, 0.0, 1.0 );
        SDL_RenderClear( renderer );

        drawBackground( renderer, gameState );
        
        // Draw player
        drawRectangle( renderer,
                       player.position - camera.position, player.size,
                       1.0, 1.0, 0.0, 1.0 );
        
        //Update screen
        SDL_RenderPresent( renderer );
}

// Update game state
GameState updateGame( const GameState oldGameState, real32 dt )
{
        const real32 VELOCITY_CONSTANT = 0.7071067811865476;
        
        const uint8* keystate = SDL_GetKeyboardState( NULL );

        V2 dPlayer = { 0.0 ,0.0 };

        // Up
        if ( keystate[ SDL_SCANCODE_W ] )
        {
                dPlayer.y += -1.0;
        }
        // Down
        if ( keystate[ SDL_SCANCODE_S ] )
        {
                dPlayer.y += 1.0;
        }
        // Left
        if ( keystate[ SDL_SCANCODE_A ] )
        {
                dPlayer.x += -1.0;
        }
        // Right
        if ( keystate[ SDL_SCANCODE_D ] )
        {
                dPlayer.x += 1.0;
        }

        real32 speed = 50.0;
        dPlayer = speed * dPlayer;
        
        if (dPlayer.x != 0.0 && dPlayer.y != 0.0)
        {
                dPlayer *= VELOCITY_CONSTANT;
        }
        
        Player player = oldGameState.player;

        // Friction force
        dPlayer += -8.0 * player.velocity;

        player.position += (square(dt) * 0.5 * dPlayer) + player.velocity;
        player.velocity += dt * dPlayer;

        // Adjust camera
        Camera camera = oldGameState.camera;

        /* Set Scrolling Type
         *
         *  Smooth Scrolling -> 0
         *  Fixed Scrolling  -> 1
         *
         */
        int32 scrollingType = 0;
        
        if ( scrollingType == 0 )
        {
                // Smooth scrolling
                camera.position.x = player.position.x - (SCREEN_WIDTH/2);
                camera.position.y = player.position.y - (SCREEN_HEIGHT/2);
        }
        else if ( scrollingType == 1 )
        {
                // Scroll by a fixed amount (full screen size)
                if (player.position.x - camera.position.x > SCREEN_WIDTH)
                {
                        camera.position.x += SCREEN_WIDTH;
                }
                if (player.position.x - camera.position.x < -player.size.x)
                {
                        camera.position.x -= SCREEN_WIDTH;
                }
                if (player.position.y - camera.position.y > SCREEN_HEIGHT)
                {
                        camera.position.y += SCREEN_HEIGHT;
                }
                if (player.position.y - camera.position.y < -player.size.y)
                {
                        camera.position.y -= SCREEN_HEIGHT;
                }
        }

        // New GameState
        GameState gameState = { player, camera };

        return gameState;
}

int32 main( int32 argc, char** argv )
{
        // Initialize SDL and create window
        SDL_Window* window = initializeSDL();
        if ( window == NULL )
        {
                printf( "Window could not be created. SDL_Error: %s\n",
                        SDL_GetError() );
                return 0;
        }

        // Create Renderer
        SDL_Renderer* renderer = createRenderer( window );
        if( renderer == NULL )
        {
                printf( "Renderer could not be created. SDL Error: %s\n",
                        SDL_GetError() );
                return 0;
        }


        Player player = {
                // position
                SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2,
                // velocity
                0, 0,
                // size
                30, 50
        };

        Camera camera = {
                // position
                0, 0,
                // size
                SCREEN_WIDTH, SCREEN_HEIGHT
        };

        GameState gameState;
        gameState.player = player;
        gameState.camera = camera;

        // While running
        bool quit = false;

        uint32 lastTime;
        uint32 currentTime = SDL_GetTicks();
        real32 dt;
        
        while ( !quit )
        {
                // Handle events on the queue
                quit = parseEvents();
                
                lastTime = currentTime;
                currentTime = SDL_GetTicks();
                dt = ((real32)currentTime - (real32)lastTime) / 1000;
                
                // Update game state
                gameState = updateGame( gameState, dt );

                // Draw to the screen
                draw( window, renderer, gameState );

                // Limit to 30 fps
                SDL_Delay( 1000 / 30 );
        }

        // Free resources and shutdown SDL
        shutdownSDL( window, renderer );
    
        return 0;
}
