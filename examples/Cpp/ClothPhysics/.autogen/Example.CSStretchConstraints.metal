#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

// Returns 2D texture coords corresponding to 1D texel buffer coords
static inline __attribute__((always_inline))
uint2 spvTexelBufferCoord(uint tc)
{
    return uint2(tc % 4096, tc / 4096);
}

template <typename ImageT>
void spvImageFence(ImageT img) { img.fence(); }

struct type_SceneState
{
    float4x4 wvpMatrix;
    float4x4 wMatrix;
    float4 gravity;
    uint2 gridSize;
    uint2 _pad0;
    float damping;
    float dTime;
    float dStiffness;
    float _pad1;
    float4 lightVec;
};

kernel void CSStretchConstraints(constant type_SceneState& SceneState [[buffer(0)]], texture2d<float> parBase [[texture(1)]], texture2d<float> parCurrPos [[texture(2)]], texture2d<float, access::write> parNextPos [[texture(3)]], texture2d<float, access::read_write> parNormal [[texture(6)]], uint3 gl_GlobalInvocationID [[thread_position_in_grid]])
{
    uint _63 = (gl_GlobalInvocationID.y * SceneState.gridSize.x) + gl_GlobalInvocationID.x;
    float4 _65 = parCurrPos.read(spvTexelBufferCoord(_63));
    float4 _67 = parBase.read(spvTexelBufferCoord(_63));
    float4 _73 = float4((_67.x * 2.0) - 1.0, 0.0, _67.y * (-2.0), 1.0);
    spvImageFence(parNormal);
    float4 _76 = parBase.read(spvTexelBufferCoord(_63));
    float _77 = _76.z;
    int2 _78 = int2(gl_GlobalInvocationID.xy);
    float4 _829;
    float4 _830;
    do
    {
        if (_77 == 0.0)
        {
            _829 = parNormal.read(spvTexelBufferCoord(_63));
            _830 = _65;
            break;
        }
        int _87;
        bool _88;
        bool _89;
        int2 _84 = _78 + int2(0, -1);
        float3 _139;
        do
        {
            _87 = _84.x;
            _88 = _87 < 0;
            _89 = !_88;
            bool _94;
            if (_89)
            {
                _94 = uint(_87) >= SceneState.gridSize.x;
            }
            else
            {
                _94 = true;
            }
            bool _100;
            if (!_94)
            {
                _100 = _84.y < 0;
            }
            else
            {
                _100 = true;
            }
            bool _109;
            if (!_100)
            {
                _109 = uint(_84.y) >= SceneState.gridSize.y;
            }
            else
            {
                _109 = true;
            }
            if (_109)
            {
                _139 = float3(0.0);
                break;
            }
            uint2 _112 = uint2(_84);
            uint _116 = (_112.y * SceneState.gridSize.x) + _112.x;
            float4 _119 = parBase.read(spvTexelBufferCoord(_116));
            float3 _129 = (_65 - parCurrPos.read(spvTexelBufferCoord(_116))).xyz;
            _139 = (fast::normalize(_129) * ((length(_129) - distance(_73, float4((_119.x * 2.0) - 1.0, 0.0, _119.y * (-2.0), 1.0))) / (_77 + parBase.read(spvTexelBufferCoord(_116)).z))) * (-_77);
            break;
        } while(false);
        int _143;
        bool _144;
        bool _145;
        int2 _140 = _78 + int2(0, 1);
        float3 _196;
        do
        {
            _143 = _140.x;
            _144 = _143 < 0;
            _145 = !_144;
            bool _150;
            if (_145)
            {
                _150 = uint(_143) >= SceneState.gridSize.x;
            }
            else
            {
                _150 = true;
            }
            bool _156;
            if (!_150)
            {
                _156 = _140.y < 0;
            }
            else
            {
                _156 = true;
            }
            bool _165;
            if (!_156)
            {
                _165 = uint(_140.y) >= SceneState.gridSize.y;
            }
            else
            {
                _165 = true;
            }
            if (_165)
            {
                _196 = _139;
                break;
            }
            uint2 _168 = uint2(_140);
            uint _172 = (_168.y * SceneState.gridSize.x) + _168.x;
            float4 _175 = parBase.read(spvTexelBufferCoord(_172));
            float3 _185 = (_65 - parCurrPos.read(spvTexelBufferCoord(_172))).xyz;
            _196 = _139 + ((fast::normalize(_185) * ((length(_185) - distance(_73, float4((_175.x * 2.0) - 1.0, 0.0, _175.y * (-2.0), 1.0))) / (_77 + parBase.read(spvTexelBufferCoord(_172)).z))) * (-_77));
            break;
        } while(false);
        int _200;
        bool _201;
        bool _202;
        int2 _197 = _78 + int2(-1, 0);
        float3 _253;
        do
        {
            _200 = _197.x;
            _201 = _200 < 0;
            _202 = !_201;
            bool _207;
            if (_202)
            {
                _207 = uint(_200) >= SceneState.gridSize.x;
            }
            else
            {
                _207 = true;
            }
            bool _213;
            if (!_207)
            {
                _213 = _197.y < 0;
            }
            else
            {
                _213 = true;
            }
            bool _222;
            if (!_213)
            {
                _222 = uint(_197.y) >= SceneState.gridSize.y;
            }
            else
            {
                _222 = true;
            }
            if (_222)
            {
                _253 = _196;
                break;
            }
            uint2 _225 = uint2(_197);
            uint _229 = (_225.y * SceneState.gridSize.x) + _225.x;
            float4 _232 = parBase.read(spvTexelBufferCoord(_229));
            float3 _242 = (_65 - parCurrPos.read(spvTexelBufferCoord(_229))).xyz;
            _253 = _196 + ((fast::normalize(_242) * ((length(_242) - distance(_73, float4((_232.x * 2.0) - 1.0, 0.0, _232.y * (-2.0), 1.0))) / (_77 + parBase.read(spvTexelBufferCoord(_229)).z))) * (-_77));
            break;
        } while(false);
        int _257;
        bool _258;
        bool _259;
        int2 _254 = _78 + int2(1, 0);
        float3 _310;
        do
        {
            _257 = _254.x;
            _258 = _257 < 0;
            _259 = !_258;
            bool _264;
            if (_259)
            {
                _264 = uint(_257) >= SceneState.gridSize.x;
            }
            else
            {
                _264 = true;
            }
            bool _270;
            if (!_264)
            {
                _270 = _254.y < 0;
            }
            else
            {
                _270 = true;
            }
            bool _279;
            if (!_270)
            {
                _279 = uint(_254.y) >= SceneState.gridSize.y;
            }
            else
            {
                _279 = true;
            }
            if (_279)
            {
                _310 = _253;
                break;
            }
            uint2 _282 = uint2(_254);
            uint _286 = (_282.y * SceneState.gridSize.x) + _282.x;
            float4 _289 = parBase.read(spvTexelBufferCoord(_286));
            float3 _299 = (_65 - parCurrPos.read(spvTexelBufferCoord(_286))).xyz;
            _310 = _253 + ((fast::normalize(_299) * ((length(_299) - distance(_73, float4((_289.x * 2.0) - 1.0, 0.0, _289.y * (-2.0), 1.0))) / (_77 + parBase.read(spvTexelBufferCoord(_286)).z))) * (-_77));
            break;
        } while(false);
        int2 _311 = _78 + int2(-1);
        float3 _367;
        do
        {
            int _314 = _311.x;
            bool _321;
            if (!(_314 < 0))
            {
                _321 = uint(_314) >= SceneState.gridSize.x;
            }
            else
            {
                _321 = true;
            }
            bool _327;
            if (!_321)
            {
                _327 = _311.y < 0;
            }
            else
            {
                _327 = true;
            }
            bool _336;
            if (!_327)
            {
                _336 = uint(_311.y) >= SceneState.gridSize.y;
            }
            else
            {
                _336 = true;
            }
            if (_336)
            {
                _367 = _310;
                break;
            }
            uint2 _339 = uint2(_311);
            uint _343 = (_339.y * SceneState.gridSize.x) + _339.x;
            float4 _346 = parBase.read(spvTexelBufferCoord(_343));
            float3 _356 = (_65 - parCurrPos.read(spvTexelBufferCoord(_343))).xyz;
            _367 = _310 + ((fast::normalize(_356) * ((length(_356) - distance(_73, float4((_346.x * 2.0) - 1.0, 0.0, _346.y * (-2.0), 1.0))) / (_77 + parBase.read(spvTexelBufferCoord(_343)).z))) * (-_77));
            break;
        } while(false);
        int2 _368 = _78 + int2(1, -1);
        float3 _424;
        do
        {
            int _371 = _368.x;
            bool _378;
            if (!(_371 < 0))
            {
                _378 = uint(_371) >= SceneState.gridSize.x;
            }
            else
            {
                _378 = true;
            }
            bool _384;
            if (!_378)
            {
                _384 = _368.y < 0;
            }
            else
            {
                _384 = true;
            }
            bool _393;
            if (!_384)
            {
                _393 = uint(_368.y) >= SceneState.gridSize.y;
            }
            else
            {
                _393 = true;
            }
            if (_393)
            {
                _424 = _367;
                break;
            }
            uint2 _396 = uint2(_368);
            uint _400 = (_396.y * SceneState.gridSize.x) + _396.x;
            float4 _403 = parBase.read(spvTexelBufferCoord(_400));
            float3 _413 = (_65 - parCurrPos.read(spvTexelBufferCoord(_400))).xyz;
            _424 = _367 + ((fast::normalize(_413) * ((length(_413) - distance(_73, float4((_403.x * 2.0) - 1.0, 0.0, _403.y * (-2.0), 1.0))) / (_77 + parBase.read(spvTexelBufferCoord(_400)).z))) * (-_77));
            break;
        } while(false);
        int2 _425 = _78 + int2(-1, 1);
        float3 _481;
        do
        {
            int _428 = _425.x;
            bool _435;
            if (!(_428 < 0))
            {
                _435 = uint(_428) >= SceneState.gridSize.x;
            }
            else
            {
                _435 = true;
            }
            bool _441;
            if (!_435)
            {
                _441 = _425.y < 0;
            }
            else
            {
                _441 = true;
            }
            bool _450;
            if (!_441)
            {
                _450 = uint(_425.y) >= SceneState.gridSize.y;
            }
            else
            {
                _450 = true;
            }
            if (_450)
            {
                _481 = _424;
                break;
            }
            uint2 _453 = uint2(_425);
            uint _457 = (_453.y * SceneState.gridSize.x) + _453.x;
            float4 _460 = parBase.read(spvTexelBufferCoord(_457));
            float3 _470 = (_65 - parCurrPos.read(spvTexelBufferCoord(_457))).xyz;
            _481 = _424 + ((fast::normalize(_470) * ((length(_470) - distance(_73, float4((_460.x * 2.0) - 1.0, 0.0, _460.y * (-2.0), 1.0))) / (_77 + parBase.read(spvTexelBufferCoord(_457)).z))) * (-_77));
            break;
        } while(false);
        int2 _482 = _78 + int2(1);
        float3 _538;
        do
        {
            int _485 = _482.x;
            bool _492;
            if (!(_485 < 0))
            {
                _492 = uint(_485) >= SceneState.gridSize.x;
            }
            else
            {
                _492 = true;
            }
            bool _498;
            if (!_492)
            {
                _498 = _482.y < 0;
            }
            else
            {
                _498 = true;
            }
            bool _507;
            if (!_498)
            {
                _507 = uint(_482.y) >= SceneState.gridSize.y;
            }
            else
            {
                _507 = true;
            }
            if (_507)
            {
                _538 = _481;
                break;
            }
            uint2 _510 = uint2(_482);
            uint _514 = (_510.y * SceneState.gridSize.x) + _510.x;
            float4 _517 = parBase.read(spvTexelBufferCoord(_514));
            float3 _527 = (_65 - parCurrPos.read(spvTexelBufferCoord(_514))).xyz;
            _538 = _481 + ((fast::normalize(_527) * ((length(_527) - distance(_73, float4((_517.x * 2.0) - 1.0, 0.0, _517.y * (-2.0), 1.0))) / (_77 + parBase.read(spvTexelBufferCoord(_514)).z))) * (-_77));
            break;
        } while(false);
        float4 _608;
        do
        {
            bool _546;
            if (_145)
            {
                _546 = uint(_143) >= SceneState.gridSize.x;
            }
            else
            {
                _546 = true;
            }
            bool _552;
            if (!_546)
            {
                _552 = _140.y < 0;
            }
            else
            {
                _552 = true;
            }
            bool _561;
            if (!_552)
            {
                _561 = uint(_140.y) >= SceneState.gridSize.y;
            }
            else
            {
                _561 = true;
            }
            bool _569;
            if (!((!_561) ? _258 : true))
            {
                _569 = uint(_257) >= SceneState.gridSize.x;
            }
            else
            {
                _569 = true;
            }
            bool _575;
            if (!_569)
            {
                _575 = _254.y < 0;
            }
            else
            {
                _575 = true;
            }
            bool _584;
            if (!_575)
            {
                _584 = uint(_254.y) >= SceneState.gridSize.y;
            }
            else
            {
                _584 = true;
            }
            if (_584)
            {
                _608 = float4(0.0);
                break;
            }
            uint2 _587 = uint2(_140);
            float3 _595 = _65.xyz;
            uint2 _597 = uint2(_254);
            float3 _606 = cross(parCurrPos.read(spvTexelBufferCoord(((_587.y * SceneState.gridSize.x) + _587.x))).xyz - _595, parCurrPos.read(spvTexelBufferCoord(((_597.y * SceneState.gridSize.x) + _597.x))).xyz - _595);
            _608 = float4(_606.x, _606.y, _606.z, float4(0.0).w);
            break;
        } while(false);
        float4 _679;
        do
        {
            bool _615;
            if (_259)
            {
                _615 = uint(_257) >= SceneState.gridSize.x;
            }
            else
            {
                _615 = true;
            }
            bool _621;
            if (!_615)
            {
                _621 = _254.y < 0;
            }
            else
            {
                _621 = true;
            }
            bool _630;
            if (!_621)
            {
                _630 = uint(_254.y) >= SceneState.gridSize.y;
            }
            else
            {
                _630 = true;
            }
            bool _638;
            if (!((!_630) ? _88 : true))
            {
                _638 = uint(_87) >= SceneState.gridSize.x;
            }
            else
            {
                _638 = true;
            }
            bool _644;
            if (!_638)
            {
                _644 = _84.y < 0;
            }
            else
            {
                _644 = true;
            }
            bool _653;
            if (!_644)
            {
                _653 = uint(_84.y) >= SceneState.gridSize.y;
            }
            else
            {
                _653 = true;
            }
            if (_653)
            {
                _679 = _608;
                break;
            }
            uint2 _656 = uint2(_254);
            float3 _664 = _65.xyz;
            uint2 _666 = uint2(_84);
            float3 _677 = _608.xyz + cross(parCurrPos.read(spvTexelBufferCoord(((_656.y * SceneState.gridSize.x) + _656.x))).xyz - _664, parCurrPos.read(spvTexelBufferCoord(((_666.y * SceneState.gridSize.x) + _666.x))).xyz - _664);
            _679 = float4(_677.x, _677.y, _677.z, _608.w);
            break;
        } while(false);
        float4 _750;
        do
        {
            bool _686;
            if (_89)
            {
                _686 = uint(_87) >= SceneState.gridSize.x;
            }
            else
            {
                _686 = true;
            }
            bool _692;
            if (!_686)
            {
                _692 = _84.y < 0;
            }
            else
            {
                _692 = true;
            }
            bool _701;
            if (!_692)
            {
                _701 = uint(_84.y) >= SceneState.gridSize.y;
            }
            else
            {
                _701 = true;
            }
            bool _709;
            if (!((!_701) ? _201 : true))
            {
                _709 = uint(_200) >= SceneState.gridSize.x;
            }
            else
            {
                _709 = true;
            }
            bool _715;
            if (!_709)
            {
                _715 = _197.y < 0;
            }
            else
            {
                _715 = true;
            }
            bool _724;
            if (!_715)
            {
                _724 = uint(_197.y) >= SceneState.gridSize.y;
            }
            else
            {
                _724 = true;
            }
            if (_724)
            {
                _750 = _679;
                break;
            }
            uint2 _727 = uint2(_84);
            float3 _735 = _65.xyz;
            uint2 _737 = uint2(_197);
            float3 _748 = _679.xyz + cross(parCurrPos.read(spvTexelBufferCoord(((_727.y * SceneState.gridSize.x) + _727.x))).xyz - _735, parCurrPos.read(spvTexelBufferCoord(((_737.y * SceneState.gridSize.x) + _737.x))).xyz - _735);
            _750 = float4(_748.x, _748.y, _748.z, _679.w);
            break;
        } while(false);
        float4 _821;
        do
        {
            bool _757;
            if (_202)
            {
                _757 = uint(_200) >= SceneState.gridSize.x;
            }
            else
            {
                _757 = true;
            }
            bool _763;
            if (!_757)
            {
                _763 = _197.y < 0;
            }
            else
            {
                _763 = true;
            }
            bool _772;
            if (!_763)
            {
                _772 = uint(_197.y) >= SceneState.gridSize.y;
            }
            else
            {
                _772 = true;
            }
            bool _780;
            if (!((!_772) ? _144 : true))
            {
                _780 = uint(_143) >= SceneState.gridSize.x;
            }
            else
            {
                _780 = true;
            }
            bool _786;
            if (!_780)
            {
                _786 = _140.y < 0;
            }
            else
            {
                _786 = true;
            }
            bool _795;
            if (!_786)
            {
                _795 = uint(_140.y) >= SceneState.gridSize.y;
            }
            else
            {
                _795 = true;
            }
            if (_795)
            {
                _821 = _750;
                break;
            }
            uint2 _798 = uint2(_197);
            float3 _806 = _65.xyz;
            uint2 _808 = uint2(_140);
            float3 _819 = _750.xyz + cross(parCurrPos.read(spvTexelBufferCoord(((_798.y * SceneState.gridSize.x) + _798.x))).xyz - _806, parCurrPos.read(spvTexelBufferCoord(((_808.y * SceneState.gridSize.x) + _808.x))).xyz - _806);
            _821 = float4(_819.x, _819.y, _819.z, _750.w);
            break;
        } while(false);
        float3 _827 = _65.xyz + ((_538 * float3(0.125)) * SceneState.dStiffness);
        _829 = _821 * float4(0.25);
        _830 = float4(_827.x, _827.y, _827.z, _65.w);
        break;
    } while(false);
    parNextPos.write(_830, spvTexelBufferCoord(_63));
    parNormal.write(_829, spvTexelBufferCoord(_63));
}

