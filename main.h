#if defined __APPLE__
#include <SDL2/SDL.h>
#else
#include "SDL.h"
#endif

#include <stdio.h>
#include <string>

typedef int8_t  int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t  uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef float  real32;
typedef double real64;

typedef int32_t bool32;

#define internal static
#define local_persist static
#define global_variable static

struct V2
{
        real32 x, y;
};

struct TileChunkPosition
{
        uint32 tileMapX;
        uint32 tileMapY;

        int32 tileX;
        int32 tileY;
};

struct WorldPosition
{
        int32 tileX;
        int32 tileY;
        V2 relative;
};

struct TileMap
{
        uint32* tiles;
};

struct World
{
        real32 tileSideInMeters;
        real32 tileSideInPixels;
        real32 metersToPixels;

        int32 tileCountX;
        int32 tileCountY;
        
        TileMap* tileMap;
};

struct Player
{
        WorldPosition position;
        V2 velocity;
        V2 size;
};

struct Camera
{
        WorldPosition position;
        V2 size;
};

struct GameState
{
        Player player;
        Camera camera;
        World* world;
};

inline V2
operator+( V2 a, V2 b )
{
        V2 result;
        
        result.x = a.x + b.x;
        result.y = a.y + b.y;

        return result;
}

inline V2 &
operator+=( V2 &a, V2 b )
{
        a = a + b;
        return a;
}

inline V2
operator-( V2 a, V2 b )
{
        V2 result;
        
        result.x = a.x - b.x;
        result.y = a.y - b.y;

        return result;
}

inline V2
operator*( real32 a, V2 b )
{
        V2 result;
        
        result.x = a * b.x;
        result.y = a * b.y;
        
        return result;
}

inline V2
operator/( V2 a, real32 b )
{
        V2 result;
        
        result.x = a.x / b;
        result.y = a.y / b;
        
        return result;
}

inline V2 &
operator*=( V2 &a, real32 b )
{
        a = b * a;
        return a;
}

inline bool
operator>( V2 a, V2 b )
{
        bool result = false;
        if (a.x > b.x && a.y > b.y)
        {
                result = true;
        }
        return result;
}

inline bool
operator<( V2 a, V2 b )
{
        bool result = b > a;
        return result;
}

inline WorldPosition
operator+( WorldPosition a, WorldPosition b )
{
        WorldPosition result;

        result.tileX = a.tileX + b.tileX;
        result.tileY = a.tileY + b.tileY;
        result.relative = a.relative + b.relative;
        
        return result;
}


inline WorldPosition
operator-( WorldPosition a, WorldPosition b )
{
        WorldPosition result;

        b.tileX *= -1;
        b.tileY *= -1;
        b.relative *= -1;
        result = a + b;
        
        return result;
}

inline bool
operator>( WorldPosition a, WorldPosition b )
{
        bool result = false;
        if (a.tileX > b.tileX &&
            a.tileY > b.tileY &&
            a.relative > b.relative)
        {
                result = true;
        }
        return result;
}

inline bool
operator<( WorldPosition a, WorldPosition b )
{
        bool result = b > a;
        return result;
}

inline uint8
colorReal32ToUint8( real32 color )
{
        uint8 result = (uint8)(color * 255);
        return result;
}

// For use in development
#define assert(expression) if(!(expression)) {*(int *)0 = 0;}

