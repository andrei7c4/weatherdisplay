#include <ets_sys.h>

/* ' ' */
static const unsigned int char32[5] ICACHE_RODATA_ATTR={
8,1,4,12,
0x00000000};

/* '!' */
static const unsigned int char33[7] ICACHE_RODATA_ATTR={
8,10,12,0,
0x08080808,0x08080808,0x00000800};

/* '"' */
static const unsigned int char34[5] ICACHE_RODATA_ATTR={
8,3,4,0,
0x00141414};

/* '#' */
static const unsigned int char35[7] ICACHE_RODATA_ATTR={
8,10,12,0,
0xFF221212,0x48FF2424,0x00004848};

/* '%' */
static const unsigned int char37[7] ICACHE_RODATA_ATTR={
8,10,12,0,
0xA8A4A440,0x25151248,0x00000225};

/* '&' */
static const unsigned int char38[7] ICACHE_RODATA_ATTR={
8,10,12,0,
0x48484830,0x8A926030,0x0000738C};

/* ''' */
static const unsigned int char39[5] ICACHE_RODATA_ATTR={
8,3,4,0,
0x00080808};

/* '(' */
static const unsigned int char40[8] ICACHE_RODATA_ATTR={
8,13,16,0,
0x10080804,0x10101010,0x08081010,0x00000004};

/* ')' */
static const unsigned int char41[8] ICACHE_RODATA_ATTR={
8,13,16,0,
0x04080810,0x04040404,0x08080404,0x00000010};

/* '*' */
static const unsigned int char42[5] ICACHE_RODATA_ATTR={
8,4,4,0,
0x14083E08};

/* '+' */
static const unsigned int char43[6] ICACHE_RODATA_ATTR={
8,7,8,2,
0x7F080808,0x00080808};

/* ',' */
static const unsigned int char44[5] ICACHE_RODATA_ATTR={
8,3,4,9,
0x00080808};

/* '-' */
static const unsigned int char45[5] ICACHE_RODATA_ATTR={
8,1,4,6,
0x0000001C};

/* '.' */
static const unsigned int char46[5] ICACHE_RODATA_ATTR={
8,1,4,9,
0x00000008};

/* '/' */
static const unsigned int char47[7] ICACHE_RODATA_ATTR={
8,10,12,0,
0x08080404,0x10101008,0x00002020};

/* '0' */
static const unsigned int char48[7] ICACHE_RODATA_ATTR={
8,10,12,0,
0x4242423C,0x42424242,0x00003C42};

/* '1' */
static const unsigned int char49[7] ICACHE_RODATA_ATTR={
8,10,12,0,
0x04140C04,0x04040404,0x00000404};

/* '2' */
static const unsigned int char50[7] ICACHE_RODATA_ATTR={
8,10,12,0,
0x0202423C,0x10080402,0x00007E20};

/* '3' */
static const unsigned int char51[7] ICACHE_RODATA_ATTR={
8,10,12,0,
0x0202423C,0x0202021C,0x00003C42};

/* '4' */
static const unsigned int char52[7] ICACHE_RODATA_ATTR={
8,10,12,0,
0x140C0C04,0x7E242414,0x00000404};

/* '5' */
static const unsigned int char53[7] ICACHE_RODATA_ATTR={
8,10,12,0,
0x4020203E,0x0202427C,0x00003C42};

/* '6' */
static const unsigned int char54[7] ICACHE_RODATA_ATTR={
8,10,12,0,
0x4040423C,0x4242625C,0x00003C42};

/* '7' */
static const unsigned int char55[7] ICACHE_RODATA_ATTR={
8,10,12,0,
0x0804047E,0x20101008,0x00002020};

/* '8' */
static const unsigned int char56[7] ICACHE_RODATA_ATTR={
8,10,12,0,
0x4242423C,0x4242423C,0x00003C42};

/* '9' */
static const unsigned int char57[7] ICACHE_RODATA_ATTR={
8,10,12,0,
0x4242423C,0x02023A46,0x00003844};

/* ':' */
static const unsigned int char58[6] ICACHE_RODATA_ATTR={
8,7,8,3,
0x00000008,0x00080000};

/* ';' */
static const unsigned int char59[7] ICACHE_RODATA_ATTR={
8,9,12,3,
0x00000008,0x08080000,0x00000008};

/* '<' */
static const unsigned int char60[6] ICACHE_RODATA_ATTR={
8,7,8,2,
0x40300C02,0x00020C30};

/* '=' */
static const unsigned int char61[5] ICACHE_RODATA_ATTR={
8,4,4,3,
0x7F00007F};

/* '>' */
static const unsigned int char62[6] ICACHE_RODATA_ATTR={
8,7,8,2,
0x020C3040,0x0040300C};

/* '?' */
static const unsigned int char63[7] ICACHE_RODATA_ATTR={
8,10,12,0,
0x0222221C,0x10100804,0x00001000};

/* '@' */
static const unsigned int char64[11] ICACHE_RODATA_ATTR={
16,13,28,0,
0x100CE003,0xA4130810,0x24286424,0x48284428,0x7027C828,0x18080410,0x0000E007};

/* 'A' */
static const unsigned int char65[7] ICACHE_RODATA_ATTR={
8,10,12,0,
0x18181800,0x427E2424,0x00008181};

/* 'B' */
static const unsigned int char66[7] ICACHE_RODATA_ATTR={
8,10,12,0,
0x4141417E,0x4141417E,0x00007E41};

/* 'C' */
static const unsigned int char67[7] ICACHE_RODATA_ATTR={
8,10,12,0,
0x4041221C,0x41404040,0x00001C22};

/* 'D' */
static const unsigned int char68[7] ICACHE_RODATA_ATTR={
8,10,12,0,
0x4141427C,0x41414141,0x00007C42};

/* 'E' */
static const unsigned int char69[7] ICACHE_RODATA_ATTR={
8,10,12,0,
0x4040407F,0x4040407F,0x00007F40};

/* 'F' */
static const unsigned int char70[7] ICACHE_RODATA_ATTR={
8,10,12,0,
0x4040407E,0x4040407C,0x00004040};

/* 'G' */
static const unsigned int char71[7] ICACHE_RODATA_ATTR={
8,10,12,0,
0x8081423C,0x81818F80,0x00003C42};

/* 'H' */
static const unsigned int char72[7] ICACHE_RODATA_ATTR={
8,10,12,0,
0x41414141,0x4141417F,0x00004141};

/* 'I' */
static const unsigned int char73[7] ICACHE_RODATA_ATTR={
8,10,12,0,
0x08080808,0x08080808,0x00000808};

/* 'J' */
static const unsigned int char74[7] ICACHE_RODATA_ATTR={
8,10,12,0,
0x02020202,0x22020202,0x00001C22};

/* 'K' */
static const unsigned int char75[7] ICACHE_RODATA_ATTR={
8,10,12,0,
0x88848281,0x84C8B090,0x00008182};

/* 'L' */
static const unsigned int char76[7] ICACHE_RODATA_ATTR={
8,10,12,0,
0x40404040,0x40404040,0x00007E40};

/* 'M' */
static const unsigned int char77[7] ICACHE_RODATA_ATTR={
8,10,12,0,
0xA5C3C381,0x999999A5,0x00008181};

/* 'N' */
static const unsigned int char78[7] ICACHE_RODATA_ATTR={
8,10,12,0,
0x51516141,0x45454949,0x00004143};

/* 'O' */
static const unsigned int char79[7] ICACHE_RODATA_ATTR={
8,10,12,0,
0x8181423C,0x81818181,0x00003C42};

/* 'P' */
static const unsigned int char80[7] ICACHE_RODATA_ATTR={
8,10,12,0,
0x4141417E,0x40407E41,0x00004040};

/* 'Q' */
static const unsigned int char81[7] ICACHE_RODATA_ATTR={
8,10,12,0,
0x8181423C,0x8D818181,0x00003D42};

/* 'R' */
static const unsigned int char82[7] ICACHE_RODATA_ATTR={
8,10,12,0,
0x4141417E,0x42447E41,0x00004142};

/* 'S' */
static const unsigned int char83[7] ICACHE_RODATA_ATTR={
8,10,12,0,
0x4041413E,0x41010638,0x00003E41};

/* 'T' */
static const unsigned int char84[7] ICACHE_RODATA_ATTR={
8,10,12,0,
0x0808087F,0x08080808,0x00000808};

/* 'U' */
static const unsigned int char85[7] ICACHE_RODATA_ATTR={
8,10,12,0,
0x41414141,0x41414141,0x00001C22};

/* 'V' */
static const unsigned int char86[7] ICACHE_RODATA_ATTR={
8,10,12,0,
0x42428181,0x18182424,0x00000000};

/* 'W' */
static const unsigned int char87[9] ICACHE_RODATA_ATTR={
16,10,20,0,
0x42218220,0x44114411,0x280A4411,0x280A280A,0x10041004};

/* 'X' */
static const unsigned int char88[7] ICACHE_RODATA_ATTR={
8,10,12,0,
0x14222241,0x22140808,0x00004122};

/* 'Y' */
static const unsigned int char89[7] ICACHE_RODATA_ATTR={
8,10,12,0,
0x14222241,0x08080814,0x00000808};

/* 'Z' */
static const unsigned int char90[7] ICACHE_RODATA_ATTR={
8,10,12,0,
0x0404023F,0x10100808,0x00007F20};

/* '[' */
static const unsigned int char91[8] ICACHE_RODATA_ATTR={
8,13,16,0,
0x10101018,0x10101010,0x10101010,0x00000018};

/* '\' */
static const unsigned int char92[7] ICACHE_RODATA_ATTR={
8,10,12,0,
0x10102020,0x08080810,0x00000404};

/* ']' */
static const unsigned int char93[8] ICACHE_RODATA_ATTR={
8,13,16,0,
0x08080818,0x08080808,0x08080808,0x00000018};

/* '^' */
static const unsigned int char94[6] ICACHE_RODATA_ATTR={
8,5,8,1,
0x14141408,0x00000022};

/* '_' */
static const unsigned int char95[5] ICACHE_RODATA_ATTR={
8,1,4,12,
0x0000007F};

/* '`' */
static const unsigned int char96[5] ICACHE_RODATA_ATTR={
8,2,4,0,
0x00000810};

/* 'a' */
static const unsigned int char97[6] ICACHE_RODATA_ATTR={
8,7,8,3,
0x1E02221C,0x001A2622};

/* 'b' */
static const unsigned int char98[7] ICACHE_RODATA_ATTR={
8,10,12,0,
0x2C202020,0x22222232,0x00003C22};

/* 'c' */
static const unsigned int char99[6] ICACHE_RODATA_ATTR={
8,7,8,3,
0x2020221C,0x001C2220};

/* 'd' */
static const unsigned int char100[7] ICACHE_RODATA_ATTR={
8,10,12,0,
0x1A020202,0x22222226,0x00001E22};

/* 'e' */
static const unsigned int char101[6] ICACHE_RODATA_ATTR={
8,7,8,3,
0x3E22221C,0x001C2220};

/* 'f' */
static const unsigned int char102[7] ICACHE_RODATA_ATTR={
8,10,12,0,
0x3810100C,0x10101010,0x00001010};

/* 'g' */
static const unsigned int char103[7] ICACHE_RODATA_ATTR={
8,10,12,3,
0x2222261A,0x021A2622,0x00001C22};

/* 'h' */
static const unsigned int char104[7] ICACHE_RODATA_ATTR={
8,10,12,0,
0x2C202020,0x22222232,0x00002222};

/* 'i' */
static const unsigned int char105[7] ICACHE_RODATA_ATTR={
8,10,12,0,
0x08000008,0x08080808,0x00000808};

/* 'j' */
static const unsigned int char106[8] ICACHE_RODATA_ATTR={
8,13,16,0,
0x04000004,0x04040404,0x04040404,0x00000018};

/* 'k' */
static const unsigned int char107[7] ICACHE_RODATA_ATTR={
8,10,12,0,
0x22202020,0x24382824,0x00002224};

/* 'l' */
static const unsigned int char108[7] ICACHE_RODATA_ATTR={
8,10,12,0,
0x08080808,0x08080808,0x00000808};

/* 'm' */
static const unsigned int char109[6] ICACHE_RODATA_ATTR={
8,7,8,3,
0x8181C9B6,0x00818181};

/* 'n' */
static const unsigned int char110[6] ICACHE_RODATA_ATTR={
8,7,8,3,
0x2222322C,0x00222222};

/* 'o' */
static const unsigned int char111[6] ICACHE_RODATA_ATTR={
8,7,8,3,
0x2222221C,0x001C2222};

/* 'p' */
static const unsigned int char112[7] ICACHE_RODATA_ATTR={
8,10,12,3,
0x2222322C,0x202C3222,0x00002020};

/* 'q' */
static const unsigned int char113[7] ICACHE_RODATA_ATTR={
8,10,12,3,
0x2222261A,0x021A2622,0x00000202};

/* 'r' */
static const unsigned int char114[6] ICACHE_RODATA_ATTR={
8,7,8,3,
0x10101814,0x00101010};

/* 's' */
static const unsigned int char115[6] ICACHE_RODATA_ATTR={
8,7,8,3,
0x1C20221C,0x001C2202};

/* 't' */
static const unsigned int char116[7] ICACHE_RODATA_ATTR={
8,9,12,1,
0x081C0808,0x08080808,0x0000000C};

/* 'u' */
static const unsigned int char117[6] ICACHE_RODATA_ATTR={
8,7,8,3,
0x22222222,0x001E2222};

/* 'v' */
static const unsigned int char118[6] ICACHE_RODATA_ATTR={
8,7,8,3,
0x14142222,0x00080814};

/* 'w' */
static const unsigned int char119[6] ICACHE_RODATA_ATTR={
8,7,8,3,
0x5A5A8181,0x0024245A};

/* 'x' */
static const unsigned int char120[6] ICACHE_RODATA_ATTR={
8,7,8,3,
0x08141422,0x00221414};

/* 'y' */
static const unsigned int char121[7] ICACHE_RODATA_ATTR={
8,10,12,3,
0x14142222,0x08080814,0x00003008};

/* 'z' */
static const unsigned int char122[6] ICACHE_RODATA_ATTR={
8,7,8,3,
0x0804043E,0x003E1010};

/* '{' */
static const unsigned int char123[8] ICACHE_RODATA_ATTR={
8,13,16,0,
0x08080804,0x08100808,0x08080808,0x00000004};

/* '|' */
static const unsigned int char124[7] ICACHE_RODATA_ATTR={
8,12,12,0,
0x08080808,0x08080808,0x08080808};

/* '}' */
static const unsigned int char125[8] ICACHE_RODATA_ATTR={
8,13,16,0,
0x08080810,0x08040808,0x08080808,0x00000010};

/* '~' */
static const unsigned int char126[5] ICACHE_RODATA_ATTR={
8,2,4,4,
0x00004F39};


const unsigned int *arial16[97] ={
(unsigned int*)32,(unsigned int*)126,
char32,
char33,
char34,
char35,
0,
char37,
char38,
char39,
char40,
char41,
char42,
char43,
char44,
char45,
char46,
char47,
char48,
char49,
char50,
char51,
char52,
char53,
char54,
char55,
char56,
char57,
char58,
char59,
char60,
char61,
char62,
char63,
char64,
char65,
char66,
char67,
char68,
char69,
char70,
char71,
char72,
char73,
char74,
char75,
char76,
char77,
char78,
char79,
char80,
char81,
char82,
char83,
char84,
char85,
char86,
char87,
char88,
char89,
char90,
char91,
char92,
char93,
char94,
char95,
char96,
char97,
char98,
char99,
char100,
char101,
char102,
char103,
char104,
char105,
char106,
char107,
char108,
char109,
char110,
char111,
char112,
char113,
char114,
char115,
char116,
char117,
char118,
char119,
char120,
char121,
char122,
char123,
char124,
char125,
char126};
