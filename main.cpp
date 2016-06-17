#include "main.h"
#include "intrinsics.h"
#include "sdl.h"

const real32 TILE_SIZE = 64;

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
                     uint32   tileX,
                     uint32   tileY)
{
        bool32 isEmpty = false;

        if (tileMap)
        {
                // int32 tileX = truncateReal32ToInt32(testTileX / world->tileSideInPixels);
                // int32 tileY = truncateReal32ToInt32(testTileY / world->tileSideInPixels);
        
                if ((tileX >= 0) && (tileX < world->tileMapCountX) &&
                    (tileY >= 0) && (tileY < world->tileMapCountY))
                {
                        uint32 tileValue = getTileValueUnchecked(world, tileMap, tileX, tileY);
                        isEmpty = (tileValue == 0);
                }
        }
        return isEmpty;
}

inline CanonicalPosition
getCanonicalPosition(World* world, RawPosition pos)
{
        CanonicalPosition result;
        result.tileMapX = pos.tileMapX;
        result.tileMapY = pos.tileMapY;

        real32 x = pos.x;
        real32 y = pos.y;
        
        result.tileX = floorReal32ToInt32( x / world->tileSideInPixels );
        result.tileY = floorReal32ToInt32( y / world->tileSideInPixels );

        result.tileRelX = x - result.tileX * world->tileSideInPixels;
        result.tileRelY = y - result.tileY * world->tileSideInPixels;

        //printf("TileRelative (%f, %f)\n", result.tileRelX, result.tileRelY);
        assert(result.tileRelX >= 0);
        assert(result.tileRelY >= 0);
        assert(result.tileRelX < world->tileSideInPixels);
        assert(result.tileRelY < world->tileSideInPixels);
        
        if (result.tileX < 0)
        {
                result.tileX = world->tileMapCountX + result.tileX;
                --result.tileMapX;
        }
        if (result.tileY < 0)
        {
                result.tileY = world->tileMapCountY + result.tileY;
                --result.tileMapY;
        }
        if (result.tileX >= world->tileMapCountX)
        {
                result.tileX = result.tileX - world->tileMapCountX;
                ++result.tileMapX;
        }
        if (result.tileY >= world->tileMapCountY)
        {
                result.tileY = result.tileY - world->tileMapCountY;
                ++result.tileMapY;
        }
        return result;
}


internal bool32
isWorldPointEmpty( World* world, RawPosition testPos )
{
        bool32 isEmpty = false;
        CanonicalPosition canonicalPosition = getCanonicalPosition(world, testPos);
        TileMap* tileMap = getTileMap(world, canonicalPosition.tileMapX,
                                             canonicalPosition.tileMapY);
        if (tileMap)
        {
                isEmpty = isTileMapPointEmpty(world, tileMap,
                                              canonicalPosition.tileX,
                                              canonicalPosition.tileY);
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
updatePlayer( GameState gameState, real32 dt )
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
        printf("New Player position (%f, %f)\n", newPosition.x, newPosition.y);
        RawPosition newPlayerPos = {
                player.tileMapX, player.tileMapY,
                newPosition.x, newPosition.y
        };
        RawPosition newPlayerLeft = newPlayerPos;
        newPlayerLeft.x = newPosition.x - (0.5f * player.size.x);
        RawPosition newPlayerRight = newPlayerPos;
        newPlayerRight.x = newPosition.x + (0.5f * player.size.x);
        
        if (isWorldPointEmpty(world, newPlayerLeft) &&
            isWorldPointEmpty(world, newPlayerRight) &&
            isWorldPointEmpty(world, newPlayerPos))
        {
                CanonicalPosition canonicalPos
                        = getCanonicalPosition(world, newPlayerPos);

                player.tileMapX = canonicalPos.tileMapX;
                player.tileMapY = canonicalPos.tileMapY;
                
                player.position.x = (canonicalPos.tileX * world->tileSideInPixels) + canonicalPos.tileRelX;
                player.position.y = (canonicalPos.tileY * world->tileSideInPixels) + canonicalPos.tileRelY;
                
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
draw( SDL_Window* window, SDL_Renderer* renderer, const GameState gameState )
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
                        { 1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1 }
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
                if (consoleCounter >= 10)
                {
                        consoleCounter = 0;
                        printf("Player (%f, %f)\n",
                               gameState.player.position.x,
                               gameState.player.position.y);
                        printf("PlayerTile (%d, %d)\n",
                               gameState.player.tileMapX,
                               gameState.player.tileMapY);
                        printf("Camera (%f, %f)\n",
                               gameState.camera.position.x,
                               gameState.camera.position.y);
                }
        }

        // Free resources and shutdown SDL
        shutdownSDL( window, renderer );
    
        return 0;
}
