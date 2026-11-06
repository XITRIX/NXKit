import AppKit
import Foundation
let root=URL(fileURLWithPath:CommandLine.arguments[1]);var result:[String:Any]=[:]
for file in try FileManager.default.contentsOfDirectory(at:root,includingPropertiesForKeys:nil) where file.pathExtension=="png" {
 let image=NSBitmapImageRep(data:try Data(contentsOf:file))!;var rows:[[String:Any]]=[]
 let metadata=try JSONSerialization.jsonObject(with:Data(contentsOf:file.deletingPathExtension().appendingPathExtension("json"))) as! [String:Any];let mutations=metadata["mutations"] as! [[String:Any]]
 for i in 0..<8 {var samples:[Int]=[];for x in 30*3..<370*3{var c=[Int](repeating:0,count:4);image.getPixel(&c,atX:x,y:(110+95*i)*3);samples.append(c[0])};rows.append(["radius":mutations[i]["radius"]!,"startPoints":30,"stepPoints":1.0/3,"samples":samples])};result[file.deletingPathExtension().lastPathComponent]=rows
}
FileHandle.standardOutput.write(try JSONSerialization.data(withJSONObject:result,options:[.prettyPrinted,.sortedKeys]))
