"""Check C data and arithmetic bounds. This does NOT execute or compile C."""
import json
import pathlib
import re

root = pathlib.Path(__file__).resolve().parents[1]
source = (root / 'src/game.c').read_text(encoding='utf-8')

def rows(symbol):
    match = re.search(r'\b' + symbol + r'\[[^]]*\]\s*=\s*\{(.*?)\};', source, re.S)
    assert match, symbol
    return [tuple(map(int, re.findall(r'\d+', row)))
            for row in re.findall(r'\{([^{}]+)\}', match.group(1))]

path = rows('path')
groups = rows('wave_groups')
waves = rows('waves')
assert len(path) == 8 and len(waves) == 8
length = 0
for (ax, ay), (bx, by) in zip(path, path[1:]):
    assert (ax == bx) != (ay == by), 'Need nonzero orthogonal segment'
    length += abs(ax-bx) + abs(ay-by)
assert length == 520 and length*16 < 65536
assert all(0 <= x <= 256 and 24 <= y < 208 for x, y in path)
used, count, layers = [], 0, 0
for start, n, bonus in waves:
    assert 0 <= start < len(groups) and 1 <= n <= len(groups)-start and bonus > 0
    for j in range(start, start+n):
        used.append(j)
        layer, amount, gap = groups[j]
        assert 1 <= layer <= 5 and 1 <= amount <= 255 and 1 <= gap <= 255
        count += amount
        layers += layer * amount
assert used == list(range(len(groups))), 'Groups omitted or reused'
assert layers < 60000
assert (256**2 + 240**2) < (1 << 23), 'CE signed int overflow'
assert 58*112 < (1 << 15), 'Dart launch velocity calculation overflow'
assert 256*16 + 112 < (1 << 15), 'Q4 projectile overflow'
assert 95*(length*16-64) < (1 << 24), 'CE unsigned intermediate overflow'
result = {'check': 'source data and integer bounds only', 'path_pixels': length,
          'waves': len(waves), 'groups': len(groups), 'scripted_bloons': count,
          'scripted_layers': layers,
          'c_compiled_or_executed': False}
print(json.dumps(result, indent=2))
