#pragma once

#include "glmath.hh"
#include "glshader.hh"
#include "glutil.hh"
#include <map>
#include <functional>
#include <SDL_events.h>

float screenW();
float screenH();
const float targetWidth = 1366.0f; // One of the most common desktop resolutions in use today.
static inline float virtH() { return float(screenH()) / screenW(); }

struct SDL_Surface;
struct SDL_Window;
class FBO;
class Window;

struct ColorTrans {
	ColorTrans(Window&, Color const& c);
	ColorTrans(Window&, glmath::mat4 const& mat);
	~ColorTrans();

  private:
	Window& m_window;
	glmath::mat4 m_old;
};

struct LyricColorTrans {
	LyricColorTrans(Window&, Color const& fill, Color const& stroke, Color const& newFill, Color const& newStroke);
	~LyricColorTrans();

  private:
	Window& m_window;
	glmath::vec4 oldFill;
	glmath::vec4 oldStroke;
};

class ViewTrans {
  public:
	/// Apply a translation on top of current viewport translation
	ViewTrans(Window&, glmath::mat4 const& m);
	/// Apply a subviewport with different perspective projection
	ViewTrans(Window&, float offsetX = 0.0f, float offsetY = 0.0f, float frac = 1.0f);
	~ViewTrans();

 private:
	Window& m_window;
	glmath::mat4 m_old;
};

/// Apply a transform to current modelview stack
class Transform {
  public:
	Transform(Window&, glmath::mat4 const& m);
	~Transform();

  private:
	Window& m_window;
	glmath::mat4 m_old;
};

/// Performs a GL transform for displaying background image at far distance
glmath::mat4 farTransform();
class Game;

/// handles the window
class Window {
public:
	Window();
	~Window();
	void render(Game &game, std::function<void (void)> drawFunc);
	/// clears window
	void blank();
	/// Initialize VAO and VBO.
	void initBuffers();
	/// swaps buffers
	void swap();
	/// Handle window events
	void event(Uint8 const& eventID, Sint32 const& data1, Sint32 const& data2);
	/// Resize window (contents) / toggle full screen according to config. Returns true if resized.
	void resize();
	/// take a screenshot
	void screenshot();

	/// Return reference to Uniform Buffer Object.
	static GLuint const& UBO() { return Window::m_ubo; }
	/// Return reference to Vertex Array Object.
	GLuint const& VAO() const { return Window::m_vao; }
	/// Return reference to Vertex Buffer Object.
	GLuint const& VBO() const { return Window::m_vbo; }
	/// Store value of GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT.
	static GLint bufferOffsetAlignment;

	/// Construct a new shader or return an existing one by name
	Shader& shader(std::string const& name) {
		ShaderMap::iterator it = m_shaders.find(name);
		if (it != m_shaders.end()) return *it->second;
		std::pair<std::string, std::unique_ptr<Shader>> kv = std::make_pair(name, std::make_unique<Shader>(name));
		return *m_shaders.insert(std::move(kv)).first->second;
	}
	/// Compiles and links all shaders.
	void createShaders();
	void resetShaders() { m_shaders.clear(); createShaders(); };
	void updateColor();
	void updateLyricHighlight(glmath::vec4 const& fill, glmath::vec4 const& stroke, glmath::vec4 const& newFill, glmath::vec4 const& newStroke);
	void updateLyricHighlight(glmath::vec4 const& fill, glmath::vec4 const& stroke);
	void updateTransforms();
private:
	const GLuint vertPos = 0;
	const GLuint vertTexCoord = 1;
	const GLuint vertNormal = 2;
	const GLuint vertColor = 3;
	void setWindowPosition(const Sint32& x, const Sint32& y);
	void setFullscreen();
	/// Setup everything for drawing a view.
	/// @param num 0 = no stereo, 1 = left eye, 2 = right eye
	void view(unsigned num);
	void updateStereo(float separation);
	bool m_fullscreen = false;
	bool m_needResize = true;
	static GLuint m_ubo;
	static GLuint m_vao;
	static GLuint m_vbo;
	glutil::stereo3dParams m_stereoUniforms;
	glutil::shaderMatrices m_matrixUniforms;
	glutil::lyricColorUniforms m_lyricColorUniforms;
	std::unique_ptr<FBO> m_fbo;
	int m_windowX = 0;
	int m_windowY = 0;
	std::unique_ptr<SDL_Window, void (*)(SDL_Window*)> screen;
	std::unique_ptr<std::remove_pointer_t<SDL_GLContext> /* SDL_GLContext is a void* */, void (*)(SDL_GLContext)> glContext;
	using ShaderMap = std::map<std::string, std::unique_ptr<Shader>>;
	// Careful, Shaders depends on SDL_Window, thus m_shaders need to be
	// destroyed before screen (and thus be creater after)
	ShaderMap m_shaders; ///< Shader programs by name
	public:
	FBO& getFBO();
};

