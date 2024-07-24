// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/testing/testing.h"
#include "impeller/entity/draw_order_resolver.h"
#include "third_party/googletest/googletest/include/gtest/gtest.h"

namespace impeller {
namespace testing {

TEST(DrawOrderResolverTest, GetSortedDrawsReturnsCorrectOrderWithNoClips) {
  DrawOrderResolver resolver;

  // Opaque items.
  resolver.AddElement(0, true);
  resolver.AddElement(1, true);
  // Translucent items.
  resolver.AddElement(2, false);
  resolver.AddElement(3, false);

  auto sorted_elements = resolver.GetSortedDraws(0, 0);

  EXPECT_EQ(sorted_elements.size(), 4u);
  // First, the opaque items are drawn in reverse order.
  EXPECT_EQ(sorted_elements[0], 1u);
  EXPECT_EQ(sorted_elements[1], 0u);
  // Then the translucent items are drawn.
  EXPECT_EQ(sorted_elements[2], 3u);
  EXPECT_EQ(sorted_elements[3], 2u);
}

TEST(DrawOrderResolverTest, GetSortedDrawsReturnsCorrectOrderWithClips) {
  DrawOrderResolver resolver;

  // Items before clip.
  resolver.AddElement(0, false);
  resolver.AddElement(1, true);
  resolver.AddElement(2, false);
  resolver.AddElement(3, true);

  // Clip.
  resolver.PushClip(4);
  {
    // Clipped items.
    resolver.AddElement(5, false);
    resolver.AddElement(6, false);
    // Clipped translucent items.
    resolver.AddElement(7, true);
    resolver.AddElement(8, true);
  }
  resolver.PopClip();

  // Items after clip.
  resolver.AddElement(9, true);
  resolver.AddElement(10, false);
  resolver.AddElement(11, true);
  resolver.AddElement(12, false);

  auto sorted_elements = resolver.GetSortedDraws(0, 0);

  EXPECT_EQ(sorted_elements.size(), 13u);
  // First, all the non-clipped opaque items are drawn in reverse order.
  EXPECT_EQ(sorted_elements[0], 11u);
  EXPECT_EQ(sorted_elements[1], 9u);
  EXPECT_EQ(sorted_elements[2], 3u);
  EXPECT_EQ(sorted_elements[3], 1u);
  // Then, the clip is drawn.
  EXPECT_EQ(sorted_elements[4], 4u);
  {
    // Opaque clipped items are drawn in reverse order.
    EXPECT_EQ(sorted_elements[5], 8u);
    EXPECT_EQ(sorted_elements[6], 7u);
    // Translucent clipped items are drawn.
    EXPECT_EQ(sorted_elements[7], 5u);
    EXPECT_EQ(sorted_elements[8], 6u);
  }
  // Finally, the non-clipped translucent items are drawn in their original
  // order.
  EXPECT_EQ(sorted_elements[9], 0u);
  EXPECT_EQ(sorted_elements[10], 2u);
  EXPECT_EQ(sorted_elements[11], 10u);
  EXPECT_EQ(sorted_elements[12], 12u);
}

TEST(DrawOrderResolverTest, GetSortedDrawsRespectsSkipCounts) {
  DrawOrderResolver resolver;

  // These items will be skipped.
  resolver.AddElement(0, false);
  resolver.AddElement(1, true);
  resolver.AddElement(2, false);
  // These ones will be included in the final draw list.
  resolver.AddElement(3, false);
  resolver.AddElement(4, true);
  resolver.AddElement(5, true);

  // Form the draw list, skipping elements 0, 1, and 2.
  // This emulates what happens when entitypass applies the clear color
  // optimization.
  auto sorted_elements = resolver.GetSortedDraws(1, 2);

  EXPECT_EQ(sorted_elements.size(), 4u);
  // First, opaque items are drawn in reverse order.
  EXPECT_EQ(sorted_elements[0], 5u);
  EXPECT_EQ(sorted_elements[1], 4u);
  // Then, translucent items are drawn.
  EXPECT_EQ(sorted_elements[2], 3u);
}

}  // namespace testing
}  // namespace impeller
