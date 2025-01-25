// Cross compiled with SPIRV-Cross from ReadAfterWrite.hlsl
// DO NOT EDIT

#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>

using namespace metal;

struct Entry
{
    uint a;
    uint b;
};

struct type_RWStructuredBuffer_Entry
{
    Entry _m0[1];
};

struct type_Globals
{
    uint readPos;
    uint writePos;
};

kernel void CSMain(
    constant type_Globals& _Globals [[buffer(0)]],
    device uint* buf1 [[buffer(1)]],
    device type_RWStructuredBuffer_Entry& buf2 [[buffer(2)]],
    texture1d<uint, access::read_write> tex1 [[texture(3)]],
    texture2d<uint, access::read_write> tex2 [[texture(4)]],
    uint3 gl_GlobalInvocationID [[thread_position_in_grid]])
{
    uint _38 = _Globals.readPos + gl_GlobalInvocationID.x;
    uint _44 = _Globals.writePos + gl_GlobalInvocationID.x;
    buf1[_44] = buf1[_38];
    buf2._m0[_44] = buf2._m0[_38];
    tex1.write(uint4(tex1.read(uint(_38)).x), uint(_44));
    tex2.write(tex2.read(uint2(uint2(_38, 0u))).xy.xyyy, uint2(uint2(_44, 0u)));
}

