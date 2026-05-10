#include "ZUC.h"
#include "Data.h"

// state registers of LFSR
unsigned int LFSR_SX[16];

/*F registers*/
unsigned int F_R1;
unsigned int F_R2;

//outputs of BitReorganization
unsigned int BRC_XX[4];

#define MulByPow2(x, k) ((((x) << k) | ((x) >> (31 - k))) & 0x7FFFFFFF)
#define ROT(a, k) (((a) << k) | ((a) >> (32 - k)))
#define ArrCount(arr) sizeof(arr)/sizeof(arr[0])

#define MAKEU32(a, b, c ,d) \
    (((unsigned int)(a) << 24) \
    | ((unsigned int)(b) << 16) \
    | ((unsigned int)(c) << 8) \
    | ((unsigned int)(d)))

#define MAKEU31(a, b, c)  \
    (((unsigned int)(a) << 23) \
    | ((unsigned int)(b) << 8) \
    | ((unsigned int)(c) << 0))

unsigned int AddM(unsigned int a, unsigned int b) {
	unsigned int c = a + b;
	return (c & 0x7FFFFFFF) + (c >> 31);
}


void LFSRWithInitializationMode(unsigned int u) {
    unsigned int f, v;

    f = LFSR_SX[0];
    v = MulByPow2(LFSR_SX[0], 8);
    f = AddM(f, v);

    v = MulByPow2(LFSR_SX[4], 20);
    f = AddM(f, v);

    v = MulByPow2(LFSR_SX[10], 21);
    f = AddM(f, v);

    v = MulByPow2(LFSR_SX[13], 17);
    f = AddM(f, v);

    v = MulByPow2(LFSR_SX[15], 15);
    f = AddM(f, v);

    f = AddM(f, u);

    /* update the state */
    for(int i = 0; i < ArrCount(LFSR_SX) - 1; ++i) {
        LFSR_SX[i] = LFSR_SX[i + 1];
    }
    LFSR_SX[ArrCount(LFSR_SX) - 1] = f;
}

/* LFSR with work mode */
void LFSRWithWorkMode() {
    unsigned int f, v;

    f = LFSR_SX[0];
    v = MulByPow2(LFSR_SX[0], 8);
    f = AddM(f, v);

    v = MulByPow2(LFSR_SX[4], 20);
    f = AddM(f, v);

    v = MulByPow2(LFSR_SX[10], 21);
    f = AddM(f, v);

    v = MulByPow2(LFSR_SX[13], 17);
    f = AddM(f, v);

    v = MulByPow2(LFSR_SX[15], 15);
    f = AddM(f, v);

    /* update state */
    for(int i = 0; i < ArrCount(LFSR_SX) - 1; ++i) {
        LFSR_SX[i] = LFSR_SX[i + 1];
    }
    LFSR_SX[ArrCount(LFSR_SX) -1] = f;
}

/* BitReorganization */ 
void BitReorganization() {
    BRC_XX[0] = ((LFSR_SX[15] & 0x7FFF8000) << 1) | (LFSR_SX[14] & 0xFFFF);
    BRC_XX[1] = ((LFSR_SX[11] & 0xFFFF) << 16) | (LFSR_SX[9] >> 15);
    BRC_XX[2] = ((LFSR_SX[7] & 0xFFFF) << 16) | (LFSR_SX[5] >> 15);
    BRC_XX[3] = ((LFSR_SX[2] & 0xFFFF) << 16) | (LFSR_SX[0] >> 15);
}


/* linear transformation L1 */
unsigned int L1(unsigned int X) {
    return (X ^ ROT(X, 2) ^ ROT(X, 10) ^ ROT(X, 18) ^ ROT(X, 24));
}

/* linear transformation L2 */
unsigned int L2(unsigned int X) 
{
    return (X ^ ROT(X, 8) ^ ROT(X, 14) ^ ROT(X, 22) ^ ROT(X, 30));
}

/* non-linear function F */
unsigned int F(void) 
{
    unsigned int W, W1, W2, u, v;

    W = (BRC_XX[0] ^ F_R1) + F_R2;
    W1 = F_R1 + BRC_XX[1];
    W2 = F_R2 ^ BRC_XX[2];
    u = L1((W1 << 16) | (W2 >> 16));
    v = L2((W2 << 16) | (W1 >> 16));
    F_R1 = MAKEU32(S0[u >> 24], S1[(u >> 16) & 0xFF],
        S0[(u >> 8) & 0xFF], S1[u & 0xFF]);
    F_R2 = MAKEU32(S0[v >> 24], S1[(v >> 16) & 0xFF],
        S0[(v >> 8) & 0xFF], S1[v & 0xFF]);

    return W;
}

void ExpandKey(unsigned char* _Key, unsigned char* _IV)
{
    for (int i = 0; i < ArrCount(LFSR_SX); ++i) {
        LFSR_SX[i] = MAKEU31(_Key[i], EK_d[i], _IV[i]);
    }
}
void ZUC::Init(unsigned char *_Key, unsigned char *_IV)
{
    F_R1 = F_R2 = 0;

    ExpandKey(_Key, _IV);

    unsigned int w, nCount = 32;
    while (nCount > 0)
    {
        BitReorganization();
        w = F();
        LFSRWithInitializationMode(w >> 1);
        nCount--;
    }
}

unsigned int* ZUC::GenerateKeyArray(unsigned char* _Key, unsigned char* _IV,
    unsigned int* _RetKeyArray, unsigned int KeyArrayCount)
{
    if (!_Key || !_IV || !_RetKeyArray || KeyArrayCount <= 0) return 0;
    ZUC _zuc;
    _zuc.Init(_Key, _IV);

    BitReorganization();
    F();
    LFSRWithWorkMode();

    for (unsigned int i = 0; i < KeyArrayCount; ++i) {
        BitReorganization();
        _RetKeyArray[i] = F() ^ BRC_XX[3];
        LFSRWithWorkMode();
    }

    F_R1 = F_R2 = 0;
    for (int i = 0; i < ArrCount(LFSR_SX); ++i) LFSR_SX[i] = 0;
    for (int i = 0; i < ArrCount(BRC_XX); ++i) BRC_XX[i] = 0;
    return _RetKeyArray;
}
