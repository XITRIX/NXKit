import AppKit
import Foundation
let root = URL(fileURLWithPath: CommandLine.arguments[1])
var result: [String:[[Int]]] = [:]
for file in try FileManager.default.contentsOfDirectory(at:root,includingPropertiesForKeys:nil) where file.pathExtension == "png" {
 let image = NSBitmapImageRep(data:try Data(contentsOf:file))!
 var samples:[[Int]]=[]
 for i in 0..<8 {
  var c=[Int](repeating:0,count:4);image.getPixel(&c,atX:164*3,y:(110+95*i)*3)
  samples.append(Array(c.prefix(3)))
 }
 result[file.deletingPathExtension().lastPathComponent]=samples
 if file.deletingPathExtension().lastPathComponent == "baseline" {
  result["__background"]=(0..<8).map { i in
   var c=[Int](repeating:0,count:4);image.getPixel(&c,atX:350*3,y:(110+95*i)*3)
   return Array(c.prefix(3))
  }
 }
}
let data=try JSONSerialization.data(withJSONObject:result,options:[.prettyPrinted,.sortedKeys]);FileHandle.standardOutput.write(data)
