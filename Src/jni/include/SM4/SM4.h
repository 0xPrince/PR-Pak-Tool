#pragma once

struct SM4 
{
    static const int Block_Size = 16;
    static void Decrypt(unsigned char* Content, unsigned int ContentLength, unsigned char* Output, unsigned char* _key);
};
