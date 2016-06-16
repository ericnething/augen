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
#define global_persist static

struct TileMap
{
        real32  tileSize;
        int32   rows;
        int32   cols;
        uint32* tiles;
};

struct V2
{
        real32 x, y;
};

struct Player
{
        V2 position;
        V2 velocity;
        V2 size;
};

struct Camera
{
        V2 position;
        V2 size;
};

struct GameState
{
        Player player;
        Camera camera;
        TileMap tileMap;
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
        bool result;

        result = b > a;
                
        return result;
}

inline real32
square( real32 a )
{
        real32 result;
        result = a * a;
        return result;
}

inline uint8
colorReal32ToUint8( real32 color )
{
        uint8 result = (uint8)(color * 255);
        return result;
}

inline int32
roundReal32ToInt32( real32 a )
{
        int32 result = (int32)(a + 0.5f);
        return result;
}

inline uint32
roundReal32ToUInt32( real32 a )
{
        uint32 result = (uint32)(a + 0.5f);
        return result;
}

inline int32
truncateReal32ToInt32( real32 a )
{
        int32 result = (int32)a;
        return result;
}
