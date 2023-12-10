import sys

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print('Usage: <text-to-utf8.py> <file>')
        exit(1)
    with open(sys.argv[1], 'rb') as f:
        print(bytearray(f.read()).decode('Shift_JIS'))