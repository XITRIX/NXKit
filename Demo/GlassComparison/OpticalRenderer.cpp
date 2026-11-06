// Production retained-view rendering; native metadata fixes the geometry.
#include "include/core/SkBitmap.h"
#include "include/core/SkSurface.h"
#include <BackdropEffectView.h>
#include <UIImageView.h>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>
#include <regex>
#include <sstream>
using namespace NXKit;
namespace NXKit {
class GlassMaterialTestHarness {
public:
  static std::shared_ptr<UITraitCollection> traits(bool dark) {
    auto t = std::make_shared<UITraitCollection>();
    t->_userInterfaceStyle =
        dark ? UIUserInterfaceStyle::dark : UIUserInterfaceStyle::light;
    t->_displayScale = 2;
    return t;
  }
  static BackdropEffect resolve(UIGlassEffect e, NXSize size, bool dark) {
    return e._backdropEffect(size, traits(dark));
  }
};
} // namespace NXKit
class OpticalRoot : public UIView {
public:
  explicit OpticalRoot(bool dark) : UIView({0, 0, 720, 440}) {
    _traitCollection = GlassMaterialTestHarness::traits(dark);
    layer()->setContentsScale(2);
  }
};
int main(int argc, char **argv) {
  if (argc < 2) {
    std::cerr << "Usage: OpticalRenderer MANIFEST [SHADER_OVERRIDE] "
                 "[--frost=SMALL,LARGE]\n";
    return 2;
  }
  std::string shader;
  std::optional<std::pair<float, float>> frost;
  for (int i = 2; i < argc; i++) {
    std::string argument = argv[i];
    if (argument.starts_with("--frost=")) {
      const auto comma = argument.find(',');
      if (comma == std::string::npos)
        return 2;
      const auto small = std::stof(argument.substr(8, comma - 8)),
                 large = std::stof(argument.substr(comma + 1));
      if (!std::isfinite(small) || !std::isfinite(large) || small < 0 ||
          large < 0)
        return 2;
      frost = std::pair{small, large};
    } else {
      std::ifstream file(argument);
      if (!file)
        return 2;
      shader = {std::istreambuf_iterator<char>(file), {}};
    }
  }
  std::ifstream input(argv[1]);
  std::string line;
  while (std::getline(input, line)) {
    std::istringstream fields(line);
    std::string path, imagePath, tint;
    int dark;
    if (!(fields >> std::quoted(path) >> std::quoted(imagePath) >> dark >>
          tint))
      return 2;
    auto root = new_shared<OpticalRoot>(dark);
    auto background = new_shared<UIImageView>();
    background->setImage(UIImage::fromPath(imagePath, 1));
    background->setFrame(root->bounds());
    root->addSubview(background);
    for (int i = 0; i < 5; i++) {
      float x, y, w, h, r;
      if (!(fields >> x >> y >> w >> h >> r))
        return 2;
      UIGlassEffect e;
      e.setInteractive(true);
      if (frost)
        e.setFrostRadius(std::min(w, h) < 100 ? frost->first : frost->second);
      if (tint == "blue")
        e.setTintColor(UIColor(0, 122, 255));
      else if (tint == "coral")
        e.setTintColor(UIColor(255, 69, 58));
      auto item = new_shared<BackdropEffectView>(e);
      item->setFrame({x, y, w, h});
      item->layer()->setCornerRadius(r);
      item->layer()->setContentsScale(2);
      root->addSubview(item);
      if (!shader.empty()) {
        auto resolved = GlassMaterialTestHarness::resolve(e, {w, h}, dark);
        BackdropEffect custom(shader, resolved.backdropShaderName(),
                              resolved.maximumSampleRadius(),
                              resolved.backdropBlurRadius());
        // Copy all declared nonautomatic defaults; shader-only iteration
        // retains production material parameters.
        const std::regex declaration(
            R"(uniform\s+(?:half|float|int)[234x]*\s+([A-Za-z_][A-Za-z0-9_]*))");
        for (auto it = std::sregex_iterator(shader.begin(), shader.end(),
                                            declaration);
             it != std::sregex_iterator(); ++it) {
          const auto name = (*it)[1].str();
          if (auto v = resolved.uniform(name))
            custom.setUniform(name, *v);
        }
        item->setEffect(custom);
      }
    }
    auto s = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(1440, 880));
    s->getCanvas()->scale(2, 2);
    s->getCanvas()->clear(SK_ColorBLACK);
    root->layer()->skiaRender(s->getCanvas());
    SkBitmap b;
    b.allocN32Pixels(1440, 880);
    if (!s->readPixels(b, 0, 0))
      return 1;
    std::ofstream file(path, std::ios::binary);
    file << "P6\n1440 880\n255\n";
    for (int y = 0; y < 880; y++)
      for (int x = 0; x < 1440; x++) {
        auto c = b.getColor(x, y);
        unsigned char rgb[] = {static_cast<unsigned char>(SkColorGetR(c)),
                               static_cast<unsigned char>(SkColorGetG(c)),
                               static_cast<unsigned char>(SkColorGetB(c))};
        file.write(reinterpret_cast<char *>(rgb), 3);
      }
    std::cout << path << std::endl;
  }
}
