// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/aiks/aiks_playground.h"

#include <memory>
#include <optional>

#include "impeller/aiks/aiks_context.h"
#include "impeller/display_list/dl_dispatcher.h"
#include "impeller/typographer/backends/skia/typographer_context_skia.h"
#include "impeller/typographer/typographer_context.h"

namespace impeller {

AiksPlayground::AiksPlayground()
    : typographer_context_(TypographerContextSkia::Make()) {}

AiksPlayground::~AiksPlayground() = default;

void AiksPlayground::SetTypographerContext(
    std::shared_ptr<TypographerContext> typographer_context) {
  typographer_context_ = std::move(typographer_context);
}

void AiksPlayground::TearDown() {
  PlaygroundTest::TearDown();
}

bool AiksPlayground::OpenPlaygroundHere(Picture picture) {
  return OpenPlaygroundHere([&picture](AiksContext& renderer) -> Picture {
    return std::move(picture);
  });
}

bool AiksPlayground::OpenPlaygroundHere(AiksPlaygroundCallback callback) {
  if (!switches_.enable_playground) {
    return true;
  }

  AiksContext renderer(GetContext(), typographer_context_);

  if (!renderer.IsValid()) {
    return false;
  }

  std::optional<Picture> last_frame;

  return Playground::OpenPlaygroundHere(
      [&renderer, &callback, &last_frame](RenderTarget& render_target) -> bool {
        std::optional<Picture> picture = callback(renderer);
        if (picture.has_value() && picture->pass != nullptr) {
          last_frame = std::move(picture);
        }
        if (!last_frame.has_value()) {
          return false;  // The first frame rendered nothing. Fail
        }
        return renderer.Render(*last_frame, render_target, true);
      });
}

bool AiksPlayground::ImGuiBegin(const char* name,
                                bool* p_open,
                                ImGuiWindowFlags flags) {
  ImGui::Begin(name, p_open, flags);
  return true;
}

bool AiksPlayground::OpenPlaygroundHere(
    const sk_sp<flutter::DisplayList>& list) {
  DlDispatcher dispatcher;
  list->Dispatch(dispatcher);
  Picture picture = dispatcher.EndRecordingAsPicture();
  return OpenPlaygroundHere(std::move(picture));
}

}  // namespace impeller
