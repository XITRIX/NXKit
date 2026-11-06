import AppKit
import Foundation
let root=URL(fileURLWithPath:CommandLine.arguments[1]);var result:[String:[[[Int]]]]=[:]
for file in try FileManager.default.contentsOfDirectory(at:root,includingPropertiesForKeys:nil) where file.pathExtension=="png" {
 let image=NSBitmapImageRep(data:try Data(contentsOf:file))!;var rows:[[[Int]]]=[]
 for row in 0..<6 {var sizes:[[Int]]=[];for x in [58,300] {var c=[Int](repeating:0,count:4);image.getPixel(&c,atX:x*3,y:Int((92.5+Double(row)*145)*3));sizes.append(Array(c.prefix(3)))};rows.append(sizes)};result[file.deletingPathExtension().lastPathComponent]=rows
}
let data=try JSONSerialization.data(withJSONObject:result,options:[.prettyPrinted,.sortedKeys]);FileHandle.standardOutput.write(data)
