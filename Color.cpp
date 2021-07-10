#include "Color.h"

Color::Color(float r, float g, float b, float a)
{
	R = r;
	G = g;
	B = b;
	A = a;
}

uint32_t
Color::ToUInt32() const
{
	uint32_t r = (((uint32_t)(R) * 31) + 127) / 255;
	uint32_t g = (((uint32_t)(G) * 31) + 127) / 255;
	uint32_t b = (((uint32_t)(B) * 31) + 127) / 255;
	uint32_t a = ((uint32_t)(A) + 127) / 255;

	return (a << 15) | (b << 10) | (g << 5) | r;
	//return (b << 15) | (g << 10) | (r << 5) | a;
	//return (((b << 10) | (g << 5) | r) << 1) | a;
}
uint32_t
Color::ToRGB32() const {
	uint32_t r = (((uint32_t)(R) * 31) + 127) / 255;
	uint32_t g = (((uint32_t)(G) * 31) + 127) / 255;
	uint32_t b = (((uint32_t)(B) * 31) + 127) / 255;
	uint32_t a = ((uint32_t)(A)+127) / 255;
	return ((r << 3) << 24) | ((g << 3) << 16) | ((b << 3) << 8) | 0xff;
}

Color
Color::operator + (const Color& c) const
{
	return Color(R + c.R, G + c.G, B + c.B, A + c.A);
}

Color
Color::operator - (const Color& c) const
{
	return Color(R - c.R, G - c.G, B - c.B, A - c.A);
}

Color
Color::operator * (float f) const
{
	return Color(R * f, G * f, B * f, A * f);
}