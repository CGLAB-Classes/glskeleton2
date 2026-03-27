#include <nanogui/nanogui.h>
#include <nanogui/opengl.h>

#include <glskeleton/shader.h>
#include <glskeleton/utils.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <memory>

class GLSkeletonApp : public nanogui::Screen {
public:
  /// @param width  Requested window width in logical units.
  /// @param height Requested window height in logical units.
  ///
  /// On HiDPI displays, the actual window and framebuffer size may be
  /// larger than these values. The rendering resolution is determined
  /// by nanogui and the system's display settings.
  GLSkeletonApp(int width = 800, int height = 600)
      : nanogui::Screen(nanogui::Vector2i(width, height), "glskeleton",
                        /* resizable */ true,
                        /* maximized */ false,
                        /* fullscreen */ false,
                        /* depth_buffer */ true,
                        /* stencil_buffer */ true,
                        /* float_buffer */ false,
                        /* gl_major */ 3,
                        /* gl_minor */ 3) {
    m_shader = std::make_unique<Shader>(glskeleton::getResourceDir() / "shaders" / "basic.vert",
                                        glskeleton::getResourceDir() / "shaders" / "basic.frag");

    set_background(nanogui::Color(0.2f, 0.3f, 0.3f, 1.0f));
  }

  ~GLSkeletonApp() override { cleanup_geometry(); }

  // --- Geometry lifecycle --------------------------------------------------
  // These three methods illustrate the OpenGL object management pattern:
  //   setup   (generate ID -> bind -> upload data -> configure -> unbind)
  //   draw    (bind -> draw call -> unbind)
  //   cleanup (delete IDs)
  //

  /// Create VAO/VBO and upload vertex data to the GPU
  ///
  /// @param vertices    Interleaved vertex data (position + color per vertex)
  /// @param dataSize    Total size of the vertex data in bytes
  /// @param vertexCount Number of vertices
  void setup_geometry(const float *vertices, size_t dataSize, GLsizei vertexCount) {
    // Clean up previous geometry if any
    cleanup_geometry();

    m_vertex_count = vertexCount;

    // 1. Generate object IDs
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    // 2. Bind VAO (records all subsequent VBO/attribute state)
    glBindVertexArray(m_vao);

    // 3. Bind VBO and upload vertex data
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, dataSize, vertices, GL_STATIC_DRAW);

    // 4. Configure vertex attribute pointers
    //    Stride = 6 floats per vertex (3 position + 3 color)
    GLsizei stride = 6 * sizeof(float);

    //    Position attribute (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void *)0);
    glEnableVertexAttribArray(0);

    //    Color attribute (location = 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // 5. Unbind VAO (VBO state is now captured inside the VAO)
    glBindVertexArray(0);
  }

  /// Bind the VAO and issue a draw call
  void draw_geometry() {
    if (m_vao == 0)
      return;
    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, m_vertex_count);
    glBindVertexArray(0);
  }

  /// Release GPU resources
  void cleanup_geometry() {
    if (m_vbo != 0) {
      glDeleteBuffers(1, &m_vbo);
      m_vbo = 0;
    }
    if (m_vao != 0) {
      glDeleteVertexArrays(1, &m_vao);
      m_vao = 0;
    }
    m_vertex_count = 0;
  }

  // --- MVP matrices (public for external modification) ---------------------
  // Modify these to transform objects, move the camera, or change projection.
  // They are sent to the shader every frame in draw_contents().

  /// Transform object from local space to world space
  glm::mat4 model = glm::mat4(1.0f);

  /// Position and orient the camera in world space
  glm::mat4 view = glm::mat4(1.0f);

  /// Perspective projection (updated every frame based on framebuffer size)
  glm::mat4 projection = glm::mat4(1.0f);

  void draw_contents() override {
    float aspect = (float)framebuffer_size().x() / (float)framebuffer_size().y();
    projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);

    clear();

    m_shader->use();
    m_shader->setMat4("model", model);
    m_shader->setMat4("view", view);
    m_shader->setMat4("projection", projection);

    draw_geometry();
  }

  /// Handle keyboard events (ESC to close)
  bool keyboard_event(int key, int scancode, int action, int modifiers) override {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
      set_visible(false);
      return true;
    }
    return nanogui::Screen::keyboard_event(key, scancode, action, modifiers);
  }

private:
  std::unique_ptr<Shader> m_shader;
  GLuint m_vao = 0;
  GLuint m_vbo = 0;
  GLsizei m_vertex_count = 0;
};

int main(int /* argc */, char * /* argv */[]) {
  nanogui::init();

  // App scope: ensures GL resources are released before nanogui::shutdown()
  {
    nanogui::ref<GLSkeletonApp> app = new GLSkeletonApp();

    // --- Define vertex data (position + color) ---------------------------
    //
    //   Each vertex has 6 floats: (x, y, z, r, g, b)
    //
    //        (0.0, 0.5)       blue
    //           *
    //          * *
    //         *   *
    //        *_____*
    // (-0.5,-0.5)  (0.5,-0.5)
    //    red           green
    //
    float vertices[] = {
        // positions         // colors
        -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left  (red)
        0.5f,  -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, // bottom-right (green)
        0.0f,  0.5f,  0.0f, 0.0f, 0.0f, 1.0f, // top-center   (blue)
    };
    app->setup_geometry(vertices, sizeof(vertices), 3);

    // --- Set up MVP matrices ---------------------------------------------
    app->model = glm::mat4(1.0f);
    app->view = glm::lookAt(glm::vec3(0.0f, 0.0f, 2.0f), // camera position
                            glm::vec3(0.0f, 0.0f, 0.0f), // look-at target
                            glm::vec3(0.0f, 1.0f, 0.0f)  // up vector
    );
    // projection is updated every frame in draw_contents()

    app->set_visible(true);
    app->draw_all();
    nanogui::run();
  } // ~GLSkeletonApp: GL resources (VAO, VBO, shader) freed while context is alive

  nanogui::shutdown();
  return 0;
}
