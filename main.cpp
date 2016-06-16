#include "main.h"

const int32 SCREEN_WIDTH = 1920/2;
const int32 SCREEN_HEIGHT = 1080/2;
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

internal TileMap*
getTileMap( World* world,
            int32 tileMapX,
            int32 tileMapY )
{
        TileMap* tileMap = 0;
        
        if ((tileMapX >= 0) && (tileMapX < world->countX) &&
            (tileMapY >= 0) && (tileMapY < world->countY))
        {
                tileMap = &world->tileMaps[(tileMapY * world->countX) + tileMapX];
        }

        return tileMap;
}

internal uint32
getTileValueUnchecked( World*   world,
                       TileMap* tileMap,
                       uint32   tileX,
                       uint32   tileY)
{
        assert(tileMap);
        assert((tileX >= 0) && (tileX < world->tileMapCountX) &&
               (tileY >= 0) && (tileY < world->tileMapCountY));

        uint32 tileValue = tileMap->tiles[(tileY * world->tileMapCountX) + tileX];
        return tileValue;
}

internal bool32
isTileMapPointEmpty( World*   world,
                     TileMap* tileMap,
                     V2       testPoint)
{
        bool32 isEmpty = false;

        if (tileMap)
        {
                int32 tileX = truncateReal32ToInt32(testPoint.x / world->tileSideInPixels);
                int32 tileY = truncateReal32ToInt32(testPoint.y / world->tileSideInPixels);
        
                if ((tileX >= 0) && (tileX < world->tileMapCountX) &&
                    (tileY >= 0) && (tileY < world->tileMapCountY))
                {
                        uint32 tileValue = getTileValueUnchecked(world, tileMap, tileX, tileY);
                        isEmpty = (tileValue == 0);
                }
        }
        return isEmpty;
}

internal bool32
isWorldPointEmpty( World* world,
                   uint32 testTileMapX,
                   uint32 testTileMapY,
                   V2     testPoint )
{
        bool32 isEmpty = false;

        int32 testTileX = truncateReal32ToInt32(testPoint.x / world->tileSideInPixels);
        int32 testTileY = truncateReal32ToInt32(testPoint.y / world->tileSideInPixels);

        if (testTileX < 0)
        {
                testTileX = world->tileMapCountX + testTileX;
                --testTileMapX;
        }
        if (testTileY < 0)
        {
                testTileY = world->tileMapCountY + testTileY;
                --testTileMapX;
        }
        if (testTileX >= world->tileMapCountX)
        {
                testTileX = testTileX - world->tileMapCountX;
                ++testTileMapX;
        }
        if (testTileY >= world->tileMapCountY)
        {
                testTileY = testTileY - world->tileMapCountY;
                ++testTileMapX;
        }

        TileMap* tileMap = getTileMap(world, testTileMapX, testTileMapY);
                
        if (tileMap)
        {
                isEmpty = isTileMapPointEmpty(world, tileMap, testPoint);
        }
        return isEmpty;
}

internal Camera
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

internal Player
updatePlayer( GameState gameState,
              real32 dt )
{
        World* world = gameState.world;
        Player player = gameState.player;
        const TileMap* tileMap = getTileMap(gameState.world, player.tileMapX, player.tileMapY);
        const real32 VELOCITY_CONSTANT = 0.7071067811865476;
        const uint8* keystate = SDL_GetKeyboardState( NULL );

        V2 dPlayer = { 0.0f ,0.0f };

        if ( keystate[ SDL_SCANCODE_W ] ) // Up
        {
                dPlayer.y += -1.0f;
        }
        if ( keystate[ SDL_SCANCODE_S ] ) // Down
        {
                dPlayer.y += 1.0f;
        }
        if ( keystate[ SDL_SCANCODE_A ] ) // Left
        {
                dPlayer.x += -1.0f;
        }
        if ( keystate[ SDL_SCANCODE_D ] ) // Right
        {
                dPlayer.x += 1.0f;
        }

        real32 speed = 180.0;
        dPlayer = speed * dPlayer;
        
        if (dPlayer.x != 0.0f && dPlayer.y != 0.0f)
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
        
        if (isWorldPointEmpty(world, player.tileMapX, player.tileMapY, newPositionLeftSide) &&
            isWorldPointEmpty(world, player.tileMapX, player.tileMapY, newPositionRightSide) &&
            isWorldPointEmpty(world, player.tileMapX, player.tileMapY, newPosition))
        {
                player.position = newPosition;
                player.velocity += dt * dPlayer;
        }

        return player;
}

// Update game state
internal GameState
updateGame( const GameState oldGameState,
            real32 dt )
{
        // Update player
        Player player = updatePlayer(oldGameState, dt);

        // int32 playerTileX = truncateReal32ToInt32(player.position.x - )

        // Adjust camera
        Camera camera = updateCamera(oldGameState);

        // New GameState
        GameState newGameState = oldGameState;
        newGameState.player = player;
        newGameState.camera = camera;

        return newGameState;
}

internal void
drawBackground( SDL_Renderer*   renderer,
                const GameState gameState )
{
        World* world = gameState.world;
        Camera camera = gameState.camera;
        TileMap* tileMap = getTileMap(gameState.world,
                                      gameState.player.tileMapX,
                                      gameState.player.tileMapY);

        assert(tileMap);
        // printf("TileMapCountY %d\n", world->tileMapCountY);
        
        for ( int row = 0; row < world->tileMapCountY; ++row )
        {
                for ( int col = 0; col < world->tileMapCountX; ++col )
                {
                        uint32 tileValue = getTileValueUnchecked(world, tileMap, col, row);
                        V2 position = {
                                col * world->tileSideInPixels - camera.position.x,
                                row * world->tileSideInPixels - camera.position.y
                        };
                        V2 size = { (real32)world->tileSideInPixels, (real32)world->tileSideInPixels };
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
internal void
draw( SDL_Window*     window,
      SDL_Renderer*   renderer,
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


        Player player;
        player.tileMapX = 0;
        player.tileMapY = 0;
        player.position = { 160, 200 }; // (map coordinates)
        player.velocity = { 0, 0 };
        player.size     = { 30, 50 };

        Camera camera;
        camera.position = { 0, 0 }; // (map coordinates)
        camera.size     = { SCREEN_WIDTH, SCREEN_HEIGHT };

        // Tilemap
        const uint32 TILE_MAP_ROWS = 12;
        const uint32 TILE_MAP_COLS = 16;
        
        uint32 tiles00[TILE_MAP_ROWS][TILE_MAP_COLS] =
                {
                        { 1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1 },
                        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1 },
                        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1 },
                        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1 },
                        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 1, 1, 1,  0, 0, 0, 1 },
                        { 1, 0, 0, 0,  0, 0, 1, 0,  0, 0, 0, 1,  0, 0, 0, 0 },
                        { 1, 0, 0, 0,  0, 0, 1, 0,  0, 0, 0, 1,  0, 0, 0, 0 },
                        { 0, 0, 0, 0,  0, 1, 1, 1,  0, 0, 0, 1,  0, 0, 0, 1 },
                        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1,  1, 0, 0, 1 },
                        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1 },
                        { 1, 0, 0, 0,  0, 0, 0, 1,  0, 0, 0, 0,  0, 0, 0, 1 },
                        { 1, 0, 0, 0,  0, 0, 0, 1,  0, 0, 0, 0,  0, 0, 0, 1 }
                };
        uint32 tiles01[TILE_MAP_ROWS][TILE_MAP_COLS] =
                {
                        { 1, 1, 1, 1,  0, 0, 1, 1,  0, 0, 0, 0,  0, 1, 1, 1 },
                        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1 },
                        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1 },
                        { 1, 0, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0 },
                        { 1, 0, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0 },
                        { 1, 0, 0, 0,  1, 1, 1, 1,  1, 0, 0, 0,  0, 0, 0, 1 },
                        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1 },
                        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1 },
                        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1, 0, 0, 1 },
                        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1 },
                        { 1, 0, 0, 0,  0, 0, 1, 1,  0, 0, 0, 0,  0, 0, 0, 1 },
                        { 1, 0, 0, 0,  0, 0, 1, 0,  0, 0, 0, 0,  0, 0, 0, 1 }
                };
        uint32 tiles10[TILE_MAP_ROWS][TILE_MAP_COLS] =
                {
                        { 1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1 },
                        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1 },
                        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1 },
                        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1 },
                        { 1, 0, 0, 0,  0, 1, 1, 1,  1, 1, 0, 0,  0, 0, 0, 1 },
                        { 0, 0, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0,  0, 1, 0, 1 },
                        { 0, 0, 0, 0,  1, 0, 0, 0,  0, 1, 0, 0,  0, 1, 0, 1 },
                        { 1, 0, 0, 0,  1, 0, 0, 0,  0, 1, 0, 0,  1, 1, 0, 1 },
                        { 1, 0, 0, 0,  1, 1, 1, 0,  1, 1, 0, 1,  1, 0, 0, 1 },
                        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1 },
                        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1 },
                        { 1, 0, 0, 1,  1, 1, 1, 1,  1, 1, 0, 0,  1, 1, 1, 1 }
                };
        uint32 tiles11[TILE_MAP_ROWS][TILE_MAP_COLS] =
                {
                        { 1, 0, 0, 1,  1, 1, 1, 1,  1, 1, 0, 0,  1, 1, 1, 1 },
                        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1 },
                        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1 },
                        { 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1 },
                        { 0, 0, 1, 1,  0, 1, 0, 0,  1, 1, 0, 0,  0, 0, 0, 1 },
                        { 1, 0, 1, 0,  0, 1, 0, 0,  0, 1, 0, 0,  0, 1, 0, 1 },
                        { 1, 0, 1, 0,  1, 1, 0, 1,  0, 1, 0, 0,  0, 1, 0, 1 },
                        { 1, 0, 1, 0,  0, 0, 0, 1,  0, 1, 0, 0,  1, 1, 0, 1 },
                        { 1, 0, 0, 0,  1, 1, 1, 1,  1, 1, 0, 1,  1, 0, 0, 1 },
                        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1 },
                        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1 },
                        { 1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1 }
                };
        
        TileMap tileMaps[2][2];

        tileMaps[0][0].tiles = (uint32*)tiles00;
        tileMaps[1][0].tiles = (uint32*)tiles01;
        tileMaps[0][1].tiles = (uint32*)tiles10;
        tileMaps[1][1].tiles = (uint32*)tiles11;

        World world;
        world.tileSideInMeters = 1.4f;
        world.tileSideInPixels = 64;
        world.countX = 2;
        world.countY = 2;
        world.tileMaps = (TileMap*)tileMaps;
        world.tileMapCountX = TILE_MAP_COLS;
        world.tileMapCountY = TILE_MAP_ROWS;

        // printf("%d %d %d\n",
        //        world.tileMaps[0].tiles[0],
        //        world.tileMaps[0].tiles[1],
        //        world.tileMaps[0].tiles[2]);

        GameState gameState;
        gameState.player = player;
        gameState.camera = camera;
        gameState.world = &world;
        
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
