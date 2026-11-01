#!/usr/bin/env python3
# Writes the leaderboard row layout (assets/control/blyt/leaderboard_row.brlyt.json5) and its
# control file (assets/control/ctrl/LeaderboardRow.brctr.json5). Run from the repo root.
import json

RANKS = ['iron', 'copper', 'bronze', 'silver', 'gold', 'platinum', 'emerald', 'ruby', 'diamond',
         'pearl', 'master', 'grandmaster']
WHITE = 255


def material(name, texture=None):
    m = {'name': name}
    for i, (r, g, b, a) in enumerate([(0, 0, 0, 0), (WHITE,) * 4, (WHITE,) * 4]):
        m.update({f'tev color {i} r': r, f'tev color {i} g': g, f'tev color {i} b': b, f'tev color {i} a': a})
    for i in range(4):
        for c in 'rgba':
            m[f'tev k color {i} {c}'] = WHITE
    m['attributes'] = {}
    if texture is not None:
        m['attributes'] = {
            'texture maps': [{'texture index': texture, 's': 0, 't': 0}],
            'texture srts': [{'translate x': 0.0, 'translate y': 0.0, 'rotate': 0.0, 'scale x': 1.0, 'scale y': 1.0}],
            'texture uv gens': [{'type': 1, 'source': 4, 'matrix': 30}],
        }
    return m


def pane(magic, name, x, w, h, visible=True):
    return {
        'magic': magic,
        'flags': {'visible': visible, 'influenced alpha': False, 'location adjust': False},
        'base position': 'center', 'opacity': 255, 'name': name, 'user data': '',
        'translation x': float(x), 'translation y': 0.0, 'translation z': 0.0,
        'rotation x': 0.0, 'rotation y': 0.0, 'rotation z': 0.0,
        'scale x': 1.0, 'scale y': 1.0, 'size x': float(w), 'size y': float(h),
    }


def picture(name, x, size, mat):
    p = pane('pic1', name, x, size, size, visible=False)
    for corner in ['top left', 'top right', 'bottom left', 'bottom right']:
        for c in 'rgba':
            p[f'vertex color {corner} {c}'] = WHITE
    p['material'] = mat
    p['uv sets'] = [{'top left u': 0.0, 'top left v': 0.0, 'top right u': 1.0, 'top right v': 0.0,
                     'bottom left u': 0.0, 'bottom left v': 1.0, 'bottom right u': 1.0, 'bottom right v': 1.0}]
    return p


def text(name, x, w, position):
    p = pane('txt1', name, x, w, 30)
    p.update({
        'maximum string size': 128, 'string size': 4, 'material': 0, 'font': 0,
        'text position': position, 'text alignment': 'unspecified', 'text': '-',
    })
    for side in ['top', 'bottom']:
        for c in 'rgba':
            p[f'{side} color {c}'] = WHITE
    p.update({'font size x': 20.0, 'font size y': 23.7, 'character space': 0.0, 'line space': 0.0})
    return p


children = [text('position', -262, 56, 'center right')]
children += [picture(f'rank_{r}', -212, 28, 1 + i) for i, r in enumerate(RANKS)]
children += [
    text('name', -50, 300, 'center left'),
    text('country', 150, 50, 'center'),
    text('lr', 245, 100, 'center right'),
]
# Text boxes are positioned by their centers: 'name' spans -200..100.
children[-3]['translation x'] = -40.0

root = pane('pan1', 'RootPane', 0, 608, 456)
root['children'] = children

layout = {
    'version': 10,
    'sections': [
        {'magic': 'lyt1', 'centered': True, 'size x': 608.0, 'size y': 456.0},
        {'magic': 'txl1', 'tpls': [{'name': f'rank_{r}.tpl'} for r in RANKS]},
        {'magic': 'fnl1', 'brfnts': [{'name': 'tt_kart_font_rodan_ntlg_pro_b.brfnt'}]},
        {'magic': 'mat1', 'materials': [material('text')] + [material(f'rank_{r}', i) for i, r in enumerate(RANKS)]},
        root,
        {'magic': 'grp1', 'name': 'RootGroup', 'panes': []},
    ],
}

ctrl = {
    'main brlyt': 'leaderboard_row', 'bmg': 'Menu', 'picture source brlyt': '',
    'groups': [], 'animations': [],
    'variants': [],
    'messages': [], 'pictures': [],
}
for i in range(10):
    y = 150.0 - i * 32.0
    v = {'name': f'Row{i}', 'opacity': 255, 'animated': False, 'animation delay': 0.0}
    for aspect in ['4:3', '16:9']:
        v.update({f'translation x {aspect}': 0.0, f'translation y {aspect}': y, f'translation z {aspect}': 0.0,
                  f'scale x {aspect}': 1.0, f'scale y {aspect}': 1.0})
    v.update({'first message': 0, 'message count': 0, 'first picture': 0, 'picture count': 0})
    ctrl['variants'].append(v)

with open('assets/control/blyt/leaderboard_row.brlyt.json5', 'w', encoding='utf-8', newline='\n') as f:
    json.dump(layout, f, indent=4, ensure_ascii=False)
with open('assets/control/ctrl/LeaderboardRow.brctr.json5', 'w', encoding='utf-8', newline='\n') as f:
    json.dump(ctrl, f, indent=4, ensure_ascii=False)
