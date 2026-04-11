#!/usr/bin/env python3
"""
Simple external level authoring helper for the ASCII level format.
Usage:
  python3 tools/level_tool.py write <out.lvl>    # writes a sample level
  python3 tools/level_tool.py print <file.lvl>  # print parsed contents
"""
import sys
import textwrap

SAMPLE = textwrap.dedent('''
# Sample level produced by tools/level_tool.py
plane 0 0 0 20 1 20 0.6 0.6 0.6
cube 0 1 0 1 1 1 0.8 0.2 0.2
cube 2 1 2 1 2 1 0.2 0.8 0.2
cube -2 1 2 1 1 1 0.2 0.2 0.8
cube 5 2 0 1 4 1 0.7 0.7 0.2
''')


def main(argv):
    if len(argv) < 2:
        print(__doc__)
        return 1

    cmd = argv[1]
    if cmd == 'write' and len(argv) == 3:
        with open(argv[2], 'w') as f:
            f.write(SAMPLE)
        print('Wrote sample level to', argv[2])
        return 0
    elif cmd == 'print' and len(argv) == 3:
        with open(argv[2], 'r') as f:
            print(f.read())
        return 0
    else:
        print('Unknown command or wrong args')
        print(__doc__)
        return 2

if __name__ == '__main__':
    sys.exit(main(sys.argv))
