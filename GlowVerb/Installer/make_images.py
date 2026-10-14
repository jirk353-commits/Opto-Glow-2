"""Draws Glow Verb installer art: wizard images + icon (cyan / violet theme)."""
import math, sys
from PIL import Image, ImageDraw, ImageFont, ImageFilter
fontpath = sys.argv[1]
CY, VI = (79, 216, 255), (154, 123, 255)

def rings(img, cx, cy, r):
    glow = Image.new('RGBA', img.size, (0, 0, 0, 0)); g = ImageDraw.Draw(glow)
    for k in range(4):
        rr = r * (0.45 + 0.28 * k)
        col = tuple(int(CY[i] + (VI[i] - CY[i]) * k / 3) for i in range(3))
        g.ellipse([cx-rr, cy-rr, cx+rr, cy+rr], outline=col + (230 - 45 * k,), width=max(2, int(r * 0.07)))
    blur = glow.filter(ImageFilter.GaussianBlur(r * 0.08))
    img.alpha_composite(blur); img.alpha_composite(glow)
    d = ImageDraw.Draw(img)
    d.ellipse([cx-r*0.2, cy-r*0.2, cx+r*0.2, cy+r*0.2], fill=(220, 248, 255))

def bg(w, h):
    img = Image.new('RGBA', (w, h)); d = ImageDraw.Draw(img)
    for y in range(h):
        t = y / h
        d.line([(0, y), (w, y)], fill=(int(22 - 14*t), int(29 - 18*t), int(46 - 28*t), 255))
    return img

W, H = 328, 628
img = bg(W, H)
rings(img, W // 2, 230, 120)
d = ImageDraw.Draw(img)
f1 = ImageFont.truetype(fontpath, 40); f2 = ImageFont.truetype(fontpath, 22); f3 = ImageFont.truetype(fontpath, 15)
def ctext(y, s, f, col):
    w = d.textlength(s, font=f); d.text(((W - w) / 2, y), s, font=f, fill=col)
ctext(400, "GLOW VERB", f1, (160, 236, 255))
ctext(460, "ALGORITHMIC REVERB", f3, (150, 165, 190))
d.line([(60, 525), (W - 60, 525)], fill=(60, 90, 140), width=2)
ctext(550, "ROHIT JI", f2, CY)
img.convert('RGB').save('wizard-large.bmp')
img.convert('RGB').resize((164, 314), Image.LANCZOS).save('wizard-large-100.bmp')

s = bg(110, 110); rings(s, 55, 55, 46)
s.convert('RGB').save('wizard-small.bmp')
s.convert('RGB').resize((55, 55), Image.LANCZOS).save('wizard-small-100.bmp')

ic = Image.new('RGBA', (512, 512), (0, 0, 0, 0))
plate = bg(480, 480); m = Image.new('L', (480, 480), 0); ImageDraw.Draw(m).rounded_rectangle([0, 0, 479, 479], 100, fill=255)
ic.paste(plate, (16, 16), m)
rings(ic, 256, 256, 165)
ic.save('icon.ico', sizes=[(16, 16), (24, 24), (32, 32), (48, 48), (64, 64), (128, 128), (256, 256)])
