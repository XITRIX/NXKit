//
//  ContentsGravityTransformation.cpp
//  SDLTest
//
//  Created by Даниил Виноградов on 26.02.2023.
//

#include <ContentsGravityTransformation.h>
#include <CALayer.h>

namespace NXKit {

ContentsGravityTransformation::ContentsGravityTransformation(CALayer* layer) {
    auto scaledContents = NXSize(
        layer->contents()->size().width / layer->contentsScale(),
        layer->contents()->size().height / layer->contentsScale()
    );

    auto bounds = layer->bounds();
    
    auto distanceToMinX = -((bounds.width() - scaledContents.width) * layer->anchorPoint().x);
    auto distanceToMinY = -((bounds.height() - scaledContents.height) * layer->anchorPoint().y);
    auto distanceToMaxX = (bounds.width() - scaledContents.width) * (1 - layer->anchorPoint().x);
    auto distanceToMaxY = (bounds.height() - scaledContents.height) * (1 - layer->anchorPoint().y);

    switch (layer->contentsGravity()) {
        case CALayerContentsGravity::resize: {
            this->offset = NXPoint::zero;
            scale = NXSize(bounds.width() / scaledContents.width, bounds.height() / scaledContents.height);
            break;
        }
        case CALayerContentsGravity::resizeAspectFill: {
            offset = NXPoint::zero;
            auto maxScale = std::max(bounds.width() / scaledContents.width, bounds.height() / scaledContents.height);
            scale = NXSize(maxScale, maxScale);
            break;
        }
        case CALayerContentsGravity::resizeAspect: {
            offset = NXPoint::zero;
            auto minScale = std::min(bounds.width() / scaledContents.width, bounds.height() / scaledContents.height);
            scale = NXSize(minScale, minScale);
            break;
        }
        case CALayerContentsGravity::center: {
            offset = NXPoint::zero;
            scale = defaultScale;
            break;
        }
        case CALayerContentsGravity::left: {
            offset = NXPoint(distanceToMinX, 0.0);
            scale = defaultScale;
            break;
        }
        case CALayerContentsGravity::right: {
            offset = NXPoint(distanceToMaxX, 0.0);
            scale = defaultScale;
            break;
        }
        case CALayerContentsGravity::top: {
            offset = NXPoint(0.0, distanceToMinY);
            scale = defaultScale;
            break;
        }
        case CALayerContentsGravity::bottom: {
            offset = NXPoint(0.0, distanceToMaxY);
            scale = defaultScale;
            break;
        }
        case CALayerContentsGravity::topLeft: {
            offset = NXPoint(distanceToMinX, distanceToMinY);
            scale = defaultScale;
            break;
        }
        case CALayerContentsGravity::topRight: {
            offset = NXPoint(distanceToMaxX, distanceToMinY);
            scale = defaultScale;
            break;
        }
        case CALayerContentsGravity::bottomLeft: {
            offset = NXPoint(distanceToMinX, distanceToMaxY);
            scale = defaultScale;
            break;
        }
        case CALayerContentsGravity::bottomRight: {
            offset = NXPoint(distanceToMaxX, distanceToMaxY);
            scale = defaultScale;
            break;
        }
    }
}

}
