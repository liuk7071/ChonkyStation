
		#version 330 core
		layout (location = 0) in vec3 aPos;
		layout (location = 1) in vec3 aColor;
		layout (location = 2) in vec2 aTexCoord;
		layout (location = 3) in vec2 aTexpageCoords;
		layout (location = 4) in vec2 aClut;

		out vec3 ourColor;
		out vec2 TexCoord;
		flat out vec2 texpageCoords;
		flat out vec2 clut;
		uniform int colourDepth;

		void main()
		{
			gl_Position = vec4(float(aPos.x) / 512 - 1, -(1 - float(aPos.y) / 256), 0.0, 1.0);
			ourColor = aColor;
			TexCoord = aTexCoord;
			texpageCoords = aTexpageCoords;
			clut = aClut;
		}
		