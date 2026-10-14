"""Draws the branded installer wizard images (Inno Setup)."""
import math, sys
from PIL import Image, ImageDraw, ImageFont, ImageFilter
fontpath = sys.argv[1]

def knob(img, cx, cy, r):
    glow = Image.new('RGBA', img.size, (0, 0, 0, 0)); g = ImageDraw.Draw(glow)
    g.ellipse([cx-r*1.35, cy-r*1.35, cx+r*1.35, cy+r*1.35], outline=(255, 170, 60, 150), width=int(r*0.25))
    img.alpha_composite(glow.filter(ImageFilter.GaussianBlur(r*0.12)))
    d = ImageDraw.Draw(img)
    w = max(2, int(r*0.15))
    d.arc([cx-r*1.3, cy-r*1.3, cx+r*1.3, cy+r*1.3], 135, 405, fill=(60, 48, 38), width=w)
    d.arc([cx-r*1.3, cy-r*1.3, cx+r*1.3, cy+r*1.3], 135, 330, fill=(255, 176, 66), width=w)
    d.ellipse([cx-r, cy-r, cx+r, cy+r], fill=(22, 17, 14), outline=(217, 169, 91), width=max(2, int(r*0.06)))
    d.ellipse([cx-r*0.7, cy-r*0.7, cx+r*0.7, cy+r*0.7], fill=(58, 48, 40))
    a = math.radians(330)
    d.line([cx+math.cos(a)*r*0.2, cy+math.sin(a)*r*0.2, cx+math.cos(a)*r*0.9, cy+math.sin(a)*r*0.9],
           fill=(255, 190, 90), width=max(2, int(r*0.14)))

W, H = 328, 628
img = Image.new('RGBA', (W, H))
d = ImageDraw.Draw(img)
for y in range(H):
    t = y / H
    d.line([(0, y), (W, y)], fill=(int(43-28*t), int(35-18*t), int(29-15*t), 255))
knob(img, W//2, 230, 80)
d = ImageDraw.Draw(img)
f1 = ImageFont.truetype(fontpath, 40); f2 = ImageFont.truetype(fontpath, 22); f3 = ImageFont.truetype(fontpath, 15)
def ctext(y, s, f, col):
    w = d.textlength(s, font=f); d.text(((W-w)/2, y), s, font=f, fill=col)
ctext(400, "OPTO GLOW", f1, (255, 200, 120))
ctext(460, "VINTAGE OPTO COMPRESSOR", f3, (200, 180, 150))
d.line([(60, 525), (W-60, 525)], fill=(140, 105, 60), width=2)
ctext(550, "ROHIT JI", f2, (217, 169, 91))
img.convert('RGB').save('wizard-large.bmp')
img.convert('RGB').resize((164, 314), Image.LANCZOS).save('wizard-large-100.bmp')

s = Image.new('RGBA', (110, 110), (32, 26, 21, 255))
knob(s, 55, 55, 34)
s.convert('RGB').save('wizard-small.bmp')
s.convert('RGB').resize((55, 55), Image.LANCZOS).save('wizard-small-100.bmp')
