/****************************************************************************
*                   parse.h
*
*  This header file is included by all all language parsing C modules in 
*  POV-Ray. 
*
*  from Persistence of Vision Raytracer
*  Copyright 1993 Persistence of Vision Team
*---------------------------------------------------------------------------
*  NOTICE: This source code file is provided so that users may experiment
*  with enhancements to POV-Ray and to port the software to platforms other 
*  than those supported by the POV-Ray Team.  There are strict rules under
*  which you are permitted to use this file.  The rules are in the file
*  named POVLEGAL.DOC which should be distributed with this file. If 
*  POVLEGAL.DOC is not available or for more info please contact the POV-Ray
*  Team Coordinator by leaving a message in CompuServe's Graphics Developer's
*  Forum.  The latest version of POV-Ray may be found there as well.
*
* This program is based on the popular DKB raytracer version 2.12.
* DKBTrace was originally written by David K. Buck.
* DKBTrace Ver 2.0-2.12 were written by David K. Buck & Aaron A. Collins.
*
*****************************************************************************/

/* Here we create our own little language for doing the parsing.  It
makes the code easier to read. */

#define EXPECT { int Exit_Flag; Exit_Flag = FALSE; \
 while (!Exit_Flag) {Get_Token();  switch (Token.Token_Id) {
#define CASE(x) case x:
#define CASE2(x, y) case x: case y:
#define CASE3(x, y, z) case x: case y: case z:
#define CASE4(w, x, y, z) case w: case x: case y: case z:
#define CASE5(v, w, x, y, z) case v: case w: case x: case y: case z:
#define CASE6(u, v, w, x, y, z) case u: case v: case w: case x: case y: case z:
#define END_CASE break;
#define EXIT Exit_Flag = TRUE;
#define OTHERWISE default:
#define END_EXPECT } } }
#define GET(x) Get_Token(); if (Token.Token_Id != x) Parse_Error (x);
#define UNGET Unget_Token();
#define CASE_VECTOR CASE3 (LEFT_PAREN_TOKEN, VECTOR_ID_TOKEN, FLOAT_ID_TOKEN)\
 CASE5 (CLOCK_TOKEN, LEFT_ANGLE_TOKEN, PLUS_TOKEN, DASH_TOKEN, FLOAT_TOKEN)\
 CASE4 (X_TOKEN, Y_TOKEN, Z_TOKEN, VERSION_TOKEN) UNGET
#define CASE_FLOAT CASE3 (LEFT_PAREN_TOKEN, FLOAT_ID_TOKEN, CLOCK_TOKEN)\
 CASE4 (PLUS_TOKEN, DASH_TOKEN, FLOAT_TOKEN, VERSION_TOKEN) UNGET

#define MAX_BRACES 200

#define CASEID(x,y) case x:Token.Token_Id=y;break;

/* Token Definitions for Parser */
                               
#define ADAPTIVE_TOKEN            0
#define AGATE_TOKEN               1
#define ALL_TOKEN                 2
#define ALPHA_TOKEN               3
#define AMBIENT_TOKEN             4
#define AMPERSAND_TOKEN           5
#define AREA_LIGHT_TOKEN          6
#define AT_TOKEN                  7
#define BACK_QUOTE_TOKEN          8
#define BACK_SLASH_TOKEN          9
#define BAR_TOKEN                10
#define BICUBIC_PATCH_TOKEN      11
#define BLOB_TOKEN               12
#define BLUE_TOKEN               13
#define BOUNDED_BY_TOKEN         14
#define BOX_TOKEN                15
#define BOZO_TOKEN               16
#define BRICK_TOKEN              17
#define BRILLIANCE_TOKEN         18
#define BUMPS_TOKEN              19
#define BUMPY1_TOKEN             20
#define BUMPY2_TOKEN             21
#define BUMPY3_TOKEN             22
#define BUMP_MAP_TOKEN           23
#define BUMP_SIZE_TOKEN          24
#define CAMERA_ID_TOKEN          25
#define CAMERA_TOKEN             26
#define CHECKER_TOKEN            27
#define CLIPPED_BY_TOKEN         28
#define CLOCK_TOKEN              29
#define COLON_TOKEN              30
#define COLOR_MAP_TOKEN          31
#define COLOR_TOKEN              32
#define COLOUR_ID_TOKEN          33
#define COLOUR_MAP_ID_TOKEN      34
#define COLOUR_MAP_TOKEN         35
#define COLOUR_TOKEN             36
#define COMMA_TOKEN              37
#define COMPONENT_TOKEN          38
#define COMPOSITE_TOKEN          39
#define CONE_TOKEN               40
#define CRAND_TOKEN              41
#define CUBIC_TOKEN              42
#define CYLINDER_TOKEN           43
#define DASH_TOKEN               44
#define DECLARE_TOKEN            45
#define DEFAULT_TOKEN            46
#define DENTS_TOKEN              47
#define DIFFERENCE_TOKEN         48
#define DIFFUSE_TOKEN            49
#define DIRECTION_TOKEN          50
#define DISC_TOKEN               51
#define DISTANCE_TOKEN           52
#define DOLLAR_TOKEN             53
#define DUMP_TOKEN               54
#define END_OF_FILE_TOKEN        55
#define EQUALS_TOKEN             56
#define EXCLAMATION_TOKEN        57
#define FALLOFF_TOKEN            58
#define FINISH_ID_TOKEN          59
#define FINISH_TOKEN             60
#define FLATNESS_TOKEN           61
#define FLOAT_ID_TOKEN           62
#define FLOAT_TOKEN              63
#define FOG_TOKEN                64
#define FREQUENCY_TOKEN          65
#define GIF_TOKEN                66
#define GRADIENT_TOKEN           67
#define GRANITE_TOKEN            68
#define GREEN_TOKEN              69
#define HASH_TOKEN               70
#define HAT_TOKEN                71
#define HEIGHT_FIELD_TOKEN       72
#define HEXAGON_TOKEN            73
#define IDENTIFIER_TOKEN         74
#define IFF_TOKEN                75
#define IMAGE_MAP_TOKEN          76
#define INCLUDE_TOKEN            77
#define INTERPOLATE_TOKEN        78
#define INTERSECTION_TOKEN       79
#define INVERSE_TOKEN            80
#define IOR_TOKEN                81
#define JITTER_TOKEN             82
#define LAMBDA_TOKEN             83
#define LEFT_ANGLE_TOKEN         84
#define LEFT_CURLY_TOKEN         85
#define LEFT_PAREN_TOKEN         86
#define LEFT_SQUARE_TOKEN        87
#define LEOPARD_TOKEN            88
#define LIGHT_SOURCE_TOKEN       89
#define LOCATION_TOKEN           90
#define LOOKS_LIKE_TOKEN         91
#define LOOK_AT_TOKEN            92
#define MANDEL_TOKEN             93 
#define MAP_TYPE_TOKEN           94
#define MARBLE_TOKEN             95
#define MATERIAL_MAP_TOKEN       96
#define MAX_INTERSECTIONS        97
#define MAX_TRACE_LEVEL_TOKEN    98
#define MERGE_TOKEN              99
#define METALLIC_TOKEN          100
#define MORTAR_TOKEN            101
#define NO_SHADOW_TOKEN         102
#define OBJECT_ID_TOKEN         103
#define OBJECT_TOKEN            104
#define OCTAVES_TOKEN           105 
#define OMEGA_TOKEN             106
#define ONCE_TOKEN              107
#define ONION_TOKEN             108
#define PAINTED1_TOKEN          109
#define PAINTED2_TOKEN          110
#define PAINTED3_TOKEN          111
#define PERCENT_TOKEN           112
#define PHASE_TOKEN             113
#define PHONG_SIZE_TOKEN        114
#define PHONG_TOKEN             115
#define PIGMENT_ID_TOKEN        116
#define PIGMENT_TOKEN           117
#define PLANE_TOKEN             118
#define PLUS_TOKEN              119
#define POINT_AT_TOKEN          120
#define POLY_TOKEN              121
#define POT_TOKEN               122
#define QUADRIC_TOKEN           123
#define QUARTIC_TOKEN           124
#define QUESTION_TOKEN          125
#define QUICK_COLOR_TOKEN       126
#define QUICK_COLOUR_TOKEN      127
#define RADIAL_TOKEN            128
#define RADIUS_TOKEN            129
#define RAW_TOKEN               130
#define RED_TOKEN               131
#define REFLECTION_TOKEN        132
#define REFRACTION_TOKEN        133
#define RGBF_TOKEN              134
#define RGB_TOKEN               135
#define RIGHT_ANGLE_TOKEN       136
#define RIGHT_CURLY_TOKEN       137
#define RIGHT_PAREN_TOKEN       138
#define RIGHT_SQUARE_TOKEN      139
#define RIGHT_TOKEN             140
#define RIPPLES_TOKEN           141
#define ROTATE_TOKEN            142
#define ROUGHNESS_TOKEN         143
#define SCALE_TOKEN             144
#define SEMI_COLON_TOKEN        145
#define SINGLE_QUOTE_TOKEN      146
#define SKY_TOKEN               147
#define SLASH_TOKEN             148
#define SMOOTH_TOKEN            149
#define SMOOTH_TRIANGLE_TOKEN   150
#define SPECULAR_TOKEN          151
#define SPHERE_TOKEN            152
#define SPOTLIGHT_TOKEN         153
#define SPOTTED_TOKEN           154
#define STAR_TOKEN              155
#define STRING_TOKEN            156
#define STURM_TOKEN             157
#define TEXTURE_ID_TOKEN        158
#define TEXTURE_TOKEN           159
#define TGA_TOKEN               160
#define THRESHOLD_TOKEN         161
#define TIGHTNESS_TOKEN         162
#define TILDE_TOKEN             163
#define TILE2_TOKEN             164
#define TILES_TOKEN             165
#define TNORMAL_ID_TOKEN        166
#define TNORMAL_TOKEN           167
#define TORUS_TOKEN             168
#define TRACK_TOKEN             169
#define TRANSFORM_ID_TOKEN      170
#define TRANSFORM_TOKEN         171
#define TRANSLATE_TOKEN         172
#define TRIANGLE_TOKEN          173
#define TURBULENCE_TOKEN        174
#define TYPE_TOKEN              175
#define UNION_TOKEN             176
#define UP_TOKEN                177
#define USE_COLOR_TOKEN         178
#define USE_COLOUR_TOKEN        179
#define USE_INDEX_TOKEN         180
#define U_STEPS_TOKEN           181
#define VECTOR_ID_TOKEN         182
#define VERSION_TOKEN           183
#define V_STEPS_TOKEN           184
#define WATER_LEVEL_TOKEN       185
#define WAVES_TOKEN             186
#define WOOD_TOKEN              187
#define WRINKLES_TOKEN          188
#define X_TOKEN                 189
#define Y_TOKEN                 190
#define Z_TOKEN                 191
#define BACKGROUND_TOKEN        192
#define OPEN_TOKEN              193
#define FILTER_TOKEN            194
#define AGATE_TURB_TOKEN        195
#define LAST_TOKEN              196
