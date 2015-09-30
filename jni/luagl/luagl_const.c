/*************************************************
*  LuaGL - an OpenGL binding for Lua
*  2003-2004(c) Fabio Guerra, Cleyde Marlyse
*  http://luagl.sourceforge.net
*-------------------------------------------------
*  Description: This file implements the OpenGL
*               binding for Lua 5
*-------------------------------------------------
* Mantained by Antonio Scuri since 2009
*-------------------------------------------------
*  See Copyright Notice in LuaGL.h
*************************************************/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* includes OpenGL, but do NOT use their functions, so no need to link */
#ifdef _WIN32
#include <windows.h>
#endif
#if defined (__APPLE__) || defined (OSX)
#include <OpenGL/gl.h>
#else
#include <GLES/gl.h>
#include <EGL/egl.h>
#include <GLES/glext.h>

#endif

#include <lua.h>
#include <lauxlib.h>

#include "luagl_const.h"


static const luaglConst luagl_const[] = {
#ifdef GL_VERSION_1_2
  { "GL_VERSION_1_2"                     , GL_VERSION_1_2                    },
#endif
#ifdef GL_VERSION_1_3
  { "GL_VERSION_1_3"                     , GL_VERSION_1_3                    },
#endif
  { "GL_ADD"                             , GL_ADD                            },
  { "GL_NEVER"                           , GL_NEVER                          },
  { "GL_LESS"                            , GL_LESS                           },
  { "GL_EQUAL"                           , GL_EQUAL                          },
  { "GL_LEQUAL"                          , GL_LEQUAL                         },
  { "GL_GREATER"                         , GL_GREATER                        },
  { "GL_NOTEQUAL"                        , GL_NOTEQUAL                       },
  { "GL_GEQUAL"                          , GL_GEQUAL                         },
  { "GL_ALWAYS"                          , GL_ALWAYS                         },
  { "GL_POINTS"                          , GL_POINTS                         },
  { "GL_LINES"                           , GL_LINES                          },
  { "GL_LINE_LOOP"                       , GL_LINE_LOOP                      },
  { "GL_LINE_STRIP"                      , GL_LINE_STRIP                     },
  { "GL_TRIANGLES"                       , GL_TRIANGLES                      },
  { "GL_TRIANGLE_STRIP"                  , GL_TRIANGLE_STRIP                 },
  { "GL_TRIANGLE_FAN"                    , GL_TRIANGLE_FAN                   },
  { "GL_ZERO"                            , GL_ZERO                           },
  { "GL_ONE"                             , GL_ONE                            },
  { "GL_SRC_COLOR"                       , GL_SRC_COLOR                      },
  { "GL_ONE_MINUS_SRC_COLOR"             , GL_ONE_MINUS_SRC_COLOR            },
  { "GL_SRC_ALPHA"                       , GL_SRC_ALPHA                      },
  { "GL_ONE_MINUS_SRC_ALPHA"             , GL_ONE_MINUS_SRC_ALPHA            },
  { "GL_DST_ALPHA"                       , GL_DST_ALPHA                      },
  { "GL_ONE_MINUS_DST_ALPHA"             , GL_ONE_MINUS_DST_ALPHA            },
  { "GL_DST_COLOR"                       , GL_DST_COLOR                      },
  { "GL_ONE_MINUS_DST_COLOR"             , GL_ONE_MINUS_DST_COLOR            },
  { "GL_SRC_ALPHA_SATURATE"              , GL_SRC_ALPHA_SATURATE             },
  { "GL_TRUE"                            , GL_TRUE                           },
  { "GL_FALSE"                           , GL_FALSE                          },
  { "GL_CLIP_PLANE0"                     , GL_CLIP_PLANE0                    },
  { "GL_CLIP_PLANE1"                     , GL_CLIP_PLANE1                    },
  { "GL_CLIP_PLANE2"                     , GL_CLIP_PLANE2                    },
  { "GL_CLIP_PLANE3"                     , GL_CLIP_PLANE3                    },
  { "GL_CLIP_PLANE4"                     , GL_CLIP_PLANE4                    },
  { "GL_CLIP_PLANE5"                     , GL_CLIP_PLANE5                    },
  { "GL_BYTE"                            , GL_BYTE                           },
  { "GL_UNSIGNED_BYTE"                   , GL_UNSIGNED_BYTE                  },
  { "GL_SHORT"                           , GL_SHORT                          },
  { "GL_UNSIGNED_SHORT"                  , GL_UNSIGNED_SHORT                 },
  { "GL_UNSIGNED_INT"                    , GL_UNSIGNED_INT                   },
  { "GL_FLOAT"                           , GL_FLOAT                          },
  { "GL_FRONT"                           , GL_FRONT                          },
  { "GL_BACK"                            , GL_BACK                           },
  { "GL_FRONT_AND_BACK"                  , GL_FRONT_AND_BACK                 },
  { "GL_NO_ERROR"                        , GL_NO_ERROR                       },
  { "GL_INVALID_ENUM"                    , GL_INVALID_ENUM                   },
  { "GL_INVALID_VALUE"                   , GL_INVALID_VALUE                  },
  { "GL_INVALID_OPERATION"               , GL_INVALID_OPERATION              },
  { "GL_STACK_OVERFLOW"                  , GL_STACK_OVERFLOW                 },
  { "GL_STACK_UNDERFLOW"                 , GL_STACK_UNDERFLOW                },
  { "GL_OUT_OF_MEMORY"                   , GL_OUT_OF_MEMORY                  },
  { "GL_EXP"                             , GL_EXP                            },
  { "GL_EXP2"                            , GL_EXP2                           },
  { "GL_CW"                              , GL_CW                             },
  { "GL_CCW"                             , GL_CCW                            },
  { "GL_CURRENT_COLOR"                   , GL_CURRENT_COLOR                  },
  { "GL_CURRENT_NORMAL"                  , GL_CURRENT_NORMAL                 },
  { "GL_CURRENT_TEXTURE_COORDS"          , GL_CURRENT_TEXTURE_COORDS         },
  { "GL_POINT_SMOOTH"                    , GL_POINT_SMOOTH                   },
  { "GL_POINT_SIZE"                      , GL_POINT_SIZE                     },
  { "GL_LINE_SMOOTH"                     , GL_LINE_SMOOTH                    },
  { "GL_LINE_WIDTH"                      , GL_LINE_WIDTH                     },
  { "GL_CULL_FACE"                       , GL_CULL_FACE                      },
  { "GL_CULL_FACE_MODE"                  , GL_CULL_FACE_MODE                 },
  { "GL_FRONT_FACE"                      , GL_FRONT_FACE                     },
  { "GL_LIGHTING"                        , GL_LIGHTING                       },
  { "GL_LIGHT_MODEL_TWO_SIDE"            , GL_LIGHT_MODEL_TWO_SIDE           },
  { "GL_LIGHT_MODEL_AMBIENT"             , GL_LIGHT_MODEL_AMBIENT            },
  { "GL_SHADE_MODEL"                     , GL_SHADE_MODEL                    },
  { "GL_COLOR_MATERIAL"                  , GL_COLOR_MATERIAL                 },
  { "GL_FOG"                             , GL_FOG                            },
  { "GL_FOG_DENSITY"                     , GL_FOG_DENSITY                    },
  { "GL_FOG_START"                       , GL_FOG_START                      },
  { "GL_FOG_END"                         , GL_FOG_END                        },
  { "GL_FOG_MODE"                        , GL_FOG_MODE                       },
  { "GL_FOG_COLOR"                       , GL_FOG_COLOR                      },
  { "GL_DEPTH_RANGE"                     , GL_DEPTH_RANGE                    },
  { "GL_DEPTH_TEST"                      , GL_DEPTH_TEST                     },
  { "GL_DEPTH_WRITEMASK"                 , GL_DEPTH_WRITEMASK                },
  { "GL_DEPTH_CLEAR_VALUE"               , GL_DEPTH_CLEAR_VALUE              },
  { "GL_DEPTH_FUNC"                      , GL_DEPTH_FUNC                     },
  { "GL_STENCIL_TEST"                    , GL_STENCIL_TEST                   },
  { "GL_STENCIL_CLEAR_VALUE"             , GL_STENCIL_CLEAR_VALUE            },
  { "GL_STENCIL_FUNC"                    , GL_STENCIL_FUNC                   },
  { "GL_STENCIL_VALUE_MASK"              , GL_STENCIL_VALUE_MASK             },
  { "GL_STENCIL_FAIL"                    , GL_STENCIL_FAIL                   },
  { "GL_STENCIL_PASS_DEPTH_FAIL"         , GL_STENCIL_PASS_DEPTH_FAIL        },
  { "GL_STENCIL_PASS_DEPTH_PASS"         , GL_STENCIL_PASS_DEPTH_PASS        },
  { "GL_STENCIL_REF"                     , GL_STENCIL_REF                    },
  { "GL_STENCIL_WRITEMASK"               , GL_STENCIL_WRITEMASK              },
  { "GL_MATRIX_MODE"                     , GL_MATRIX_MODE                    },
  { "GL_NORMALIZE"                       , GL_NORMALIZE                      },
  { "GL_VIEWPORT"                        , GL_VIEWPORT                       },
  { "GL_MODELVIEW_STACK_DEPTH"           , GL_MODELVIEW_STACK_DEPTH          },
  { "GL_PROJECTION_STACK_DEPTH"          , GL_PROJECTION_STACK_DEPTH         },
  { "GL_TEXTURE_STACK_DEPTH"             , GL_TEXTURE_STACK_DEPTH            },
  { "GL_MODELVIEW_MATRIX"                , GL_MODELVIEW_MATRIX               },
  { "GL_PROJECTION_MATRIX"               , GL_PROJECTION_MATRIX              },
  { "GL_TEXTURE_MATRIX"                  , GL_TEXTURE_MATRIX                 },
  { "GL_ALPHA_TEST"                      , GL_ALPHA_TEST                     },
  { "GL_ALPHA_TEST_FUNC"                 , GL_ALPHA_TEST_FUNC                },
  { "GL_ALPHA_TEST_REF"                  , GL_ALPHA_TEST_REF                 },
  { "GL_DITHER"                          , GL_DITHER                         },
  { "GL_BLEND_DST"                       , GL_BLEND_DST                      },
  { "GL_BLEND_SRC"                       , GL_BLEND_SRC                      },
  { "GL_BLEND"                           , GL_BLEND                          },
  { "GL_LOGIC_OP_MODE"                   , GL_LOGIC_OP_MODE                  },
  { "GL_COLOR_LOGIC_OP"                  , GL_COLOR_LOGIC_OP                 },
  { "GL_SCISSOR_BOX"                     , GL_SCISSOR_BOX                    },
  { "GL_SCISSOR_TEST"                    , GL_SCISSOR_TEST                   },
  { "GL_COLOR_CLEAR_VALUE"               , GL_COLOR_CLEAR_VALUE              },
  { "GL_COLOR_WRITEMASK"                 , GL_COLOR_WRITEMASK                },
  { "GL_PERSPECTIVE_CORRECTION_HINT"     , GL_PERSPECTIVE_CORRECTION_HINT    },
  { "GL_POINT_SMOOTH_HINT"               , GL_POINT_SMOOTH_HINT              },
  { "GL_LINE_SMOOTH_HINT"                , GL_LINE_SMOOTH_HINT               },
  { "GL_FOG_HINT"                        , GL_FOG_HINT                       },
  { "GL_UNPACK_ALIGNMENT"                , GL_UNPACK_ALIGNMENT               },
  { "GL_PACK_ALIGNMENT"                  , GL_PACK_ALIGNMENT                 },
  { "GL_ALPHA_SCALE"                     , GL_ALPHA_SCALE                    },
  { "GL_MAX_LIGHTS"                      , GL_MAX_LIGHTS                     },
  { "GL_MAX_CLIP_PLANES"                 , GL_MAX_CLIP_PLANES                },
  { "GL_MAX_TEXTURE_SIZE"                , GL_MAX_TEXTURE_SIZE               },
  { "GL_MAX_MODELVIEW_STACK_DEPTH"       , GL_MAX_MODELVIEW_STACK_DEPTH      },
  { "GL_MAX_PROJECTION_STACK_DEPTH"      , GL_MAX_PROJECTION_STACK_DEPTH     },
  { "GL_MAX_TEXTURE_STACK_DEPTH"         , GL_MAX_TEXTURE_STACK_DEPTH        },
  { "GL_MAX_VIEWPORT_DIMS"               , GL_MAX_VIEWPORT_DIMS              },
  { "GL_SUBPIXEL_BITS"                   , GL_SUBPIXEL_BITS                  },
  { "GL_RED_BITS"                        , GL_RED_BITS                       },
  { "GL_GREEN_BITS"                      , GL_GREEN_BITS                     },
  { "GL_BLUE_BITS"                       , GL_BLUE_BITS                      },
  { "GL_ALPHA_BITS"                      , GL_ALPHA_BITS                     },
  { "GL_DEPTH_BITS"                      , GL_DEPTH_BITS                     },
  { "GL_STENCIL_BITS"                    , GL_STENCIL_BITS                   },
  { "GL_TEXTURE_2D"                      , GL_TEXTURE_2D                     },
  { "GL_DONT_CARE"                       , GL_DONT_CARE                      },
  { "GL_FASTEST"                         , GL_FASTEST                        },
  { "GL_NICEST"                          , GL_NICEST                         },
  { "GL_LIGHT0"                          , GL_LIGHT0                         },
  { "GL_LIGHT1"                          , GL_LIGHT1                         },
  { "GL_LIGHT2"                          , GL_LIGHT2                         },
  { "GL_LIGHT3"                          , GL_LIGHT3                         },
  { "GL_LIGHT4"                          , GL_LIGHT4                         },
  { "GL_LIGHT5"                          , GL_LIGHT5                         },
  { "GL_LIGHT6"                          , GL_LIGHT6                         },
  { "GL_LIGHT7"                          , GL_LIGHT7                         },
  { "GL_AMBIENT"                         , GL_AMBIENT                        },
  { "GL_DIFFUSE"                         , GL_DIFFUSE                        },
  { "GL_SPECULAR"                        , GL_SPECULAR                       },
  { "GL_POSITION"                        , GL_POSITION                       },
  { "GL_SPOT_DIRECTION"                  , GL_SPOT_DIRECTION                 },
  { "GL_SPOT_EXPONENT"                   , GL_SPOT_EXPONENT                  },
  { "GL_SPOT_CUTOFF"                     , GL_SPOT_CUTOFF                    },
  { "GL_CONSTANT_ATTENUATION"            , GL_CONSTANT_ATTENUATION           },
  { "GL_LINEAR_ATTENUATION"              , GL_LINEAR_ATTENUATION             },
  { "GL_QUADRATIC_ATTENUATION"           , GL_QUADRATIC_ATTENUATION          },
  { "GL_CLEAR"                           , GL_CLEAR                          },
  { "GL_AND"                             , GL_AND                            },
  { "GL_AND_REVERSE"                     , GL_AND_REVERSE                    },
  { "GL_COPY"                            , GL_COPY                           },
  { "GL_AND_INVERTED"                    , GL_AND_INVERTED                   },
  { "GL_NOOP"                            , GL_NOOP                           },
  { "GL_XOR"                             , GL_XOR                            },
  { "GL_OR"                              , GL_OR                             },
  { "GL_NOR"                             , GL_NOR                            },
  { "GL_EQUIV"                           , GL_EQUIV                          },
  { "GL_INVERT"                          , GL_INVERT                         },
  { "GL_OR_REVERSE"                      , GL_OR_REVERSE                     },
  { "GL_COPY_INVERTED"                   , GL_COPY_INVERTED                  },
  { "GL_OR_INVERTED"                     , GL_OR_INVERTED                    },
  { "GL_NAND"                            , GL_NAND                           },
  { "GL_SET"                             , GL_SET                            },
  { "GL_EMISSION"                        , GL_EMISSION                       },
  { "GL_SHININESS"                       , GL_SHININESS                      },
  { "GL_AMBIENT_AND_DIFFUSE"             , GL_AMBIENT_AND_DIFFUSE            },
  { "GL_MODELVIEW"                       , GL_MODELVIEW                      },
  { "GL_PROJECTION"                      , GL_PROJECTION                     },
  { "GL_TEXTURE"                         , GL_TEXTURE                        },
  { "GL_ALPHA"                           , GL_ALPHA                          },
  { "GL_RGB"                             , GL_RGB                            },
  { "GL_RGBA"                            , GL_RGBA                           },
  { "GL_LUMINANCE"                       , GL_LUMINANCE                      },
  { "GL_LUMINANCE_ALPHA"                 , GL_LUMINANCE_ALPHA                },
  { "GL_FLAT"                            , GL_FLAT                           },
  { "GL_SMOOTH"                          , GL_SMOOTH                         },
  { "GL_KEEP"                            , GL_KEEP                           },
  { "GL_REPLACE"                         , GL_REPLACE                        },
  { "GL_INCR"                            , GL_INCR                           },
  { "GL_DECR"                            , GL_DECR                           },
  { "GL_VENDOR"                          , GL_VENDOR                         },
  { "GL_RENDERER"                        , GL_RENDERER                       },
  { "GL_VERSION"                         , GL_VERSION                        },
  { "GL_EXTENSIONS"                      , GL_EXTENSIONS                     },
  { "GL_MODULATE"                        , GL_MODULATE                       },
  { "GL_DECAL"                           , GL_DECAL                          },
  { "GL_TEXTURE_ENV_MODE"                , GL_TEXTURE_ENV_MODE               },
  { "GL_TEXTURE_ENV_COLOR"               , GL_TEXTURE_ENV_COLOR              },
  { "GL_TEXTURE_ENV"                     , GL_TEXTURE_ENV                    },
  { "GL_NEAREST"                         , GL_NEAREST                        },
  { "GL_LINEAR"                          , GL_LINEAR                         },
  { "GL_NEAREST_MIPMAP_NEAREST"          , GL_NEAREST_MIPMAP_NEAREST         },
  { "GL_LINEAR_MIPMAP_NEAREST"           , GL_LINEAR_MIPMAP_NEAREST          },
  { "GL_NEAREST_MIPMAP_LINEAR"           , GL_NEAREST_MIPMAP_LINEAR          },
  { "GL_LINEAR_MIPMAP_LINEAR"            , GL_LINEAR_MIPMAP_LINEAR           },
  { "GL_TEXTURE_MAG_FILTER"              , GL_TEXTURE_MAG_FILTER             },
  { "GL_TEXTURE_MIN_FILTER"              , GL_TEXTURE_MIN_FILTER             },
  { "GL_TEXTURE_WRAP_S"                  , GL_TEXTURE_WRAP_S                 },
  { "GL_TEXTURE_WRAP_T"                  , GL_TEXTURE_WRAP_T                 },
  { "GL_REPEAT"                          , GL_REPEAT                         },
#ifdef GL_MIRRORED_REPEAT
  { "GL_MIRRORED_REPEAT"                 , GL_MIRRORED_REPEAT                },
#endif
#ifdef GL_VERSION_1_2
  { "GL_CLAMP_TO_EDGE"                   , GL_CLAMP_TO_EDGE                  },
#endif
  { "GL_POLYGON_OFFSET_FACTOR"           , GL_POLYGON_OFFSET_FACTOR          },
  { "GL_POLYGON_OFFSET_UNITS"            , GL_POLYGON_OFFSET_UNITS           },
  { "GL_POLYGON_OFFSET_FILL"             , GL_POLYGON_OFFSET_FILL            },
  { "GL_TEXTURE_BINDING_2D"              , GL_TEXTURE_BINDING_2D             },
  { "GL_VERTEX_ARRAY"                    , GL_VERTEX_ARRAY                   },
  { "GL_NORMAL_ARRAY"                    , GL_NORMAL_ARRAY                   },
  { "GL_COLOR_ARRAY"                     , GL_COLOR_ARRAY                    },
  { "GL_TEXTURE_COORD_ARRAY"             , GL_TEXTURE_COORD_ARRAY            },
  { "GL_VERTEX_ARRAY_SIZE"               , GL_VERTEX_ARRAY_SIZE              },
  { "GL_VERTEX_ARRAY_TYPE"               , GL_VERTEX_ARRAY_TYPE              },
  { "GL_VERTEX_ARRAY_STRIDE"             , GL_VERTEX_ARRAY_STRIDE            },
  { "GL_NORMAL_ARRAY_TYPE"               , GL_NORMAL_ARRAY_TYPE              },
  { "GL_NORMAL_ARRAY_STRIDE"             , GL_NORMAL_ARRAY_STRIDE            },
  { "GL_COLOR_ARRAY_SIZE"                , GL_COLOR_ARRAY_SIZE               },
  { "GL_COLOR_ARRAY_TYPE"                , GL_COLOR_ARRAY_TYPE               },
  { "GL_COLOR_ARRAY_STRIDE"              , GL_COLOR_ARRAY_STRIDE             },
  { "GL_TEXTURE_COORD_ARRAY_SIZE"        , GL_TEXTURE_COORD_ARRAY_SIZE       },
  { "GL_TEXTURE_COORD_ARRAY_TYPE"        , GL_TEXTURE_COORD_ARRAY_TYPE       },
  { "GL_TEXTURE_COORD_ARRAY_STRIDE"      , GL_TEXTURE_COORD_ARRAY_STRIDE     },
  { "GL_VERTEX_ARRAY_POINTER"            , GL_VERTEX_ARRAY_POINTER           },
  { "GL_NORMAL_ARRAY_POINTER"            , GL_NORMAL_ARRAY_POINTER           },
  { "GL_COLOR_ARRAY_POINTER"             , GL_COLOR_ARRAY_POINTER            },
  { "GL_TEXTURE_COORD_ARRAY_POINTER"     , GL_TEXTURE_COORD_ARRAY_POINTER    },
#ifdef GL_EXT_vertex_array
  { "GL_EXT_vertex_array"                , GL_EXT_vertex_array               },
#endif
#ifdef GL_EXT_bgra
  { "GL_EXT_bgra"                        , GL_EXT_bgra                       },
#endif
#ifdef GL_EXT_paletted_texture
  { "GL_EXT_paletted_texture"            , GL_EXT_paletted_texture           },
#endif
#ifdef GL_WIN_swap_hint
  { "GL_WIN_swap_hint"                   , GL_WIN_swap_hint                  },
  { "GL_WIN_draw_range_elements"         , GL_WIN_draw_range_elements        },
#endif
#ifdef GL_VERTEX_ARRAY_EXT
  { "GL_VERTEX_ARRAY_EXT"                , GL_VERTEX_ARRAY_EXT               },
  { "GL_NORMAL_ARRAY_EXT"                , GL_NORMAL_ARRAY_EXT               },
  { "GL_COLOR_ARRAY_EXT"                 , GL_COLOR_ARRAY_EXT                },
  { "GL_INDEX_ARRAY_EXT"                 , GL_INDEX_ARRAY_EXT                },
  { "GL_TEXTURE_COORD_ARRAY_EXT"         , GL_TEXTURE_COORD_ARRAY_EXT        },
  { "GL_EDGE_FLAG_ARRAY_EXT"             , GL_EDGE_FLAG_ARRAY_EXT            },
  { "GL_VERTEX_ARRAY_SIZE_EXT"           , GL_VERTEX_ARRAY_SIZE_EXT          },
  { "GL_VERTEX_ARRAY_TYPE_EXT"           , GL_VERTEX_ARRAY_TYPE_EXT          },
  { "GL_VERTEX_ARRAY_STRIDE_EXT"         , GL_VERTEX_ARRAY_STRIDE_EXT        },
  { "GL_VERTEX_ARRAY_COUNT_EXT"          , GL_VERTEX_ARRAY_COUNT_EXT         },
  { "GL_NORMAL_ARRAY_TYPE_EXT"           , GL_NORMAL_ARRAY_TYPE_EXT          },
  { "GL_NORMAL_ARRAY_STRIDE_EXT"         , GL_NORMAL_ARRAY_STRIDE_EXT        },
  { "GL_NORMAL_ARRAY_COUNT_EXT"          , GL_NORMAL_ARRAY_COUNT_EXT         },
  { "GL_COLOR_ARRAY_SIZE_EXT"            , GL_COLOR_ARRAY_SIZE_EXT           },
  { "GL_COLOR_ARRAY_TYPE_EXT"            , GL_COLOR_ARRAY_TYPE_EXT           },
  { "GL_COLOR_ARRAY_STRIDE_EXT"          , GL_COLOR_ARRAY_STRIDE_EXT         },
  { "GL_COLOR_ARRAY_COUNT_EXT"           , GL_COLOR_ARRAY_COUNT_EXT          },
  { "GL_INDEX_ARRAY_TYPE_EXT"            , GL_INDEX_ARRAY_TYPE_EXT           },
  { "GL_INDEX_ARRAY_STRIDE_EXT"          , GL_INDEX_ARRAY_STRIDE_EXT         },
  { "GL_INDEX_ARRAY_COUNT_EXT"           , GL_INDEX_ARRAY_COUNT_EXT          },
  { "GL_TEXTURE_COORD_ARRAY_SIZE_EXT"    , GL_TEXTURE_COORD_ARRAY_SIZE_EXT   },
  { "GL_TEXTURE_COORD_ARRAY_TYPE_EXT"    , GL_TEXTURE_COORD_ARRAY_TYPE_EXT   },
  { "GL_TEXTURE_COORD_ARRAY_STRIDE_EXT"  , GL_TEXTURE_COORD_ARRAY_STRIDE_EXT },
  { "GL_TEXTURE_COORD_ARRAY_COUNT_EXT"   , GL_TEXTURE_COORD_ARRAY_COUNT_EXT  },
  { "GL_EDGE_FLAG_ARRAY_STRIDE_EXT"      , GL_EDGE_FLAG_ARRAY_STRIDE_EXT     },
  { "GL_EDGE_FLAG_ARRAY_COUNT_EXT"       , GL_EDGE_FLAG_ARRAY_COUNT_EXT      },
  { "GL_VERTEX_ARRAY_POINTER_EXT"        , GL_VERTEX_ARRAY_POINTER_EXT       },
  { "GL_NORMAL_ARRAY_POINTER_EXT"        , GL_NORMAL_ARRAY_POINTER_EXT       },
  { "GL_COLOR_ARRAY_POINTER_EXT"         , GL_COLOR_ARRAY_POINTER_EXT        },
  { "GL_INDEX_ARRAY_POINTER_EXT"         , GL_INDEX_ARRAY_POINTER_EXT        },
  { "GL_TEXTURE_COORD_ARRAY_POINTER_EXT" , GL_TEXTURE_COORD_ARRAY_POINTER_EXT},
  { "GL_EDGE_FLAG_ARRAY_POINTER_EXT"     , GL_EDGE_FLAG_ARRAY_POINTER_EXT    },
#endif
#ifdef GL_BGR_EXT
  { "GL_BGR_EXT"                         , GL_BGR_EXT                        },
  { "GL_BGRA_EXT"                        , GL_BGRA_EXT                       },
#endif
#ifdef GL_COLOR_TABLE_FORMAT_EXT
  { "GL_COLOR_TABLE_FORMAT_EXT"          , GL_COLOR_TABLE_FORMAT_EXT         },
  { "GL_COLOR_TABLE_WIDTH_EXT"           , GL_COLOR_TABLE_WIDTH_EXT          },
  { "GL_COLOR_TABLE_RED_SIZE_EXT"        , GL_COLOR_TABLE_RED_SIZE_EXT       },
  { "GL_COLOR_TABLE_GREEN_SIZE_EXT"      , GL_COLOR_TABLE_GREEN_SIZE_EXT     },
  { "GL_COLOR_TABLE_BLUE_SIZE_EXT"       , GL_COLOR_TABLE_BLUE_SIZE_EXT      },
  { "GL_COLOR_TABLE_ALPHA_SIZE_EXT"      , GL_COLOR_TABLE_ALPHA_SIZE_EXT     },
  { "GL_COLOR_TABLE_LUMINANCE_SIZE_EXT"  , GL_COLOR_TABLE_LUMINANCE_SIZE_EXT },
  { "GL_COLOR_TABLE_INTENSITY_SIZE_EXT"  , GL_COLOR_TABLE_INTENSITY_SIZE_EXT },
#endif
#ifdef GL_COLOR_INDEX1_EXT
  { "GL_COLOR_INDEX1_EXT"                , GL_COLOR_INDEX1_EXT               },
  { "GL_COLOR_INDEX2_EXT"                , GL_COLOR_INDEX2_EXT               },
  { "GL_COLOR_INDEX4_EXT"                , GL_COLOR_INDEX4_EXT               },
  { "GL_COLOR_INDEX8_EXT"                , GL_COLOR_INDEX8_EXT               },
  { "GL_COLOR_INDEX12_EXT"               , GL_COLOR_INDEX12_EXT              },
  { "GL_COLOR_INDEX16_EXT"               , GL_COLOR_INDEX16_EXT              },
#endif
#ifdef GL_MAX_ELEMENTS_VERTICES_WIN
  { "GL_MAX_ELEMENTS_VERTICES_WIN"       , GL_MAX_ELEMENTS_VERTICES_WIN      },
  { "GL_MAX_ELEMENTS_INDICES_WIN"        , GL_MAX_ELEMENTS_INDICES_WIN       },
#endif
#ifdef GL_PHONG_WIN
  { "GL_PHONG_WIN"                       , GL_PHONG_WIN                      },
  { "GL_PHONG_HINT_WIN"                  , GL_PHONG_HINT_WIN                 },
#endif
#ifdef GL_FOG_SPECULAR_TEXTURE_WIN
  { "GL_FOG_SPECULAR_TEXTURE_WIN"        , GL_FOG_SPECULAR_TEXTURE_WIN       },
#endif
  { "GL_DEPTH_BUFFER_BIT"                , GL_DEPTH_BUFFER_BIT               },
  { "GL_STENCIL_BUFFER_BIT"              , GL_STENCIL_BUFFER_BIT             },
  { "GL_COLOR_BUFFER_BIT"                , GL_COLOR_BUFFER_BIT               },
#ifdef GL_CLIENT_ALL_ATTRIB_BITS
  { "GL_CLIENT_ALL_ATTRIB_BITS"          , GL_CLIENT_ALL_ATTRIB_BITS         },
#endif
  { 0, 0}
};

unsigned int luagl_get_gl_enum(lua_State *L, int index)
{
  return luagl_get_enum(L, index, luagl_const);
}

const char *luagl_get_str_gl_enum(unsigned int num)
{
  unsigned int i = 0;

  /* works only for simple enums */
  while(luagl_const[i].str != 0)
  {
    if(num == luagl_const[i].value)
      return luagl_const[i].str;

    i++;
  }
  return NULL;
}

void luagl_pushenum(lua_State *L, GLenum num)
{
  const char* str = luagl_get_str_gl_enum(num);
  if (str)
    lua_pushstring(L, str);
  else
    lua_pushinteger(L, num);
}

static unsigned int luagl_find_enum(const luaglConst* gl_const, const char *str, int n)
{
  int i = 0;

  while(gl_const[i].str != 0)
  {
    if(strncmp(str, gl_const[i].str, n) == 0 && gl_const[i].str[n] == 0)
      return gl_const[i].value;
    i++;
  }
  return LUAGL_ENUM_ERROR;
}

unsigned int luagl_get_enum(lua_State *L, int index, const luaglConst* gl_const)
{
  if (lua_isnumber(L, index))
  {
    return (unsigned int)luaL_checkinteger(L, index);
  }
  else
  {
    unsigned int i;
    const char *str = luaL_checkstring(L, index);
    unsigned int len = (unsigned int)strlen(str);
    unsigned int temp = 0, ret = 0, found = 0;

    for(i = 0; i < len; i++)
    {
      if(str[i] == ',')
      {
        temp = luagl_find_enum(gl_const, str, i);
        if (temp != LUAGL_ENUM_ERROR)
        {
          ret |= temp;
          found = 1;
        }

        str += i+1;
        len -= i+1;
        i = 0;
      }
    }
    temp = luagl_find_enum(gl_const, str, len);

    if (temp == LUAGL_ENUM_ERROR)
    {
      if (!found)
      {
        luaL_argerror(L, index, "unknown or invalid enumeration");
        return LUAGL_ENUM_ERROR;
      }

      return ret;
    }

    return ret | temp;
  }
}

void luagl_initconst(lua_State *L, const luaglConst *gl_const)
{
  for (; gl_const->str; gl_const++) 
  {
    if (gl_const->value == LUAGL_ENUM_ERROR)
      luaL_error(L, "WARNING: INVALID CONSTANT=LUAGL_ENUM_ERROR");

    lua_pushstring(L, gl_const->str);
    lua_pushinteger(L, gl_const->value);
    lua_settable(L, -3);
  }
}

void luagl_open_const(lua_State *L) 
{
  luagl_initconst(L, luagl_const);
}
