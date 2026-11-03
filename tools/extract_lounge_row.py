#!/usr/bin/env python3
# Builds the Lounge leaderboard row from the Mario Kart Channel rankings row in the game's
# files (ranking/blyt/common_w078_ghost_top10.brlyt in Channel.szs), so nothing of
# Nintendo's is kept in the repo. Run from the repo root before building:
#
#   DolphinTool extract -i "<Mario Kart Wii disc>" -o <dir> -g -s Scene/UI/Channel.szs
#   python tools/extract_lounge_row.py <dir>/DATA/files/Scene/UI/Channel.szs
#
# Writes (git-ignored) assets/globe/blyt/lounge_row.brlyt.json5 and the row's textures in
# assets/globe/timg/, and assets/globe/ctrl/LoungeRow.brctr.json5. Changes to the Channel
# row: the time shows the LR, the controller is hidden, the flag is one of the game's flags
# (one hidden picture each, flag_<Wii country code>), and the rank icon sits left of the LR.
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

ROW = 'ranking/blyt/common_w078_ghost_top10.brlyt'
ROW_CTRL = 'ranking/ctrl/RankingDetailItem.brctr'

# Where the added pictures go, in the row's coordinates (the LR is right-aligned at x ~217,
# so a 5-digit LR starts around x ~107).
FLAG_X, FLAG_W, FLAG_H = 12.75, 33.0, 22.0
PAD_X, PAD_SIZE = 56.0, 28.0
RANK_X, RANK_SIZE = 84.0, 26.0

# Controller pictures (the game's "connect a controller" art, pad_recognize/timg): a body and
# a coloured light on top. Panes pad_<name>_body / pad_<name>_light.
PADS = [('wheel', 'tt_remocon_han'), ('nunchuk', 'tt_remocon_nun'), ('classic', 'tt_clacon'),
        ('gc', 'tt_gccon')]


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

    for _, tex in PADS:
        for part in ('body', 'light'):
            name = f'{tex}_{part}.tpl'
            data = next(c for p, c in files.items() if p.endswith('/timg/' + name))
            open(os.path.join('assets/globe/timg', name), 'wb').write(data)

    tpls = sections['txl1']['tpls']
    materials = sections['mat1']['materials']

    # The time is drawn dark on a light outline; the LR is the rank colour (set at runtime)
    # on a black outline, like the position and name.
    time = next(m for m in materials if m['name'] == 'time')
    for c, v in zip('rgba', (0, 0, 0, 0)):
        time[f'tev color 0 {c}'] = v
    for c in 'rgba':
        time[f'tev color 1 {c}'] = 255
    first_tex = len(tpls)
    tpls += [{'name': f'rank_{r}.tpl'} for r in RANKS] + [{'name': f + '.tpl'} for f in FLAGS]

    root = next(s for s in layout['sections'] if s.get('magic') == 'pan1')
    row = next(c for c in root['children'] if c['name'] == 'null_all')
    for child in row['children']:
        if child['name'] in ('controller', 'national_flag'):
            child['flags']['visible'] = False

    for i, f in enumerate(FLAGS):
        p = picture('flag_' + f.split('_')[1], FLAG_X, FLAG_H, len(materials))
        p['size x'] = FLAG_W
        p['translation y'] = 4.0
        materials.append(material('flag_' + f.split('_')[1], first_tex + len(RANKS) + i))
        row['children'].append(p)
    for pad, tex in PADS:
        for part in ('body', 'light'):
            p = picture(f'pad_{pad}_{part}', PAD_X, PAD_SIZE, len(materials))
            p['translation y'] = 4.0
            materials.append(material(f'pad_{pad}_{part}', len(tpls)))
            tpls.append({'name': f'{tex}_{part}.tpl'})
            row['children'].append(p)
    for i, r in enumerate(RANKS):
        p = picture(f'rank_{r}', RANK_X, RANK_SIZE, len(materials))
        p['translation y'] = 4.0
        materials.append(material(f'rank_{r}', first_tex + i))
        row['children'].append(p)

    with open('assets/globe/blyt/lounge_row.brlyt.json5', 'w', encoding='utf-8', newline='\n') as out:
        json.dump(layout, out, indent=4, ensure_ascii=False)

    # The rows where the Channel puts them (RankingDetailItem's Item1-10), named Row0-9.
    vanilla = unpack_brctr(files[ROW_CTRL])
    ctrl = {
        'main brlyt': 'lounge_row', 'bmg': 'Menu', 'picture source brlyt': '',
        'groups': [], 'animations': [], 'variants': [], 'messages': [], 'pictures': [],
    }
    for i in range(10):
        v = dict(next(v for v in vanilla['variants'] if v['name'] == f'Item{i + 1}'))
        v.update({'name': f'Row{i}', 'animated': False, 'first message': 0, 'message count': 0,
                  'first picture': 0, 'picture count': 0})
        ctrl['variants'].append(v)
    with open('assets/globe/ctrl/LoungeRow.brctr.json5', 'w', encoding='utf-8', newline='\n') as out:
        json.dump(ctrl, out, indent=4, ensure_ascii=False)
    print('Wrote the Lounge row and', len(sections['txl1']['tpls']) - len(RANKS) - len(FLAGS), 'textures')


if __name__ == '__main__':
    main()
