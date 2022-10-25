#pragma once
#include <cstdint>
#include <cstddef>
#define NUM_LIGHTS 32
#define NUM_LIGHTS_SUPPORTED 32

typedef uint32_t hash;

enum struct PsoDataType : uint8_t {
    Bool = 0x00,
    s8 = 0x01,
    u8 = 0x02,
    s16 = 0x03,
    u16 = 0x04,
    s32 = 0x05,
    u32 = 0x06,
    f32 = 0x07,
    Vector2 = 0x08,
    Vector3 = 0x09,
    Vector4 = 0x0a,
    String = 0x0b,
    Structure = 0x0c,
    Array = 0x0d,
    Enum = 0x0e,
    Flags = 0x0f,
    Map = 0x10,
    f16 = 0x1e,
    s64 = 0x1f,
    u64 = 0x20,
    f64 = 0x21
};

enum struct parMemberArraySubtype : uint8_t // 0xADE25B1B
{
    ATARRAY = 0,                        // 0xABE40192
    ATFIXEDARRAY = 1,                   // 0x3A523E81
    ATRANGEARRAY = 2,                   // 0x18A25B6B
    POINTER = 3,                        // 0x47073D6E
    MEMBER = 4,                         // 0x6CC11BB4
    _0x2087BB00 = 5,                    // 0x2087BB00
    POINTER_WITH_COUNT = 6,             // 0xE2980EB5
    POINTER_WITH_COUNT_8BIT_IDX = 7,    // 0x254D33B1
    POINTER_WITH_COUNT_16BIT_IDX = 8,   // 0xB66B6752
    VIRTUAL = 9,                        // 0xAC01A1DC
};

struct parMemberDefinition {
	hash name;
	uint8_t padding[4];
	uint64_t offset;
    PsoDataType type;
};

struct parMemberArrayDefinition {
    hash name;
    uint8_t padding[4];
    uint64_t offset;
    PsoDataType type;
    parMemberArraySubtype subtype;
    uint8_t padding2[0xe];
    uint32_t eltSize;
    uint8_t padding3[0x4];
    uint8_t eltCount;
};

uint32_t joaat(const char* string);

struct parserInfo {
    hash name;
    uint8_t padding[12];
    uint64_t parStructurePtr;
    parMemberDefinition** members;
    uint32_t* offsets;

    parMemberDefinition* FindMember(const char* name);
};
static_assert(offsetof(parserInfo, members) == 0x18, "parserInfo fail");

struct EmergencyLight {
    float RotationDelta;
    float RotationStart;
    float RotationSpeed;
    uint32_t RotationSeq;
    uint8_t RotationMult;
    bool RotationDirection;
    bool RotationSyncToBpm;
    uint8_t pad1;
    float FlashDelta;
    float FlashStart;
    float FlashSpeed;
    float FlashSeq;
    uint8_t FlashMult;
    bool FlashDirection;
    bool FlashSyncToBpm;
    uint8_t pad2;
    float CoronaIntensity;
    float CoronaSize;
    float CoronaPull;
    bool CoronaFaceCamera;
    uint8_t field_0x35[3];
    uint32_t Color;
    float Intensity;
    uint8_t LightGroup;
    uint8_t ScaleFactor;
    bool Scale;
    bool Rotate;
    bool Flash;
    bool Light;
    bool SpotLight;
    bool CastShadows;
};

static_assert(sizeof(EmergencyLight) == 0x48, "EmergencyLight is the wrong size");

struct CSirenSettings {
    uint32_t Id;
    float TimeMultiplier;
    float LightFallOffMax;
    float LightFallOffExponent;
    float LightInnerConeAngle;
    float LightOuterConeAngle;
    float LightOffset;
    uint32_t TextureHash;
    uint32_t Bpm;
    uint32_t LHeadlightSeq;
    uint32_t RHeadlightSeq;
    uint32_t LTaillightSeq;
    uint32_t RTaillightSeq;
    uint8_t LHeadlightMult;
    uint8_t RHeadlightMult;
    uint8_t LTaillightMult;
    uint8_t RTaillightMult;
    bool UseRealLights;
    uint8_t padding[7];
    EmergencyLight* Lights;
    uint16_t lightCount;
    uint16_t lightSize;
    uint8_t padding2[0x5a0 - 0x10];
    uint32_t NumSirens;
    char* Name;
};
static_assert(sizeof(CSirenSettings) == 0x5e8, "SirenSettings wrong size");

struct CSirenSettingsExpanded {
    uint32_t Id;
    float TimeMultiplier;
    float LightFallOffMax;
    float LightFallOffExponent;
    float LightInnerConeAngle;
    float LightOuterConeAngle;
    float LightOffset;
    uint32_t TextureHash;
    uint32_t Bpm;
    uint32_t LHeadlightSeq;
    uint32_t RHeadlightSeq;
    uint32_t LTaillightSeq;
    uint32_t RTaillightSeq;
    uint8_t LHeadlightMult;
    uint8_t RHeadlightMult;
    uint8_t LTaillightMult;
    uint8_t RTaillightMult;
    bool UseRealLights;
    uint8_t padding[3];
    EmergencyLight Lights[NUM_LIGHTS];
    uint32_t NumSirens;
    char* Name;
};

static_assert(sizeof(CSirenSettingsExpanded) == 0x48 + NUM_LIGHTS * sizeof(EmergencyLight), "Expanded SirenSettings wrong size");

struct SirenBuffer {
    uint32_t Seed;
    int32_t AdjustedTime;
    int32_t BeatNumber;
    int32_t RotatorLastBeatMask[NUM_LIGHTS];
    int32_t RotatorLastBeatTime[NUM_LIGHTS];
    int32_t FlasherLastBeatMask[NUM_LIGHTS];
    int32_t FlasherLastBeatTime[NUM_LIGHTS];
    int32_t HeadlightLastBeatMask[4];
    int32_t HeadlightLastBeatTime[4];
    float HeadlightIntensity[4];
    uint32_t HeadlightStatus;
    uint64_t RotatorStatus;
    uint64_t FlasherStatus;
};

static_assert(sizeof(SirenBuffer) == 0x50 + 0x10 * NUM_LIGHTS, "SirenBuffer wrong size");

struct CSirenSettings_atArray
{
    CSirenSettings* sirens;
    uint16_t count;
    uint16_t size;
};

typedef struct CVehicleModelInfoVariation CVehicleModelInfoVariation, * PCVehicleModelInfoVariation;

struct CVehicleModelInfoVariation {
    char* name;
    uint8_t padding[0x40];
    uint8_t lightIndex;
    uint8_t sirenIndex;
    uint8_t field_0x4a;
    uint8_t sirenId;
    uint8_t field_0x4c;
    uint8_t field_0x4d;
    uint8_t field_0x4e;
    uint8_t field_0x4f;
};

typedef struct CVehicleModelInfoVarGlobal CVehicleModelInfoVarGlobal, * PCVehicleModelInfoVarGlobal;

typedef struct CSirenSetting_List CSirenSetting_List, * PCSirenSetting_List;

struct CSirenSetting_List {
    struct CSirenSettings* sirens;
    short count;
    short maxCount;
    uint8_t field_0xc;
    uint8_t field_0xd;
    uint8_t field_0xe;
    uint8_t field_0xf;
};

struct CVehicleModelInfoVarGlobal {
    uint64_t vtable;
    uint8_t padding[0x40];
    struct CSirenSetting_List sirens;
};
static_assert(offsetof(CVehicleModelInfoVarGlobal, sirens) == 0x48, "Carcols issue");

struct CVehicleModelInfo
{
    uint8_t padding[0x18];
    uint32_t nameHash;
};

struct CVehicle {
    uint8_t padding[32];
    CVehicleModelInfo* modelInfo;
};


extern uint16_t SirenBoneTags[64];
extern uint16_t GlassBoneTags[64];