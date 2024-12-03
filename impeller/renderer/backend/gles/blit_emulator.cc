#include "flutter/impeller/renderer/backend/gles/blit_emulator.h"

#include "fml/closure.h"
#include "impeller/base/validation.h"
#include "impeller/renderer/backend/gles/proc_table_gles.h"

namespace impeller {

static const GLchar* kBlitVertexShaderCode =
    "attribute vec4 a_Position;\n"
    "varying highp vec2 v_TexCoordinate;\n"
    "void main() {\n"
    "  v_TexCoordinate = a_Position.xy * 0.5 + 0.5;\n"
    "  gl_Position = a_Position;\n"
    "}\n";

static const GLchar* kBlitFragmentShaderCode =
    "uniform sampler2D u_Texture;\n"
    "varying highp vec2 v_TexCoordinate;\n"
    "void main() {\n"
    "  gl_FragColor = texture2D(u_Texture, v_TexCoordinate);\n"
    "}\n";

BlitEmulator::BlitEmulator(ProcTableGLES& gl) : gl_(gl) {}

BlitEmulator::~BlitEmulator() {
  if (emulated_blit_program_.has_value()) {
    gl_.DeleteProgram(*emulated_blit_program_);
  }
}

bool BlitEmulator::Blit(const std::shared_ptr<Texture>& source,
                        const std::shared_ptr<Texture>& destination,
                        IRect source_region,
                        IRect destination_region,
                        impeller::MinMagFilter filter) {
  auto& gl = gl_;

  if (!gl.BlitFramebuffer.IsAvailable()) {
    if (!UseBlitProgram()) {
      return false;
    }

    gl.Disable(GL_SCISSOR_TEST);
    gl.Disable(GL_DEPTH_TEST);
    gl.Disable(GL_STENCIL_TEST);

    

    float vertices[] = {0.5f,  0.5f,  0.0f,  //
                        0.5f,  -0.5f, 0.0f,  //
                        -0.5f, -0.5f, 0.0f,  //
                        -0.5f, 0.5f,  0.0f};

    // TODO: Use the blit program.

    return true;
  }

  return true;
}

bool BlitEmulator::UseBlitProgram() {
  auto& gl = gl_;

  if (!emulated_blit_program_.has_value()) {
    auto vert_shader = gl.CreateShader(GL_VERTEX_SHADER);
    auto frag_shader = gl.CreateShader(GL_FRAGMENT_SHADER);
    if (vert_shader == 0 || frag_shader == 0) {
      VALIDATION_LOG << "Could not create shader handles.";
      return false;
    }

    fml::ScopedCleanupClosure delete_vert_shader(
        [&gl, vert_shader]() { gl.DeleteShader(vert_shader); });
    fml::ScopedCleanupClosure delete_frag_shader(
        [&gl, frag_shader]() { gl.DeleteShader(frag_shader); });

    {
      const GLchar* sources[] = {kBlitVertexShaderCode};
      const GLint lengths[] = {155};
      gl.ShaderSource(vert_shader, 1u, sources, lengths);
      gl.CompileShader(vert_shader);
    }
    {
      const GLchar* sources[] = {kBlitFragmentShaderCode};
      const GLint lengths[] = {138};
      gl.ShaderSource(frag_shader, 1u, sources, lengths);
      gl.CompileShader(frag_shader);
    }

    emulated_blit_program_ = gl.CreateProgram();

    gl.LinkProgram(*emulated_blit_program_);

    GLint link_status = GL_FALSE;
    gl.GetProgramiv(*emulated_blit_program_, GL_LINK_STATUS, &link_status);

    if (link_status != GL_TRUE) {
      VALIDATION_LOG << "Could not link shader program: "
                     << gl.GetProgramInfoLogString(*emulated_blit_program_);
      return false;
    }
  }

  gl_.UseProgram(*emulated_blit_program_);
  return true;
}

}  // namespace impeller
