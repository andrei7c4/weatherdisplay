#include <ets_sys.h>

/* ' ' */
static const unsigned int char32[5] ICACHE_RODATA_ATTR={
8,1,4,17,
0x00000000};

/* '!' */
static const unsigned int char33[8] ICACHE_RODATA_ATTR={
8,13,16,1,
0x18181818,0x18181818,0x18001818,0x00000018};

/* '"' */
static const unsigned int char34[6] ICACHE_RODATA_ATTR={
8,5,8,1,
0x36363636,0x00000036};

/* '#' */
static const unsigned int char35[8] ICACHE_RODATA_ATTR={
8,13,16,1,
0xFF361212,0x6C7E36FF,0x486CFFFF,0x00000048};

/* '$' */
static const unsigned int char36[8] ICACHE_RODATA_ATTR={
8,16,16,0,
0xC37E3C00,0x1E78E0C0,0xE7C3030F,0x00003C7E};

/* '%' */
static const unsigned int char37[11] ICACHE_RODATA_ATTR={
16,13,28,1,
0x6066303C,0xC0666066,0xC067C066,0xE601BC3D,0x66036603,0x66066606,0x00003C0C};

/* '&' */
static const unsigned int char38[8] ICACHE_RODATA_ATTR={
8,13,16,1,
0x6C6C7C38,0x52703068,0x7ECE8E8A,0x00000071};

/* ''' */
static const unsigned int char39[6] ICACHE_RODATA_ATTR={
8,5,8,1,
0x18181818,0x00000018};

/* '(' */
static const unsigned int char40[9] ICACHE_RODATA_ATTR={
8,17,20,1,
0x18080402,0x30303018,0x30303030,0x04081818,0x00000002};

/* ')' */
static const unsigned int char41[9] ICACHE_RODATA_ATTR={
8,17,20,1,
0x0C081020,0x0606060C,0x06060606,0x10080C0C,0x00000020};

/* '*' */
static const unsigned int char42[6] ICACHE_RODATA_ATTR={
8,7,8,0,
0x63633E1C,0x001C3E63};

/* '+' */
static const unsigned int char43[7] ICACHE_RODATA_ATTR={
8,10,12,3,
0x18181818,0x1818FFFF,0x00001818};

/* ',' */
static const unsigned int char44[5] ICACHE_RODATA_ATTR={
8,4,4,12,
0x10081818};

/* '-' */
static const unsigned int char45[5] ICACHE_RODATA_ATTR={
8,2,4,8,
0x00003E3E};

/* '.' */
static const unsigned int char46[5] ICACHE_RODATA_ATTR={
8,2,4,12,
0x00001818};

/* '/' */
static const unsigned int char47[8] ICACHE_RODATA_ATTR={
8,13,16,1,
0x0C0C0606,0x1818180C,0x60303030,0x00000060};

/* '0' */
static const unsigned int char48[8] ICACHE_RODATA_ATTR={
8,13,16,1,
0xC3E77E3C,0xC3C3C3C3,0x7EE7C3C3,0x0000003C};

/* '1' */
static const unsigned int char49[8] ICACHE_RODATA_ATTR={
8,13,16,1,
0x361E0E06,0x06060626,0x06060606,0x00000006};

/* '2' */
static const unsigned int char50[8] ICACHE_RODATA_ATTR={
8,13,16,1,
0xC3E77E3C,0x0C060303,0xFF603018,0x000000FF};

/* '3' */
static const unsigned int char51[8] ICACHE_RODATA_ATTR={
8,13,16,1,
0x06C67E38,0x031E1C06,0x7EC3C303,0x0000003C};

/* '4' */
static const unsigned int char52[8] ICACHE_RODATA_ATTR={
8,13,16,1,
0x1C0C0C0C,0xCC6C3C3C,0x0C0CFFFF,0x0000000C};

/* '5' */
static const unsigned int char53[8] ICACHE_RODATA_ATTR={
8,13,16,1,
0xC0607E7E,0x03C3FEDC,0x7EC3C303,0x0000003C};

/* '6' */
static const unsigned int char54[8] ICACHE_RODATA_ATTR={
8,13,16,1,
0xC0637E3C,0xE3FEDCC0,0x7E63C3C3,0x0000003C};

/* '7' */
static const unsigned int char55[8] ICACHE_RODATA_ATTR={
8,13,16,1,
0x0602FFFF,0x18180C0C,0x30301818,0x00000030};

/* '8' */
static const unsigned int char56[8] ICACHE_RODATA_ATTR={
8,13,16,1,
0xC3C37E3C,0xC37E7EC3,0x7EC3C3C3,0x0000003C};

/* '9' */
static const unsigned int char57[8] ICACHE_RODATA_ATTR={
8,13,16,1,
0xC3C37E3C,0x3B7FC7C3,0x7EC60303,0x0000003C};

/* ':' */
static const unsigned int char58[7] ICACHE_RODATA_ATTR={
8,10,12,4,
0x00001818,0x00000000,0x00001818};

/* ';' */
static const unsigned int char59[7] ICACHE_RODATA_ATTR={
8,12,12,4,
0x00001818,0x00000000,0x10081818};

/* '<' */
static const unsigned int char60[7] ICACHE_RODATA_ATTR={
8,9,12,3,
0x701E0701,0x071E70C0,0x00000001};

/* '=' */
static const unsigned int char61[6] ICACHE_RODATA_ATTR={
8,6,8,4,
0x0000FFFF,0x0000FFFF};

/* '>' */
static const unsigned int char62[7] ICACHE_RODATA_ATTR={
8,9,12,3,
0x0E78E080,0xE0780E03,0x00000080};

/* '?' */
static const unsigned int char63[8] ICACHE_RODATA_ATTR={
8,13,16,1,
0xC3E77E3C,0x180C0603,0x30003030,0x00000030};

/* '@' */
static const unsigned int char64[13] ICACHE_RODATA_ATTR={
16,17,36,1,
0xF80FE003,0xBE311C1C,0xE366F763,0x63CC63CE,0xC6CC43CC,0xFCE7CECE,0x0770F067,0xFC1F0E3C,0x0000F007};

/* 'A' */
static const unsigned int char65[8] ICACHE_RODATA_ATTR={
8,13,16,1,
0x3C3C1818,0x7E66663C,0x8142427E,0x00000081};

/* 'B' */
static const unsigned int char66[8] ICACHE_RODATA_ATTR={
8,13,16,1,
0xC3C3FEFC,0xC3FEFEC3,0xFEC3C3C3,0x000000FC};

/* 'C' */
static const unsigned int char67[11] ICACHE_RODATA_ATTR={
16,13,28,1,
0xF807E003,0x0C0C1C0E,0x00180018,0x00180018,0x0C0C0018,0xF807180E,0x0000E001};

/* 'D' */
static const unsigned int char68[11] ICACHE_RODATA_ATTR={
16,13,28,1,
0xF01FE01F,0x1C183818,0x0C180C18,0x0C180C18,0x1C180C18,0xF01F3818,0x0000E01F};

/* 'E' */
static const unsigned int char69[8] ICACHE_RODATA_ATTR={
8,13,16,1,
0xC0C0FFFF,0xC0FEFEC0,0xFFC0C0C0,0x000000FF};

/* 'F' */
static const unsigned int char70[8] ICACHE_RODATA_ATTR={
8,13,16,1,
0xC0C0FFFF,0xC0FEFEC0,0xC0C0C0C0,0x000000C0};

/* 'G' */
static const unsigned int char71[11] ICACHE_RODATA_ATTR={
16,13,28,1,
0xF80FE003,0x0C18181C,0x00300030,0xFC30FC30,0x0C180C30,0xF80F1C1C,0x0000E003};

/* 'H' */
static const unsigned int char72[11] ICACHE_RODATA_ATTR={
16,13,28,1,
0x0C180C18,0x0C180C18,0xFC1F0C18,0x0C18FC1F,0x0C180C18,0x0C180C18,0x00000C18};

/* 'I' */
static const unsigned int char73[8] ICACHE_RODATA_ATTR={
8,13,16,1,
0x18181818,0x18181818,0x18181818,0x00000018};

/* 'J' */
static const unsigned int char74[8] ICACHE_RODATA_ATTR={
8,13,16,1,
0x03030303,0x03030303,0x7E636303,0x0000003C};

/* 'K' */
static const unsigned int char75[8] ICACHE_RODATA_ATTR={
8,13,16,1,
0x988C8E86,0xD8F0F0B0,0x868E8C8C,0x00000083};

/* 'L' */
static const unsigned int char76[8] ICACHE_RODATA_ATTR={
8,13,16,1,
0xC0C0C0C0,0xC0C0C0C0,0xFFC0C0C0,0x000000FF};

/* 'M' */
static const unsigned int char77[11] ICACHE_RODATA_ATTR={
16,13,28,1,
0x1E3C0E38,0x1E3C1E3C,0x36363636,0x26323636,0x66336633,0xC6316633,0x0000C631};

/* 'N' */
static const unsigned int char78[11] ICACHE_RODATA_ATTR={
16,13,28,1,
0x0C1C0C18,0x0C1E0C1E,0x8C190C1B,0xCC18CC19,0x3C186C18,0x1C183C18,0x00000C18};

/* 'O' */
static const unsigned int char79[11] ICACHE_RODATA_ATTR={
16,13,28,1,
0xF00FE003,0x1818381C,0x0C300C30,0x0C300C30,0x18180C30,0xF00F381C,0x0000C003};

/* 'P' */
static const unsigned int char80[8] ICACHE_RODATA_ATTR={
8,13,16,1,
0xC3C3FEFC,0xFCFEC3C3,0xC0C0C0C0,0x000000C0};

/* 'Q' */
static const unsigned int char81[11] ICACHE_RODATA_ATTR={
16,13,28,1,
0xF00FE007,0x1C18381C,0x0C300C30,0x0C300C30,0xD8180C30,0xF00F701C,0x0000DC07};

/* 'R' */
static const unsigned int char82[11] ICACHE_RODATA_ATTR={
16,13,28,1,
0xF03FE03F,0x18303830,0xF03F3830,0xC030E03F,0x7030E030,0x38303030,0x00001C30};

/* 'S' */
static const unsigned int char83[8] ICACHE_RODATA_ATTR={
8,13,16,1,
0xC3C37E3C,0x0E3C70C0,0x7EC3C303,0x0000003C};

/* 'T' */
static const unsigned int char84[8] ICACHE_RODATA_ATTR={
8,13,16,1,
0x1818FFFF,0x18181818,0x18181818,0x00000018};

/* 'U' */
static const unsigned int char85[11] ICACHE_RODATA_ATTR={
16,13,28,1,
0x0C180C18,0x0C180C18,0x0C180C18,0x0C180C18,0x0C180C18,0xF80F1C1C,0x0000E003};

/* 'V' */
static const unsigned int char86[8] ICACHE_RODATA_ATTR={
8,13,16,1,
0x42428181,0x66666642,0x183C3C3C,0x00000018};

/* 'W' */
static const unsigned int char87[11] ICACHE_RODATA_ATTR={
16,13,28,1,
0x83C183C1,0xC663C663,0xC663C663,0x6C364666,0x6C366C36,0x381C6C36,0x0000381C};

/* 'X' */
static const unsigned int char88[8] ICACHE_RODATA_ATTR={
8,13,16,1,
0x3C3C6666,0x3C181818,0x42667E3C,0x000000C3};

/* 'Y' */
static const unsigned int char89[8] ICACHE_RODATA_ATTR={
8,13,16,1,
0x666642C3,0x18181824,0x18181818,0x00000018};

/* 'Z' */
static const unsigned int char90[8] ICACHE_RODATA_ATTR={
8,13,16,1,
0x04027F7F,0x30180C0C,0xFF402030,0x000000FF};

/* '[' */
static const unsigned int char91[9] ICACHE_RODATA_ATTR={
8,17,20,1,
0x30303C3C,0x30303030,0x30303030,0x3C303030,0x0000003C};

/* '\' */
static const unsigned int char92[8] ICACHE_RODATA_ATTR={
8,13,16,1,
0x30306060,0x18181830,0x060C0C0C,0x00000006};

/* ']' */
static const unsigned int char93[9] ICACHE_RODATA_ATTR={
8,17,20,1,
0x0C0C3C3C,0x0C0C0C0C,0x0C0C0C0C,0x3C0C0C0C,0x0000003C};

/* '^' */
static const unsigned int char94[6] ICACHE_RODATA_ATTR={
8,7,8,1,
0x36141C08,0x00632236};

/* '_' */
static const unsigned int char95[5] ICACHE_RODATA_ATTR={
8,2,4,16,
0x0000FFFF};

/* '`' */
static const unsigned int char96[5] ICACHE_RODATA_ATTR={
8,2,4,1,
0x00000C18};

/* 'a' */
static const unsigned int char97[7] ICACHE_RODATA_ATTR={
8,10,12,4,
0x0FC37F3E,0xC7C3F37F,0x00007BFF};

/* 'b' */
static const unsigned int char98[8] ICACHE_RODATA_ATTR={
8,13,16,1,
0xDCC0C0C0,0xC3C3E7FE,0xFEE7C3C3,0x000000DC};

/* 'c' */
static const unsigned int char99[7] ICACHE_RODATA_ATTR={
8,10,12,4,
0x60733F1E,0x73606060,0x00001E3F};

/* 'd' */
static const unsigned int char100[8] ICACHE_RODATA_ATTR={
8,13,16,1,
0x3B030303,0xC3C3E77F,0x7FE7C3C3,0x0000003B};

/* 'e' */
static const unsigned int char101[7] ICACHE_RODATA_ATTR={
8,10,12,4,
0xC3E67E3C,0xE3C0FFFF,0x00003C7E};

/* 'f' */
static const unsigned int char102[8] ICACHE_RODATA_ATTR={
8,13,16,1,
0x7C303E1E,0x3030307C,0x30303030,0x00000030};

/* 'g' */
static const unsigned int char103[8] ICACHE_RODATA_ATTR={
8,14,16,4,
0xC3E77F3B,0xE7C3C3C3,0xC7033B7F,0x00007CFE};

/* 'h' */
static const unsigned int char104[8] ICACHE_RODATA_ATTR={
8,13,16,1,
0xDEC0C0C0,0xC3C3E3FF,0xC3C3C3C3,0x000000C3};

/* 'i' */
static const unsigned int char105[8] ICACHE_RODATA_ATTR={
8,13,16,1,
0x18001818,0x18181818,0x18181818,0x00000018};

/* 'j' */
static const unsigned int char106[9] ICACHE_RODATA_ATTR={
8,17,20,1,
0x0C000C0C,0x0C0C0C0C,0x0C0C0C0C,0x3C0C0C0C,0x00000038};

/* 'k' */
static const unsigned int char107[8] ICACHE_RODATA_ATTR={
8,13,16,1,
0xC7C0C0C0,0xF8F0DCCE,0xC6CCCCD8,0x000000C7};

/* 'l' */
static const unsigned int char108[8] ICACHE_RODATA_ATTR={
8,13,16,1,
0x18181818,0x18181818,0x18181818,0x00000018};

/* 'm' */
static const unsigned int char109[9] ICACHE_RODATA_ATTR={
16,10,20,4,
0x7C3F3837,0x8C31CC39,0x8C318C31,0x8C318C31,0x8C318C31};

/* 'n' */
static const unsigned int char110[7] ICACHE_RODATA_ATTR={
8,10,12,4,
0xC3E3FFDE,0xC3C3C3C3,0x0000C3C3};

/* 'o' */
static const unsigned int char111[7] ICACHE_RODATA_ATTR={
8,10,12,4,
0xC3E77E3C,0xE7C3C3C3,0x00003C7E};

/* 'p' */
static const unsigned int char112[8] ICACHE_RODATA_ATTR={
8,14,16,4,
0xC3E7FEDC,0xE7C3C3C3,0xC0C0DCFE,0x0000C0C0};

/* 'q' */
static const unsigned int char113[8] ICACHE_RODATA_ATTR={
8,14,16,4,
0xC3E77F3B,0xE7C3C3C3,0x03033B7F,0x00000303};

/* 'r' */
static const unsigned int char114[7] ICACHE_RODATA_ATTR={
8,10,12,4,
0x30383E36,0x30303030,0x00003030};

/* 's' */
static const unsigned int char115[7] ICACHE_RODATA_ATTR={
8,10,12,4,
0x70637F3E,0x63070E3C,0x00003E7F};

/* 't' */
static const unsigned int char116[8] ICACHE_RODATA_ATTR={
8,13,16,1,
0x3C181818,0x1818183C,0x1C181818,0x0000001C};

/* 'u' */
static const unsigned int char117[7] ICACHE_RODATA_ATTR={
8,10,12,4,
0xC3C3C3C3,0xC7C3C3C3,0x00007BFF};

/* 'v' */
static const unsigned int char118[7] ICACHE_RODATA_ATTR={
8,10,12,4,
0x6666C3C3,0x183C3C66,0x00000018};

/* 'w' */
static const unsigned int char119[9] ICACHE_RODATA_ATTR={
16,10,20,4,
0xC6318630,0x4C19C631,0x6C1B6C1B,0x380E280A,0x3006380E};

/* 'x' */
static const unsigned int char120[7] ICACHE_RODATA_ATTR={
8,10,12,4,
0x3C6666C3,0x663C1818,0x0000C366};

/* 'y' */
static const unsigned int char121[8] ICACHE_RODATA_ATTR={
8,14,16,4,
0x6666C3C3,0x1C1C3C36,0x10080808,0x00007070};

/* 'z' */
static const unsigned int char122[7] ICACHE_RODATA_ATTR={
8,10,12,4,
0x0E07FFFF,0xE070381C,0x0000FFFF};

/* '{' */
static const unsigned int char123[9] ICACHE_RODATA_ATTR={
8,17,20,1,
0x18181E0E,0x70381818,0x18183870,0x1E181818,0x0000000E};

/* '|' */
static const unsigned int char124[9] ICACHE_RODATA_ATTR={
8,17,20,1,
0x18181818,0x18181818,0x18181818,0x18181818,0x00000018};

/* '}' */
static const unsigned int char125[9] ICACHE_RODATA_ATTR={
8,17,20,1,
0x18187870,0x0E1C1818,0x18181C0E,0x78181818,0x00000070};

/* '~' */
static const unsigned int char126[5] ICACHE_RODATA_ATTR={
8,4,4,6,
0x069FF960};


const unsigned int *arial21[97] ={
(unsigned int*)32,(unsigned int*)126,
char32,
char33,
char34,
char35,
char36,
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
