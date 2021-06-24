#include <cstdio>
#include <cstring>
#include <cmath>
#include "Rasterizer.h"


Edge::Edge(const Color& color1, int x1, int y1,
	const Color& color2, int x2, int y2)
{
	if (y1 < y2) {
		Color1 = color1;
		X1 = x1;
		Y1 = y1;
		Color2 = color2;
		X2 = x2;
		Y2 = y2;
	}
	else {
		Color1 = color2;
		X1 = x2;
		Y1 = y2;
		Color2 = color1;
		X2 = x1;
		Y2 = y1;
	}
}

Span::Span(const Color& color1, int x1, const Color& color2, int x2)
{
	if (x1 < x2) {
		Color1 = color1;
		X1 = x1;
		Color2 = color2;
		X2 = x2;
	}
	else {
		Color1 = color2;
		X1 = x2;
		Color2 = color1;
		X2 = x1;
	}
}

uint16_t Rasterizer::vram_read(int x, int y) {
	return vram[y * 1024 + x];
}
uint16_t Rasterizer::fetch_texel(int x, int y, point clut, point page, Depth depth) {
	switch (depth) {
	case Depth::BITS4: {
		uint16_t texel = vram_read(page.x + x / 4, page.y + y);
		int index = (texel >> (x % 4) * 4) & 0xF;
		return vram_read(clut.x + index, clut.y);
	}
	case Depth::BITS8: {
		uint16_t texel = vram_read(x / 2 + page.x, y + page.y);
		int index = (texel >> (x % 2) * 8) & 0xFF;
		return vram_read(clut.x + index, clut.y);
	}
	case Depth::BITS16: {
		return vram_read(x + page.x, y + page.y);
	}
	}
}
void
Rasterizer::SetFrameBuffer(uint32_t* frameBuffer,
	unsigned int width, unsigned int height)
{
	m_FrameBuffer = frameBuffer;
	m_Width = width;
	m_Height = height;
}

void
Rasterizer::SetPixel(unsigned int x, unsigned int y, const Color& color)
{
	if (x >= m_Width || y >= m_Height)
		return;

	//if (textured) {
	//	m_FrameBuffer[y * m_Width + x] = fetch_texel(xpos, ypos, clut, page, Depth::BITS16);	// Assume 16bit colour depth
	//	xpos++;
	//	
	//}
	//else 
	m_FrameBuffer[y * m_Width + x] = color.ToUInt32();
	vram[y * 1024 + x] = uint16_t(color.ToUInt32());

}

void
Rasterizer::SetPixel(int x, int y, const Color& color)
{
	SetPixel((unsigned int)x, (unsigned int)y, color);
}

void
Rasterizer::SetPixel(float x, float y, const Color& color)
{
	if (x < 0.0f || y < 0.0f)
		return;

	SetPixel((unsigned int)x, (unsigned int)y, color);
}

void
Rasterizer::Clear()
{
	memset(m_FrameBuffer, 0, sizeof(uint32_t) * m_Height * m_Width);
}

void
Rasterizer::DrawSpan(const Span& span, int y)
{
	int xdiff = span.X2 - span.X1;
	if (xdiff == 0)
		return;

	Color colordiff = span.Color2 - span.Color1;

	float factor = 0.0f;
	float factorStep = 1.0f / (float)xdiff;

	// draw each pixel in the span
	for (int x = span.X1; x < span.X2; x++) {
		SetPixel(x, y, span.Color1 + (colordiff * factor));
		factor += factorStep;
	}
}

void
Rasterizer::DrawSpansBetweenEdges(const Edge& e1, const Edge& e2)
{
	// calculate difference between the y coordinates
	// of the first edge and return if 0
	float e1ydiff = (float)(e1.Y2 - e1.Y1);
	if (e1ydiff == 0.0f)
		return;

	// calculate difference between the y coordinates
	// of the second edge and return if 0
	float e2ydiff = (float)(e2.Y2 - e2.Y1);
	if (e2ydiff == 0.0f)
		return;

	// calculate differences between the x coordinates
	// and colors of the points of the edges
	float e1xdiff = (float)(e1.X2 - e1.X1);
	float e2xdiff = (float)(e2.X2 - e2.X1);
	Color e1colordiff = (e1.Color2 - e1.Color1);
	Color e2colordiff = (e2.Color2 - e2.Color1);

	// calculate factors to use for interpolation
	// with the edges and the step values to increase
	// them by after drawing each span
	float factor1 = (float)(e2.Y1 - e1.Y1) / e1ydiff;
	float factorStep1 = 1.0f / e1ydiff;
	float factor2 = 0.0f;
	float factorStep2 = 1.0f / e2ydiff;

	// loop through the lines between the edges and draw spans
	for (int y = e2.Y1; y < e2.Y2; y++) {
		// create and draw span
		Span span(e1.Color1 + (e1colordiff * factor1),
			e1.X1 + (int)(e1xdiff * factor1),
			e2.Color1 + (e2colordiff * factor2),
			e2.X1 + (int)(e2xdiff * factor2));
		DrawSpan(span, y);

		// increase factors
		factor1 += factorStep1;
		factor2 += factorStep2;
	}
}

void
Rasterizer::DrawTriangle(const Color& color1, float x1, float y1,
	const Color& color2, float x2, float y2,
	const Color& color3, float x3, float y3)
{
	// create edges for the triangle
	Edge edges[3] = {
		Edge(color1, (int)x1, (int)y1, color2, (int)x2, (int)y2),
		Edge(color2, (int)x2, (int)y2, color3, (int)x3, (int)y3),
		Edge(color3, (int)x3, (int)y3, color1, (int)x1, (int)y1)
	};

	int maxLength = 0;
	int longEdge = 0;

	// find edge with the greatest length in the y axis
	for (int i = 0; i < 3; i++) {
		int length = edges[i].Y2 - edges[i].Y1;
		if (length > maxLength) {
			maxLength = length;
			longEdge = i;
		}
	}

	int shortEdge1 = (longEdge + 1) % 3;
	int shortEdge2 = (longEdge + 2) % 3;

	// draw spans between edges; the long edge can be drawn
	// with the shorter edges to draw the full triangle
	DrawSpansBetweenEdges(edges[longEdge], edges[shortEdge1]);
	DrawSpansBetweenEdges(edges[longEdge], edges[shortEdge2]);
}

void
Rasterizer::DrawLine(const Color& color1, float x1, float y1,
	const Color& color2, float x2, float y2)
{
	float xdiff = (x2 - x1);
	float ydiff = (y2 - y1);

	if (xdiff == 0.0f && ydiff == 0.0f) {
		SetPixel(x1, y1, color1);
		return;
	}

	if (fabs(xdiff) > fabs(ydiff)) {
		float xmin, xmax;

		// set xmin to the lower x value given
		// and xmax to the higher value
		if (x1 < x2) {
			xmin = x1;
			xmax = x2;
		}
		else {
			xmin = x2;
			xmax = x1;
		}

		// draw line in terms of y slope
		float slope = ydiff / xdiff;
		for (float x = xmin; x <= xmax; x += 1.0f) {
			float y = y1 + ((x - x1) * slope);
			Color color = color1 + ((color2 - color1) * ((x - x1) / xdiff));
			SetPixel(x, y, color);
		}
	}
	else {
		float ymin, ymax;

		// set ymin to the lower y value given
		// and ymax to the higher value
		if (y1 < y2) {
			ymin = y1;
			ymax = y2;
		}
		else {
			ymin = y2;
			ymax = y1;
		}

		// draw line in terms of x slope
		float slope = xdiff / ydiff;
		for (float y = ymin; y <= ymax; y += 1.0f) {
			float x = x1 + ((y - y1) * slope);
			Color color = color1 + ((color2 - color1) * ((y - y1) / ydiff));
			SetPixel(x, y, color);
		}
	}
}