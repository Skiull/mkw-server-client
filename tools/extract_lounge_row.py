#!/usr/bin/env python3
# Builds the Lounge leaderboard row from the online race results row in the game's files
# (result/blyt/position_12players.brlyt in Race.szs, placed like ResultVS), so nothing of
# Nintendo's is kept in the repo. Run from the repo root before building:
#
#   DolphinTool extract -i "<Mario Kart Wii disc>" -o <dir> -g -s Scene/UI/Race.szs
#   python tools/extract_lounge_row.py <dir>/DATA/files/Scene/UI/Race.szs
#
# Writes (git-ignored) assets/globe/blyt/lounge_row.brlyt.json5 and the row's textures in
# assets/globe/timg/, and assets/globe/ctrl/LoungeRow.brctr.json5. Changes to the results
# row: the score shows the LR and "VR" becomes "LR" (set at runtime), the points gained and
# the time are hidden, and the flag (one hidden picture per game flag, flag_<Wii country
# code>), controller and rank icon sit between the name and the LR.
import json
import os
import sys

sys.path.insert(0, os.path.dirname(__file__))
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'vendor', 'wuj5'))
from gen_leaderboard_layout import FLAGS, RANKS, material, picture  # noqa: E402
from brctr import unpack_brctr  # noqa: E402
from brlyt import unpack_brlyt  # noqa: E402
from u8 import unpack_u8  # noqa: E402
from yaz import unpack_yaz  # noqa: E402

ROW = 'result/blyt/position_12players.brlyt'
ROW_CTRL = 'result/ctrl/ResultVS.brctr'

# Where the added pictures go, in the row's coordinates: the name ends at x ~49 and the
# LR starts at x 146.
FLAG_X, FLAG_W, FLAG_H = 66.0, 33.0, 22.0
PAD_X, PAD_SIZE = 98.0, 28.0
RANK_X, RANK_SIZE = 128.0, 26.0
ICON_Y = 3.0

# The results rows are a little big for the page (the first one cuts into the title bar):
# 90% size, from y 150 down, spaced to match.
ROW_SCALE = 0.9
ROW_TOP, ROW_STEP = 150.0, 34.0 * ROW_SCALE

# Panes of the results row that aren't used.
HIDDEN = ['get_point', 'time', 'handle_text', 'team_color_c', 'select_gold_01', 'select_gold_02',
          'select_base']

# Controller icons (assets/globe/timg/ctrl_<name>.tpl, made with tools/png2tpl.py from
# assets/globe/controller-icons-src). Panes pad_<name>; pane names are 16 bytes at most.
PADS = ['whl', 'nun', 'cla', 'ccp', 'gc']


def archive_files(path):
    data = open(path, 'rb').read()
    if data[:4] == b'Yaz0':
        data = unpack_yaz(data)
    files = {}

    def walk(node, prefix):
        for child in node.get('children', []):
            name = prefix + child['name']
            if child['is_dir']:
                walk(child, name + '/')
            else:
                files[name] = child['content']
    walk(unpack_u8(data), '')
    return files


def main():
    files = archive_files(sys.argv[1])
    layout = unpack_brlyt(files[ROW])
    sections = {s['magic']: s for s in layout['sections'] if 'magic' in s}

    # The row's own textures, from the same archive.
    os.makedirs('assets/globe/timg', exist_ok=True)
    for tpl in sections['txl1']['tpls']:
        name = tpl['name']
        data = next(c for p, c in files.items() if p.endswith('/timg/' + name))
        open(os.path.join('assets/globe/timg', name), 'wb').write(data)

    tpls = sections['txl1']['tpls']
    materials = sections['mat1']['materials']

    # The name is drawn dark on a light outline in the results: make it light on a black
    # outline, like the position and the LR.
    name = next(m for m in materials if m['name'] == 'player_name')
    for c, v in zip('rgba', (0, 0, 0, 0)):
        name[f'tev color 0 {c}'] = v
    for c in 'rgba':
        name[f'tev color 1 {c}'] = 255
    first_tex = len(tpls)
    tpls += [{'name': f'rank_{r}.tpl'} for r in RANKS] + [{'name': f + '.tpl'} for f in FLAGS]

    root = next(s for s in layout['sections'] if s.get('magic') == 'pan1')
    row = next(c for c in root['children'] if c['name'] == 'ranking_null')
    for child in row['children']:
        if child['name'] in HIDDEN:
            child['flags']['visible'] = False

    for i, f in enumerate(FLAGS):
        p = picture('flag_' + f.split('_')[1], FLAG_X, FLAG_H, len(materials))
        p['size x'] = FLAG_W
        p['translation y'] = ICON_Y
        materials.append(material('flag_' + f.split('_')[1], first_tex + len(RANKS) + i))
        row['children'].append(p)
    for pad in PADS:
        p = picture(f'pad_{pad}', PAD_X, PAD_SIZE, len(materials))
        p['translation y'] = ICON_Y
        materials.append(material(f'pad_{pad}', len(tpls)))
        tpls.append({'name': f'ctrl_{pad}.tpl'})
        row['children'].append(p)
    for i, r in enumerate(RANKS):
        p = picture(f'rank_{r}', RANK_X, RANK_SIZE, len(materials))
        p['translation y'] = ICON_Y
        materials.append(material(f'rank_{r}', first_tex + i))
        row['children'].append(p)

    with open('assets/globe/blyt/lounge_row.brlyt.json5', 'w', encoding='utf-8', newline='\n') as out:
        json.dump(layout, out, indent=4, ensure_ascii=False)

    # The VS results rows (rank1-10), smaller and a little lower, named Row0-9.
    vanilla = unpack_brctr(files[ROW_CTRL])
    ctrl = {
        'main brlyt': 'lounge_row', 'bmg': 'Menu', 'picture source brlyt': '',
        'groups': [], 'animations': [], 'variants': [], 'messages': [], 'pictures': [],
    }
    for i in range(10):
        v = dict(next(v for v in vanilla['variants'] if v['name'] == f'rank{i + 1}'))
        v.update({'name': f'Row{i}', 'animated': False, 'first message': 0, 'message count': 0,
                  'first picture': 0, 'picture count': 0})
        for aspect in ['4:3', '16:9']:
            v[f'translation y {aspect}'] = ROW_TOP - i * ROW_STEP
            v[f'scale x {aspect}'] = v[f'scale y {aspect}'] = ROW_SCALE
        ctrl['variants'].append(v)
    with open('assets/globe/ctrl/LoungeRow.brctr.json5', 'w', encoding='utf-8', newline='\n') as out:
        json.dump(ctrl, out, indent=4, ensure_ascii=False)
    print('Wrote the Lounge row and', len(sections['txl1']['tpls']) - len(RANKS) - len(FLAGS), 'textures')


if __name__ == '__main__':
    main()
