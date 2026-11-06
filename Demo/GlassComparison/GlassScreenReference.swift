import SwiftUI
import UIKit

@MainActor final class ScreenState: ObservableObject {
    @Published var paths = [[1], [1], [1], [1]]
    @Published var tab = 0
    @Published var accent = 0
    @Published var backdrop = 0
    @Published var details = false
    @Published var added = 0
    @Published var reversed = false
    @Published var completed: Set<Int> = []
    @Published var dark = false
    let titles = ["Home", "Favorites", "Library", "Settings"]
    var color: Color { [Color.blue, Color(red: 1, green: 45.0/255, blue: 85.0/255), Color(red: 0, green: 163.0/255, blue: 140.0/255)][accent] }
    var background: Color { backdrop == 0 ? Color(uiColor: .systemGroupedBackground) : (backdrop == 1 ? Color(red: 31.0/255, green: 94.0/255, blue: 184.0/255) : Color(red: 209.0/255, green: 71.0/255, blue: 61.0/255)) }
    func reset() { accent = 0; backdrop = 0; details = false; added = 0; reversed = false; completed = [] }
}
struct ScreenTab: View {
    @ObservedObject var state: ScreenState
    let index: Int
    var body: some View {
        NavigationStack(path: $state.paths[index]) {
            List { NavigationLink("Full-screen iOS layout", value: 1) }.navigationTitle("Glass")
                .navigationDestination(for: Int.self) { _ in detail }
        }
    }
    var detail: some View {
        List {
            Section("Appearance") {
                Button { state.backdrop = (state.backdrop + 1) % 3 } label: { row("Background", ["System", "Blue", "Coral"][state.backdrop]) }.accessibilityIdentifier("background")
                Button { state.accent = (state.accent + 1) % 3 } label: { row("Accent color", ["Blue", "Pink", "Mint"][state.accent]) }.accessibilityIdentifier("accent")
            }
            Section {
                Button("Add item") { state.added = min(30, state.added + 1) }.foregroundStyle(state.color).accessibilityIdentifier("add-row")
                Button { state.reversed.toggle() } label: { row("Sort order", state.reversed ? "Descending" : "Ascending") }.accessibilityIdentifier("sort")
                Button("Reset demo") { state.reset() }.foregroundStyle(state.color).accessibilityIdentifier("reset")
            } header: { Text("Collection") } footer: { Text("Use + to add an item. More shows additional details.") }
            Section(state.titles[index] + " collection") {
                ForEach(state.reversed ? Array((1...(8 + state.added)).reversed()) : Array(1...(8 + state.added)), id: \.self) { item in
                    itemRow(item)
                }
            }
            Section { Text("\(state.completed.count) completed · \(8 + state.added) items").foregroundStyle(.secondary) }
        }
        .buttonStyle(.plain)
        .listStyle(.insetGrouped)
        .scrollContentBackground(.hidden)
        .background(state.background)
        .navigationTitle(state.titles[index])
        .navigationBarTitleDisplayMode(.inline)
        .navigationBarBackButtonHidden()
        .toolbar {
            ToolbarItem(placement: .topBarLeading) {
                Button { state.paths[index].removeLast() } label: { Image(systemName: "chevron.left") }.accessibilityIdentifier("back")
            }
            ToolbarItem(placement: .topBarTrailing) {
                Button { state.details.toggle() } label: { Image(systemName: "ellipsis") }.accessibilityIdentifier("more")
            }
            ToolbarItem(placement: .topBarTrailing) {
                Button { state.added = min(30, state.added + 1) } label: { Image(systemName: "plus") }.buttonStyle(.glassProminent).accessibilityIdentifier("add")
            }
        }
    }
    func itemRow(_ item: Int) -> some View {
        Button {
            if state.completed.contains(item) { state.completed.remove(item) } else { state.completed.insert(item) }
        } label: {
            VStack(alignment: .leading, spacing: 4) {
                row("Item \(item)", state.completed.contains(item) ? "Done" : "Open")
                if state.details { Text("Tap to change this item's status.").font(.footnote).foregroundStyle(.secondary) }
            }
        }.accessibilityIdentifier("item-\(item)")
    }
    func row(_ title: String, _ value: String) -> some View {
        HStack { Text(title).foregroundStyle(.primary); Spacer(); Text(value).foregroundStyle(.secondary) }.font(.body)
    }
}
struct ScreenShell: View {
    @ObservedObject var state: ScreenState
    var body: some View {
        TabView(selection: $state.tab) {
            ForEach(0..<4) { index in
                ScreenTab(state: state, index: index)
                    .tabItem { Label(state.titles[index], systemImage: ["house", "star", "books.vertical", "slider.horizontal.3"][index]) }.tag(index)
            }
        }.tint(state.color).preferredColorScheme(state.dark ? .dark : .light)
    }
}
@MainActor class ReferenceDelegate: UIResponder, UIApplicationDelegate {
    var window: UIWindow?
    let state = ScreenState()
    var timer: Timer?
    var pending = false
    var documents: URL { FileManager.default.urls(for: .documentDirectory, in: .userDomainMask)[0] }
    func application(_ application: UIApplication, didFinishLaunchingWithOptions options: [UIApplication.LaunchOptionsKey: Any]? = nil) -> Bool {
        true
    }
    func application(_ application: UIApplication, configurationForConnecting session: UISceneSession, options: UIScene.ConnectionOptions) -> UISceneConfiguration {
        let configuration = UISceneConfiguration(name: "Default", sessionRole: session.role)
        configuration.delegateClass = ReferenceSceneDelegate.self
        return configuration
    }
    func connect(_ scene: UIWindowScene) {
        let window = UIWindow(windowScene: scene)
        window.rootViewController = UIHostingController(rootView: ScreenShell(state: state))
        window.makeKeyAndVisible(); self.window = window
        try? FileManager.default.removeItem(at: documents.appendingPathComponent("request.json"))
        timer = Timer.scheduledTimer(withTimeInterval: 0.1, repeats: true) { [weak self] _ in
            Task { @MainActor in self?.readRequest() }
        }
    }
    func readRequest() {
        let url = documents.appendingPathComponent("request.json")
        guard !pending, let data = try? Data(contentsOf: url), let request = (try? JSONSerialization.jsonObject(with: data)) as? [String: Any], let id = request["id"] as? String else { return }
        pending = true; try? FileManager.default.removeItem(at: url)
        state.tab = min(3, max(0, request["tab"] as? Int ?? 0))
        state.dark = request["dark"] as? Bool ?? false
        state.accent = min(2, max(0, request["accent"] as? Int ?? 0))
        state.backdrop = min(2, max(0, request["backdrop"] as? Int ?? 0))
        state.details = request["details"] as? Bool ?? false
        state.added = min(30, max(0, request["added"] as? Int ?? 0))
        state.reversed = request["reversed"] as? Bool ?? false
        DispatchQueue.main.asyncAfter(deadline: .now() + 0.6) {
            if let scroll = self.allViews().compactMap({ $0 as? UIScrollView }).first(where: { $0.bounds.height > 200 && $0.contentSize.height > $0.bounds.height }) {
                let offset = request["scroll"] as? CGFloat ?? 0
                scroll.setContentOffset(CGPoint(x: 0, y: -scroll.adjustedContentInset.top + offset), animated: false)
            }
            DispatchQueue.main.asyncAfter(deadline: .now() + 0.3) { self.capture(id); self.pending = false }
        }
    }
    func allViews() -> [UIView] {
        guard let window else { return [] }
        func flatten(_ view: UIView) -> [UIView] { [view] + view.subviews.flatMap(flatten) }
        return flatten(window)
    }
    func capture(_ id: String) {
        guard let window else { return }
        let format = UIGraphicsImageRendererFormat(); format.scale = window.screen.scale; format.preferredRange = .standard
        let image = UIGraphicsImageRenderer(bounds: window.bounds, format: format).image { _ in window.drawHierarchy(in: window.bounds, afterScreenUpdates: true) }
        try? image.pngData()?.write(to: documents.appendingPathComponent(id + ".png"))
        func rect(_ r: CGRect) -> [CGFloat] { [r.minX, r.minY, r.width, r.height] }
        let views = allViews()
        let bars = views.filter { $0 is UINavigationBar || $0 is UITabBar }.filter { !$0.isHidden && $0.window != nil }.map { ["class": NSStringFromClass(type(of: $0)), "frame": rect($0.convert($0.bounds, to: window))] as [String: Any] }
        let cells = views.compactMap { $0 as? UICollectionViewCell }.filter { !$0.isHidden }.map { rect($0.convert($0.bounds, to: window)) }
        let safe = window.safeAreaInsets
        let result: [String: Any] = ["id": id, "bounds": rect(window.bounds), "safe": [safe.top, safe.left, safe.bottom, safe.right], "scale": window.screen.scale, "os": UIDevice.current.systemVersion, "bars": bars, "cells": cells, "barParts": views.filter { !$0.isHidden && (NSStringFromClass(type(of: $0)).contains("TabBar") || NSStringFromClass(type(of: $0)).contains("LiquidLens") || $0 is UIVisualEffectView || ["back", "more", "add"].contains($0.accessibilityIdentifier ?? "")) }.map { ["class": NSStringFromClass(type(of: $0)), "id": $0.accessibilityIdentifier ?? "", "frame": rect($0.convert($0.bounds, to: window))] as [String: Any] }]
        if let data = try? JSONSerialization.data(withJSONObject: result, options: [.prettyPrinted, .sortedKeys]) { try? data.write(to: documents.appendingPathComponent(id + ".json")) }
    }
}
@MainActor class ReferenceSceneDelegate: UIResponder, UIWindowSceneDelegate {
    var window: UIWindow?
    func scene(_ scene: UIScene, willConnectTo session: UISceneSession, options: UIScene.ConnectionOptions) {
        guard let scene = scene as? UIWindowScene, let app = UIApplication.shared.delegate as? ReferenceDelegate else { return }
        app.connect(scene); window = app.window
    }
}
@main struct Entry {
    static func main() { UIApplicationMain(CommandLine.argc, CommandLine.unsafeArgv, nil, NSStringFromClass(ReferenceDelegate.self)) }
}
