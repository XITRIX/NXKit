/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTools/EventTracingPriv.h"

#include "include/utils/SkEventTracer.h"
#include "src/core/SkATrace.h"
#include "src/core/SkTraceEvent.h"
#include "SkTools/SkDebugfTracer.h"

// SkPerfettoTrace is only relevant when Perfetto is requested, and for in-process tracing. It is
// incompatible with the alternate "direct macro override" approach to using Perfetto, which is
// currently used for SK_BUILD_FOR_ANDROID_FRAMEWORK. Skia's Perfetto integration is currently in
// in a transitionary period, see go/skia-perfetto for details.
#if defined(SK_USE_PERFETTO)
  #if defined(SK_ANDROID_FRAMEWORK_USE_PERFETTO)
    #error "SK_USE_PERFETTO and SK_ANDROID_FRAMEWORK_USE_PERFETTO are mutually exclusive"
  #endif

  #include "tools/trace/SkPerfettoTrace.h"
#endif

void initializeEventTracingForTools(const char* traceFlag) {
    SkEventTracer* eventTracer = nullptr;
    eventTracer = new SkDebugfTracer();
    SkAssertResult(SkEventTracer::SetInstance(eventTracer));
}

uint8_t* SkEventTracingCategories::getCategoryGroupEnabled(const char* name) {
    static_assert(0 == offsetof(CategoryState, fEnabled), "CategoryState");

    // We ignore the "disabled-by-default-" prefix in our internal tools
    if (SkStrStartsWith(name, TRACE_CATEGORY_PREFIX)) {
        name += strlen(TRACE_CATEGORY_PREFIX);
    }

    // Chrome's implementation of this API does a two-phase lookup (once without a lock, then again
    // with a lock. But the tracing macros avoid calling these functions more than once per site,
    // so just do something simple (and easier to reason about):
    SkAutoMutexExclusive lock(fMutex);
    for (int i = 0; i < fNumCategories; ++i) {
        if (0 == strcmp(name, fCategories[i].fName)) {
            return reinterpret_cast<uint8_t*>(&fCategories[i]);
        }
    }

    if (fNumCategories >= kMaxCategories) {
        SkDEBUGFAIL("Exhausted event tracing categories. Increase kMaxCategories.");
        return reinterpret_cast<uint8_t*>(&fCategories[0]);
    }

    fCategories[fNumCategories].fEnabled = SkEventTracer::kEnabledForRecording_CategoryGroupEnabledFlags;

    fCategories[fNumCategories].fName = name;
    return reinterpret_cast<uint8_t*>(&fCategories[fNumCategories++]);
}

const char* SkEventTracingCategories::getCategoryGroupName(const uint8_t* categoryEnabledFlag) {
    if (categoryEnabledFlag) {
        return reinterpret_cast<const CategoryState*>(categoryEnabledFlag)->fName;
    }
    return nullptr;
}
