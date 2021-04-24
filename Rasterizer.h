#ifndef __RASTERIZER_H__
#define __RASTERIZER_H__

#include "Color.h"

class Edge
{
public:
	Color Color1, Color2;
	int X1, Y1, X2, Y2;

	Edge(const Color& color1, int x1, int y1, const Color& color2, int x2, int y2);
};

class Span
{
public:
	Color Color1, Color2;
	int X1, X2;

	Span(const Color& color1, int x1, const Color& color2, int x2);
};

class Rasterizer
{
protected:
	uint32_t* m_FrameBuffer;
	unsigned int m_Width, m_Height;

	void DrawSpan(const Span& span, int y);
	void DrawSpansBetweenEdges(const Edge& e1, const Edge& e2);

public:
	void SetFrameBuffer(uint32_t* frameBuffer, unsigned int width, unsigned int height);
	void SetPixel(unsigned int x, unsigned int y, const Color& color = Color());
	void SetPixel(int x, int y, const Color& color = Color());
	void SetPixel(float x, float y, const Color& color = Color());
	void Clear();

	void DrawTriangle(const Color& color1, float x1, float y1, const Color& color2, float x2, float y2, const Color& color3, float x3, float y3);

	void DrawLine(const Color& color1, float x1, float y1, const Color& color2, float x2, float y2);
};

#endif /* __RASTERIZER_H__ */