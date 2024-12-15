#include <UILabel.h>
#include <CATextLayer.h>

using namespace NXKit;

UILabel::UILabel(): UIView(NXRect(), new_shared<CATextLayer>()) {

}
