#pragma once
#include "Specific/trmath.h"

enum RENDERER_BUCKETS
{
	RENDERER_BUCKET_SOLID = 0,
	RENDERER_BUCKET_TRANSPARENT = 1,
};

enum RENDERER_PASSES
{
	RENDERER_PASS_DEPTH = 0,
	RENDERER_PASS_DRAW = 1,
	RENDERER_PASS_SHADOW_MAP = 2,
	RENDERER_PASS_GBUFFER = 3,
	RENDERER_PASS_TRANSPARENT = 4,
	RENDERER_PASS_RECONSTRUCT_DEPTH = 5
};

enum MODEL_TYPES
{
	MODEL_TYPE_HORIZON = 0,
	MODEL_TYPE_ROOM = 1,
	MODEL_TYPE_MOVEABLE = 2,
	MODEL_TYPE_STATIC = 3,
	MODEL_TYPE_INVENTORY = 4,
	MODEL_TYPE_PICKUP = 5,
	MODEL_TYPE_LARA = 6,
	MODEL_TYPE_SKY = 7,
	MODEL_TYPE_WATER_SURFACE = 8,
	MODEL_TYPE_ROOM_UNDERWATER = 9
};

enum LIGHT_TYPES
{
	LIGHT_TYPE_SUN = 0,
	LIGHT_TYPE_POINT = 1,
	LIGHT_TYPE_SPOT = 2,
	LIGHT_TYPE_SHADOW = 3
};

enum BLEND_MODES
{
	BLENDMODE_OPAQUE = 0,
	BLENDMODE_ALPHATEST = 1,
	BLENDMODE_ADDITIVE = 2,
	BLENDMODE_NOZTEST = 4,
	BLENDMODE_SUBTRACTIVE = 5,
	BLENDMODE_WIREFRAME = 6,
	BLENDMODE_EXCLUDE = 8,
	BLENDMODE_SCREEN = 9,
	BLENDMODE_LIGHTEN = 10,
	BLENDMODE_ALPHABLEND = 11,
	BLENDMODE_UNSET = -1
};

enum CULL_MODES
{
	CULL_MODE_NONE = 0,
	CULL_MODE_CW = 1,
	CULL_MODE_CCW = 2,
	CULL_MODE_UNSET = -1
};

enum class ShadowMode
{
	None,
	Lara,
	All
};

enum class AntialiasingMode
{
	None,
	Low,
	Medium,
	High
};

enum LIGHT_MODES
{
	LIGHT_MODE_DYNAMIC,
	LIGHT_MODE_STATIC
};

enum DEPTH_STATES
{
	DEPTH_STATE_WRITE_ZBUFFER = 0,
	DEPTH_STATE_READ_ONLY_ZBUFFER = 1,
	DEPTH_STATE_NONE = 2,
	DEPTH_STATE_UNSET = -1
};

enum RENDERER_SPRITE_TYPE
{
	SPRITE_TYPE_BILLBOARD,
	SPRITE_TYPE_3D,
	SPRITE_TYPE_BILLBOARD_CUSTOM,
	SPRITE_TYPE_BILLBOARD_LOOKAT
};

enum RENDERER_POLYGON_SHAPE
{
	RENDERER_POLYGON_QUAD,
	RENDERER_POLYGON_TRIANGLE
};

enum RENDERER_FADE_STATUS
{
	NO_FADE,
	FADE_IN,
	FADE_OUT
};

enum RENDERER_DEBUG_PAGE
{
	NO_PAGE,
	RENDERER_STATS,
	DIMENSION_STATS,
	LARA_STATS,
	LOGIC_STATS
};

enum RendererTransparentFaceType
{
	TRANSPARENT_FACE_ROOM,
	TRANSPARENT_FACE_MOVEABLE,
	TRANSPARENT_FACE_STATIC,
	TRANSPARENT_FACE_SPRITE,
	TRANSPARENT_FACE_NONE
};

enum ConstantBufferRegister
{
	CameraBuffer = 0,
	ItemBuffer = 1,
	LightBuffer = 2,
	MiscBuffer = 3,
	ShadowLightBuffer = 4,
	RoomBuffer = 5,
	AnimatedTexturesBuffer = 6
};

enum TEXTURE_REGISTERS
{
	TEXTURE_COLOR_MAP = 0,
	TEXTURE_NORMAL_MAP = 1,
	TEXTURE_CAUSTICS = 2,
	TEXTURE_SHADOW_MAP = 3,
	TEXTURE_REFLECTION_MAP = 4,
	TEXTURE_HUD = 5,
	TEXTURE_DEPTH_MAP = 6
};

enum SAMPLER_STATES
{
	SAMPLER_NONE = 0,
	SAMPLER_POINT_WRAP = 1,
	SAMPLER_LINEAR_WRAP = 2,
	SAMPLER_LINEAR_CLAMP = 3,
	SAMPLER_ANISOTROPIC_WRAP = 4,
	SAMPLER_ANISOTROPIC_CLAMP = 5,
	SAMPLER_SHADOW_MAP = 6
};

enum CONSTANT_BUFFERS
{
	CB_CAMERA = 0,
	CB_ITEM = 1,
	CB_LIGHTS = 2,
	CB_MISC = 3,
	CB_SHADOW_LIGHT = 4,
	CB_ROOM = 5,
	CB_ANIMATED_TEXTURES = 6,
	CB_POSTPROCESS = 7,
	CB_STATIC = 8,
	CB_SPRITE = 9,
	CB_HUD = 10,
	CB_HUD_BAR = 11,
	CB_ALPHA_TEST = 12
};

enum ALPHA_TEST_MODES 
{
	ALPHA_TEST_NONE = 0,
	ALPHA_TEST_GREATER_THAN = 1,
	ALPHA_TEST_LESS_THAN = 2
};

constexpr auto TEXTURE_HEIGHT = 256;
constexpr auto TEXTURE_WIDTH = 256;
constexpr auto TEXTURE_PAGE = (TEXTURE_HEIGHT * TEXTURE_WIDTH);

#define TEXTURE_ATLAS_SIZE 4096
#define TEXTURE_PAGE_SIZE 262144
#define NUM_TEXTURE_PAGES_PER_ROW 16
#define MAX_SHADOW_MAPS 8
#define GET_ATLAS_PAGE_X(p) ((p) % NUM_TEXTURE_PAGES_PER_ROW) * 256.0f
#define GET_ATLAS_PAGE_Y(p) floor((p) / NUM_TEXTURE_PAGES_PER_ROW) * 256.0f
#define SHAPE_RECTANGLE 0
#define SHAPE_TRIANGLE	1
#define MAX_VERTICES 200000
#define MAX_INDICES 400000
#define MAX_RECTS_2D 32
#define MAX_LINES_2D 256
#define MAX_LINES_3D 16384
#define NUM_BUCKETS 4
#define AMBIENT_CUBE_MAP_SIZE 64
#define NUM_SPRITES_PER_BUCKET 4096
#define NUM_LINES_PER_BUCKET 4096
#define NUM_CAUSTICS_TEXTURES	16
#define	FADEMODE_NONE 0
#define FADEMODE_FADEIN 1
#define FADEMODE_FADEOUT 2
#define PRINTSTRING_CENTER 1
#define PRINTSTRING_BLINK 2
#define PRINTSTRING_OUTLINE	8
#define PRINTSTRING_COLOR_ORANGE D3DCOLOR_ARGB(255, 216, 117, 49)
#define PRINTSTRING_COLOR_WHITE D3DCOLOR_ARGB(255, 255, 255, 255)
#define PRINTSTRING_COLOR_BLACK D3DCOLOR_ARGB(255, 0, 0, 0)
#define PRINTSTRING_COLOR_YELLOW D3DCOLOR_ARGB(255, 240, 220, 32)

constexpr auto FADE_FRAMES_COUNT = 16;
constexpr auto FADE_FACTOR = 0.0625f;

constexpr auto NUM_LIGHTS_PER_BUFFER = 48;
constexpr auto MAX_LIGHTS_PER_ITEM = 8;
constexpr auto MAX_LIGHTS = 100;
constexpr auto AMBIENT_LIGHT_INTERPOLATION_STEP = 1.0f / 10.0f;
constexpr auto MAX_DYNAMIC_SHADOWS = 1;
constexpr auto MAX_DYNAMIC_LIGHTS = 1024;
constexpr auto ITEM_LIGHT_COLLECTION_RADIUS = SECTOR(1);
constexpr auto CAMERA_LIGHT_COLLECTION_RADIUS = SECTOR(4);

constexpr auto MAX_TRANSPARENT_FACES = 16384;
constexpr auto MAX_TRANSPARENT_VERTICES = (MAX_TRANSPARENT_FACES * 6);
constexpr auto MAX_TRANSPARENT_FACES_PER_ROOM = 16384;
constexpr auto TRANSPARENT_BUCKET_SIZE = (3840 * 16);
constexpr auto ALPHA_TEST_THRESHOLD = 0.5f;
constexpr auto FAST_ALPHA_BLEND_THRESHOLD = 0.5f;

constexpr auto MAX_BONES = 32;

constexpr auto REFERENCE_FONT_SIZE = 35.0f;
constexpr auto REFERENCE_RES_WIDTH = 800.0f;
constexpr auto REFERENCE_RES_HEIGHT = 600.0f;
constexpr auto HUD_UNIT_X = 1.0f / REFERENCE_RES_WIDTH;
constexpr auto HUD_UNIT_Y = 1.0f / REFERENCE_RES_HEIGHT;
constexpr auto HUD_ZERO_Y = -REFERENCE_RES_HEIGHT;

constexpr auto UNDERWATER_FOG_MIN_DISTANCE = 4;
constexpr auto UNDERWATER_FOG_MAX_DISTANCE = 30;
constexpr auto MAX_ROOM_BOUNDS = 256;

constexpr auto MIN_FAR_VIEW = 3200.0f;
constexpr auto DEFAULT_FAR_VIEW = 102400.0f;