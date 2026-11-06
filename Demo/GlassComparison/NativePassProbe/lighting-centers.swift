import AppKit
import Foundation
let root=URL(fileURLWithPath:CommandLine.arguments[1]);var result:[String:[[Int]]]=[:]
for file in try FileManager.default.contentsOfDirectory(at:root,includingPropertiesForKeys:nil) where file.pathExtension=="png" && file.lastPathComponent.contains("uniform") {
 let image=NSBitmapImageRep(data:try Data(contentsOf:file))!;var rows:[[Int]]=[]
 for row in 0..<3 {for (x,size) in [(20,44),(90,80),(220,120)] {var c=[Int](repeating:0,count:4);image.getPixel(&c,atX:(x+size/2)*3,y:(80+row*240+size/2)*3);rows.append(Array(c.prefix(3)))}}
 result[file.deletingPathExtension().lastPathComponent]=rows
}
FileHandle.standardOutput.write(try JSONSerialization.data(withJSONObject:result,options:[.prettyPrinted,.sortedKeys]))
