#!/usr/bin/env python3
# Writes the Lounge leaderboard row (assets/globe/blyt/lounge_row.brlyt.json5) and its control
# file (assets/globe/ctrl/LoungeRow.brctr.json5), in the style of the Mario Kart Channel
# rankings: a wide bar with "#N", a Mii, the name, the country flag, the rank icon and the LR.
# Run from the repo root. Textures are the rank icons (shipped) and the game's flags and
# Mii placeholder (globe/timg in Globe.szs, loaded on the online menus).
import json
import os
import sys

sys.path.insert(0, os.path.dirname(__file__))
from gen_leaderboard_layout import FLAGS, RANKS, material, pane, picture  # noqa: E402

WHITE = 255


def text(name, x, w, h, position, font):
    p = pane('txt1', name, x, w, h)
    p.update({
        'maximum string size': 128, 'string size': 4, 'material': 0, 'font': 0,
        'text position': position, 'text alignment': 'unspecified', 'text': '-',
    })
    for side in ['top', 'bottom']:
        for c in 'rgba':
            p[f'{side} color {c}'] = WHITE
    p.update({'font size x': font, 'font size y': font * 1.18, 'character space': 0.0, 'line space': 0.0})
    return p


def bar(name, w, h, top, bottom, alpha):
    p = pane('pic1', name, 0, w, h)
    for corner, rgb in [('top left', top), ('top right', top), ('bottom left', bottom), ('bottom right', bottom)]:
        for c, v in zip('rgba', (*rgb, alpha)):
            p[f'vertex color {corner} {c}'] = v
    p['material'] = 1
    p['uv sets'] = [{'top left u': 0.0, 'top left v': 0.0, 'top right u': 1.0, 'top right v': 0.0,
                     'bottom left u': 0.0, 'bottom left v': 1.0, 'bottom right u': 1.0, 'bottom right v': 1.0}]
    return p


TEX = [f'rank_{r}.tpl' for r in RANKS] + [f + '.tpl' for f in FLAGS] + ['tt_mario_64x64.tpl']
MII_TEX = len(TEX) - 1

# Materials: 0 text, 1 bar (vertex colors only), then the pictures'.
materials = [material('text'), material('bar')]
children = [
    bar('bar', 560, 30, (255, 255, 255), (150, 196, 255), 200),
    bar('bar_line', 560, 2, (40, 40, 110), (40, 40, 110), 255),
]
children[1]['translation y'] = -16.0
children.append(text('position', -232, 74, 30, 'center', 26.0))

mii = picture('mii', -172, 30, len(materials))
mii['flags']['visible'] = True
materials.append(material('mii', MII_TEX))
children.append(mii)

children.append(text('name', -38, 230, 30, 'center', 24.0))

for i, f in enumerate(FLAGS):
    flag = picture('flag_' + f.split('_')[1], 98, 22, len(materials))
    flag['size x'] = 33.0
    materials.append(material('flag_' + f.split('_')[1], len(RANKS) + i))
    children.append(flag)

for i, r in enumerate(RANKS):
    children.append(picture(f'rank_{r}', 142, 28, len(materials)))
    materials.append(material(f'rank_{r}', i))

children.append(text('lr', 222, 130, 30, 'center right', 26.0))

root = pane('pan1', 'RootPane', 0, 608, 456)
root['children'] = children

layout = {
    'version': 10,
    'sections': [
        {'magic': 'lyt1', 'centered': True, 'size x': 608.0, 'size y': 456.0},
        {'magic': 'txl1', 'tpls': [{'name': t} for t in TEX]},
        {'magic': 'fnl1', 'brfnts': [{'name': 'tt_kart_font_rodan_ntlg_pro_b.brfnt'}]},
        {'magic': 'mat1', 'materials': materials},
        root,
        {'magic': 'grp1', 'name': 'RootGroup', 'panes': []},
    ],
}

ctrl = {
    'main brlyt': 'lounge_row', 'bmg': 'Menu', 'picture source brlyt': '',
    'groups': [], 'animations': [], 'variants': [], 'messages': [], 'pictures': [],
}
for i in range(10):
    v = {'name': f'Row{i}', 'opacity': 255, 'animated': False, 'animation delay': 0.0}
    for aspect in ['4:3', '16:9']:
        v.update({f'translation x {aspect}': 0.0, f'translation y {aspect}': 160.0 - i * 33.0,
                  f'translation z {aspect}': 0.0, f'scale x {aspect}': 1.0, f'scale y {aspect}': 1.0})
    v.update({'first message': 0, 'message count': 0, 'first picture': 0, 'picture count': 0})
    ctrl['variants'].append(v)

with open('assets/globe/blyt/lounge_row.brlyt.json5', 'w', encoding='utf-8', newline='\n') as f:
    json.dump(layout, f, indent=4, ensure_ascii=False)
with open('assets/globe/ctrl/LoungeRow.brctr.json5', 'w', encoding='utf-8', newline='\n') as f:
    json.dump(ctrl, f, indent=4, ensure_ascii=False)
