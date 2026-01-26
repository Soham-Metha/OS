/* keyboard.h */

enum special_keys {
    FUN_1 = 0,
    FUN_2 = 0,
    FUN_3 = 0,
    FUN_4 = 0,
    FUN_5 = 0,
    FUN_6 = 0,
    FUN_7 = 0,
    FUN_8 = 0,
    FUN_9 = 0,
    FUN10 = 0,
    FUN11 = 0,
    FUN12 = 0,
    ESCPE = 0,
    LCTRL = 0,
    LSHFT = 0,
    RSHFT = 0,
    L_ALT = 0,
    CAPSL = 0,
    NUMLK = 0,
    SCRLL = 0,
    COUNT = 1,
};

// https://users.utcluj.ro/~baruch/sie/labor/PS2/Scan_Codes_Set_1.htm
char scan_code_ascii[128] = {
    [0x01] =   ESCPE,   '1',   '2',   '3',   '4',   '5',   '6',   '7',   '8',   '9',   '0',   '-',   '=',   '\b',
    [0x0F] =    '\t',   'q',   'w',   'e',   'r',   't',   'y',   'u',   'i',   'o',   'p',   '[',   ']',   '\n',
    [0x1D] =   LCTRL,   'a',   's',   'd',   'f',   'g',   'h',   'j',   'k',   'l',   ';',  '\'',   '`',
    [0x2A] =   LSHFT,  '\\',   'z',   'x' ,  'c',   'v',   'b',   'n',   'm',   ',',   '.',   '/', RSHFT,    '*',
    [0x38] =   L_ALT,   ' ', CAPSL,
    [0x3B] =   FUN_1, FUN_2, FUN_3, FUN_4, FUN_5, FUN_6, FUN_7, FUN_8, FUN_9, FUN10, NUMLK, SCRLL, 
    [0x47] =     '7',   '8',   '9',   '-',
    [0x4B] =     '4',   '5',   '6',   '+',
    [0x4f] =     '1',   '2',   '3',
    [0x52] =     '0',   '.',
    [0x57] =   FUN11, FUN12,
    [0x54] =       0,     0,    0, // undefined
};
