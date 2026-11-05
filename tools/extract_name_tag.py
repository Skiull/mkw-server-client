#!/usr/bin/env python3
# Builds the in-race name tag with Lounge rank icons from the game's own one
# (game_image/blyt/chara_name.brlyt in Race.szs), so nothing of Nintendo's is kept in the
# repo. Run from the repo root before building:
#
#   DolphinTool extract -i "<Mario Kart Wii disc>" -o <dir> -g -s Scene/UI/Race.szs
#   python tools/extract_name_tag.py <dir>/DATA/files/Scene/UI/Race.szs
#
# Writes (git-ignored) assets/game_image/blyt/chara_name.brlyt.json5: the game's tag plus one
# hidden picture per Lounge rank (rank_<name>) where the star rank (handle_text) goes.
import json
import os
import sys

sys.path.insert(0, os.path.dirname(__file__))
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'vendor', 'wuj5'))
from extract_lounge_row import archive_files  # noqa: E402
from gen_leaderboard_layout import RANKS, material, picture  # noqa: E402
from brlyt import unpack_brlyt  # noqa: E402

RANK_SIZE = 34.0


def main():
    files = archive_files(sys.argv[1])
    layout = unpack_brlyt(files['game_image/blyt/chara_name.brlyt'])
    sections = {s['magic']: s for s in layout['sections'] if 'magic' in s}
    tpls, materials = sections['txl1']['tpls'], sections['mat1']['materials']
    root = next(s for s in layout['sections'] if s.get('magic') == 'pan1')
    tag = next(c for c in root['children'] if c['name'] == 'chara_name_null')
    stars = next(c for c in tag['children'] if c['name'] == 'handle_text')
    for r in RANKS:
        p = picture(f'rank_{r}', stars['translation x'], RANK_SIZE, len(materials))
        p['translation y'] = stars['translation y']
        materials.append(material(f'rank_{r}', len(tpls)))
        tpls.append({'name': f'rank_{r}.tpl'})
        tag['children'].append(p)
    with open('assets/game_image/blyt/chara_name.brlyt.json5', 'w', encoding='utf-8', newline='\n') as out:
        json.dump(layout, out, indent=4, ensure_ascii=False)
    print('Wrote the name tag')


if __name__ == '__main__':
    main()
