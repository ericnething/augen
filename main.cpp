#include "main.h"

const int32 SCREEN_WIDTH = 1920/2;
const int32 SCREEN_HEIGHT = 1080/2;
const uint32 TILE_MAP_ROWS = 36;
const uint32 TILE_MAP_COLS = 16;
const real32 TILE_SIZE = 64;

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

internal void
drawBackground( SDL_Renderer* renderer,
                const GameState gameState )
{
        const Camera camera = gameState.camera;
        const TileMap tileMap = gameState.tileMap;
        
        for ( int row = 0; row < tileMap.rows; ++row )
        {
                for ( int col = 0; col < tileMap.cols; ++col )
                {
                        int32 tileValue = tileMap.tiles[(row * tileMap.cols) + col];
                        V2 position = {
                                col*tileMap.tileSize - camera.position.x,
                                row*tileMap.tileSize - camera.position.y
                        };
                        V2 size = { tileMap.tileSize, tileMap.tileSize };
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
void
draw( SDL_Window* window,
      SDL_Renderer* renderer,
      const GameState gameState )
{
        Player player = gameState.player;
        Camera camera = gameState.camera;
        
        //Clear screen
        setRenderDrawColor( renderer, 0.0, 0.0, 0.0, 1.0 );
        SDL_RenderClear( renderer );

        drawBackground( renderer, gameState );
        
        // Draw player
        V2 playerCenter = { player.size.x / 2, player.size.y };
        drawRectangle( renderer,
                       player.position - camera.position - playerCenter,
                       player.size,
                       1.0, 1.0, 0.0, 1.0 );
        
        //Update screen
        SDL_RenderPresent( renderer );
}

Camera
updateCamera( GameState gameState )
{
        Camera camera = gameState.camera;
        Player player = gameState.player;
        int32 scrollingType = 0;

        // Smooth scrolling
        if ( scrollingType == 0 )
        {
                camera.position.x = player.position.x - (SCREEN_WIDTH/2);
                camera.position.y = player.position.y - (SCREEN_HEIGHT/2);
        }
        // Scroll by a fixed amount (full screen size)
        else if ( scrollingType == 1 )
        {
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

        return camera;
}

internal bool32
isTileMapPointEmpty( TileMap tileMap,
                     V2 testPoint )
{
        int32 tileX = truncateReal32ToInt32(testPoint.x / tileMap.tileSize);
        int32 tileY = truncateReal32ToInt32(testPoint.y / tileMap.tileSize);
        bool32 isEmpty = false;
        
        if ((tileX >= 0) && (tileX < tileMap.cols) &&
            (tileY >= 0) && (tileY < tileMap.rows))
        {
                uint32 tileValue = tileMap.tiles[(tileY * tileMap.cols) + tileX];
                isEmpty = (tileValue == 0);
        }
        return isEmpty;
}

Player
updatePlayer( GameState gameState,
              real32 dt )
{
        Player player = gameState.player;
        const TileMap tileMap = gameState.tileMap;
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

        real32 speed = 250.0;
        dPlayer = speed * dPlayer;
        
        if (dPlayer.x != 0.0 && dPlayer.y != 0.0)
        {
                dPlayer *= VELOCITY_CONSTANT;
        }

        // Friction force
        //dPlayer += -8.0 * player.velocity;

        // Check if the move is valid
        // V2 newPosition = player.position + (square(dt) * 0.5 * dPlayer) + player.velocity;
        V2 newPosition = player.position + (dt * dPlayer);
        V2 newPositionLeftSide = { newPosition.x - (0.5f * player.size.x), newPosition.y };
        V2 newPositionRightSide = { newPosition.x + (0.5f * player.size.x), newPosition.y };
        
        if (isTileMapPointEmpty(tileMap, newPositionLeftSide) &&
            isTileMapPointEmpty(tileMap, newPositionRightSide) &&
            isTileMapPointEmpty(tileMap, newPosition))
        {
                player.position = newPosition;
                player.velocity += dt * dPlayer;
        }

        return player;
}

// Update game state
GameState
updateGame( const GameState oldGameState,
            real32 dt )
{
        // Update player
        Player player = updatePlayer(oldGameState, dt);

        // int32 playerTileX = truncateReal32ToInt32(player.position.x - )

        // Adjust camera
        Camera camera = updateCamera(oldGameState);

        // New GameState
        GameState gameState = { player, camera, oldGameState.tileMap };

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
                160, 200, // position (map coordinates)
                0, 0, // velocity
                30, 50 // size
        };

        Camera camera = {
                0, 0, // position (map coordinates)
                SCREEN_WIDTH, SCREEN_HEIGHT // size
        };

        // Tilemap
        uint32 tiles1[TILE_MAP_ROWS][TILE_MAP_COLS] =
                {
                        { 1, 1, 1, 1,  1, 0, 0, 0,  1, 1, 1, 1,  1, 1, 1, 1 },
                        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1 },
                        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1 },
                        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1 },
                        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 1, 1, 1,  0, 0, 0, 1 },
                        { 1, 0, 0, 0,  0, 0, 1, 0,  0, 0, 0, 1,  0, 0, 0, 1 },
                        { 1, 0, 0, 0,  0, 0, 1, 0,  0, 0, 0, 1,  0, 0, 0, 1 },
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
        
        TileMap tileMap1 = {
                TILE_SIZE,
                TILE_MAP_ROWS,
                TILE_MAP_COLS,
                (uint32*)tiles1
        };
        
        GameState gameState;
        gameState.player = player;
        gameState.camera = camera;
        gameState.tileMap = tileMap1;

        // While running
        bool quit = false;

        uint32 lastTime;
        uint32 currentTime = SDL_GetTicks();
        real32 dt;
        int32 consoleCounter = 0;
        
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

                // Limit to 60 fps
                SDL_Delay( 1000 / 60 );
                
                consoleCounter++;
                if (consoleCounter >= 60)
                {
                        consoleCounter = 0;
                        printf("Player (%f, %f)\n",
                               gameState.player.position.x,
                               gameState.player.position.y);
                        printf("Camera (%f, %f)\n",
                               gameState.camera.position.x,
                               gameState.camera.position.y);
                }
        }

        // Free resources and shutdown SDL
        shutdownSDL( window, renderer );
    
        return 0;
}
