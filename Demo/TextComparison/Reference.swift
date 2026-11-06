import UIKit
import CoreText
import CryptoKit

struct Sample {
    let id: String, text: String
    let size: CGFloat, weight: CGFloat
    let custom: Bool
    var height: CGFloat { id == "multiline" ? 72 : 52 }
}
let samples: [Sample] = [
    Sample(id: "caption", text: "Home · Favorites 123", size: 11, weight: 400, custom: true),
    Sample(id: "footnote", text: "Agjpqy ÉÅ accents 0123", size: 13, weight: 400, custom: true),
    Sample(id: "body", text: "Background · Item 1", size: 17, weight: 400, custom: true),
    Sample(id: "title", text: "Glass Agjpqy", size: 22, weight: 400, custom: true),
    Sample(id: "large", text: "Typography Agjpqy", size: 34, weight: 400, custom: true),
    Sample(id: "system-body", text: "Background · Item 1", size: 17, weight: 400, custom: false),
    Sample(id: "system-headline", text: "Home · Collection", size: 17, weight: 600, custom: false),
    Sample(id: "system-bold", text: "Bold text 012345", size: 22, weight: 700, custom: false),
    Sample(id: "multiline", text: "Agjpqy first line\nSecond line ÉÅ", size: 17, weight: 400, custom: true),
    Sample(id: "fallback", text: "Hello Привет 😀", size: 17, weight: 400, custom: false),
    Sample(id: "empty", text: "", size: 17, weight: 400, custom: true)
]
@MainActor class Delegate: UIResponder, UIApplicationDelegate {
    var window: UIWindow?
    func application(_ application: UIApplication, configurationForConnecting session: UISceneSession, options: UIScene.ConnectionOptions) -> UISceneConfiguration {
        let c = UISceneConfiguration(name: "Default", sessionRole: session.role); c.delegateClass = Scene.self; return c
    }
    func capture() {
        let url = Bundle.main.url(forResource: "Roboto-Regular", withExtension: "ttf")!
        CTFontManagerRegisterFontsForURL(url as CFURL, .process, nil)
        let docs = FileManager.default.urls(for: .documentDirectory, in: .userDomainMask)[0]
        for scale: CGFloat in [1, 2, 3] { for dark in [false, true] {
            let id = "\(Int(scale))x-" + (dark ? "dark" : "light")
            let format = UIGraphicsImageRendererFormat(); format.scale = scale; format.opaque = true; format.preferredRange = .standard
            var records: [[String: Any]] = []
            let image = UIGraphicsImageRenderer(size: CGSize(width: 400, height: samples.reduce(0) { $0 + $1.height }), format: format).image { context in
                (dark ? UIColor.black : UIColor.white).setFill(); context.fill(CGRect(x: 0, y: 0, width: 400, height: 800))
                var y: CGFloat = 0
                for s in samples {
                    let label = UILabel(frame: CGRect(x: 12, y: y, width: 376, height: s.height))
                    label.traitOverrides.displayScale = scale; label.updateTraitsIfNeeded()
                    label.numberOfLines = 0; label.text = s.text; label.textColor = dark ? .white : .black
                    label.font = s.custom ? UIFont(name: "Roboto-Regular", size: s.size)! : UIFont.systemFont(ofSize: s.size, weight: s.weight == 600 ? .semibold : (s.weight == 700 ? .bold : .regular))
                    label.layer.contentsScale = scale; label.contentScaleFactor = scale
                    context.cgContext.saveGState(); context.cgContext.translateBy(x: 12, y: y); label.layer.render(in: context.cgContext); context.cgContext.restoreGState()
                    let rect = label.textRect(forBounds: label.bounds, limitedToNumberOfLines: 0), fits = label.sizeThatFits(CGSize(width: 376, height: 1000))
                    let host = UIView(frame:CGRect(x:0,y:0,width:400,height:200)), probe = UILabel()
                    host.traitOverrides.displayScale = scale; probe.traitOverrides.displayScale = scale; host.updateTraitsIfNeeded(); probe.updateTraitsIfNeeded()
                    probe.text = s.text; probe.font = label.font; probe.numberOfLines = 0; probe.translatesAutoresizingMaskIntoConstraints = false; host.addSubview(probe)
                    NSLayoutConstraint.activate([probe.leadingAnchor.constraint(equalTo:host.leadingAnchor),probe.widthAnchor.constraint(equalToConstant:376),probe.heightAnchor.constraint(equalToConstant:s.height),probe.firstBaselineAnchor.constraint(equalTo:host.topAnchor,constant:100)])
                    host.layoutIfNeeded()
                    let baseline = 100-probe.frame.minY
                    records.append(["id": s.id, "text": s.text, "size": s.size, "weight": s.weight, "custom": s.custom, "frame": [12,y,376,s.height], "textRect": [rect.minX,rect.minY,rect.width,rect.height], "fits": [fits.width,fits.height], "font": label.font.fontName, "displayScale":label.traitCollection.displayScale,"firstBaseline": baseline, "ascender": label.font.ascender, "descender": label.font.descender, "leading": label.font.leading, "lineHeight": label.font.lineHeight])
                    y += s.height
                }
            }
            // A known-baseline Core Text control lets the analysis recover the
            // actual UILabel baseline from ink centroids, without private APIs.
            let baselineImage = UIGraphicsImageRenderer(size: image.size, format: format).image { context in
                (dark ? UIColor.black : UIColor.white).setFill(); context.fill(CGRect(origin: .zero, size: image.size))
                var y: CGFloat = 0
                for s in samples {
                    let font = s.custom ? UIFont(name: "Roboto-Regular", size: s.size)! : UIFont.systemFont(ofSize: s.size, weight: s.weight == 600 ? .semibold : (s.weight == 700 ? .bold : .regular))
                    let attrs: [NSAttributedString.Key: Any] = [.font:font,.foregroundColor:dark ? UIColor.white : UIColor.black]
                    let line = CTLineCreateWithAttributedString(NSAttributedString(string:s.text,attributes:attrs))
                    context.cgContext.saveGState(); context.cgContext.translateBy(x:12,y:y+40); context.cgContext.scaleBy(x:1,y:-1); context.cgContext.textMatrix = .identity; CTLineDraw(line,context.cgContext); context.cgContext.restoreGState()
                    y += s.height
                }
            }
            try! baselineImage.pngData()!.write(to: docs.appendingPathComponent(id + "-baseline.png"))
            try! image.pngData()!.write(to: docs.appendingPathComponent(id + ".png"))
            let data: [String: Any] = ["sourceSHA256":Bundle.main.object(forInfoDictionaryKey:"NXKitTextFixtureSourceSHA256")!,"id":id,"scale":scale,"dark":dark,"os":UIDevice.current.systemVersion,"fontSHA256":SHA256.hash(data:try! Data(contentsOf:url)).map{String(format:"%02x",$0)}.joined(),"samples":records]
            try! JSONSerialization.data(withJSONObject: data, options: [.prettyPrinted,.sortedKeys]).write(to: docs.appendingPathComponent(id + ".json"))
        }}
    }
}
@MainActor class Scene: UIResponder, UIWindowSceneDelegate {
    func scene(_ scene: UIScene, willConnectTo session: UISceneSession, options: UIScene.ConnectionOptions) {
        let app = UIApplication.shared.delegate as! Delegate
        let w = UIWindow(windowScene:scene as! UIWindowScene); w.rootViewController = UIViewController(); w.rootViewController!.view.backgroundColor = .systemBackground; w.makeKeyAndVisible(); app.window = w
        DispatchQueue.main.asyncAfter(deadline:.now()+0.5) { app.capture() }
    }
}
@main struct Main { static func main() { UIApplicationMain(CommandLine.argc, CommandLine.unsafeArgv, nil, NSStringFromClass(Delegate.self)) } }
