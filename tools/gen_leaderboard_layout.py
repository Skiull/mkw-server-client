#!/usr/bin/env python3
# Writes the leaderboard row layout (assets/globe/blyt/leaderboard_row.brlyt.json5) and its
# control file (assets/globe/ctrl/LeaderboardRow.brctr.json5). Run from the repo root.
# They live in 'globe' so the flag pictures find the game's own flag textures
# (globe/timg/<group>_<Wii country code>_<name>.tpl in Globe.szs, loaded on the online menus).
# Each row has one hidden picture per flag, named flag_<Wii country code>; the page shows one.
import json

FLAGS = ['001_001_japan', '002_136_koreaRep', '150_049_usa', '151_018_canada', '152_036_mexico', '153_008_anguilla', '153_009_antigua', '153_010_argentina', '153_011_aruba', '153_012_bahamas', '153_013_barbados', '153_014_belize', '153_015_bolivia', '153_016_brazil', '153_017_britishVirginIslands', '153_019_caymanIslands', '153_020_chile', '153_021_colombia', '153_022_costaRica', '153_023_dominica', '153_024_dominicanRep', '153_025_ecuador', '153_026_elSalvador', '153_028_grenada', '153_030_guatemala', '153_031_guyana', '153_032_haiti', '153_033_honduras', '153_034_jamaica', '153_037_montserrat', '153_038_netherlandsAntilles', '153_039_nicaragua', '153_040_panama', '153_041_paraguay', '153_042_peru', '153_043_stKitts', '153_044_stLucia', '153_045_stVincent', '153_046_suriname', '153_047_trinidadAndTobago', '153_048_turksCaicosIslands', '153_050_uruguay', '153_051_usVirginIslands', '153_052_venezuela', '200_065_australia', '200_082_ireland', '200_095_newZealand', '200_110_uk', '201_066_austria', '201_078_germany', '201_108_switzerland', '202_067_belgium', '202_077_france', '202_088_luxembourg', '203_083_italy', '204_098_portugal', '204_105_spain', '205_094_netherlands', '206_074_denmark', '206_076_finland', '206_079_greece', '206_096_norway', '206_107_sweden']

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
for i, f in enumerate(FLAGS):
    flag = picture('flag_' + f.split('_')[1], 150, 24, 1 + len(RANKS) + i)
    flag['size x'] = 36.0
    children.append(flag)
children += [
    text('name', -50, 300, 'center left'),
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
        {'magic': 'txl1', 'tpls': [{'name': f'rank_{r}.tpl'} for r in RANKS] + [{'name': f + '.tpl'} for f in FLAGS]},
        {'magic': 'fnl1', 'brfnts': [{'name': 'tt_kart_font_rodan_ntlg_pro_b.brfnt'}]},
        {'magic': 'mat1', 'materials': [material('text')] + [material(f'rank_{r}', i) for i, r in enumerate(RANKS)] + [material('flag_' + f.split('_')[1], len(RANKS) + i) for i, f in enumerate(FLAGS)]},
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

with open('assets/globe/blyt/leaderboard_row.brlyt.json5', 'w', encoding='utf-8', newline='\n') as f:
    json.dump(layout, f, indent=4, ensure_ascii=False)
with open('assets/globe/ctrl/LeaderboardRow.brctr.json5', 'w', encoding='utf-8', newline='\n') as f:
    json.dump(ctrl, f, indent=4, ensure_ascii=False)
