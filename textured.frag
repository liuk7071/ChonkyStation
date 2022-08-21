
		#version 430 core
		out vec4 FragColor;
		in vec3 ourColor;
		in vec2 TexCoord;
		flat in vec2 texpageCoords;
		flat in vec2 clut;

		uniform sampler2D vram;
		uniform int colourDepth;
		uniform ivec4 texWindow;
		int floatToU5(float f) {
			return int(floor(f * 31.0 + 0.5));
		}
		int sample16(ivec2 coords) {
			vec4 colour = texelFetch(vram, coords, 0);
			int r = floatToU5(colour.r);
			int g = floatToU5(colour.g);
			int b = floatToU5(colour.b);
			int msb = int(ceil(colour.a)) << 15;
			return r | (g << 5) | (b << 10) | msb;
		}
		vec4 fetchTexel4Bit(ivec2 coords) {
			int texel = sample16(ivec2(coords.x / 4, coords.y) + ivec2(texpageCoords));
			int idx = (texel >> ((coords.x % 4) * 4)) & 0xf;
			return texelFetch(vram, ivec2(clut.x + idx, clut.y), 0);
		}
		vec4 fetchTexel8Bit(ivec2 coords) {
			int texel = sample16(ivec2(coords.x / 2, coords.y) + ivec2(texpageCoords));
			int idx = (texel >> ((coords.x % 2) * 8)) & 0xff;
			return texelFetch(vram, ivec2(clut.x + idx, clut.y), 0);
		}
		void main()
		{
			ivec2 TexCoord_ = ivec2(floor(TexCoord)) & ivec2(0xff);
			ivec2 UV = (ivec2(TexCoord_) & texWindow.xy) | texWindow.zw;
			vec4 colour;
			if(colourDepth == 0) {
				colour = fetchTexel4Bit(UV);
			}
			else if (colourDepth == 1) {
				colour = fetchTexel8Bit(UV);
			} 
			else if (colourDepth == 2) {
				vec2 TexCoords = vec2(float(UV.x + texpageCoords.x) / 1024.f - 1, -(1 - float(UV.y + texpageCoords.y) / 512.f));
				colour = texture(vram, TexCoords);
			} else colour = vec4(1.f, 0.f, 0.f, 1.f);
			if(colour.rgb == vec3(0.f, 0.f, 0.f)) discard;
			colour = (colour * vec4(ourColor.rgb, 1.f)) / (128.f);
			colour.a = 1.f;
			FragColor = colour;
		}
		