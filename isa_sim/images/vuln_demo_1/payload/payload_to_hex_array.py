# Converts the binary payload to a hex array.

import sys

def main():
    input_file = sys.argv[1]

    with open(input_file, 'rb') as f:
        payload = f.read()

    hex_array = ', '.join([f'0x{byte:02x}' for byte in payload])

    print("unsigned char payload[] = {")
    print(hex_array)
    print("};")

if __name__ == '__main__':
    main()
    