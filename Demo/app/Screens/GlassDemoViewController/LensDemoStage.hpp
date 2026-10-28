#pragma once
#include "LensTabBarView.hpp"
class LensDemoStage final : public NXKit::UIView {
public:
    explicit LensDemoStage(bool tabBarOnly = false);
    void setBackdrop(int value);
    int backdrop() const { return _backdrop; }
    void setLifted(bool lifted);
    bool isLifted() const { return _lifted; }
    void selectNext();
    void toggleSize();
    void layoutSubviews() override;
private:
    int _backdrop=0;
    bool _lifted=false, _large=false;
    bool _tabBarOnly=false;
    std::shared_ptr<NXKit::UIImageView> _artwork;
    std::shared_ptr<NXKit::UIView> _grid;
    std::vector<std::shared_ptr<NXKit::UIView>> _stripes;
    std::shared_ptr<LensTabBarView> _compact, _wide;
    std::vector<std::shared_ptr<NXKit::LensView>> _samples;
};
