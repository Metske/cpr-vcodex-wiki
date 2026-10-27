#!/usr/bin/env python3
import sys
import os
import glob
from fontTools.ttLib import TTFont
from PIL import Image, ImageDraw, ImageFont

def render_glyph(codepoint, font):
    img = Image.new("L", (32, 32), 0)
    draw = ImageDraw.Draw(img)
    char = chr(codepoint)
    try:
        draw.text((16, 25), char, font=font, fill=255, anchor="ms")
    except Exception:
        return bytearray(128)
        
    bitmap = bytearray(128)
    bytes_per_line = 32 // 8
    pixels = img.load()
    is_empty = True
    for row in range(32):
        for col in range(32):
            if pixels[col, row] > 64:
                is_empty = False
                byte_idx = row * bytes_per_line + (col // 8)
                bit_idx = 7 - (col % 8)
                bitmap[byte_idx] |= (1 << bit_idx)
                
    if is_empty:
        return bytearray(128)
    return bitmap

def main():
    bin_path = sys.argv[1] if len(sys.argv) > 1 else "fallback_noto.bin"
    if not os.path.exists(bin_path):
        print(f"Error: {bin_path} not found.")
        sys.exit(1)
        
    fonts = glob.glob("/usr/share/fonts/truetype/noto/*-Regular.ttf")
    # Sort fonts to ensure stable priority. 
    # NotoSans-Regular first, then specific scripts.
    fonts.sort() 
    
    with open(bin_path, "r+b") as f:
        header = f.read(16)
        if header[:4] != b'FBF1':
            print("Error: Not a valid FBF1 font file!")
            sys.exit(1)
            
        total_patched = 0
        for font_path in fonts:
            print(f"Merging {font_path}...")
            try:
                tt = TTFont(font_path)
                cmap = tt['cmap'].getBestCmap()
                codepoints = sorted(list(cmap.keys()))
            except Exception as e:
                print(f"Skipping {font_path} due to cmap error: {e}")
                continue
                
            try:
                font = ImageFont.truetype(font_path, 26, layout_engine=ImageFont.Layout.BASIC)
            except AttributeError:
                font = ImageFont.truetype(font_path, 26)
            except Exception as e:
                print(f"Failed to load font {font_path}: {e}")
                continue
                
            for cp in codepoints:
                if cp >= 131072: continue
                isControl = (0x200B <= cp <= 0x200F) or (0x202A <= cp <= 0x202E) or (0x2060 <= cp <= 0x206F) or (cp == 0xFEFF) or (0x0000 <= cp <= 0x001F) or (0x007F <= cp <= 0x00A0)
                if isControl: continue
                
                # To prevent overriding a good glyph with a bad one, we could check if it's already written.
                # But reading 128 bytes every time is slow. Let's just overwrite for now.
                bitmap = render_glyph(cp, font)
                if any(b != 0 for b in bitmap):
                    offset = 16 + cp * 128
                    f.seek(offset)
                    f.write(bitmap)
                    total_patched += 1
                    
        print(f"Total glyphs patched across all fonts: {total_patched}")

if __name__ == '__main__':
    main()
