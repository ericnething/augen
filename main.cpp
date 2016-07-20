#include "main.h"
#include "intrinsics.h"
#include "sdl.h"

const real32 TILE_SIZE = 64.0f;

internal WorldPosition
recanonicalizePosition(World* world, WorldPosition pos)
{
        real32 tileSize = world->tileSideInMeters;
        real32 tileRadius = tileSize / 2;
        
        if (pos.relative.x < -tileRadius)
        {
                int32 tileOffset = floorReal32ToInt32(pos.relative.x / tileSize);
                pos.tileX += tileOffset;
                pos.relative.x -= tileSize * tileOffset;
        }
        if (pos.relative.x >= tileRadius)
        {
                int32 tileOffset = floorReal32ToInt32(pos.relative.x / tileSize) + 1;
                pos.tileX += tileOffset;
                pos.relative.x -= tileSize * tileOffset;
        }
        if (pos.relative.y < -tileRadius)
        {
                int32 tileOffset = floorReal32ToInt32(pos.relative.y / tileSize);
                pos.tileY += tileOffset;
                pos.relative.y -= tileSize * tileOffset;
        }
        if (pos.relative.y >= tileRadius)
        {
                int32 tileOffset = floorReal32ToInt32(pos.relative.y / tileSize) + 1;
                pos.tileY += tileOffset;
                pos.relative.y -= tileSize * tileOffset;
        }
        
        return pos;
}

internal uint32
getTileValue(World* world, WorldPosition pos)
{
        if ((pos.tileX < 0 || pos.tileX >= world->tileCountX) ||
            (pos.tileY < 0 || pos.tileY >= world->tileCountY))
        {
                // Invalid position. Out of bounds.
                return 2;
        }
        uint32 tileValue = world->tileMap->tiles[ pos.tileY * world->tileCountX + pos.tileX ];
        return tileValue;
}

internal bool32
isTileEmpty(World* world, WorldPosition pos)
{
        bool32 isEmpty = false;
        uint32 tileValue = getTileValue(world, pos);
        
        if (tileValue == 0)
        {
                isEmpty = true;
        }
        return isEmpty;
}

internal V2
getScreenCoordinates(World* world, WorldPosition pos)
{
        real32 tileSize = world->tileSideInPixels;
        real32 offsetForRightHandCoordinates = SCREEN_HEIGHT;
        V2 screenCoordinates = {
                tileSize * pos.tileX + pos.relative.x * world->metersToPixels,
                offsetForRightHandCoordinates - (tileSize * pos.tileY + pos.relative.y * world->metersToPixels)
        };
        return screenCoordinates;
}

internal Camera
updateCamera( GameState gameState )
{
        Camera camera = gameState.camera;
        Player player = gameState.player;
        V2 screenSize = { SCREEN_WIDTH, SCREEN_HEIGHT };
        
        int32 scrollingType = 0;

        // Smooth scrolling
        if ( scrollingType == 0 )
        {
                real32 tileRadius = gameState.world->tileSideInMeters / 2;
                V2 offsetForTileCenterAsOrigin = { tileRadius, tileRadius };
                V2 offset = (1/gameState.world->metersToPixels) * (-0.5f)*screenSize + offsetForTileCenterAsOrigin;
                camera.position = player.position;
                camera.position.relative += offset;
                        
        }
        // Scroll by a fixed amount (full screen size)
        else if ( scrollingType == 1 )
        {
                WorldPosition differenceInPosition = player.position - camera.position;
                differenceInPosition = recanonicalizePosition(gameState.world, differenceInPosition);
                V2 origin = getScreenCoordinates(gameState.world, differenceInPosition);
                
                if (origin.x > screenSize.x)
                {
                        camera.position.relative.x += (1/gameState.world->metersToPixels) * screenSize.x;
                }
                if (origin.y > screenSize.y)
                {
                        // note: subtract the screen height instead of adding for right-hand coordinates
                        camera.position.relative.y += (1/gameState.world->metersToPixels) * (-1)*screenSize.y;
                }
                if (origin.x < (-1/2)*player.size.x)
                {
                        camera.position.relative.x += (1/gameState.world->metersToPixels) * (-1)*screenSize.x;
                }
                if (origin.y < (-1)*player.size.y)
                {
                        // note: add the screen height instead of subtracting for right-hand coordinates
                        camera.position.relative.y += (1/gameState.world->metersToPixels) * screenSize.y;
                }
        }
        camera.position = recanonicalizePosition(gameState.world, camera.position);
        return camera;
}

internal Player
updatePlayer( GameState gameState, real32 dt )
{
        World* world = gameState.world;
        Player player = gameState.player;
        uint32 tileSize = world->tileSideInMeters;
        
        const real32 VELOCITY_CONSTANT = 0.7071067811865476;
        const uint8* keystate = SDL_GetKeyboardState( NULL );

        V2 dPlayer = { 0.0f ,0.0f };

        if ( keystate[ SDL_SCANCODE_W ] ) // Up
        {
                dPlayer.y += 1.0f;
        }
        if ( keystate[ SDL_SCANCODE_S ] ) // Down
        {
                dPlayer.y += -1.0f;
        }
        if ( keystate[ SDL_SCANCODE_A ] ) // Left
        {
                dPlayer.x += -1.0f;
        }
        if ( keystate[ SDL_SCANCODE_D ] ) // Right
        {
                dPlayer.x += 1.0f;
        }

        // if ( keystate[ SDL_SCANCODE_UP ] ) // Zoom in
        // {
        //         world->tileSideInPixels += 1.0f;
        // }
        // if ( keystate[ SDL_SCANCODE_DOWN ] ) // Zoom out
        // {
        //         world->tileSideInPixels -= 1.0f;
        // }

        real32 speed = 4.0;
        dPlayer = speed * dPlayer;
        
        if (dPlayer.x != 0.0f && dPlayer.y != 0.0f)
        {
                dPlayer *= VELOCITY_CONSTANT;
        }

        // Friction force
        // dPlayer += -10.0 * player.velocity;
        
        // V2 newRelativePosition = player.position.relative + (square(dt) * 0.5 * dPlayer) + player.velocity;
        V2 newRelativePosition = player.position.relative + dt*dPlayer;
        
        WorldPosition newPosition = player.position;
        newPosition.relative = newRelativePosition;
        newPosition = recanonicalizePosition(world, newPosition);

        WorldPosition newPositionLeft = newPosition;
        newPositionLeft.relative.x -= 0.5f * player.size.x;
        newPositionLeft = recanonicalizePosition(world, newPositionLeft);
        
        WorldPosition newPositionRight = newPosition;
        newPositionRight.relative.x += 0.5f * player.size.x;
        newPositionRight = recanonicalizePosition(world, newPositionRight);

        // Check if the move is valid
        if (isTileEmpty(world, newPosition) &&
            isTileEmpty(world, newPositionLeft) &&
            isTileEmpty(world, newPositionRight))
        {
                // set player position and velocity
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
        TileMap* tileMap = world->tileMap;
        Camera camera = gameState.camera;
        Player player = gameState.player;

        // for (int32 row = 0; row < world->tileCountY; ++row)
        // {
        //         for (int32 col = 0; col < world->tileCountX; ++col)
        //         {
        //                 // uint32 tileValue = tileMap->tiles[row * world->tileCountX + col];
        //                 WorldPosition testPosition;
        //                 testPosition.tileX = col;
        //                 testPosition.tileY = row;
        //                 testPosition.relative = { 0.0f, 0.0f };
        //                 uint32 tileValue = getTileValue(world, testPosition);

        //                 real32 color = 0.0f;

        //                 if (tileValue == 0)
        //                 {
        //                         color = 0.5f;
        //                 }
        //                 if (tileValue == 1)
        //                 {
        //                         color = 1.0f;
        //                 }
        //                 if (player.position.tileX == col && player.position.tileY == row)
        //                 {
        //                         color = 0.0f;
        //                 }

        //                 V2 screenSize = { SCREEN_WIDTH, SCREEN_HEIGHT };
        //                 real32 tileSize = world->tileSideInPixels;
        //                 V2 size = { tileSize, tileSize };
                        
        //                 WorldPosition differenceInPosition = testPosition - camera.position;
        //                 differenceInPosition = recanonicalizePosition(world, differenceInPosition);
        //                 V2 origin = getScreenCoordinates(world, differenceInPosition);
        //                 origin.y -= tileSize; // to account for flipped y-coordinate

        //                 if ( origin > (-1)*size && origin < screenSize)
        //                 {
        //                         drawRectangle( renderer, origin, size, color, color, color, 1.0 );
        //                 }
        //         }
        // }


        int32 cameraMaxY = camera.position.tileY + camera.size.y;
        int32 cameraMaxX = camera.position.tileX + camera.size.x;
        
        for (int32 row = camera.position.tileY - 1; row < cameraMaxY; ++row)
        {
                for (int32 col = camera.position.tileX - 1; col < cameraMaxX; ++col)
                {
                        WorldPosition testPosition;
                        testPosition.tileX = col;
                        testPosition.tileY = row;
                        testPosition.relative = { 0.0f, 0.0f };
                        uint32 tileValue = getTileValue(world, testPosition);

                        real32 color = 0.0f;

                        if (tileValue == 2)
                        {
                                color = 0.0f;
                        }
                        if (tileValue == 0)
                        {
                                color = 0.5f;
                        }
                        if (tileValue == 1)
                        {
                                color = 1.0f;
                        }
                        if (player.position.tileX == col && player.position.tileY == row)
                        {
                                color = 0.0f;
                        }

                        V2 screenSize = { SCREEN_WIDTH, SCREEN_HEIGHT };
                        real32 tileSize = world->tileSideInPixels;
                        V2 size = { tileSize, tileSize };
                        WorldPosition differenceInPosition = testPosition - camera.position;
                        differenceInPosition = recanonicalizePosition(world, differenceInPosition);
                        V2 origin = getScreenCoordinates(world, differenceInPosition);
                        origin.y -= tileSize; // to account for flipped y-coordinate

                        if ( origin > (-1)*size && origin < screenSize)
                        {
                                drawRectangle( renderer, origin, size, color, color, color, 1.0 );
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
        real32 tileRadius = gameState.world->tileSideInMeters / 2;
        V2 offsetForCenterOfTile = { tileRadius, tileRadius };
        V2 playerCenter = { player.size.x / 2, -player.size.y }; // negate player height for flipped y-coordinate
        player.position.relative = player.position.relative - playerCenter + offsetForCenterOfTile;

        WorldPosition differenceInPosition = player.position - camera.position;
        differenceInPosition = recanonicalizePosition(gameState.world, differenceInPosition);
        V2 origin = getScreenCoordinates(gameState.world, differenceInPosition);
        V2 size = gameState.world->metersToPixels * player.size;
        
        drawRectangle( renderer, origin, size, 1.0, 1.0, 0.0, 1.0 );
        
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

        // Tilemap
        const uint32 TILE_MAP_ROWS = 24;
        const uint32 TILE_MAP_COLS = 32;

        printf("before tempTiles");
        
        uint32 tempTiles[TILE_MAP_ROWS][TILE_MAP_COLS] = {
                
                { 1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1 },
                { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1 },
                { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1 },
                { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1 },
                { 1, 0, 0, 0,  0, 0, 0, 0,  0, 1, 1, 1,  0, 0, 0, 1,  0, 0, 0, 0,  0, 1, 1, 1,  1, 1, 0, 0,  0, 0, 0, 1 },
                { 1, 0, 0, 0,  0, 0, 1, 0,  0, 0, 0, 1,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0,  0, 1, 0, 1 },
                { 1, 0, 0, 0,  0, 0, 1, 0,  0, 0, 0, 1,  0, 0, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 1, 0, 0,  0, 1, 0, 1 },
                { 0, 0, 0, 0,  0, 1, 1, 1,  0, 0, 0, 1,  0, 0, 0, 1,  1, 0, 0, 0,  1, 0, 0, 0,  0, 1, 0, 0,  1, 1, 0, 1 },
                { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1,  1, 0, 0, 1,  0, 0, 0, 0,  1, 1, 1, 0,  1, 1, 0, 1,  1, 0, 0, 1 },
                { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1 },
                { 1, 0, 0, 0,  0, 0, 0, 1,  0, 0, 0, 0,  0, 0, 0, 1,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1 },
                { 1, 0, 0, 0,  0, 0, 0, 1,  0, 0, 0, 0,  0, 0, 0, 1,  0, 0, 0, 1,  1, 1, 1, 1,  0, 1, 0, 0,  0, 0, 0, 1 },
                { 1, 1, 1, 1,  0, 0, 1, 1,  0, 0, 0, 0,  0, 1, 1, 1,  0, 0, 0, 1,  1, 0, 0, 1,  0, 1, 0, 0,  1, 1, 1, 1 },
                { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1,  1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1 },
                { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1 },
                { 1, 0, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1 },
                { 1, 0, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 1, 1,  0, 1, 0, 0,  1, 1, 0, 0,  0, 0, 0, 1 },
                { 1, 0, 0, 0,  1, 1, 1, 1,  1, 0, 0, 0,  0, 0, 0, 1,  1, 0, 1, 0,  0, 1, 0, 0,  0, 1, 0, 0,  0, 1, 0, 1 },
                { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1, 0, 1, 0,  1, 1, 0, 1,  0, 1, 0, 0,  0, 1, 0, 1 },
                { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1, 0, 1, 0,  0, 0, 0, 1,  0, 1, 0, 0,  1, 1, 0, 1 },
                { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  1, 0, 0, 0,  1, 1, 1, 1,  0, 1, 0, 1,  1, 0, 0, 1 },
                { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1 },
                { 1, 0, 0, 0,  0, 0, 1, 1,  0, 0, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1 },
                { 1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1 }
        };

        TileMap tempTileMap;
        tempTileMap.tiles = (uint32*)tempTiles;
        
        World world;
        world.tileSideInMeters = 1.4f;
        world.tileSideInPixels = TILE_SIZE;
        world.metersToPixels = world.tileSideInPixels / world.tileSideInMeters;
        world.tileMap = &tempTileMap;
        world.tileCountX = TILE_MAP_COLS;
        world.tileCountY = TILE_MAP_ROWS;
        
        Player player;
        player.position.tileX = 2;
        player.position.tileY = 2;
        player.position.relative = { 0.2f, 0.5f };
        player.velocity = { 0.0f, 0.0f };
        player.size     = { 0.46875f * world.tileSideInMeters, 0.78125f * world.tileSideInMeters };

        Camera camera;
        camera.position = player.position;
        camera.position.relative = { 0.0f, 0.0f };
        camera.size = { SCREEN_WIDTH / world.tileSideInPixels + 1, SCREEN_HEIGHT / world.tileSideInPixels + 1 };
        
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
                               gameState.player.position.relative.x,
                               gameState.player.position.relative.y);
                        printf("PlayerTile (%d, %d)\n",
                               gameState.player.position.tileX,
                               gameState.player.position.tileY);
                        printf("Camera (%f, %f)\n",
                               gameState.camera.position.relative.x,
                               gameState.camera.position.relative.y);
                        printf("CameraTile (%d, %d)\n",
                               gameState.camera.position.tileX,
                               gameState.camera.position.tileY);
                }
        }

        // Free resources and shutdown SDL
        shutdownSDL( window, renderer );
    
        return 0;
}
