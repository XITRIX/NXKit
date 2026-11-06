// Read raw sRGB PNG components. Do not convert through NSColor.
import AppKit
import Foundation
let directory=URL(fileURLWithPath:CommandLine.arguments[1])
let distances:[Double]=[-1.25,-0.75,-0.25,0.25,0.75,1.25]
var result:[String:Any]=[:]
for file in try FileManager.default.contentsOfDirectory(at:directory,includingPropertiesForKeys:nil) where file.pathExtension=="png" {
 let image=NSBitmapImageRep(data:try Data(contentsOf:file))!
 let metadata=try JSONSerialization.jsonObject(with:Data(contentsOf:file.deletingPathExtension().appendingPathExtension("json"))) as! [String:Any]
 let scale=(metadata["scale"] as! NSNumber).doubleValue
 let frames=metadata["frames"] as! [[String:Any]]
 var shapes:[[String:Any]]=[]
 for entry in frames {
  let rect=NSRectFromString(entry["frame"] as! String)
  var samples:[[String:Any]]=[]
  for angle in stride(from:0.0,to:360.0,by:90.0) {
   let radians=angle*Double.pi/180
   for distance in distances {
    let radius=Double(rect.width/2)+distance
    let px=Int(floor((Double(rect.midX)+cos(radians)*radius)*scale))
    let py=Int(floor((Double(rect.midY)+sin(radians)*radius)*scale))
    guard px>=0 && py>=0 && px<image.pixelsWide && py<image.pixelsHigh else {continue}
    var color=[Int](repeating:0,count:4); image.getPixel(&color,atX:px,y:py)
    samples.append(["angle":angle,"distance":distance,"pixel":[px,py],"rgb":Array(color.prefix(3))])
   }
  }
  shapes.append(["frame":entry["frame"]!,"gray":entry["gray"]!,"samples":samples])
 }
 result[file.deletingPathExtension().lastPathComponent]=shapes
}
let data=try JSONSerialization.data(withJSONObject:result,options:[.sortedKeys]);FileHandle.standardOutput.write(data)
