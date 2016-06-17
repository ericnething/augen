// Intrinsics
#include <math.h>

inline real32
square( real32 a )
{
        real32 result;
        result = a * a;
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

inline int32
floorReal32ToInt32( real32 a )
{
        int32 result = (int32)floorf(a);
        return result;
}
