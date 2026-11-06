"""Create an optimized scheme while preserving subsequent launch customizations."""
import copy
import pathlib
import sys
import xml.etree.ElementTree as ET

project, target = pathlib.Path(sys.argv[1]), sys.argv[2]
schemes = project / "xcshareddata/xcschemes"
destination = schemes / f"{target} Performance.xcscheme"
generated = ET.parse(schemes / f"{target}.xcscheme")
existing = ET.parse(destination) if destination.exists() else None
for action in ("LaunchAction", "ProfileAction"):
    node = generated.getroot().find(action)
    node.set("buildConfiguration", "RelWithDebInfo")
    # These values match CMake's OFF/Disabled Xcode scheme properties.
    node.set("enableGPUValidationMode", "1")
    node.set("enableGPUFrameCaptureMode", "3")
    previous = existing.getroot().find(action) if existing is not None else None
    if previous is not None:
        for name in ("EnvironmentVariables", "CommandLineArguments"):
            current = node.find(name)
            if current is not None:
                node.remove(current)
            saved = previous.find(name)
            if saved is not None:
                node.append(copy.deepcopy(saved))
    env = node.find("EnvironmentVariables")
    if env is None:
        env = ET.SubElement(node, "EnvironmentVariables")
    if existing is None:
        # Start paced and without shader capture; later benchmark overrides are
        # user-owned and survive regenerated CMake target identifiers.
        for item in list(env):
            if item.get("key", "").startswith(("NXKIT_METAL_", "NXKIT_UNCAPPED", "NXKIT_CONTINUOUS_RENDERING", "NXKIT_RENDER_", "MTL_DEBUG_LAYER")):
                env.remove(item)
        ET.SubElement(env, "EnvironmentVariable", key="NXKIT_UNCAPPED", value="0", isEnabled="YES")
    for item in list(env):
        if item.get("key") == "MTL_DEBUG_LAYER":
            env.remove(item)
    ET.SubElement(env, "EnvironmentVariable", key="MTL_DEBUG_LAYER", value="0", isEnabled="YES")
ET.indent(generated, space="   ")
generated.write(destination, encoding="UTF-8", xml_declaration=True)
