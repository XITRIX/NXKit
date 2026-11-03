#include "RobotoTestFontManager.hpp"
#include "include/ports/SkFontMgr_mac_ct.h"
#include "include/core/SkStream.h"
#include "include/core/SkData.h"
#include "include/core/SkString.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkTypeface.h"
namespace {
// Resolve the same registered Roboto file through Core Text, including the
// actual Skia Core Text rasterizer. No alternate font engine or fake bold.
class RobotoManager final : public SkFontMgr {
    sk_sp<SkFontMgr> base=SkFontMgr_New_CoreText(nullptr);
    int onCountFamilies() const override { return 1; }
    void onGetFamilyName(int,SkString* name) const override { *name=SkString("Roboto"); }
    sk_sp<SkFontStyleSet> onCreateStyleSet(int) const override { return base->matchFamily("Roboto"); }
    sk_sp<SkFontStyleSet> onMatchFamily(const char*) const override { return base->matchFamily("Roboto"); }
    sk_sp<SkTypeface> onMatchFamilyStyle(const char*,const SkFontStyle& s) const override { return base->matchFamilyStyle("Roboto",s); }
    sk_sp<SkTypeface> onMatchFamilyStyleCharacter(const char*,const SkFontStyle& s,const char* langs[],int count,SkUnichar c) const override { return base->matchFamilyStyleCharacter("Roboto",s,langs,count,c); }
    sk_sp<SkTypeface> onMakeFromData(sk_sp<SkData> d,int i) const override { return base->makeFromData(std::move(d),i); }
    sk_sp<SkTypeface> onMakeFromStreamIndex(std::unique_ptr<SkStreamAsset> s,int i) const override { return base->makeFromStream(std::move(s),i); }
    sk_sp<SkTypeface> onMakeFromStreamArgs(std::unique_ptr<SkStreamAsset> s,const SkFontArguments& a) const override { return base->makeFromStream(std::move(s),a); }
    sk_sp<SkTypeface> onMakeFromFile(const char* p,int i) const override { return base->makeFromFile(p,i); }
    sk_sp<SkTypeface> onLegacyMakeTypeface(const char*,SkFontStyle s) const override { return base->matchFamilyStyle("Roboto",s); }
};
}
sk_sp<SkFontMgr> NXKit::Internal::makeRobotoTestFontManager(){return sk_make_sp<RobotoManager>();}
