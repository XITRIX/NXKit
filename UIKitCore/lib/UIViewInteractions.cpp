#include <UIView.h>

#include <algorithm>
#include <deque>
#include <stdexcept>

namespace NXKit {
namespace {

// Serialize callback-driven changes across views, including an interaction
// moving from one view to another. No owning references survive the drain.
void performInteractionMutation(std::function<void()> mutation) {
    static thread_local std::deque<std::function<void()>> pending;
    static thread_local bool draining = false;
    pending.push_back(std::move(mutation));
    if (draining) return;
    draining = true;
    try {
        while (!pending.empty()) {
            auto operation = std::move(pending.front());
            pending.pop_front();
            operation();
        }
    } catch (...) {
        pending.clear();
        draining = false;
        throw;
    }
    draining = false;
}

}

void UIView::addInteraction(const std::shared_ptr<UIInteraction>& interaction) {
    if (!interaction) throw std::invalid_argument("addInteraction requires a non-null interaction");
    performInteractionMutation([self = shared_from_this(), interaction] {
        self->addInteractionNow(interaction);
    });
}

void UIView::removeInteraction(const std::shared_ptr<UIInteraction>& interaction) {
    if (!interaction) throw std::invalid_argument("removeInteraction requires a non-null interaction");
    performInteractionMutation([self = shared_from_this(), interaction] {
        self->removeInteractionNow(interaction);
    });
}

void UIView::addInteractionNow(const std::shared_ptr<UIInteraction>& interaction) {
    if (std::find(_interactions.begin(), _interactions.end(), interaction) != _interactions.end()) return;
    if (const auto previous = interaction->_attachedView.lock()) previous->removeInteractionNow(interaction);
    const auto self = shared_from_this();
    interaction->willMoveToView(self);
    _interactions.push_back(interaction);
    interaction->_attachedView = self;
    interaction->didMoveToView(self);
}

void UIView::removeInteractionNow(const std::shared_ptr<UIInteraction>& interaction) {
    if (std::find(_interactions.begin(), _interactions.end(), interaction) == _interactions.end()) return;
    interaction->willMoveToView(nullptr);
    std::erase(_interactions, interaction);
    interaction->_attachedView.reset();
    interaction->didMoveToView(nullptr);
}

void UIView::setInteractions(std::vector<std::shared_ptr<UIInteraction>> interactions) {
    std::vector<std::shared_ptr<UIInteraction>> unique;
    unique.reserve(interactions.size());
    for (const auto& interaction : interactions) {
        if (!interaction) throw std::invalid_argument("setInteractions requires non-null entries");
        if (std::find(unique.begin(), unique.end(), interaction) == unique.end()) unique.push_back(interaction);
    }
    performInteractionMutation([self = shared_from_this(), interactions = std::move(unique)] {
        const auto previous = self->_interactions;
        for (const auto& interaction : previous) self->removeInteractionNow(interaction);
        for (const auto& interaction : interactions) self->addInteractionNow(interaction);
    });
}

}
