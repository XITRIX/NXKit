import AppKit
import Foundation
let root=URL(fileURLWithPath:CommandLine.arguments[1]);var result:[String:Any]=[:]
for file in try FileManager.default.contentsOfDirectory(at:root,includingPropertiesForKeys:nil) where file.pathExtension=="png" {
 let image=NSBitmapImageRep(data:try Data(contentsOf:file))!;var rows:[[String:Any]]=[]
 for i in 0..<8 {var samples:[Int]=[];for x in 100*3..<228*3{var c=[Int](repeating:0,count:4);image.getPixel(&c,atX:x,y:(110+95*i)*3);samples.append(c[0])};let mean=Double(samples.reduce(0,+))/Double(samples.count);let variance=samples.map{pow(Double($0)-mean,2)}.reduce(0,+)/Double(samples.count);rows.append(["mean":mean,"sd":sqrt(variance),"minimum":samples.min()!,"maximum":samples.max()!,"samples":samples])};result[file.deletingPathExtension().lastPathComponent]=rows
}
FileHandle.standardOutput.write(try JSONSerialization.data(withJSONObject:result,options:[.prettyPrinted,.sortedKeys]))
