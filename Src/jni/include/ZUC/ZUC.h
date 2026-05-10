#ifndef ZUC_H
#define ZUC_H

class ZUC
{
private:

	void Init(unsigned char* _Key, unsigned char* _IV);

public:
	static unsigned int* GenerateKeyArray(unsigned char* _Key, unsigned char* _IV,
		unsigned int* _RetKeyArray, unsigned int KeyArrayCount);
};


#endif
