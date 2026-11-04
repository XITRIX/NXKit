#include <UIView.h>
#include <UIInteraction.h>
#include <UIRectEdge.h>

#include <algorithm>
#include <functional>
#include <iostream>
#include <stdexcept>

using namespace NXKit;
namespace {
int failures = 0;
void expect(bool condition, const char* message) {
    if (!condition) { ++failures; std::cerr << "FAIL: " << message << '\n'; }
}
class OwnedProbe final : public UIInteraction {
public:
    std::weak_ptr<UIView> attached;
    std::function<void(const std::shared_ptr<UIView>&)> will, did;
    std::weak_ptr<UIView> view() const override { return attached; }
    void willMoveToView(const std::shared_ptr<UIView>& destination) override { if (will) will(destination); }
    void didMoveToView(const std::shared_ptr<UIView>& destination) override {
        attached = destination;
        if (did) did(destination);
    }
};
std::shared_ptr<UIView> host(const char* name) {
    auto view = new_shared<UIView>(); view->tag = name; return view;
}

void attachmentAndLifetime() {
    auto a = host("a"), b = host("b");
    auto interaction = std::make_shared<OwnedProbe>();
    std::vector<std::string> events;
    interaction->will = [&](const auto& destination) {
        const auto previous = interaction->view().lock();
        events.push_back("will:" + (destination ? destination->tag : "nil")
                         + ":from:" + (previous ? previous->tag : "nil"));
        if (previous) expect(previous->interactions() == std::vector<std::shared_ptr<UIInteraction>> {interaction},
                             "willMove sees the old owner's intact array");
    };
    interaction->did = [&](const auto& destination) {
        events.push_back("did:" + (destination ? destination->tag : "nil"));
        if (destination) expect(destination->interactions() == std::vector<std::shared_ptr<UIInteraction>> {interaction},
                                "didMove sees committed new membership");
    };
    a->addInteraction(interaction);
    a->addInteraction(interaction);
    expect(events == std::vector<std::string> {"will:a:from:nil", "did:a"},
           "adding an already attached interaction is an idempotent no-op");
    auto snapshot = a->interactions(); snapshot.clear();
    expect(a->interactions().size() == 1, "interactions returns a snapshot rather than mutable storage");
    b->removeInteraction(interaction);
    expect(events.size() == 2 && interaction->view().lock() == a, "removing from another view is a no-op");
    b->addInteraction(interaction);
    expect(events == std::vector<std::string> {"will:a:from:nil", "did:a", "will:nil:from:a", "did:nil", "will:b:from:nil", "did:b"}
           && a->interactions().empty() && interaction->view().lock() == b,
           "moving an interaction sends a removal pair before the new attachment pair");
    b->removeInteraction(interaction);
    expect(interaction->view().expired() && b->interactions().empty(), "explicit removal clears attachment and ownership");
    b->addInteraction(interaction);
    const auto beforeDestruction = events.size();
    std::weak_ptr<UIView> weakView = b;
    b.reset();
    expect(weakView.expired() && interaction->view().expired() && events.size() == beforeDestruction,
           "destroying a view clears its weak back-reference without move callbacks, matching UIKit");
    interaction->will = nullptr; interaction->did = nullptr;
    a->addInteraction(interaction);
    std::weak_ptr<UIInteraction> retained = interaction;
    interaction.reset();
    expect(!retained.expired(), "the attached view owns its interaction");
    a->removeInteraction(retained.lock());
    expect(retained.expired(), "removal releases view ownership without a retained queue entry");
}

void arraysAndValidation() {
    auto a = host("a"), b = host("b");
    auto first = std::make_shared<OwnedProbe>(), second = std::make_shared<OwnedProbe>();
    int callbacks = 0;
    for (const auto& interaction : {first, second}) {
        interaction->will = [&](const auto&) { ++callbacks; };
        interaction->did = [&](const auto&) { ++callbacks; };
    }
    a->setInteractions({first, second});
    expect(a->interactions() == std::vector<std::shared_ptr<UIInteraction>> {first, second} && callbacks == 4,
           "array assignment attaches entries in their requested order");
    a->setInteractions({second, first});
    expect(a->interactions() == std::vector<std::shared_ptr<UIInteraction>> {second, first} && callbacks == 12,
           "array reassignment detaches all previous interactions before adding the requested order, matching UIKit");
    a->setInteractions({second, first});
    expect(callbacks == 20, "assigning the same array still sends full removal and attachment callbacks");
    a->setInteractions({second, second, first});
    expect(a->interactions() == std::vector<std::shared_ptr<UIInteraction>> {second, first} && callbacks == 28,
           "duplicate array entries attach only once, matching UIKit");
    b->setInteractions({second});
    expect(a->interactions() == std::vector<std::shared_ptr<UIInteraction>> {first}
           && second->view().lock() == b, "array assignment moves an interaction from its previous view");
    a->setInteractions({});
    expect(first->view().expired() && a->interactions().empty(), "empty array detaches existing interactions");
    for (int operation : {0, 1, 2}) {
        const auto before = callbacks;
        bool rejected = false;
        try {
            if (operation == 0) a->addInteraction(nullptr);
            else if (operation == 1) a->removeInteraction(nullptr);
            else a->setInteractions({first, nullptr});
        } catch (const std::invalid_argument&) { rejected = true; }
        expect(rejected && a->interactions().empty() && callbacks == before,
               "null arguments are rejected before ownership or callbacks change");
    }
}

void reentrantMutations() {
    auto a = host("a"), b = host("b");
    auto interaction = std::make_shared<OwnedProbe>();
    bool moved = false;
    interaction->did = [&](const auto& destination) {
        if (destination == a && !moved) {
            moved = true;
            b->addInteraction(interaction);
            expect(interaction->view().lock() == a && b->interactions().empty(),
                   "callback reparenting waits until its current callback pair finishes");
        }
    };
    a->addInteraction(interaction);
    expect(a->interactions().empty() && interaction->view().lock() == b && b->interactions().size() == 1,
           "queued callback reparenting settles on exactly one owner");
    interaction->did = nullptr;
    b->removeInteraction(interaction);
    interaction->will = [&](const auto& destination) {
        if (destination == a) a->removeInteraction(interaction);
    };
    a->addInteraction(interaction);
    expect(a->interactions().empty() && interaction->view().expired(),
           "removal requested during willMove applies after attachment rather than getting lost");
    interaction->will = [](const auto&) { throw std::runtime_error("callback"); };
    bool threw = false;
    try { a->addInteraction(interaction); } catch (const std::runtime_error&) { threw = true; }
    expect(threw && a->interactions().empty() && interaction->view().expired(),
           "a throwing willMove callback leaves ownership intact and resets the mutation queue");
    interaction->will = nullptr;
    a->addInteraction(interaction);
    expect(interaction->view().lock() == a, "operations remain usable after a throwing callback");
    interaction->did = nullptr; interaction->will = nullptr;
}
}

int main() {
    static_assert(static_cast<unsigned>(UIRectEdge::all) == 15);
    static_assert((UIRectEdge::top | UIRectEdge::left) == static_cast<UIRectEdge>(3));
    attachmentAndLifetime(); arraysAndValidation(); reentrantMutations();
    if (failures) return 1;
    std::cout << "UIView interaction attachment, ownership, arrays, and reentrancy passed\n";
    return 0;
}
