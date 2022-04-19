//
// Created by caiiiycuk on 10.02.22.
//

#include "keyboard_codes.h"

#include <unordered_map>

#include "xtool.h"

namespace {
// clang-format off
std::unordered_map<sapp_keycode, int> mapping = {
    { SAPP_KEYCODE_INVALID          , VK_UNKNOWN         },
    { SAPP_KEYCODE_SPACE            , VK_SPACE           },
//    { SAPP_KEYCODE_APOSTROPHE       , VK_APOSTROPHE      },
//    { SAPP_KEYCODE_COMMA            , VK_COMMA           },
//    { SAPP_KEYCODE_MINUS            , VK_MINUS           },
//    { SAPP_KEYCODE_PERIOD           , VK_PERIOD          },
    { SAPP_KEYCODE_SLASH            , VK_SLASH           },
    { SAPP_KEYCODE_0                , VK_0               },
    { SAPP_KEYCODE_1                , VK_1               },
    { SAPP_KEYCODE_2                , VK_2               },
    { SAPP_KEYCODE_3                , VK_3               },
    { SAPP_KEYCODE_4                , VK_4               },
    { SAPP_KEYCODE_5                , VK_5               },
    { SAPP_KEYCODE_6                , VK_6               },
    { SAPP_KEYCODE_7                , VK_7               },
    { SAPP_KEYCODE_8                , VK_8               },
    { SAPP_KEYCODE_9                , VK_9               },
//    { SAPP_KEYCODE_SEMICOLON        , VK_SEMICOLON       },
//    { SAPP_KEYCODE_EQUAL            , VK_EQUAL           },
    { SAPP_KEYCODE_A                , VK_A               },
    { SAPP_KEYCODE_B                , VK_B               },
    { SAPP_KEYCODE_C                , VK_C               },
    { SAPP_KEYCODE_D                , VK_D               },
    { SAPP_KEYCODE_E                , VK_E               },
    { SAPP_KEYCODE_F                , VK_F               },
    { SAPP_KEYCODE_G                , VK_G               },
    { SAPP_KEYCODE_H                , VK_H               },
    { SAPP_KEYCODE_I                , VK_I               },
    { SAPP_KEYCODE_J                , VK_J               },
    { SAPP_KEYCODE_K                , VK_K               },
    { SAPP_KEYCODE_L                , VK_L               },
    { SAPP_KEYCODE_M                , VK_M               },
    { SAPP_KEYCODE_N                , VK_N               },
    { SAPP_KEYCODE_O                , VK_O               },
    { SAPP_KEYCODE_P                , VK_P               },
    { SAPP_KEYCODE_Q                , VK_Q               },
    { SAPP_KEYCODE_R                , VK_R               },
    { SAPP_KEYCODE_S                , VK_S               },
    { SAPP_KEYCODE_T                , VK_T               },
    { SAPP_KEYCODE_U                , VK_U               },
    { SAPP_KEYCODE_V                , VK_V               },
    { SAPP_KEYCODE_W                , VK_W               },
    { SAPP_KEYCODE_X                , VK_X               },
    { SAPP_KEYCODE_Y                , VK_Y               },
    { SAPP_KEYCODE_Z                , VK_Z               },
//    { SAPP_KEYCODE_LEFT_BRACKET     , VK_LEFT_BRACKET    },
//    { SAPP_KEYCODE_BACKSLASH        , VK_BACKSLASH       },
//    { SAPP_KEYCODE_RIGHT_BRACKET    , VK_RIGHT_BRACKET   },
//    { SAPP_KEYCODE_GRAVE_ACCENT     , VK_GRAVE_ACCENT    },
//    { SAPP_KEYCODE_WORLD_1          , VK_WORLD_1         },
//    { SAPP_KEYCODE_WORLD_2          , VK_WORLD_2         },
    { SAPP_KEYCODE_ESCAPE           , VK_ESCAPE          },
    { SAPP_KEYCODE_ENTER            , VK_RETURN          },
    { SAPP_KEYCODE_TAB              , VK_TAB             },
    { SAPP_KEYCODE_BACKSPACE        , VK_BACK            },
    { SAPP_KEYCODE_INSERT           , VK_INSERT          },
    { SAPP_KEYCODE_DELETE           , VK_DELETE          },
    { SAPP_KEYCODE_RIGHT            , VK_RIGHT           },
    { SAPP_KEYCODE_LEFT             , VK_LEFT            },
    { SAPP_KEYCODE_DOWN             , VK_DOWN            },
    { SAPP_KEYCODE_UP               , VK_UP              },
//    { SAPP_KEYCODE_PAGE_UP          , VK_PAGE_UP         },
//    { SAPP_KEYCODE_PAGE_DOWN        , VK_PAGE_DOWN       },
    { SAPP_KEYCODE_HOME             , VK_HOME            },
    { SAPP_KEYCODE_END              , VK_END             },
//    { SAPP_KEYCODE_CAPS_LOCK        , VK_CAPS_LOCK       },
//    { SAPP_KEYCODE_SCROLL_LOCK      , VK_SCROLL_LOCK     },
//    { SAPP_KEYCODE_NUM_LOCK         , VK_NUM_LOCK        },
//    { SAPP_KEYCODE_PRINT_SCREEN     , VK_PRINT_SCREEN    },
    { SAPP_KEYCODE_PAUSE            , VK_PAUSE           },
    { SAPP_KEYCODE_F1               , VK_F1              },
    { SAPP_KEYCODE_F2               , VK_F2              },
    { SAPP_KEYCODE_F3               , VK_F3              },
    { SAPP_KEYCODE_F4               , VK_F4              },
    { SAPP_KEYCODE_F5               , VK_F5              },
    { SAPP_KEYCODE_F6               , VK_F6              },
    { SAPP_KEYCODE_F7               , VK_F7              },
    { SAPP_KEYCODE_F8               , VK_F8              },
    { SAPP_KEYCODE_F9               , VK_F9              },
    { SAPP_KEYCODE_F10              , VK_F10             },
    { SAPP_KEYCODE_F11              , VK_F11             },
    { SAPP_KEYCODE_F12              , VK_F12             },
    { SAPP_KEYCODE_F13              , VK_F13             },
    { SAPP_KEYCODE_F14              , VK_F14             },
    { SAPP_KEYCODE_F15              , VK_F15             },
    { SAPP_KEYCODE_F16              , VK_F16             },
    { SAPP_KEYCODE_F17              , VK_F17             },
    { SAPP_KEYCODE_F18              , VK_F18             },
    { SAPP_KEYCODE_F19              , VK_F19             },
    { SAPP_KEYCODE_F20              , VK_F20             },
    { SAPP_KEYCODE_F21              , VK_F21             },
    { SAPP_KEYCODE_F22              , VK_F22             },
    { SAPP_KEYCODE_F23              , VK_F23             },
    { SAPP_KEYCODE_F24              , VK_F24             },
//    { SAPP_KEYCODE_F25              , VK_F25             },
    { SAPP_KEYCODE_KP_0             , VK_NUMPAD0         },
    { SAPP_KEYCODE_KP_1             , VK_NUMPAD1         },
    { SAPP_KEYCODE_KP_2             , VK_NUMPAD2         },
    { SAPP_KEYCODE_KP_3             , VK_NUMPAD3         },
    { SAPP_KEYCODE_KP_4             , VK_NUMPAD4         },
    { SAPP_KEYCODE_KP_5             , VK_NUMPAD5         },
    { SAPP_KEYCODE_KP_6             , VK_NUMPAD6         },
    { SAPP_KEYCODE_KP_7             , VK_NUMPAD7         },
    { SAPP_KEYCODE_KP_8             , VK_NUMPAD8         },
    { SAPP_KEYCODE_KP_9             , VK_NUMPAD9         },
//    { SAPP_KEYCODE_KP_DECIMAL       , VK_KP_DECIMAL      },
//    { SAPP_KEYCODE_KP_DIVIDE        , VK_KP_DIVIDE       },
//    { SAPP_KEYCODE_KP_MULTIPLY      , VK_KP_MULTIPLY     },
//    { SAPP_KEYCODE_KP_SUBTRACT      , VK_KP_SUBTRACT     },
//    { SAPP_KEYCODE_KP_ADD           , VK_KP_ADD          },
//    { SAPP_KEYCODE_KP_ENTER         , VK_KP_ENTER        },
//    { SAPP_KEYCODE_KP_EQUAL         , VK_KP_EQUAL        },
    { SAPP_KEYCODE_LEFT_SHIFT       , VK_LSHIFT          },
    { SAPP_KEYCODE_LEFT_CONTROL     , VK_LCONTROL        },
//    { SAPP_KEYCODE_LEFT_ALT         , VK_LALT            },
//    { SAPP_KEYCODE_LEFT_SUPER       , VK_LSUPER          },
    { SAPP_KEYCODE_RIGHT_SHIFT      , VK_RSHIFT          },
    { SAPP_KEYCODE_RIGHT_CONTROL    , VK_RCONTROL        },
//    { SAPP_KEYCODE_RIGHT_ALT        , VK_RALT            },
//    { SAPP_KEYCODE_RIGHT_SUPER      , VK_RSUPER          },
    { SAPP_KEYCODE_MENU             , VK_MENU            },
};

// clang-format on

}
int platform::keyFromSappKeyCode(sapp_keycode event) {
  auto found = mapping.find(event);
  if (found == mapping.end()) {
    return VK_UNKNOWN;
  }
  return found->second;
}