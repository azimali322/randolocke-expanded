#!/usr/bin/env python3
"""Render a pokeemerald map layout to a PNG, so terrain edits can be checked without Porymap."""
import json, struct, sys
from pathlib import Path
from PIL import Image

ROOT = Path('/Users/azima/Desktop/Python_Fun_Scripts/randolocke-expanded')
NUM_TILES_IN_PRIMARY = 512
NUM_METATILES_IN_PRIMARY = 512
NUM_PALS_IN_PRIMARY = 6

def tileset_dir(sym):
    name = sym.replace('gTileset_', '')
    snake = ''.join('_'+c.lower() if c.isupper() else c for c in name).lstrip('_')
    for sub in ('primary', 'secondary'):
        p = ROOT/'data/tilesets'/sub/snake
        if p.is_dir(): return p
    raise SystemExit(f'tileset dir not found for {sym} ({snake})')

def read_pal(p):
    lines = p.read_text().split('\n')[3:]
    out = []
    for l in lines[:16]:
        l = l.strip()
        if not l: break
        out.append(tuple(int(v) for v in l.split()))
    while len(out) < 16: out.append((0,0,0))
    return out

def load(sym):
    d = tileset_dir(sym)
    return {
        'tiles': (d/'tiles.4bpp').read_bytes(),
        'metatiles': (d/'metatiles.bin').read_bytes(),
        'pals': {i: read_pal(d/'palettes'/f'{i:02d}.pal') for i in range(16)
                 if (d/'palettes'/f'{i:02d}.pal').exists()},
    }

def draw_tile(px, ox, oy, entry, prim, sec, opaque):
    tid = entry & 0x3FF
    xflip, yflip = bool(entry & 0x400), bool(entry & 0x800)
    pal_i = (entry >> 12) & 0xF
    src, idx = (prim, tid) if tid < NUM_TILES_IN_PRIMARY else (sec, tid - NUM_TILES_IN_PRIMARY)
    pal = (prim if pal_i < NUM_PALS_IN_PRIMARY else sec)['pals'].get(pal_i) or [(0,0,0)]*16
    data = src['tiles'][idx*32:(idx+1)*32]
    if len(data) < 32: return
    for y in range(8):
        for x in range(8):
            b = data[y*4 + x//2]
            c = (b & 0xF) if x % 2 == 0 else (b >> 4)
            if c == 0 and not opaque: continue
            sx = ox + (7-x if xflip else x)
            sy = oy + (7-y if yflip else y)
            px[sx, sy] = pal[c]

def render(layout_id, out, marks=()):
    L = {x['id']: x for x in json.loads((ROOT/'data/layouts/layouts.json').read_text())['layouts']}
    l = L[layout_id]
    w, h = l['width'], l['height']
    prim, sec = load(l['primary_tileset']), load(l['secondary_tileset'])
    img = Image.new('RGB', (w*16, h*16), (0,0,0)); px = img.load()
    blocks = (ROOT/l['blockdata_filepath']).read_bytes()
    for my in range(h):
        for mx in range(w):
            mid = struct.unpack_from('<H', blocks, (my*w+mx)*2)[0] & 0x3FF
            src, i = (prim, mid) if mid < NUM_METATILES_IN_PRIMARY else (sec, mid - NUM_METATILES_IN_PRIMARY)
            base = i*16
            for layer in (0, 1):
                for q in range(4):
                    e = struct.unpack_from('<H', src['metatiles'], base + (layer*4+q)*2)[0]
                    draw_tile(px, mx*16 + (q%2)*8, my*16 + (q//2)*8, e, prim, sec, layer == 0)
    img = img.resize((w*32, h*32), Image.NEAREST)
    if marks:
        from PIL import ImageDraw
        d = ImageDraw.Draw(img)
        for (x0,y0,x1,y1) in marks:
            d.rectangle([x0*32, y0*32, (x1+1)*32-1, (y1+1)*32-1], outline=(255,0,0), width=3)
    img.save(out); print('wrote', out, img.size)

# ------------------------------------------------------------------- checks ---
# Water tiles are collision 0 -- that is what makes them surfable -- so collision alone
# does NOT tell you whether an NPC can stand somewhere. Read the metatile's behaviour.
# Getting this wrong is how the Route 103 fisherman ended up standing in the sea.

def _behaviors():
    import re
    src = (ROOT / 'include/constants/metatile_behaviors.h').read_text()
    body = re.search(r'\{(.*?)\};', src, re.S).group(1)
    out, val = {}, 0
    for line in body.split('\n'):
        line = line.split('//')[0].strip().rstrip(',')
        if not line:
            continue
        if '=' in line:
            name, v = line.split('=', 1)
            name, val = name.strip(), int(v.strip(), 0)
        else:
            name = line
        out[val] = name
        val += 1
    return out


def check(layout_id, coords):
    """Print metatile, collision, elevation and behaviour for each 'x,y'."""
    import struct as _s
    names = _behaviors()
    L = {x['id']: x for x in json.loads((ROOT/'data/layouts/layouts.json').read_text())['layouts']}
    l = L[layout_id]
    w, h = l['width'], l['height']
    blocks = (ROOT/l['blockdata_filepath']).read_bytes()
    attrs = {}
    for which, key in (('primary', 'primary_tileset'), ('secondary', 'secondary_tileset')):
        attrs[which] = (tileset_dir(l[key])/'metatile_attributes.bin').read_bytes()
    for c in coords:
        x, y = (int(v) for v in c.split(','))
        v = _s.unpack_from('<H', blocks, (y*w + x)*2)[0]
        mid, coll, elev = v & 0x3FF, (v >> 10) & 3, (v >> 12) & 0xF
        buf, i = (attrs['primary'], mid) if mid < NUM_METATILES_IN_PRIMARY else \
                 (attrs['secondary'], mid - NUM_METATILES_IN_PRIMARY)
        bh = _s.unpack_from('<H', buf, i*2)[0] & 0xFF
        name = names.get(bh, '?')
        ok = 'STANDABLE' if (coll == 0 and name == 'MB_NORMAL') else 'NOT standable'
        print(f'  ({x:3d},{y:3d})  metatile 0x{mid:03X}  coll {coll}  elev {elev:2d}  '
              f'{name:22s} {ok}')


if __name__ == '__main__':
    if sys.argv[1] == 'check':
        check(sys.argv[2], sys.argv[3:])
    else:
        render(sys.argv[1], sys.argv[2])
