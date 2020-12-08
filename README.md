This repository and the following document introduces OpenGL during the M2 Gamagora training.


### Requirements

Before attending this course, you must:

- Remember basic linear algebra: vector, matrices operations.
- Build the following project and run it on the configuration of your choice. A
  nix file is provided with the required environment and a Cmake file does the
  compilation. You are allowed to build that on something different than Linux.


# First session

- We experiment with https://www.shadertoy.com/
- Session about homogeneous coordinates

# Second session

- OpenGL shader
- Buffer creation
- VAO creation
- Binding and drawing
- Shader `in` and `uniform`

# Second session

- Digresssion about game loop
- OpenGL debugger

## Textures

- create a texture:

```cpp
GLuint tex;
glCreateTextures(GL_TEXTURE_2D, 1, &tex);
glTextureStorage2D(tex, 1, GL_RGB8, im.width, im.height);
```

- Note: watch for `glGenTextures`, this is the old API and comes with many drawback. Use a debugger (such as `apitrace` and the debug output)
- Note: discussion on texture level, sampling, `glGenerateMipmaps`.

- fill it and bind it.

```cpp
glTextureSubImage2D(tex, 0, 0, 0, im.width, im.height, GL_RGB, GL_UNSIGNED_BYTE, im.data.data());
glBindTextureUnit(texUnit, tex);
```

Note: discussion on texture units

- Use it in a shader:


```glsl
layout(binding=texUnit) uniform sampler2D tex;
```

or

```cpp
auto id = glGetUniformLocation(prog, "tex");
glUniform1i(id, texUnit);
```


```glsl
texelFetch(tex, ivec2(gl_FragCoord.xy), 0)

// or

texture(tex, uv);
```

For the next session, have fun with textures. Add many of them. Draw quads, with texture on it. Try to load advanced meshes (using tinyply, or anything else) and put texture on them using the UV.


## Framebuffer

Next. We'll see framebuffer indirect rendering and clipping.


```cpp
	// FB
	GLuint fbo;
	glCreateFramebuffers(1, &fbo);
```

```cpp
	GLuint ct;
	glCreateTextures(GL_TEXTURE_2D, 1, &ct);
	glTextureStorage2D(ct, 1, GL_RGB8, im.width, im.height);

	GLuint dt;
	glCreateTextures(GL_TEXTURE_2D, 1, &dt);
	glTextureStorage2D(dt, 1, GL_DEPTH_COMPONENT32F, im.width, im.height);

	glNamedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, ct, 0);
	glNamedFramebufferTexture(fbo, GL_DEPTH_ATTACHMENT, dt, 0);
```

```cpp
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
```

This allows the creation of an indirect framebuffer. You will be able to render on a texture.

Carefully check your OpenGL errors.

Applications:

- Shadow map
- Portal


## Clipping


- Mirrors

# Future sessions

Many things:

- Tesselation shader
- Geometry shader
- Computer shader
- Instance rendering

# Ranking

At the end of this course, you must be able to provide a small interactive environment (i.e. game) which showcase the different elements you've learned during this course.
