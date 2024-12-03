// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_IMPELLER_RENDERER_BACKEND_BLIT_EMULATOR_H_
#define FLUTTER_IMPELLER_RENDERER_BACKEND_BLIT_EMULATOR_H_

#include <memory>
#include "impeller/core/formats.h"
#include "impeller/geometry/rect.h"
#include "impeller/renderer/backend/gles/proc_table_gles.h"

namespace impeller {

/// Utility that can perform GLES texture-to-texture blits even when
/// glBlitFramebuffer (GLES 3) is unavailable.
class BlitEmulator {
 public:
  explicit BlitEmulator(ProcTableGLES& gl);
  ~BlitEmulator();

  /// @brief  Copies the `source_region` of `source` to the `destination_region`
  ///         of `destination`, applying the given `filter`.
  ///         Lazily compiles and links a blit program if `glBlitFramebuffer` is
  ///         not available.
  bool Blit(const std::shared_ptr<Texture>& source,
            const std::shared_ptr<Texture>& destination,
            IRect source_region,
            IRect destination_region,
            impeller::MinMagFilter filter);

 private:
  ProcTableGLES& gl_;
  std::optional<GLuint> emulated_blit_program_;
  GLuint vao_;

  bool UseBlitProgram();
};

}  // namespace impeller

#endif  // FLUTTER_IMPELLER_RENDERER_BACKEND_BLIT_EMULATOR_H_
