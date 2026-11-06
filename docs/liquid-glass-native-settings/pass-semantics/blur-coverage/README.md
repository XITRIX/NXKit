# Blur coverage isolation (bounded observation)

`capture-blur-coverage.py` uses the independent striped KernelProbe fixture,
fresh filters and three processes: sharp, full blur, and manually alternating
blur-opacity stops at signed distances−32/−24/−8/0. Framebuffer results change
with those stops. However, fitting a single blend weight between the first two
frames does not uniquely establish the native distance interpolation: source
footprints cross the stripe/background band boundaries and black/white stripe
samples imply different effective weights. No production interpolation change
is justified by this fixture. The four-stop portable reconstruction remains
explicitly approximate; these captures are retained rather than presented as
proof of a private shader equation.
