"""Draws the installer icon (amber knob on dark plate) and saves a multi-size .ico."""
import math
import sys
from PIL import Image, ImageDraw, ImageFilter

S = 512
img = Image.new("RGBA", (S, S), (0, 0, 0, 0))
d = ImageDraw.Draw(img)

# rounded dark plate
d.rounded_rectangle([16, 16, S - 16, S - 16], radius=110, fill=(32, 26, 21, 255), outline=(90, 70, 45, 255), width=6)

c = S / 2
# glow
glow = Image.new("RGBA", (S, S), (0, 0, 0, 0))
gd = ImageDraw.Draw(glow)
gd.ellipse([c - 200, c - 200, c + 200, c + 200], outline=(255, 170, 60, 140), width=40)
glow = glow.filter(ImageFilter.GaussianBlur(18))
img.alpha_composite(glow)
d = ImageDraw.Draw(img)

# value arc
start, end = 135, 135 + 230
d.arc([c - 190, c - 190, c + 190, c + 190], start=135, end=405, fill=(60, 48, 38, 255), width=26)
d.arc([c - 190, c - 190, c + 190, c + 190], start=start, end=end, fill=(255, 176, 66, 255), width=26)

# knob body
d.ellipse([c - 140, c - 140, c + 140, c + 140], fill=(22, 17, 14, 255), outline=(217, 169, 91, 255), width=8)
d.ellipse([c - 98, c - 98, c + 98, c + 98], fill=(58, 48, 40, 255))

# pointer
ang = math.radians(end)
x1, y1 = c + math.cos(ang) * 30, c + math.sin(ang) * 30
x2, y2 = c + math.cos(ang) * 125, c + math.sin(ang) * 125
d.line([x1, y1, x2, y2], fill=(255, 190, 90, 255), width=22)

out = sys.argv[1]
img.save(out, sizes=[(16, 16), (24, 24), (32, 32), (48, 48), (64, 64), (128, 128), (256, 256)])
img.save(out.replace(".ico", ".png"))
