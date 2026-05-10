#pragma once


struct RSA_Data
{
	uint8_t Exponent[0x4];
	uint8_t Modulus[0x100];
};